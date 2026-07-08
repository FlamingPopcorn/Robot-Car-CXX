#include <cstdio>

#include "pico/stdlib.h"

#include "Motor.hpp"
#include "Drivetrain.hpp"

void  Drivetrain::setVelocities(float left_mm_s, float right_mm_s){
    if (mode == Music_mode) {setMotorMode(Drive_mode);}
    // Normalize and scale directly to PWM (No mixing)
    int32_t left_pwm = (int32_t)((left_mm_s / MAX_MM_S) * PWM_MAX_f);
    int32_t right_pwm = (int32_t)((right_mm_s / MAX_MM_S) * PWM_MAX_f);

    // printf("MM/S: %f - %f\n", left_mm_s, right_mm_s);
    // printf("RPS/S: %f - %f\n", left_mm_s / MAX_MM_S, right_mm_s / MAX_MM_S);
    // printf("PWM: %d - %d\n", left_pwm, right_pwm);

    motorL.pwm = left_pwm;
    motorR.pwm = right_pwm;

    motorL.setVelocity(left_pwm);
    motorR.setVelocity(right_pwm);

    // motorL.setVelocity(PWM_MAX);
    // motorR.setVelocity(PWM_MAX);
}



