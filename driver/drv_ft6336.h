/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: FT6336 touch pad driver.
 * Author: leon sun
 * Create: 2023-5-15
 ************************************************************************************************/


#ifndef _DRV_FT6336_H
#define _DRV_FT6336_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "cmsis_os.h"
#include "hal_touch.h"

#define FT6336_I2C_ADDR                 0x38


/// @brief FT6336 touch pad init.
void Ft6336Init(void);


/// @brief FT6336 open.
void Ft6336Open(void);


/// @brief Get touch status, including touch state, X/Y coordinate.
/// @param status TouchStatus struct addr.
int32_t Ft6336GetStatus(TouchStatus_t *status);


#endif
