#include <cmath>

#include "Motor.hpp"

void Motor::setVelocity(int16_t velocity){

    // velocity should be between -PWM_MAX and PWM_MAX
    uint16_t speed = std::abs(velocity);
    if (speed > PWM_MAX) speed = PWM_MAX; 

    // pwm_set_both_levels() takes two uint16_t as the level foor both channels
    if (velocity > 0) {
        pwm_set_both_levels(pwmSlice, speed, 0);
    } else if (velocity < 0) {
        pwm_set_both_levels(pwmSlice, 0, speed);
    } else {
        pwm_set_both_levels(pwmSlice, 0, 0);
    }    
}