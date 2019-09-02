#ifndef  __USART_H_
#define  __USART_H_
#include "stm32f10x.h"
#include <string.h>
#include <stdbool.h>

extern bool serial_in(uint8_t *pchByte);
extern bool serial_out(uint8_t chByte);
extern void uart_init(void);

#endif

