#include <cstdio>
#include <iostream>
#include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/i2c.h"       // For more I2C examples see https://github.com/raspberrypi/pico-examples/tree/master/i2c
// #include "hardware/adc.h"
#include "hardware/pio.h"       // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio
#include "hardware/watchdog.h"

#include "Constants.hpp"

#include "Multicore.hpp"

#include "Subsystems\Ports\UART.hpp"
#include "Subsystems\Ports\I2C.hpp"
#include "Subsystems\Drivetrain\Drivetrain.hpp"
#include "Subsystems\RemoteControl\RemoteControl.hpp"
#include "Subsystems\LineFollower\LineFollower.hpp"
#include "Subsystems\Distance\Distance.hpp"
#include "Subsystems\Screen\Screen.hpp"
#include "Subsystems\PIDController.hpp"

UARTBus UART0(uart0, UART_IBUS_TX, UART_IBUS_RX, UART_IBUS_BAUDRATE, false, true);

I2CBus I2C0(i2c0, I2C0_SDA, I2C0_SCL);
I2CBus I2C1(i2c1, I2C1_SDA, I2C1_SCL);

Drivetrain drivetrain(LEFT_MOTOR_FWD_PIN, LEFT_MOTOR_RVS_PIN, RIGHT_MOTOR_FWD_PIN, RIGHT_MOTOR_RVS_PIN);

RemoteControl ibus(UART0);
LineFollower lineFollower(I2C1, I2C_LINEFOLLOWER_ADDR);
Distance distance(I2C1);

Core1 MultiCore1;

Screen screen(I2C0.port, SCREEN_ADDRESS, PIO_BLOCK_MENU_BUTTONS, BTN_UP, BTN_DOWN, BTN_SELECT, BTN_BACK);

int main(){
    stdio_init_all();

    sleep_ms(3000); 
    
    // Check if we rebooted because of a watchdog timeout
    if (watchdog_caused_reboot()) {
        printf("Rebooted from Watchdog!\n");
    } else printf("Rebooting System...\n");

    // if (cyw43_arch_init()) return -1;

    MultiCore1.enabled = true;
    printf("Waiting on Core 1...\n");
    while(!MultiCore1.booted){
        // printf("Waiting on core1 to boot\n");
        sleep_ms(50);
    } printf("Core 1 Booted!\n");
    // MultiCore1.setMode(Remote_Control);

    printf("Starting Control Loop...\n");
    start_control_loop();

    // printf("Starting Watchdog Timer...\n");
    // Enable watchdog for 500ms
    // The second parameter is 'pause_on_debug' (true means it won't trigger while you're debugging)
    // watchdog_enable(500, true);
 
    printf("Starting Main Loop\n");
    while (true) {
        // auto [L, R] = drivetrain.getSpeeds();
        // std::cout << "Speeds:" << L * MOTOR_OUTPUT_RATIO * WHEEL_CIRCUMFERENCE << " | " << R * MOTOR_OUTPUT_RATIO * WHEEL_CIRCUMFERENCE << "\n";
        // std::cout << "Mode:" << MultiCore1.getMode() << " | " << "Channels: " << ibus.getChannel(CHANNEL_LSTICK_Y).value << " - " << ibus.getChannel(CHANNEL_RSTICK_X).value << " | " << "Target Speeds:" << drivetrain.target_drive_mm_s.load() << " | " << drivetrain.target_turn_mm_s.load() << " | " << "Input Speeds:" << drivetrain.pidL.input << " | " << drivetrain.pidR.input << " | " << "Setpoint Speeds:" << drivetrain.pidL.setpoint << " | " << drivetrain.pidR.setpoint << "\n";

        // distance.readDistances();
        // auto [Ld, Cd, Rd] = distance.getDistances();
        // std::cout << "Distances: " << Ld << " - " << Cd << " - " << Rd << "\n";

        if (screen.updated){
            screen.updateMenu();
            screen.updated = false;
        }

        // printf("Hello, world!\n");
        sleep_ms(100);
    }
}