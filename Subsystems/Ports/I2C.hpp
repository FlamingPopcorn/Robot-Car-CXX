#ifndef _I2C_Port
#define _I2C_Port

#include "hardware/i2c.h"
#include "pico/stdlib.h"

class I2CBus{
private:
    uint sda, scl;
    int baudrate;
public:
    i2c_inst_t *port;
    
    I2CBus(i2c_inst_t* i2c_port, uint sda_pin, uint scl_pin, uint baudrate_speed = 400000)
        : port(i2c_port), sda(sda_pin), scl(scl_pin), baudrate(baudrate_speed){
    gpio_init(sda);
    gpio_init(scl);
    gpio_disable_pulls(sda);
    gpio_disable_pulls(scl);

    // I2C Initialisation. Using it at 400kHz.
    i2c_init(port, 400*1000);
    
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
    }
};

#endif