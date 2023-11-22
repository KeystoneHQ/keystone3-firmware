#ifndef _FINGERPRINT_TASK_H
#define _FINGERPRINT_TASK_H

#include "stdint.h"

#define FINGER_PRINT_EVENT_UART_RECV    0x1
#define FINGER_PRINT_EVENT_RESTART      0x2
#define FINGER_PRINT_EVENT_LOW_POWER    0x4
#define FINGER_PRINT_ALL_EVENT          (FINGER_PRINT_EVENT_UART_RECV | FINGER_PRINT_EVENT_RESTART | FINGER_PRINT_EVENT_LOW_POWER)

void FingerPrintGroupSetBit(uint32_t bits);
void CreateFingerprintTask(void);
void CloseFingerInitTimer(void);
#endif
