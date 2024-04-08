#include "drv_cst3xx.h"
#include "stdio.h"
#include "mhscpu.h"
#include "drv_i2c.h"
#include "log_print.h"
#include "user_delay.h"
#include "hardware_version.h"

/// @brief CST3xx touch pad init.
void Cst3xxInit(void)
{
    uint8_t cmd[2];
    I2cInit();

    cmd[0] = 0xD1;
    cmd[1] = 0x09;
    I2cSendData(CST3XX_I2C_ADDR, cmd, 2);
}

/// @brief CST3xx open.
void Cst3xxOpen(void)
{
    Cst3xxInit();
}

/// @brief Get touch status, including touch state, X/Y coordinate.
/// @param status TouchStatus struct addr.
int32_t Cst3xxGetStatus(TouchStatus_t *status)
{
    uint8_t readBuff[7] = {0}, cmd[3];

    I2cSendCmdAndReceiveData(CST3XX_I2C_ADDR, 0xD000, readBuff, 7);
    //PrintArray("read touch", readBuff, 7);
    cmd[0] = 0xD0;
    cmd[1] = 0x00;
    cmd[2] = 0xAB;
    I2cSendData(CST3XX_I2C_ADDR, cmd, 3);
    if (readBuff[6] == 0xAB) {
        status->x = TOUCH_PAD_RES_X - (unsigned int)((readBuff[1] << 4) | ((readBuff[3] >> 4) & 0x0F)) - 1;
        status->y = TOUCH_PAD_RES_Y - (unsigned int)((readBuff[2] << 4) | (readBuff[3] & 0x0F)) - 1;
        status->touch = readBuff[5] > 0 ? true : false;
    }
    return SUCCESS_CODE;
}
