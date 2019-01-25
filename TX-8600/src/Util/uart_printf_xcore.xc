/*
 * easy_uart.xc
 *
 *  Created on: Nov 21, 2017
 *      Author: root
 */
#include <platform.h>
#include <xs1.h>
#include <timer.h>

#define BAUDRATE    115200
#define BIT_TIME    XS1_TIMER_HZ / BAUDRATE
#define DATA_BITS   8

//on tile[1]: out port p_debug = XS1_PORT_1B;

static inline void uart_send_byte(out port ?uartTx, unsigned char byte)
{
    if(isnull(uartTx)) return;
    int t;
    uartTx <: 0 @ t; //send start bit and timestamp (grab port timer value)
    t += BIT_TIME;
#pragma loop unroll(8)
    for(int i = 0; i < 8; i++) {
        uartTx @ t <: >> byte; //timed output with post right shift
        t += BIT_TIME;
    }
    uartTx @ t <: 1; //send stop bit
    t += BIT_TIME;
    uartTx @ t <: 1; //wait until end of stop bit
}

void uart_flush(const char buf[], unsigned count)
{
    for(int i=0; i!=count; i++)
        uart_send_byte(null, buf[i]);
}
