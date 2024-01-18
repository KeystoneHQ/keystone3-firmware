#include "drv_otp.h"
#include "mhscpu.h"
#include "assert.h"
#include "user_utils.h"

const uint32_t g_tamperFlag = 0x1234ABCD;

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

int32_t WriteTamperFlag(void)
{
    return WriteOtpData(OTP_ADDR_TAMPER, (uint8_t *)&g_tamperFlag, sizeof(g_tamperFlag));
}

bool ReadTamperFlag(void)
{
    uint32_t flag;
    OTP_PowerOn();
    memcpy(&flag, (uint8_t *)OTP_ADDR_TAMPER, sizeof(flag));
    if (CheckEntropy((uint8_t *)&flag, sizeof(flag)) == false) {
        return false;
    } else if (flag == g_tamperFlag) {
        return true;
    }
    return false;
}
