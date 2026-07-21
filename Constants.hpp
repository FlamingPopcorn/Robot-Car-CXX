#ifndef _Constants
#define _Constants

// Math PI value
constexpr double PI = 3.14159265358979323846;
constexpr double DEG2RAD = PI / 180.0;
constexpr double RAD2DEG = 180.0 / PI;

// UART Instances
    #define UART_IBUS uart0

// I2C Instances
    // I2C0
    #define I2C0_PORT i2c0

    // I2C1
    #define I2C1_PORT i2c1
    #define I2C_VL53L1X_INST i2c1
    #define I2C_LINEFOLLOWER i2c1

// PIO Instances
    #define PIO_BLOCK_MENU_BUTTONS pio1
    #define PIO_BLOCK_MOTOR_ENCODERS_A pio2
    #define PIO_BLOCK_MOTOR_ENCODERS_B pio2

// Pins
    // IBUS UART0
    #define UART_IBUS_TX 0  // There is no sending data to the radio supported
    #define UART_IBUS_RX 1 

    // I2C1
    #define I2C1_SDA 2
    #define I2C1_SCL 3

    // I2C0
    #define I2C0_SDA 4
    #define I2C0_SCL 5

    // Screen Buttons
    #define BTN_UP          21   // '^' Button
    #define BTN_DOWN        20   // 'v' Button
    #define BTN_SELECT      19   // '#' Button
    #define BTN_BACK        18   // '*' Button

    // Motors
    #define LEFT_MOTOR_FWD_PIN      6
    #define LEFT_MOTOR_RVS_PIN      7
    #define RIGHT_MOTOR_FWD_PIN     8
    #define RIGHT_MOTOR_RVS_PIN     9
    
    // Encoders
    #define LEFT_ENCODER_A_PIN      16
    #define LEFT_ENCODER_B_PIN      17
    #define RIGHT_ENCODER_A_PIN     14
    #define RIGHT_ENCODER_B_PIN     15

    // ToF Sensors
    #define LEFT_TOF_XSHUT_PIN      13
    #define CENTER_TOF_XSHUT_PIN    11
    #define RIGHT_TOF_XSHUT_PIN     12

// PID Constants
    #define PID_PERIOD     0.020f

    #define LEFT_P          2.00f 
    #define LEFT_I          0.10f
    #define LEFT_D          0.00f
    #define LEFT_FF         0.80f 

    #define RIGHT_P         2.00f // Start identical to left
    #define RIGHT_I         0.10f
    #define RIGHT_D         0.00f
    #define RIGHT_FF        0.80f

// Odometry Info
    // Update Loop Period
    #define ODOMETRY_PERIOD 0.010f

// Motor Info
    // Wheel Dimensions
    #define WHEEL_DIAMETER_MM 33.0f // 34.0mm specs
    #define WHEEL_CIRCUMFERENCE_MM (PI * WHEEL_DIAMETER_MM)
    #define WHEEL_TRACK_WIDTH_MM 68.7f
    #define WHEEL_BASE_LENGTH_MM 69.925f

    // Motor Speed
    #define MAX_RPS 6.00f
    #define MAX_MM_S (MAX_RPS * WHEEL_CIRCUMFERENCE_MM)
    #define PWM_MAX 7499
    #define PWM_MAX_f (float)(PWM_MAX)

    // Output Ratio
    #define PPR 7 * 4 // 28 counts per revolution
    #define MOTOR_GEAR_RATIO 118 // 118:1 | See https://ae01.alicdn.com/kf/S9e4e949a5d8243abbc72038df690dcc7d.jpg
    #define MOTOR_OUTPUT_RATIO 1 / float(PPR * MOTOR_GEAR_RATIO)

// Screen Info
    #define SCREEN_ADDRESS 0x3C

    // Screen Dimensions
    #define SCREEN_WIDTH 128
    #define SCREEN_HEIGHT 64

    // Screen Menu
    #define MENU_BUTTONS ((const int[]){BTN_UP, BTN_DOWN, BTN_SELECT, BTN_BACK})
    #define MAX_PAGES 4

// IMU BMI160 Sensor info
    // Defualt I2C address
    #define I2C_BMI160_ADDR 0x69

    #define IMU_CALIBRATION_SAMPLES 128

// Line Following Sensor Info
    // Defualt I2C address
    #define I2C_LINEFOLLOWER_ADDR 0x50

    #define LF_NUM_CHANNELS 5

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