#ifndef _DRV_UART_H
#define _DRV_UART_H

#include "stdint.h"
#include "stdbool.h"


typedef void (*UartRcvByteCallbackFunc_t)(uint8_t byte);

void Uart0Init(UartRcvByteCallbackFunc_t func);
void Uart0OpenPort(void);
void Uart1Init(UartRcvByteCallbackFunc_t func);
void Uart1OpenPort(void);
void Uart2Init(UartRcvByteCallbackFunc_t func);
void Uart2OpenPort(void);
void Uart2DeInit(void);

#endif
