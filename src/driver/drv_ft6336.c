#include "drv_ft6336.h"
#include "stdio.h"
#include "mhscpu.h"
#include "drv_i2c.h"
#include "log_print.h"
#include "user_delay.h"
#include "drv_i2c_io.h"


/// @brief FT6336 touch pad init.
/// @param func Interrupt callback function, called when EXTINT gpio rasing/falling.
void Ft6336Init(void)
{
    I2cInit();
}


/// @brief FT6336 open.
void Ft6336Open(void)
{
    I2cInit();
}


/// @brief Get touch status, including touch state, X/Y coordinate.
/// @param status TouchStatus struct addr.
int32_t Ft6336GetStatus(TouchStatus_t *status)
{
    uint8_t sendByte, readBuff[5] = {0};

    sendByte = 0x02;
    I2cSendAndReceiveData(FT6336_I2C_ADDR, &sendByte, 1, readBuff, 5);

    //PrintArray("read touch", readBuff, 5);
    status->touch = readBuff[0] > 0 ? true : false;
    status->x = ((readBuff[1] & 0x0F) << 8) + readBuff[2];
#if (FT6336_REVERSE_X)
    status->x = TOUCH_PAD_RES_X - status->x - 1;
#endif
    status->y = ((readBuff[3] & 0x0F) << 8) + readBuff[4];
#if (FT6336_REVERSE_Y)
    status->y = TOUCH_PAD_RES_Y - status->y - 1;
#endif

    return SUCCESS_CODE;
}


