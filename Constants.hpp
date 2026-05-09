#ifndef _Constants
#define _Constants

// Math PI value
constexpr double PI = 3.14159265358979323846;

// UART Instances
    #define UART_IBUS uart0

// I2C Instances
    // I2C0
    #define I2C0_PORT i2c0

    // I2C1
    #define I2C1_PORT i2c1
    #define I2C_VL53L1X_INST i2c1

// PIO Instances
    #define PIO_BLOCK_MENU_BUTTONS pio0

// Pins
    // IBUS UART0
    #define UART_IBUS_TX 0  // There is no sending data to the radio supported
    #define UART_IBUS_RX 1 

    // I2C0
    #define I2C0_SDA 4
    #define I2C0_SCL 5

    // Screen Buttons
    #define BTN_UP      6       // '^' Button
    #define BTN_DOWN    7       // 'v' Button
    #define BTN_SELECT  8       // '#' Button
    #define BTN_BACK    9       // '*' Button

    // Motors
    #define LEFT_MOTOR_FWD_PIN      10
    #define LEFT_MOTOR_RVS_PIN      11
    #define RIGHT_MOTOR_FWD_PIN     12
    #define RIGHT_MOTOR_RVS_PIN     13

    // I2C1
    #define I2C1_SDA 2
    #define I2C1_SCL 3

    // ToF Sensors
    #define LEFT_TOF_XSHUT_PIN      16
    #define CENTER_TOF_XSHUT_PIN    17
    #define RIGHT_TOF_XSHUT_PIN     22

// PID Constants
    #define LEFT_P          0.05f 
    #define LEFT_I          0.05f
    #define LEFT_D          0.02f
    #define LEFT_FF         1.38f 
    #define LEFT_PERIOD     0.010f

    #define RIGHT_P         0.05f // Start identical to left
    #define RIGHT_I         0.05f
    #define RIGHT_D         0.02f
    #define RIGHT_FF        1.30f
    #define RIGHT_PERIOD    0.010f

// Motor Info
    // Motor Speed
    #define MAX_RPS 3.00f
    #define PWM_MAX 7499
    #define PWM_MAX_f (float)(PWM_MAX)

    // Wheel Dimensions
    #define WHEEL_DIAMETER 34.0f // 34.0 mm
    #define WHEEL_CIRCUMFERENCE (PI * WHEEL_DIAMETER)

// Screen Info
    #define SCREEN_ADDRESS 0x3C

    // Screen Dimensions
    #define SCREEN_WIDTH 128
    #define SCREEN_HEIGHT 64

    // Screen Menu
    #define MENU_BUTTONS ((const int[]){BTN_UP, BTN_DOWN, BTN_SELECT, BTN_BACK})
    #define MAX_PAGES 4

// Distance Sensor Info
    // Defualt I2C address
    #define I2C_VL53L1X_ADDR 0x29

    // Custom I2C Addresses
    #define I2C_CENTER_ADDR 0x11
    #define I2C_LEFT_ADDR   0x12
    #define I2C_RIGHT_ADDR  0x13

    // XSHUT pins
    #define XSHUT_PINS ((const int[]){LEFT_TOF_XSHUT_PIN, CENTER_TOF_XSHUT_PIN, RIGHT_TOF_XSHUT_PIN})

    // Maze Algorithm Turn Away Threshold
    #define TURN_AWAY_THRESHOLD 100.0f

// UART0 (IBUS) Info
    // UART Speed
    #define UART_IBUS_BAUDRATE 115200 // 8N1
    
    // IBUS Frame Layout
    #define IBUS_FRAME_LENGTH 32
    #define IBUS_NUM_CHANNELS 6

    // Channel Layout
    #define CHANNEL_RSTICK_X 1
    #define CHANNEL_RSTICK_Y 2
    #define CHANNEL_LSTICK_Y 3
    #define CHANNEL_LSTICK_X 4
    #define CHANNEL_AUX_VRA 5
    #define CHANNEL_AUX_VRB 6

    // Deadzone
    #define VELOCITY_DEADZONE 50


#endif