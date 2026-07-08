// Library docs
// https://github.com/jamon/pi-pico-pio-quadrature-encoder

#ifndef _Encoder
#define _Encoder

#include <cstdint>

#include "hardware/pio.h"
#include "quadrature.pio.h"

#include "Constants.hpp"

class Encoder {
    private:
        PIO pio_inst;
        uint8_t A_Pin, B_Pin;
        uint offset, sm;
        int last_val;
        uint64_t last_time;
        bool reversed;
    public:
        float velocity = 0.0;
        Encoder(PIO pio, uint8_t a_pin, uint8_t b_pin, uint offset, bool rev) : pio_inst(pio), A_Pin(a_pin), B_Pin(b_pin), reversed(rev), sm(pio_claim_unused_sm(pio, true)){
            quadratureA_program_init(pio_inst, sm, offset, a_pin, b_pin);
        }

        uint read(){
            pio_sm_exec(pio_inst, sm, pio_encode_in(pio_x, 32));
            int val = pio_sm_get_blocking(pio_inst, sm);

            // printf("SM: %d | PC: %d\n", sm, pio_sm_get_pc(pio_inst, sm));
            // printf("%d\n", val);

            return val * (reversed ? -1 : 1);
        }

        float getSpeed(){
            // Get current time and current count
            uint64_t current_time = time_us_64();
            int32_t current_val = read(); // Calls blocking read function

            // Calculate the deltas (changes)
            int32_t delta_val = current_val - last_val;
            uint64_t delta_time_us = current_time - last_time;

            // Prevent divide-by-zero on the very first loop
            if (delta_time_us == 0) {
                return 0.0f; 
            }

            // Convert microsecond delta to seconds for standard units
            float delta_time_s = delta_time_us / 1000000.0f;

            // Calculate velocity (Counts per Second)
            velocity = delta_val / delta_time_s;

            // Save current states for the next loop
            last_time = current_time;
            last_val = current_val;

            return velocity;
        }

        void reset(){
            pio_sm_exec(pio_inst, sm, pio_encode_set(pio_x, 0));
        }
};

#endif