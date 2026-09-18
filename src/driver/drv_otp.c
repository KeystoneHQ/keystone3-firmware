#include "drv_otp.h"
#include "mhscpu.h"
#include "assert.h"
#include "FreeRTOS.h"
#include "user_utils.h"

#define OTP_READ_WINDOW_START       0x40009000UL
#define OTP_READ_WINDOW_END         0x4000A000UL

void ReadOtpData(uint32_t addr, uint8_t *data, uint32_t len) PRIVILEGED_FUNCTION;

const uint32_t g_tamperFlag = 0x1234ABCD;

/// @param[in] addr OTP address.
void ReadOtpData(uint32_t addr, uint8_t *data, uint32_t len)
{
    uint32_t end = addr + len;

    ASSERT(data != NULL);
    ASSERT(len > 0U);
    ASSERT(end >= addr);
    ASSERT(addr >= OTP_READ_WINDOW_START && end <= OTP_READ_WINDOW_END);

    OTP_PowerOn();
    memcpy(data, (const void *)(uintptr_t)addr, len);
}

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
    ReadOtpData(OTP_ADDR_TAMPER, (uint8_t *)&flag, sizeof(flag));
    if (CheckEntropy((uint8_t *)&flag, sizeof(flag)) == false) {
        return false;
    } else if (flag == g_tamperFlag) {
        return true;
    }
    return false;
}
