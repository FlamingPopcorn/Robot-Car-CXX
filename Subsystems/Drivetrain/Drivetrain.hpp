#ifndef _Drivetrain_Subsystem
#define _Drivetrain_Subsystem

#include <utility> // Required for std::pair

#include "pico/stdlib.h"

#include "Constants.hpp"

#include "Subsystems/PIDController.hpp"
#include "Motor.hpp"
#include "Encoder.hpp"

enum MotorMode{
    Drive_mode,
    Music_mode
};

class Drivetrain{
private:
    Motor motorL, motorR;
    uint pio_offset;
    Encoder encoderL, encoderR;
public:
    enum MotorMode mode;
    PIDController pidL, pidR;
    std::atomic<float> target_drive_power, target_turn_power;
    std::atomic<float> target_drive_mm_s, target_turn_mm_s;
    Drivetrain(int pLfwd, int pLrvs, int pRfwd, int pRrvs)
        : motorL(pLfwd, pLrvs, false), motorR(pRfwd, pRrvs, true), pio_offset(pio_add_program(PIO_BLOCK_MOTOR_ENCODERS, &quadratureA_program)), encoderL(PIO_BLOCK_MOTOR_ENCODERS, LEFT_ENCODER_A_PIN, LEFT_ENCODER_B_PIN, pio_offset, true),
        encoderR(PIO_BLOCK_MOTOR_ENCODERS, RIGHT_ENCODER_A_PIN, RIGHT_ENCODER_B_PIN, pio_offset, true), pidL(LEFT_P, LEFT_I, LEFT_D, LEFT_FF, -MAX_MM_S, MAX_MM_S, LEFT_PERIOD),
        pidR(RIGHT_P, RIGHT_I, RIGHT_D, RIGHT_FF, -MAX_MM_S, MAX_MM_S, RIGHT_PERIOD), mode(Drive_mode){
            encoderL.reset();
            encoderR.reset();
        }

    ~Drivetrain(){
            // Safety: stop the motors if this object is destroyed
            pwm_set_gpio_level(LEFT_MOTOR_FWD_PIN, 0);
            pwm_set_gpio_level(LEFT_MOTOR_RVS_PIN, 0);
            pwm_set_gpio_level(RIGHT_MOTOR_FWD_PIN, 0);
            pwm_set_gpio_level(RIGHT_MOTOR_RVS_PIN, 0);
    }

    void setMotorMode(enum MotorMode m) {
        mode = m;
    }

    void setVelocities(float left_mm_s, float right_mm_s);

    std::pair<float, float> getSpeeds(){
        return {encoderL.velocity, encoderR.velocity};
    }

    std::pair<float, float> readEncoders(){
        return {encoderL.getSpeed(), encoderR.getSpeed()};
    }

    std::pair<int, int> readEncodersCounts(){
        return {encoderL.read(), encoderR.read()};
    }
};

#endif