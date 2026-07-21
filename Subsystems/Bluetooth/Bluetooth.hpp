#ifndef BLUETOOTH
#define BLUETOOTH

#include <cstdint>
#include <cstring>
#include <cstdio>

#include "pico/cyw43_arch.h"
#include "btstack.h"         
#include "ble/att_db_util.h"
#include "bluetooth_gatt.h"
#include "ble/gatt-service/nordic_spp_service_server.h"
#include "pico/async_context.h"

#include "Subsystems\Odometry\Odometry.hpp"

// The raw advertising payload that broadcasts into the air
alignas(4) const uint8_t adv_data[] = {
    // Flags (General Discoverable)
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06, // Length, Type (Flags), General Discoverable + BR/EDR Not Supported
    
    // Length, Type (Complete Local Name), Name: Cheeto
    0x07, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'C', 'h', 'e', 'e', 't', 'o',
        
    // HM-10 BLE Service UUID (16-bit: 0xFFE0, reversed for Little-Endian)
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0xE0, 0xFF
};

#define DABBLE_MAGIC_BYTE 0xFF

#define BUTTON_SELECT_MASK 0x01
#define BUTTON_START_MASK 0x02
#define BUTTON_TRIANGLE_MASK 0x04
#define BUTTON_CIRCLE_MASK 0x08
#define BUTTON_CROSS_MASK 0x10
#define BUTTON_SQUARE_MASK 0x20

#define FLUTTER_MAGIC_BYTE 0xA5

#define BTN1_MASK 0x01
#define BTN2_MASK 0x02
#define BTN3_MASK 0x04
#define BTN4_MASK 0x08
#define SW1_MASK 0x10
#define SW2_MASK 0x20
#define SW3_MASK 0x40
#define SW4_MASK 0x80

#define FLUTTER_PATH_MAGIC_BYTE 0xBC

struct ControllerState {
    uint8_t controller = 0x0;

    // --- Analog Joystick Data ---
    uint8_t joystick_angle = 0;
    uint8_t joystick_radius = 0;
    
    // --- Directional Pad ---
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;

    // --- Action Buttons ---
    bool triangle = false;
    bool circle = false;
    bool cross = false;
    bool square = false;

    bool btn1 = false;
    bool btn2 = false;
    bool btn3 = false;
    bool btn4 = false;
    
    // --- System Buttons ---
    bool start = false;
    bool select = false;

    // --- System Switches ---
    bool sw1 = false;
    bool sw2 = false;
    bool sw3 = false;
    bool sw4 = false;
};

class BluetoothLE {
    private:
        bool new_data_ready = false;

        uint16_t nus_rx_handle = 0;
        uint16_t nus_tx_handle = 0;
        hci_con_handle_t active_connection_handle = 0; // Tracks the active phone connection

        // Tiny 16-point ring buffer to hold incoming Pose2D setpoints (16 * 12 bytes = 192 bytes RAM)
        struct Pose2DPoint { float x; float y; float theta; };
        Pose2DPoint path_buffer[64];
        uint8_t buffer_head = 0; // Where we write new incoming points
        uint8_t buffer_tail = 0; // Where your main drive loop consumes points from
        uint8_t buffer_count = 0; // Total active elements currently waiting in queue

        // Tracks if notifications are allowed by the phone's Client Configuration Descriptor (CCCD)
        bool notifications_enabled = false; 

        uint8_t cccd_buffer[2] = {0, 0};
        
        // A modern C++ trick to allow the C-based Bluetooth stack to talk to our class
        inline static BluetoothLE* instance = nullptr;
        btstack_packet_callback_registration_t hci_event_callback_registration;

        volatile bool radio_booted = false;

