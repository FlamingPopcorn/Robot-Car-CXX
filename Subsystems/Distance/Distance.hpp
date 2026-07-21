#ifndef _Distance
#define _Distance

#include <cstdint>

extern "C" {
    #include "Sensor_Libs/VL53L1x/VL53L1X_types.h"
    #include "Sensor_Libs/VL53L1x/VL53L1X_api.h"
}

#include "Sensor_Libs\VL53L1x\VL53L1X.hpp"

#include "Constants.hpp"

#include "Subsystems\Ports\I2C.hpp"

class Distance{
    private:
        I2CBus i2c;
        VL53L1X_Sensor Left, Center, Right;
    public:
        Distance(I2CBus i2c_inst) : i2c(i2c_inst){}

        void init(){
            // init all 3 ToF sensors
            for (uint8_t i = 0; i < 3; i++){
                gpio_init(XSHUT_PINS[i]);
                gpio_set_dir(XSHUT_PINS[i], GPIO_OUT);
                gpio_put(XSHUT_PINS[i], 0);
            }
            sleep_ms(10);

            VL53L1X_SetI2CInstance(I2C_VL53L1X_INST);

            Left.init(I2C_LEFT_ADDR, XSHUT_PINS[0]);
            Center.init(I2C_CENTER_ADDR, XSHUT_PINS[1]);
            Right.init(I2C_RIGHT_ADDR, XSHUT_PINS[2]);
        }

        void readDistances(){
            Left.read(); Center.read(); Right.read();
        }

        std::tuple<uint16_t, uint16_t, uint16_t> getDistances(){
            return {Left.getDistance(), Center.getDistance(), Right.getDistance()};
        }
};

#endif