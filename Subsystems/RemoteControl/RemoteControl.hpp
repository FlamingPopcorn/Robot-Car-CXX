#ifndef _RemoteControl_Subsystem
#define _RemoteControl_Subsystem

#include <atomic>

#include "Subsystems\Ports\UART.hpp"

#include "Constants.hpp"

enum ChannelType{
    MOTOR,
    SERVO,
    SWITCH
};

struct Channel{
    uint8_t channelNumber;
    enum ChannelType channelType;
    uint16_t value;
};

class RemoteControl {
    private:
        UARTBus uart;

        uint8_t numChannels = IBUS_NUM_CHANNELS;
        Channel channels[IBUS_NUM_CHANNELS];
    public:
    RemoteControl(UARTBus uartBus) : uart(uartBus){
        for (uint8_t i = 0; i < IBUS_NUM_CHANNELS; i++){
            channels[i].channelNumber = i;
            channels[i].channelType = i < 5 ? MOTOR : SERVO;
            channels[i].value = channels[i].channelType != SWITCH ? 1500 : 1000;
        }
    }
    
    Channel getChannel(uint8_t channelNum){return channels[channelNum - 1];}

    void readIBus();
};

#endif