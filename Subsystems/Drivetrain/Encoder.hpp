// Library docs
// https://github.com/jamon/pi-pico-pio-quadrature-encoder

#ifndef _Encoder
#define _Encoder

#include <cstdint>

#include "hardware/pio.h"
#include "quadrature.pio.h"

#include "Constants.hpp"
#include <iostream>

class Encoder {
    private:
        PIO pio_inst_A = PIO_BLOCK_MOTOR_ENCODERS_A, pio_inst_B = PIO_BLOCK_MOTOR_ENCODERS_B;
        uint8_t A_Pin, B_Pin;
        uint offsetA, offsetB, smA, smB;
        int last_val;
        uint64_t last_time;
        bool reversed;
        bool swapChBDir = false;
    public:
        float velocity = 0.0f;
        Encoder(uint8_t a_pin, uint8_t b_pin, uint offsetA, uint offsetB, bool rev, bool swapChBDir) : A_Pin(a_pin), B_Pin(b_pin), reversed(rev), smA(pio_claim_unused_sm(PIO_BLOCK_MOTOR_ENCODERS_A, true)), smB(pio_claim_unused_sm(PIO_BLOCK_MOTOR_ENCODERS_B, true)){
            quadratureA_program_init(pio_inst_A, smA, offsetA, a_pin, b_pin);
            quadratureB_program_init(pio_inst_B, smB, offsetB, swapChBDir ? a_pin : b_pin, swapChBDir ? b_pin : a_pin);
        }

        int32_t read(){
            pio_sm_exec(pio_inst_A, smA, pio_encode_in(pio_x, 32));
            int32_t valA = pio_sm_get_blocking(pio_inst_A, smA);

            pio_sm_exec(pio_inst_B, smB, pio_encode_in(pio_x, 32));
            int32_t valB = pio_sm_get_blocking(pio_inst_B, smB) * (reversed ? -1 : 1);

            // printf("SM%d: %d | SM%d: %d\n", smA, valA, smB, valB);

            return (valA + valB) * (reversed ? -1 : 1);
        }

        std::pair<float, float> getVelocity(){
            // Get current time and current count
            uint64_t current_time = time_us_64();
            int32_t current_val = read(); // Calls blocking read function

            // Calculate the deltas (changes)
            int32_t delta_val = current_val - last_val;
            uint64_t delta_time_us = current_time - last_time;

            // Prevent divide-by-zero on the very first loop
            if (delta_time_us == 0) {
                return {0.0f, 0.0f}; 
            }

            float distance = delta_val * MOTOR_OUTPUT_RATIO * WHEEL_CIRCUMFERENCE_MM;

            // Convert microsecond delta to seconds for standard units
            float delta_time_s = delta_time_us / 1000000.0f;

            // Calculate velocity (Counts per Second)
            velocity = distance / delta_time_s;

            // Save current states for the next loop
            last_time = current_time;
            last_val = current_val;

            return {distance, velocity};
        }

        void reset(){
            pio_sm_exec(pio_inst_A, smA, pio_encode_set(pio_x, 0));
            pio_sm_exec(pio_inst_B, smB, pio_encode_set(pio_x, 0));
        }
};

#endif