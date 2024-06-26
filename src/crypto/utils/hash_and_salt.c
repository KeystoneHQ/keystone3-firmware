#include "hash_and_salt.h"
#include "stdio.h"
#include "string.h"
#include "mhscpu.h"
#include "hmac.h"
#include "drv_otp.h"
#include "user_utils.h"
#include "drv_trng.h"
#include "log_print.h"

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

void HashWithSalt(uint8_t *outData, const uint8_t *inData, uint32_t inLen, const char *saltString)
{
    uint8_t saltData[SALT_DATA_LEN];
    uint8_t tempData[32];
#ifdef HASH_AND_SALT_TEST_MODE
    memcpy(saltData, g_saltData, sizeof(saltData));
#else
    //Get salt data from OTP, if salt data does not exist, then generate a ramdom salt data.
    OTP_PowerOn();
    memcpy(saltData, (uint8_t *)OTP_ADDR_SALT, SALT_DATA_LEN);
    //PrintArray("saltData", saltData, SALT_DATA_LEN);
    if (CheckEntropy(saltData, SALT_DATA_LEN) == false) {
        printf("need generate salt\r\n");
        TrngGet(saltData, SALT_DATA_LEN);
        WriteOtpData(OTP_ADDR_SALT, saltData, SALT_DATA_LEN);
        //PrintArray("generate saltData", saltData, SALT_DATA_LEN);
    }
#endif
    hmac_sha256(saltData, SALT_DATA_LEN, (uint8_t *)inData, inLen, tempData);
    hmac_sha256((uint8_t *)saltString, strlen(saltString), tempData, 32, outData);
    memset(saltData, 0, sizeof(saltData));
}

void HashWithSalt512(uint8_t *outData, const uint8_t *inData, uint32_t inLen, const char *saltString)
{
    uint8_t saltData[SALT_DATA_LEN];
    uint8_t tempData[64];
#ifdef HASH_AND_SALT_TEST_MODE
    memcpy(saltData, g_saltData, sizeof(saltData));
#else
    //Get salt data from OTP, if salt data does not exist, then generate a ramdom salt data.
    OTP_PowerOn();
    memcpy(saltData, (uint8_t *)OTP_ADDR_SALT, SALT_DATA_LEN);
    //PrintArray("saltData", saltData, SALT_DATA_LEN);
    if (CheckEntropy(saltData, SALT_DATA_LEN) == false) {
        printf("need generate salt\r\n");
        TrngGet(saltData, SALT_DATA_LEN);
        WriteOtpData(OTP_ADDR_SALT, saltData, SALT_DATA_LEN);
        //PrintArray("generate saltData", saltData, SALT_DATA_LEN);
    }
#endif
    hmac_sha512(saltData, SALT_DATA_LEN, (uint8_t *)inData, inLen, tempData);
    hmac_sha512((uint8_t *)saltString, strlen(saltString), tempData, 64, outData);
    memset(saltData, 0, sizeof(saltData));
}
