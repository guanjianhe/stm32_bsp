#ifndef UART_H
#define UART_H

#include "stm32f10x.h"

extern void uart_init(void);
extern void xputc(uint8_t d);
extern void xputs(const char* str);

#endif
