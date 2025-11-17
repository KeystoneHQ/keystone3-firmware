#include "hash_and_salt.h"
#include "stdio.h"
#include "string.h"
#include "mhscpu.h"
#include "hmac.h"
#include "drv_otp.h"
#include "user_utils.h"
#include "drv_trng.h"
#include "log_print.h"
#include "drv_mpu.h"
#include "user_memory.h"

#define SALT_DATA_LEN                                   32

//#define HASH_AND_SALT_TEST_MODE

#ifdef HASH_AND_SALT_TEST_MODE

static const uint8_t g_saltData[] = {
    0x9A, 0x77, 0x5D, 0x17, 0x05, 0x26, 0x64, 0xC2,
    0xA8, 0xFD, 0x6E, 0xC4, 0x9E, 0xAC, 0xEC, 0xA8,
    0xD2, 0x05, 0xB8, 0xD1, 0xE1, 0x10, 0x2B, 0x90,
    0x15, 0xBF, 0x5B, 0x15, 0xC9, 0x0D, 0x2D, 0xC6
};

#endif

/**
 * HashWithSaltImpl - Generate a salted hash using HMAC
 *
 * This function produces different hash results on different devices because it uses
 * device-specific random salt data stored in OTP (One-Time Programmable) memory.
 * Each device has its own unique salt, making the hash output device-specific.
 */
static void HashWithSaltImpl(uint8_t *outData, const uint8_t *inData, uint32_t inLen, const char *saltString, bool useSha512)
{
    uint8_t saltData[SALT_DATA_LEN];
    uint8_t tempData256[32];
    uint8_t tempData512[64];
#ifdef HASH_AND_SALT_TEST_MODE
    memcpy_s(saltData, sizeof(saltData), g_saltData, sizeof(g_saltData));
#else
    // Get salt data from OTP, if salt data does not exist, then generate a random salt data.
    MpuSetOtpProtection(false);
    OTP_PowerOn();
    memcpy_s(saltData, sizeof(saltData), (uint8_t *)OTP_ADDR_SALT, SALT_DATA_LEN);
    if (CheckEntropy(saltData, SALT_DATA_LEN) == false) {
        printf("need generate salt\r\n");
        TrngGet(saltData, SALT_DATA_LEN);
        ASSERT(SUCCESS_CODE == WriteOtpData(OTP_ADDR_SALT, saltData, SALT_DATA_LEN));
    }
    MpuSetOtpProtection(true);
#endif

    if (useSha512) {
        hmac_sha512(saltData, SALT_DATA_LEN, (uint8_t *)inData, inLen, tempData512);
        hmac_sha512((uint8_t *)saltString, strlen(saltString), tempData512, sizeof(tempData512), outData);
    } else {
        hmac_sha256(saltData, SALT_DATA_LEN, (uint8_t *)inData, inLen, tempData256);
        hmac_sha256((uint8_t *)saltString, strlen(saltString), tempData256, sizeof(tempData256), outData);
    }

    memset_s(saltData, sizeof(saltData), 0, sizeof(saltData));
    memset_s(tempData256, sizeof(tempData256), 0, sizeof(tempData256));
    memset_s(tempData512, sizeof(tempData512), 0, sizeof(tempData512));
}

void HashWithSalt(uint8_t *outData, const uint8_t *inData, uint32_t inLen, const char *saltString)
{
    return HashWithSaltImpl(outData, inData, inLen, saltString, false);
}

void HashWithSalt512(uint8_t *outData, const uint8_t *inData, uint32_t inLen, const char *saltString)
{
    return HashWithSaltImpl(outData, inData, inLen, saltString, true);
}