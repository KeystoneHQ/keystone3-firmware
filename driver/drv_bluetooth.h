/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Bluetooth driver.
 * Author: leon sun
 * Create: 2023-5-18
 ************************************************************************************************/


#ifndef _DRV_BLUETOOTH_H
#define _DRV_BLUETOOTH_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

void BluetoothInit(void);
void BluetoothSend(const char *str);
void BluetoothTest(int argc, char *argv[]);

#endif
