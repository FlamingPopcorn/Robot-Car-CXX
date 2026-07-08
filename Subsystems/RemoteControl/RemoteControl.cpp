#include <cstdint>

#include "RemoteControl.hpp"
#include <cstdio>

void RemoteControl::readIBus(){
    uint8_t byte = 0;
    uint8_t prevByte = 0;

    while (byte != 0x40 && prevByte != 0x20){ // Look for IBus header 0x20 0x40
        if (uart_is_readable(this->uart.getPort())){
            prevByte = byte;
            byte = uart_getc(this->uart.getPort());
        }
    }

    uint8_t buffer[IBUS_FRAME_LENGTH - 2] = {0};
    uart_read_blocking(this->uart.getPort(), buffer, IBUS_FRAME_LENGTH - 2);

    uint16_t calc_checksum = 0xFFFF - 0x20 - 0x40;
    for (int i = 0; i < 28; i++) {
        calc_checksum -= buffer[i];
    }

    // The received checksum is in the last 2 bytes of our 30-byte buffer
    uint16_t rx_checksum = buffer[28] | (buffer[29] << 8);

    if (rx_checksum == calc_checksum){
        for (int i = 0; i < IBUS_NUM_CHANNELS; i++){
            // this->channels[i]->value = 0;
            uint16_t value = (buffer[2*i+1] << 8) | buffer[2*i];

            if (value > 2000){
                value = 2000;
            } else if (value < 1000){
                value = 1000;
            }

            this->channels[i].value = value;
        }
    }
    return;
}