#ifndef _UART_Port
#define _UART_Port

#include "hardware/uart.h"
#include "pico/stdlib.h"

class UARTBus{
private:
    uart_inst_t *port;
    uint tx, rx;
    int baudrate;
public:
    UARTBus(uart_inst_t* uart_port, uint tx_pin, uint rx_pin, uint baudrate_speed = 115200, bool txON = true, bool rxON = true)
        : port(uart_port), tx(tx_pin), rx(rx_pin), baudrate(baudrate_speed){

    // UART Initialization
    uint baudrate_negotiated = uart_init(port, baudrate);
    if (txON) gpio_set_function(tx, GPIO_FUNC_UART);
    if (rxON) gpio_set_function(rx, GPIO_FUNC_UART);
    }

    uart_inst_t *getPort(){return port;}
};

#endif