#ifndef _LineFollower
#define _LineFollower

#include <cstdint>

#include "Constants.hpp"

#include "Subsystems\Ports\I2C.hpp"

class LineFollower{
    private:
        I2CBus i2c;
        short address;

        uint8_t digital;
        uint8_t analog[5];

        // Register Maps
        const uint8_t LF_REG_DIGITAL_MASK  = 0x00;
        const uint8_t LF_REG_ANALOG_CH0 = 0x01;
        const uint8_t LF_REG_ANALOG_CH1 = 0x02;
        const uint8_t LF_REG_ANALOG_CH2 = 0x03;
        const uint8_t LF_REG_ANALOG_CH3 = 0x04;
        const uint8_t LF_REG_ANALOG_CH4 = 0x05;

        const uint8_t LF_REG_ANALOG_BLOCK = 0x10;
        const uint8_t LF_REG_DIGITAL_BLOCK = 0x1A;
    public:
        LineFollower(I2CBus i2c_inst, short addr) : i2c(i2c_inst), address(addr){}

        void readChannelsDigital(){
            int ret = i2c_write_blocking(i2c.port, address, &LF_REG_DIGITAL_MASK, 1, true);
            if (ret != PICO_ERROR_GENERIC){
                i2c_read_blocking(i2c.port, address, &digital, 1, false);
            }
        }

        void readChannelAnalog(short chan){
            int ret = i2c_write_blocking(i2c.port, address, &LF_REG_ANALOG_CH0 + chan, 1, true);
            if (ret != PICO_ERROR_GENERIC){
                i2c_read_blocking(i2c.port, address, analog + chan, 2, false);
            }
        }

        void readChannelsAnalog(){
            int ret = i2c_write_blocking(i2c.port, address, &LF_REG_ANALOG_BLOCK, 1, true);
            if (ret != PICO_ERROR_GENERIC){
                i2c_read_blocking(i2c.port, address, analog, 10, false);
            }
        }
};

#endif