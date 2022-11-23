/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: CST3XX touch pad driver.
 * Author: leon sun
 * Create: 2023-1-16
 ************************************************************************************************/


#ifndef _DRV_CST3XX_H
#define _DRV_CST3XX_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "cmsis_os.h"
#include "hal_touch.h"

#define CST3XX_I2C_ADDR                 0x5A


/// @brief CST3XX touch pad init.
void Cst3xxInit(void);


/// @brief CST3XX open.
void Cst3xxOpen(void);


/// @brief Get touch status, including touch state, X/Y coordinate.
/// @param status TouchStatus struct addr.
int32_t Cst3xxGetStatus(TouchStatus_t *status);



#endif
