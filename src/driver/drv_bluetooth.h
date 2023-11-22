#ifndef _DRV_BLUETOOTH_H
#define _DRV_BLUETOOTH_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

void BluetoothInit(void);
void BluetoothSend(const char *str);
void BluetoothTest(int argc, char *argv[]);

#endif
