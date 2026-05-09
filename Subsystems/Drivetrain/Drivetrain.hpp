#ifndef _Drivetrain_Subsystem
#define _Drivetrain_Subsystem

#include "pico/stdlib.h"

#include "Constants.hpp"

#include "Subsystems/PIDController.hpp"
#include "Motor.hpp"

enum MotorMode{
    Drive_mode,
    Music_mode
};

class Drivetrain{
private:
    Motor motorL, motorR;
public:
    enum MotorMode mode;
    PIDController pidL, pidR;
    Drivetrain(int pLfwd, int pLrvs, int pRfwd, int pRrvs)
        : motorL(pLfwd, pLrvs), motorR(pRfwd, pRrvs), pidL(LEFT_P, LEFT_I, LEFT_D, LEFT_FF, -PWM_MAX, PWM_MAX, LEFT_PERIOD),
        pidR(RIGHT_P, RIGHT_I, RIGHT_D, RIGHT_FF, -PWM_MAX, PWM_MAX, RIGHT_PERIOD), mode(Drive_mode){}

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

    void  setVelocities(float left_rps, float right_rps);
};

#endif