        void rpi_ble_gatt_setup() {
            // Initialize the database
            att_db_util_init();

            // Add GAP Primary Service (Standard Bluetooth Hex ID: 0x1800)
            att_db_util_add_service_uuid16(0x1800);
            
            // Add Device Name Characteristic (Standard Bluetooth Hex ID: 0x2A00)
            att_db_util_add_characteristic_uuid16(
                0x2A00,             // Characteristic UUID
                ATT_PROPERTY_READ,  // Properties flags
                0,                  // Read Security Permission (0 = ATT_SECURITY_NONE)
                0,                  // Write Security Permission (0 = ATT_SECURITY_NONE)
                (uint8_t*)"Cheeto", // Pointer to static data string
                6                  // Length of data string
            );

            // Add HM-10 Primary Service (0xFFE0)
            att_db_util_add_service_uuid16(0xFFE0);

            nus_rx_handle = att_db_util_add_characteristic_uuid16(
                0xFFE1, 
                ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_WRITE_WITHOUT_RESPONSE | ATT_PROPERTY_NOTIFY | ATT_PROPERTY_DYNAMIC, 
                0,                  // Read Security Permission
                0,                  // Write Security Permission
                NULL,               // Handled dynamically by callback
                0                   // 0 length
            );
            
            // Add Client Characteristic Configuration Descriptor (Standard Hex ID: 0x2902)
            // This explicitly allows the phone to turn notifications ON or OFF for your TX pin
            att_db_util_add_descriptor_uuid16(
                0x2902,             // Descriptor UUID
                ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_DYNAMIC, 
                0,                  // Read Security Permission
                0,                  // Write Security Permission
                NULL,               // Dynamic runtime value
                2                   // Size of a standard CCCD buffer is 2 bytes (16-bit integer)
            );

            att_server_init(att_db_util_get_address(), &att_read_callback, &att_write_callback);
        }

        void rpi_ble_configure_advertising() {
            // Set the advertisement data profile
            gap_advertisements_set_data(sizeof(adv_data), (uint8_t*)adv_data);
            
            // Enable to connectable advertising mode
            gap_advertisements_enable(1);
        }

        // --- THE GATT WRITE CALLBACK ---
        static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
            
            // Un-comment this if you want to see EVERY write attempt to the Pico
            // printf("BLE Write Intercepted! Handle: %d\n", att_handle);
            
