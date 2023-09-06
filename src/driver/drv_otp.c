/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: OTP(One-Time Programmable memory) driver.
 * Author: leon sun
 * Create: 2023-3-16
 ************************************************************************************************/

#include "drv_otp.h"
#include "mhscpu.h"
#include "assert.h"

/// @brief Write data to OTP zone.
/// @param[in] addr OTP address.
/// @param[in] data Input data.
/// @param[in] len data length.
/// @return err code.
int32_t WriteOtpData(uint32_t addr, const uint8_t *data, uint32_t len)
{
    int32_t ret;
    ASSERT(len <= 256);
    OTP_PowerOn();
    OTP_Unlock();
    OTP_UnProtect(addr);
    for (uint32_t i = 0; i < len; i += 4) {
        ret = OTP_WriteWord(addr + i, *(uint32_t *)(data + i));
        if (ret != 0) {
            break;
        }
    }
    OTP_Lock();
    OTP_SetProtect(addr);
    return ret;
}


