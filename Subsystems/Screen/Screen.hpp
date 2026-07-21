#ifndef _Screen_Subsystem
#define _Screen_Subsystem

#include <cstring>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/irq.h"

extern "C" {
    #include "Sensor_Libs/ssd1306.h"
}

#include "Constants.hpp"
#include "menuButtons.pio.h"
#include "Multicore.hpp"

struct MenuState {
    volatile int currentPageID = 0;
    volatile int cursorOption = 0;
};

class Screen{
    private:
        i2c_inst_t *i2c;
        int addr;
        ssd1306_t disp;
        PIO pio;
        uint pio_offset;
        int btn_up, btn_down, btn_select, btn_back;
        uint32_t sm[4];

        inline static Screen* instance = nullptr; // Hidden inside the class

    public:
        MenuState menu;
        bool updated;
    
        static void static_irq_handler() {
            if (instance) instance->handle_irq();
        }

        // CONSTRUCTOR: Only saves variables. No hardware touching!
        Screen(i2c_inst_t *i2c_port, int i2c_addr, PIO pio_block, int btnUp, int btnDown, int btnSelect, int btnBack)
            : i2c(i2c_port), addr(i2c_addr), pio(pio_block), btn_up(btnUp), btn_down(btnDown), btn_select(btnSelect), btn_back(btnBack), updated(false) {
            
            instance = this; // Safe to do before main()
        }

        // INIT FUNCTION: Call this inside main() after BLE.init()
        void init() {
            // Initialize Display Hardware
            ssd1306_init(&disp, SCREEN_WIDTH, SCREEN_HEIGHT, addr, i2c);
            ssd1306_contrast(&disp, 0xFF);

            ssd1306_clear(&disp);
            ssd1306_draw_string(&disp, (SCREEN_WIDTH - strlen("Cheeto") * 5) / 2, SCREEN_HEIGHT * (2.0 / 5.0), 1, "Cheeto");
            ssd1306_show(&disp);

            // Initialize PIO Hardware safely (No memory clearing!)
            pio_offset = pio_add_program(pio, &menuButtons_program);

            // Set the function that will handle the IRQ
            irq_set_exclusive_handler(PIO1_IRQ_0, Screen::static_irq_handler); 
            irq_set_enabled(PIO1_IRQ_0, true); 

            for (int i = 0; i < 4; i++){
                sm[i] = pio_claim_unused_sm(pio, true); // Safely find a free state machine
                menuButtons_program_init(pio, sm[i], pio_offset, MENU_BUTTONS[i]); // Start the State Machine

                // Tell the PIO to route SM interrupts to the system-level IRQ
                pio_set_irq0_source_enabled(pio, (enum pio_interrupt_source)(pis_interrupt0 + sm[i]), true);
            }
        }

        void handle_irq(){
            if (instance == nullptr){return;}
            
            // Read the current IRQ status
            uint32_t irq_status = pio1_hw->irq;
            
            for (int i = 0; i < 4; i++) {
                if (irq_status & (1 << sm[i])) {
                    process_menuButton(MENU_BUTTONS[i]); // Call a helper with your switch logic
                    pio_interrupt_clear(PIO_BLOCK_MENU_BUTTONS, sm[i]); // Clear the flag
                }
            }
            this->updated = true;
        }

        void process_menuButton(int button);
        void updateMenu();
};

#endif