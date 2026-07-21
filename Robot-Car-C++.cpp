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
#include "Subsystems\Bluetooth\Bluetooth.hpp"
#include "Subsystems\Drivetrain\Drivetrain.hpp"
#include "Subsystems\Odometry\Odometry.hpp"
#include "Subsystems\Odometry\Trajectory.hpp"
#include "Subsystems\IMU\IMU.hpp"
#include "Subsystems\RemoteControl\RemoteControl.hpp"
#include "Subsystems\LineFollower\LineFollower.hpp"
#include "Subsystems\Distance\Distance.hpp"
#include "Subsystems\Screen\Screen.hpp"
#include "Subsystems\PIDController.hpp"

UARTBus UART0(uart0, UART_IBUS_TX, UART_IBUS_RX, UART_IBUS_BAUDRATE, false, true);

I2CBus I2C0(i2c0, I2C0_SDA, I2C0_SCL);
I2CBus I2C1(i2c1, I2C1_SDA, I2C1_SCL);

BluetoothLE BLE;

Drivetrain drivetrain(LEFT_MOTOR_FWD_PIN, LEFT_MOTOR_RVS_PIN, RIGHT_MOTOR_FWD_PIN, RIGHT_MOTOR_RVS_PIN);
DifferentialOdometry odometry;
PathTracker pathTracker;

BMI160 imu(I2C1, I2C_BMI160_ADDR);
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

    printf("Starting Screen...\n");
    screen.init();

    printf("Starting IMU System...\n");
    imu.init();
    imu.calibrate();

    // printf("Starting Line Following System...\n");
    // LineFollower.init();

    printf("Starting Distance System...\n");
    distance.init();

    printf("Waiting on Core 1...\n");
    MultiCore1.init();
    while(!MultiCore1.booted){
        printf("Waiting on core1 to boot\n");
        sleep_ms(10);
    } printf("Core 1 Booted!\n");
    // MultiCore1.setMode(Remote_Control);

    printf("Starting BLE Radio...\n");
    BLE.init();
    BLE.waitForBoot();
    BLE.pauseRadio();

    printf("Starting Control Loop...\n");
    start_control_loop();

    printf("Starting Watchdog Timer...\n");
    // Enable watchdog for 500ms
    // The second parameter is 'pause_on_debug' (true means it won't trigger while you're debugging)
    watchdog_enable(500, true);

    printf("Starting Main Loop\n");
    while (true) {
        // std::cout << "Mode:" << MultiCore1.getMode() << "\n";

        // IMUData motion = imu.getMotion();
        // std::cout << "Motion: " << "X:" << motion.accelX << " Y:" << motion.accelY << " Z:" << motion.accelZ << " R:" << motion.gyroX << " P:" << motion.gyroY << " Y:" << motion.gyroZ << "\n";
        // Pose2D pos = odometry.getPose();
        // std::cout << "X:" << pos.X << " Y:" << pos.Y << " Theta:" << pos.theta << " - " << pos.theta_rads << "\n";

        // printf("Joystick: %d | %d\n", BLE.state.joystick_radius, BLE.state.joystick_angle);
        // std::cout << "Dabble Controller State: " << BLE.state.joystick_radius << " | " << BLE.state.joystick_angle << " | " << BLE.state.select << " | " << BLE.state.start << " | " << BLE.state.triangle << " | " << BLE.state.circle << " | " << BLE.state.cross << " | " << BLE.state.square << "\n";
        // std::cout << "Channels: " << ibus.getChannel(CHANNEL_LSTICK_Y).value << " | " << ibus.getChannel(CHANNEL_RSTICK_X).value << "\n";

        // auto [pwmL, pwmR] = drivetrain.getMotorPWM();
        // std::cout << "Motor PWM: " << pwmL << " | " << pwmR << "\n";

        // std::cout << "Target Speeds:" << drivetrain.target_drive_mm_s.load() << " | " << drivetrain.target_turn_mm_s.load() << " PID Setpoints: " << drivetrain.pidL.setpoint << " | " << drivetrain.pidR.setpoint << " PID Inputs: " << drivetrain.pidL.input << " | " << drivetrain.pidR.input << "\n";
        
        // auto [Lv, Rv] = drivetrain.getVelocities();
        // std::cout << "Wheel Velocities:" << Lv << " | " << Rv << "\n";

        // distance.readDistances();
        // auto [Ld, Cd, Rd] = distance.getDistances();
        // std::cout << "Distances: " << Ld << " - " << Cd << " - " << Rd << "\n";

        if (screen.updated){
            screen.updateMenu();
            screen.updated = false;
        }

        // printf("Hello, world!\n");
        sleep_ms(250);
    }
}