#ifndef _Motor
#define _Motor

#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "Constants.hpp"

class Motor{
private:
    int pinFWD, pinRVS;
    uint pwmSlice;
    bool reversed;
public:
    float rps = 0;
    int pwm;

    Motor(int pfwd, int prvs, bool rev) : pinFWD(pfwd), pinRVS(prvs), reversed(rev), pwmSlice(pwm_gpio_to_slice_num(pfwd)){
        gpio_set_function(pfwd, GPIO_FUNC_PWM);
        gpio_set_function(prvs, GPIO_FUNC_PWM);

        // Standard 1kHz - 20kHz frequency setup
        // 150,000,000 / (1.0 * 7499) = 20kHz
        pwm_set_clkdiv(pwmSlice, 1.0f); 
        pwm_set_wrap(pwmSlice, PWM_MAX); // 0 to 7500 range for speed resolution

        pwm_set_both_levels(pwmSlice, 0, 0);
        // pwm_set_chan_level(pwmSlice, PWM_CHAN_A, 0);
        // pwm_set_chan_level(pwmSlice, PWM_CHAN_B, 0);

        // pwm_set_phase_correct(pwmSlice, true);

        pwm_set_enabled(pwmSlice, true);
    }

    void setVelocity(int16_t velocity);
};

#endif