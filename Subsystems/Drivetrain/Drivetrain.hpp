#ifndef _Drivetrain_Subsystem
#define _Drivetrain_Subsystem

#include <utility> // Required for std::pair
#include <tuple>

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
    uint pio_offsetA, pio_offsetB;
    Encoder encoderL, encoderR;
public:
    enum MotorMode mode;
    PIDController pidL, pidR;
    std::atomic<float> target_drive_power, target_turn_power = 0.0f;
    std::atomic<float> target_drive_mm_s, target_turn_mm_s = 0.0f;
    Drivetrain(int pLfwd, int pLrvs, int pRfwd, int pRrvs)
        : motorL(pLfwd, pLrvs, false), motorR(pRfwd, pRrvs, true), pio_offsetA(pio_add_program(PIO_BLOCK_MOTOR_ENCODERS_A, &quadratureA_program)), pio_offsetB(pio_add_program(PIO_BLOCK_MOTOR_ENCODERS_B, &quadratureB_program)), encoderL(LEFT_ENCODER_A_PIN, LEFT_ENCODER_B_PIN, pio_offsetA, pio_offsetA, true, true),
        encoderR(RIGHT_ENCODER_A_PIN, RIGHT_ENCODER_B_PIN, pio_offsetA, pio_offsetB, true, false), pidL(LEFT_P, LEFT_I, LEFT_D, LEFT_FF, -MAX_MM_S, MAX_MM_S, PID_PERIOD),
        pidR(RIGHT_P, RIGHT_I, RIGHT_D, RIGHT_FF, -MAX_MM_S, MAX_MM_S, PID_PERIOD), mode(Drive_mode){
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

    std::pair<float, float> getVelocities(){
        return {encoderL.velocity, encoderR.velocity};
    }

    std::pair<int32_t, int32_t> getMotorPWM(){
        return {motorL.pwm, motorR.pwm};
    }

    std::pair<int32_t, int32_t> getTicks(){
        return {encoderL.read(), encoderR.read()};
    }

    std::tuple<float, float, float, float> readEncoders(){
        auto [L, Lv] = encoderL.getVelocity();
        auto [R, Rv] = encoderR.getVelocity();
        return std::make_tuple(L, Lv, R, Rv);
    }

    std::pair<int, int> readEncodersCounts(){
        return {encoderL.read(), encoderR.read()};
    }
};

#endif