/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: CST726 touch pad driver.
 * Author: leon sun
 * Create: 2023-2-20
 ************************************************************************************************/

#include "drv_cst726.h"
#include "stdio.h"
#include "mhscpu.h"
#include "drv_i2c.h"
#include "log_print.h"
#include "user_delay.h"
#include "hardware_version.h"


/// @brief CST726 touch pad init.
void Cst726Init(void)
{
    I2cInit();
}


/// @brief CST726 open.
void Cst726Open(void)
{
    I2cInit();
}


/// @brief Get touch status, including touch state, X/Y coordinate.
/// @param status TouchStatus struct addr.
int32_t Cst726GetStatus(TouchStatus_t *status)
{
    uint8_t readBuff[7];

    readBuff[6] = 0x55;
    I2cSendCmdAndReceiveData(CST726_I2C_ADDR, 0x0000, readBuff, 7);
    //PrintArray("read touch", readBuff, 7);
    if ((readBuff[3] & 0xF0) != 0x80 && (readBuff[3] & 0xF0) != 0x40) {
        return ERR_TOUCHPAD_NODATA;
    }
    status->touch = (readBuff[3] & 0xF0) == 0x80 ? true : false;
    status->x = ((readBuff[3] & 0x0F) << 8) + readBuff[4];
    status->y = (readBuff[5] << 8) + readBuff[6];
    if (GetHardwareVersion() == VERSION_EVT0) {
        status->x = TOUCH_PAD_RES_X - status->x - 1;
        status->y = TOUCH_PAD_RES_Y - status->y - 1;
    }
    return SUCCESS_CODE;
}