            if (instance != nullptr) {
                // Did the phone just write to our HM-10 Characteristic?
                if (att_handle == instance->nus_rx_handle) {
                    instance->parsePacket(buffer, buffer_size);
                } // Intercept writes to the CCCD descriptor right after your RX handle
                else if (att_handle == instance->nus_rx_handle + 1 && buffer_size >= 2) {
                    uint16_t cccd_value = buffer[0] | (buffer[1] << 8);
                    instance->notifications_enabled = (cccd_value & 0x0001) != 0; // 0x0001 is the universal Bluetooth SIG spec code to identify Notification Enable
                    printf("Phone notification status updated: %s\n", instance->notifications_enabled ? "ENABLED" : "DISABLED");
                }
            }
            return 0; 
        }

        // --- THE GATT READ CALLBACK INTERCEPT LAYER ---
        static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size) {
            if (instance != nullptr) {
                // Did the browser try to read our 0x2902 Client Notification Configuration state descriptor?
                if (att_handle == instance->nus_rx_handle + 1) {
                    if (buffer && buffer_size >= 2) {
                        // Return 0x0001 if enabled, or 0x0000 if disabled to inform the browser table architecture
                        uint16_t cccd_state = instance->notifications_enabled ? 0x0001 : 0x0000;
                        buffer[0] = cccd_state & 0xFF;
                        buffer[1] = (cccd_state >> 8) & 0xFF;
                        return 2; // Return 2 bytes read successfully
                    }
                }
            }
            return 0;
        }

    public:
        ControllerState state;

        // Default constructor
        BluetoothLE() {
            state = {false};
            instance = this; // Link this physical object to our static callback
        }

        // Destructor: Safely powers down the wireless radio
        ~BluetoothLE() {
            hci_power_control(HCI_POWER_OFF);
            cyw43_arch_deinit();
        }

        // The complete Initialization Pattern
        void init() {
            // Boot up the CYW43439 wireless chip on the Pico W
            if (cyw43_arch_init()) {
                printf("Failed to initialize CYW43 Architecture\n");
                return;
            } printf("Successfully initialized CYW43 Architecture\n");

            // sleep_ms(250);

            // Initialize the Core Bluetooth Stack
            l2cap_init();
            sm_init();

            rpi_ble_gatt_setup();
            rpi_ble_configure_advertising();


            hci_event_callback_registration.callback = &packet_handler;
            hci_add_event_handler(&hci_event_callback_registration);

            // Turn on physical power to the Radio
            hci_power_control(HCI_POWER_ON);
        }

        void waitForBoot() {
            printf("Downloading BT Firmware...\n");
            while (!radio_booted) {
                poll();        // Keep pumping the background download
                sleep_ms(2);   // Tiny sleep to prevent locking the bus
            }
            printf("BT Firmware Download Complete!\n");
        }

        // Call this when leaving Remote Control mode
        void pauseRadio() {
            // Stop blasting advertisements into the air
            gap_advertisements_enable(0); 
            
            // Put the physical wireless chip into a low-power sleep state
            hci_power_control(HCI_POWER_OFF); 
            printf("BLE Radio Paused.\n");
        }

        // Call this when entering Remote Control mode
        void resumeRadio() {
            // Wake the chip back up
            hci_power_control(HCI_POWER_ON);
            
            // Resume broadcasting so phone can find it
            gap_advertisements_enable(1);
            printf("BLE Radio Resumed.\n");
        }

        // The only function the main loop ever needs to call
        bool poll() {
            // Tick the radio (Processes BLE events instantly behind the scenes)
            async_context_poll(cyw43_arch_async_context());

            // Did ticking the radio result in a new command?
            if (new_data_ready) {
                new_data_ready = false; // Reset the flag for the next cycle
                return true;            // Signal that data is ready!
            }
            
            return false; // No new data, move on
        }

        // This parses the physical data bytes once they reach our class
        void parsePacket(const uint8_t*packet, uint16_t length) {

            // Detect My Flutter App Magic Byte and packet length
            if ((length == 4) && (packet[0] == FLUTTER_MAGIC_BYTE)) {
                state.controller = packet[0];

                state.joystick_radius = packet[1];
                state.joystick_angle = packet[2];
                
                uint8_t but_data = packet[3];
                state.sw1 = but_data & SW1_MASK;
                state.sw2 = but_data & SW2_MASK;
                state.sw3 = but_data & SW3_MASK;
                state.sw4 = but_data & SW4_MASK;
                state.btn1 = but_data & BTN1_MASK;
                state.btn2 = but_data & BTN2_MASK;
                state.btn3 = but_data & BTN3_MASK;
                state.btn4 = but_data & BTN4_MASK;
            }

            else if ((length == 61) && (packet[0] == FLUTTER_PATH_MAGIC_BYTE)) {
                state.controller = FLUTTER_PATH_MAGIC_BYTE;

                int byte_offset = 1;

                // Loop through the 5 serialized poses packed inside the payload
                for (int i = 0; i < 5; i++) {
                    // Stop if our small onboard queue becomes full to avoid overflow
                    if (buffer_count >= 64) {
                        printf("[WARN] Spline buffer filled up entirely! Dropping excess points.\n");
                        break;
                    }

                    // Extract Floats directly using a basic memory copying casting layer
                    float x, y, theta;
                    memcpy(&x, &packet[byte_offset], 4);
                    memcpy(&y, &packet[byte_offset + 4], 4);
                    memcpy(&theta, &packet[byte_offset + 8], 4);

                    // Skip zero padding entries that fill out the end of a path string
                    if (x == 0.0f && y == 0.0f && theta == 0.0f && buffer_count > 0 && buffer_head == 0) {
                        byte_offset += 12;
                        continue; 
                    }

                    // Insert directly into the circular execution queue
                    path_buffer[buffer_head].x = x;
                    path_buffer[buffer_head].y = y;
                    path_buffer[buffer_head].theta = theta;

                    buffer_head = (buffer_head + 1) % 64;
                    buffer_count++;
                    byte_offset += 12;
                }

                printf("[BUFFER] Path packet received successfully. Queue current depth status: %d/64\n", buffer_count);
            }

            // Detect Dabble's signature 0xFF or is too short
            else if ((length == 6 || length == 8) && packet[0] == DABBLE_MAGIC_BYTE) {
                state.controller = DABBLE_MAGIC_BYTE;

                uint8_t module_id   = packet[1];
                uint8_t command_id  = packet[2];
                uint8_t data_length = packet[4];
                
                // HANDLE JOYSTICK DATA (Module 0x01)
                if (module_id == 0x01 && data_length == 2) {
                    // The actual data is hiding entirely in packet[6] and packet[5]
                    uint8_t but_data = packet[5];
                    state.select = but_data & BUTTON_SELECT_MASK;
                    state.start = but_data & BUTTON_START_MASK;
                    state.triangle = but_data & BUTTON_TRIANGLE_MASK;
                    state.circle = but_data & BUTTON_CIRCLE_MASK;
                    state.cross = but_data & BUTTON_CROSS_MASK;
                    state.square = but_data & BUTTON_SQUARE_MASK;

                    uint8_t joy_data = packet[6];

                    // Shift right by 3 bits to isolate the top 5 bits, then multiply by 15
                    state.joystick_angle = (joy_data >> 3);

                    // Mask with 0x07 (binary 00000111) to isolate the bottom 3 bits
                    state.joystick_radius = joy_data & 0x07;
                }

                // HANDLE BUTTON DATA (Module 0x00)
                else if (module_id == 0x00 && data_length == 2) {
                    uint8_t button_group = packet[5];
                    uint8_t button_id    = packet[6];
                    printf("Button Pressed! Group: %d | ID: %d\n", button_group, button_id);
                }
            } else {return;}
            
            // Tell the main loop that new data successfully arrived
            new_data_ready = true;
        }

        bool hasPathPoints() {
            return buffer_count > 0;
        }

        // Retrieves the next target point and steps your tail index forward
        bool popNextWaypoint(float &outX, float &outY, float &outTheta) {
            if (buffer_count == 0) return false;

            outX = path_buffer[buffer_tail].x;
            outY = path_buffer[buffer_tail].y;
            outTheta = path_buffer[buffer_tail].theta;

            buffer_tail = (buffer_tail + 1) % 64;
            buffer_count--;

            // --- WATERMARK THRESHOLD TRIGGER ---
            // If our buffer depth hits our safety watermark of 5 points, trigger a text chunk update request!
            if (buffer_count <= 20) {
                requestMorePoints();
            }

            return true;
        }

        // Request the next 5-pose chunk from the phone
        void requestMorePoints() {
            // if (!notifications_enabled || active_connection_handle == 0) return;

            // Packet payload: 1 Byte Header (0xFE) + 1 Byte Parameter (Let's request 5 points)
            uint8_t request_packet[2] = { 0xFE, 5 };

            // Send a direct BLE Notification down the pipeline
            att_server_notify(active_connection_handle, nus_rx_handle, request_packet, sizeof(request_packet));
            
            printf("[TELEMETRY] Sent low-watermark notification. Requesting next 5 poses.\n");
        }

        ControllerState getState() {
            return state;
        }

        // --- THE BTSTACK CALLBACK ---
        static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {            
            // Handle standard Bluetooth State Events
            if (packet_type == HCI_EVENT_PACKET) {
                uint8_t event_type = hci_event_packet_get_type(packet);
                
                switch (event_type) {

                    case BTSTACK_EVENT_STATE:
                        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                            printf("Radio fully booted! Starting Advertising...\n");

                            uint16_t adv_int_min = 0x0030;
                            uint16_t adv_int_max = 0x0030;
                            uint8_t adv_type = 0; // Connectable undirected advertising
                            bd_addr_t null_addr;
                            memset(null_addr, 0, 6);
                            
                            gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
                            gap_advertisements_set_data(sizeof(adv_data), (uint8_t*) adv_data);
                            gap_advertisements_enable(1); 

                            bd_addr_t local_addr;
                            gap_local_bd_addr(local_addr);
                            printf("Robot BLE MAC Address: %s\n", bd_addr_to_str(local_addr));

                            if (instance != nullptr) {
                                instance->radio_booted = true; 
                            }
                        }
                        break;

                    case HCI_EVENT_LE_META:
                        if (hci_event_le_meta_get_subevent_code(packet) ==  HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
                            instance->active_connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                            printf("Phone Connected! Connection Handle: %d\n", instance->active_connection_handle);
                        }
                        break;

                    case HCI_EVENT_DISCONNECTION_COMPLETE:
                        instance->active_connection_handle = 0;
                        instance->notifications_enabled = false;
                        // Automatically restart advertising when the phone disconnects
                        gap_advertisements_enable(1);
                        printf("Disconnected. Advertising restarted.\n");
                        break;
                }
            }
        }
};

#endif