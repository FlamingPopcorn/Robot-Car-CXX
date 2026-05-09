#include <cstdio>

#include "pico/stdlib.h"

#include "Motor.hpp"
#include "Drivetrain.hpp"

void  Drivetrain::setVelocities(float left_rps, float right_rps){
    if (mode == Music_mode) {setMotorMode(Drive_mode);}
    // Normalize and scale directly to PWM (No mixing)
    int32_t left_pwm = (int32_t)((left_rps / MAX_RPS) * PWM_MAX_f);
    int32_t right_pwm = (int32_t)((right_rps / MAX_RPS) * PWM_MAX_f);

    motorL.pwm = left_pwm;
    motorR.pwm = right_pwm;

    motorL.setVelocity(left_pwm);
    motorR.setVelocity(right_pwm);
}



