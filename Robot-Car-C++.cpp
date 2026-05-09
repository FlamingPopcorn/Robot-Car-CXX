#include <cstdio>
#include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/i2c.h"       // For more I2C examples see https://github.com/raspberrypi/pico-examples/tree/master/i2c
// #include "hardware/adc.h"
#include "hardware/pio.h"       // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio
#include "hardware/watchdog.h"

#include "Constants.hpp"

#include "Subsystems\Ports\UART.hpp"
#include "Subsystems\Ports\I2C.hpp"
#include "Subsystems\Drivetrain\Drivetrain.hpp"
#include "Subsystems\Screen\Screen.hpp"
#include "Subsystems\PIDController.hpp"

UARTBus UART0(uart0, UART_IBUS_TX, UART_IBUS_RX, UART_IBUS_BAUDRATE, false, true);

I2CBus I2C0(i2c0, I2C0_SDA, I2C0_SCL);
I2CBus I2C1(i2c1, I2C1_SDA, I2C1_SCL);

Drivetrain drivetrain(LEFT_MOTOR_FWD_PIN, LEFT_MOTOR_RVS_PIN, RIGHT_MOTOR_FWD_PIN, RIGHT_MOTOR_RVS_PIN);

Screen screen(I2C0.port, SCREEN_ADDRESS, PIO_BLOCK_MENU_BUTTONS, BTN_UP, BTN_DOWN, BTN_SELECT, BTN_BACK);

int main(){
    stdio_init_all();

    multicore_reset_core1();

    pio_clear_instruction_memory(pio0);
    pio_clear_instruction_memory(pio1);
    pio_clear_instruction_memory(pio2);

    sleep_ms(3000); 
    
    // Check if we rebooted because of a watchdog timeout
    if (watchdog_caused_reboot()) {
        printf("Rebooted from Watchdog!\n");
    } else printf("Rebooting System...\n");

    // if (cyw43_arch_init()) return -1;

    printf("Starting Watchdog Timer...\n");
    // Enable watchdog for 500ms
    // The second parameter is 'pause_on_debug' (true means it won't trigger while you're debugging)
    watchdog_enable(500, true);

    printf("Starting Main Loop\n");
    while (true) {
        if (screen.updated){
            screen.updateMenu();
            screen.updated = false;
        }

        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
