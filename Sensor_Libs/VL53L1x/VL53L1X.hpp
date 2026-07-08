#ifndef _VL53L1X
#define _VL53L1X

#include <cstdint>
#include <stdint.h>
#include <cstdlib>

extern "C" {
    #include "Sensor_Libs/VL53L1x/VL53L1X_types.h"
    #include "Sensor_Libs/VL53L1x/VL53L1X_api.h"
}

#include "Constants.hpp"

#include "Subsystems\Ports\I2C.hpp"

class VL53L1X_Sensor{
    private:
        VL53L1X_Status_t status;
        VL53L1X_Result_t* results;
        uint8_t address;
        unsigned int XSHUTpin;
    public:
        VL53L1X_Sensor() : status(0), results(nullptr), address(0), XSHUTpin(0) {}

        void init(uint8_t addr, unsigned int xshut_pin) {
            this->address = addr;
            this->XSHUTpin = xshut_pin;

            VL53L1X_Result_t* VL53L1X_Result_t_ptr = (VL53L1X_Result_t*) malloc(sizeof(VL53L1X_Result_t));
            if (VL53L1X_Result_t_ptr == NULL) {
                printf("CRITICAL: Time of Flight Sensor Results Malloc Failed!\n");
            }

            // Wake THIS specific sensor up
            gpio_put(XSHUTpin, 1);
            sleep_ms(5);

            // Ensure the sensor has booted
            status = 0; 
            uint8_t bootState;
            do {
                status += VL53L1X_BootState(I2C_VL53L1X_ADDR, &bootState);
                VL53L1X_WaitMs(I2C_VL53L1X_ADDR, 2);
            } while (bootState == 0);

            // Change its address away from the default 0x29
            if (addr != I2C_VL53L1X_ADDR){
                VL53L1X_SetI2CAddress(I2C_VL53L1X_ADDR, addr << 1);
            }

            // Initialize and configure sensor
            status = VL53L1X_SensorInit(addr);
            status += VL53L1X_SetDistanceMode(addr, 1);
            status += VL53L1X_SetTimingBudgetInMs(addr, 100);
            status += VL53L1X_SetInterMeasurementInMs(addr, 100);
            status += VL53L1X_StartRanging(addr);
            
            VL53L1X_Result_t_ptr->status = status;
            results = VL53L1X_Result_t_ptr;
            status = 0;
        }

        void read(){
            status += VL53L1X_GetResult(address, results);
            if (results->status != 0) results->distance = 65535;

            // Clear the sensor for a new measurement
            status += VL53L1X_ClearInterrupt(address);
        }

        uint16_t getDistance(){
            return results->distance;
        }
};

#endif