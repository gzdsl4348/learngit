/*
 * uart_printf.h
 *
 *  Created on: Nov 21, 2017
 *      Author: root
 */


#ifndef UART_PRINTF_H_
#define UART_PRINTF_H_

#include "stdio.h"

void uart_printstr(const char s[]);

#define uart_printstr(s)
#define uart_printf(...) //uart_printf(__VA_ARGS__)

#endif /* UART_PRINTF_H_ */
