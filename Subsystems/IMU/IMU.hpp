#ifndef _IMU
#define _IMU

#include <cstdint>

#include "pico/stdlib.h"

#include "Subsystems/Ports/I2C.hpp"

// BMI160 Core Registers
const uint8_t REG_CHIP_ID = 0x00;
const uint8_t REG_DATA_START = 0x0C; // Start of Gyro Data
const uint8_t REG_ACC_CONF = 0x40;
const uint8_t REG_GYR_RANGE = 0x43;
const uint8_t REG_CMD = 0x7E;        // Command register for waking up

struct IMUData {
    // Gyroscope (Degrees per second)
    float gyroX;
    float gyroY;
    float gyroZ;    
    // Accelerometer (G-forces)
    float accelX;
    float accelY;
    float accelZ;
};

struct IMUBias {
    // Gyroscope Bias (Degrees per second)
    float gyroX_bias;
    float gyroY_bias;
    float gyroZ_bias;
    // Accelerometer Bias (G-forces)
    float accelX_bias;
    float accelY_bias;
    float accelZ_bias;
};

class BMI160 {
    private:
        I2CBus i2c;
        uint8_t address;

        IMUBias bias = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        IMUData imuData = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        // Helper function to write one byte over I2C
        void writeRegister(uint8_t reg, uint8_t data) {
            uint8_t buffer[2] = {reg, data};
            i2c_write_blocking(i2c.port, address, buffer, 2, false);
        }

        IMUData getRawMotion() {
            uint8_t rawData[12];
            
            // Tell the BMI160 we want to start reading at 0x0C (Gyro X)
            i2c_write_blocking(i2c.port, address, &REG_DATA_START, 1, true);
            
            // Read 12 sequential bytes (6 for Gyro, 6 for Accel)
            i2c_read_blocking(i2c.port, address, rawData, 12, false);

            IMUData data = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

            // Bitwise magic: Combine the High and Low bytes into 16-bit integers
            int16_t rawGyroX = (rawData[1] << 8) | rawData[0];
            int16_t rawGyroY = (rawData[3] << 8) | rawData[2];
            int16_t rawGyroZ = (rawData[5] << 8) | rawData[4];

            int16_t rawAccelX = (rawData[7] << 8) | rawData[6];
            int16_t rawAccelY = (rawData[9] << 8) | rawData[8];
            int16_t rawAccelZ = (rawData[11] << 8) | rawData[10];

            // Convert raw integers to floats using the sensitivity scale
            // Configurations: Gyro = +/- 1000 deg/s, Accel = +/- 2g
            data.gyroX = rawGyroX * (1000.0f / 32768.0f);
            data.gyroY = rawGyroY * (1000.0f / 32768.0f);
            data.gyroZ = -rawGyroZ * (1000.0f / 32768.0f); // Z axis is inverted in Cheeto

            data.accelX = rawAccelX * (2.0f / 32768.0f);
            data.accelY = rawAccelY * (2.0f / 32768.0f);
            data.accelZ = rawAccelZ * (2.0f / 32768.0f);

            return data;
        }

    public:
        BMI160(I2CBus i2c_inst, uint8_t addr) : i2c(i2c_inst), address(addr) {}

        void init() {
            // Wake up the Accelerometer (Normal Mode)
            writeRegister(REG_CMD, 0x11); 
            sleep_ms(5); // The chip needs a few milliseconds to power up

            // Wake up the  Gyroscope (Normal Mode)
            writeRegister(REG_CMD, 0x15); 
            sleep_ms(100); // Gyro takes longer to stabilize

            writeRegister(REG_GYR_RANGE, 0x01); // Set Gyroscope Range to 1000 deg/s
            sleep_ms(5);
        }

        void calibrate(){
            for (uint16_t i = 0; i < IMU_CALIBRATION_SAMPLES; i++){
                IMUData motion = getRawMotion();
                bias.accelX_bias += motion.accelX; bias.accelY_bias += motion.accelY; //bias.accelZ_bias += motion.accelZ;
                bias.gyroX_bias += motion.gyroX; bias.gyroY_bias += motion.gyroY; bias.gyroZ_bias += motion.gyroZ;
                sleep_ms(10);
            }
            bias.accelX_bias /= float(IMU_CALIBRATION_SAMPLES); bias.accelY_bias /= float(IMU_CALIBRATION_SAMPLES); //bias.accelZ_bias /= float(IMU_CALIBRATION_SAMPLES);
            bias.gyroX_bias /= float(IMU_CALIBRATION_SAMPLES); bias.gyroY_bias /= float(IMU_CALIBRATION_SAMPLES); bias.gyroZ_bias /= float(IMU_CALIBRATION_SAMPLES);
        }

        IMUData getMotion(){
            IMUData data = getRawMotion();
            data.accelX -= bias.accelX_bias; data.accelY -= bias.accelY_bias; data.accelZ -= bias.accelZ_bias;
            data.gyroX -= bias.gyroX_bias; data.gyroY -= bias.gyroY_bias; data.gyroZ -= bias.gyroZ_bias;
            imuData = data;
            return data;
        }
};

#endif