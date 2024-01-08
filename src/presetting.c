#include "presetting.h"
#include "stdio.h"
#include "drv_otp.h"
#include "log_print.h"
#include "user_utils.h"
#include "user_memory.h"
#include "err_code.h"
#include "assert.h"
#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#else
#define memset_s(p, s, c, l) memset(p, c, l)
#endif


#define FACTORY_RESULT_CHECK_ENABLE         1


int32_t GetSerialNumber(char *serialNumber)
{
    char temp[256];
    OTP_PowerOn();
    memcpy(temp, (uint8_t *)OTP_ADDR_SN, 256);
    //PrintArray("sn otp", (uint8_t *)temp, 256);
    if (CheckEntropy((uint8_t *)temp, 256) == false) {
        serialNumber[0] = '\0';
        return ERR_SERIAL_NUMBER_NOT_EXIST;
    }
    if (strlen(temp) >= SERIAL_NUMBER_MAX_LEN) {
        serialNumber[0] = '\0';
        return ERR_SERIAL_NUMBER_INVALID;
    }
    strcpy(serialNumber, temp);
    return SUCCESS_CODE;
}


int32_t SetSerialNumber(const char *serialNumber)
{
    int32_t ret;
    char temp[SERIAL_NUMBER_MAX_LEN];

    ret = GetSerialNumber(temp);
    if (ret == SUCCESS_CODE) {
        return ERR_SERIAL_NUMBER_ALREADY_EXIST;
    } else if (ret == ERR_SERIAL_NUMBER_INVALID) {
        return ret;
    }
    ASSERT(strlen(serialNumber) < SERIAL_NUMBER_MAX_LEN);
    OTP_PowerOn();
    CLEAR_ARRAY(temp);
    strcpy(temp, serialNumber);
    WriteOtpData(OTP_ADDR_SN, (uint8_t *)temp, SERIAL_NUMBER_MAX_LEN);
    return SUCCESS_CODE;
}


int32_t GetWebAuthRsaKey(uint8_t *key)
{
    uint8_t *data;

    OTP_PowerOn();
    data = SRAM_MALLOC(WEB_AUTH_RSA_KEY_LEN);
    memcpy(data, (uint8_t *)OTP_ADDR_WEB_AUTH_RSA_KEY, WEB_AUTH_RSA_KEY_LEN);
    if (CheckEntropy(data, WEB_AUTH_RSA_KEY_LEN) == false) {
        SRAM_FREE(data);
        return ERR_WEB_AUTH_KEY_NOT_EXIST;
    }
    SRAM_FREE(data);
    memcpy(key, data, WEB_AUTH_RSA_KEY_LEN);
    return SUCCESS_CODE;
}


int32_t SetWebAuthRsaKey(const uint8_t *key)
{
    int32_t ret;
    uint8_t *data;

    data = SRAM_MALLOC(WEB_AUTH_RSA_KEY_LEN);
    ret = GetWebAuthRsaKey(data);
    if (ret == SUCCESS_CODE) {
        SRAM_FREE(data);
        return ERR_WEB_AUTH_KEY_ALREADY_EXIST;
    }
    OTP_PowerOn();
    memcpy(data, key, WEB_AUTH_RSA_KEY_LEN);
    for (uint32_t i = 0; i < WEB_AUTH_RSA_KEY_LEN; i += 256) {
        WriteOtpData(OTP_ADDR_WEB_AUTH_RSA_KEY + i, data + i, 256);
    }
    memset_s(data, WEB_AUTH_RSA_KEY_LEN, 0, WEB_AUTH_RSA_KEY_LEN);
    SRAM_FREE(data);
    return SUCCESS_CODE;
}


bool GetFactoryResult(void)
{
#if (FACTORY_RESULT_CHECK_ENABLE == 1)
    uint32_t data;
    OTP_PowerOn();
    memcpy(&data, (uint32_t *)OTA_ADDR_FACTORY_BASE, 4);
    if (data != 0xFFFFFFFF) {
        printf("data=%#x........\n", data);
        printf("factory pass\n");
        return true;
    } else {
        printf("factory not pass\n");
#ifndef BUILD_PRODUCTION
        return true;
#else
        return false;
#endif
    }
#else
    return true;
#endif
}

int SetFactoryResult(void)
{
    int ret;
    uint8_t cTemp[4];
    *(uint32_t *)&cTemp[0] = '1';
    ret = WriteOtpData(OTA_ADDR_FACTORY_BASE, cTemp, 1);
    printf("factory = %d\n", GetFactoryResult());
    return ret;
}

int32_t GetUpdatePubKey(uint8_t *pubKey)
{
    uint8_t data[UPDATE_PUB_KEY_LEN];
    uint32_t addr;

    OTP_PowerOn();
    for (addr = OTP_ADDR_UPDATE_PUB_KEY + 1024 - UPDATE_PUB_KEY_LEN; addr >= OTP_ADDR_UPDATE_PUB_KEY; addr -= UPDATE_PUB_KEY_LEN) {
        memcpy(data, (uint8_t *)addr, UPDATE_PUB_KEY_LEN);
        PrintArray("read pub key", data, UPDATE_PUB_KEY_LEN);
        if (CheckAllFF(data, UPDATE_PUB_KEY_LEN) == false) {
            if (CheckEntropy(data, UPDATE_PUB_KEY_LEN)) {
                //Found
                printf("found,addr=0x%X\n", addr);
                pubKey[0] = 4;
                memcpy(pubKey + 1, data, UPDATE_PUB_KEY_LEN);
                memset(data, 0, UPDATE_PUB_KEY_LEN);
                return SUCCESS_CODE;
            }
            printf("not found,addr=0x%X\n", addr);
            memset(data, 0, UPDATE_PUB_KEY_LEN);
            memset(pubKey, 0, UPDATE_PUB_KEY_LEN + 1);
            return ERR_UPDATE_PUB_KEY_NOT_EXIST;
        }
    }
    printf("not found,addr=0x%X\n", addr);
    memset(data, 0, UPDATE_PUB_KEY_LEN);
    memset(pubKey, 0, UPDATE_PUB_KEY_LEN + 1);
    return ERR_UPDATE_PUB_KEY_NOT_EXIST;
}


/// @brief
/// @param key Key with it's 0x04 head, 65 bytes.
/// @return err code.
int32_t SetUpdatePubKey(const uint8_t *pubKey)
{
    uint8_t data[UPDATE_PUB_KEY_LEN];
    uint32_t addr;

    OTP_PowerOn();
    for (addr = OTP_ADDR_UPDATE_PUB_KEY; addr < OTP_ADDR_UPDATE_PUB_KEY + 1024; addr += UPDATE_PUB_KEY_LEN) {
        memcpy(data, (uint8_t *)addr, UPDATE_PUB_KEY_LEN);
        PrintArray("read pub key", data, UPDATE_PUB_KEY_LEN);
        if (CheckAllFF(data, UPDATE_PUB_KEY_LEN)) {
            printf("writeable addr found:0x%08X\n", addr);
            //write OTP
            memcpy(data, pubKey + 1, UPDATE_PUB_KEY_LEN);
            WriteOtpData(addr, data, UPDATE_PUB_KEY_LEN);
            return SUCCESS_CODE;
        }
    }
    memset(data, 0, UPDATE_PUB_KEY_LEN);
    return ERR_UPDATE_PUB_KEY_NO_SPACE;
}


void PresettingTest(int argc, char *argv[])
{
    int32_t ret;
    char serialNumber[SERIAL_NUMBER_MAX_LEN];
    uint8_t *data;
    uint32_t len;

    if (strcmp(argv[0], "get_serial_number") == 0) {
        ret = GetSerialNumber(serialNumber);
        CHECK_ERRCODE_RETURN(ret);
        printf("serialNumber=%s\n", serialNumber);
    } else if (strcmp(argv[0], "set_serial_number") == 0) {
        VALUE_CHECK(argc, 2);
        ret = SetSerialNumber(argv[1]);
        CHECK_ERRCODE_RETURN(ret);
        printf("set serialNumber=%s\n", argv[1]);
    } else if (strcmp(argv[0], "get_web_auth_key") == 0) {
        data = SRAM_MALLOC(WEB_AUTH_RSA_KEY_LEN);
        ret = GetWebAuthRsaKey(data);
        printf("ret=%d\n", ret);
        PrintArray("web auth key", data, WEB_AUTH_RSA_KEY_LEN);
        SRAM_FREE(data);
    } else if (strcmp(argv[0], "set_web_auth_key") == 0) {
        VALUE_CHECK(argc, 2);
        len = strlen(argv[1]);
        if (len != 2048) {
            printf("set_web_auth_key err input,len=%d\n", len);
            return;
        }
        data = SRAM_MALLOC(WEB_AUTH_RSA_KEY_LEN);
        len = StrToHex(data, argv[1]);
        if (len != WEB_AUTH_RSA_KEY_LEN) {
            printf("set_web_auth_key err hex,len=%d\n", len);
            SRAM_FREE(data);
            return;
        }
        PrintArray("hex", data, len);
        ret = SetWebAuthRsaKey(data);
        printf("set web auth key ret=%d\n", ret);
        SRAM_FREE(data);
    } else if (strcmp(argv[0], "get_update_pub_key") == 0) {
        data = SRAM_MALLOC(UPDATE_PUB_KEY_LEN + 1);
        ret = GetUpdatePubKey(data);
        printf("ret=%d\n", ret);
        PrintArray("update pub key", data, UPDATE_PUB_KEY_LEN + 1);
        SRAM_FREE(data);
    } else if (strcmp(argv[0], "set_update_pub_key") == 0) {
        VALUE_CHECK(argc, 2);
        len = strlen(argv[1]);
        if (len != 130) {
            printf("set_update_pub_key err input,len=%d\n", len);
            return;
        }
        data = SRAM_MALLOC(UPDATE_PUB_KEY_LEN + 1);
        len = StrToHex(data, argv[1]);
        if (len != UPDATE_PUB_KEY_LEN + 1) {
            printf("set_update_pub_key err hex,len=%d\n", len);
            SRAM_FREE(data);
            return;
        }
        if (data[0] != 0x04) {
            printf("pub key head err\n");
            SRAM_FREE(data);
            return;
        }
        PrintArray("hex", data, len);
        ret = SetUpdatePubKey(data);
        printf("SetUpdatePubKey ret=%d\n", ret);
        SRAM_FREE(data);
    } else if (strcmp(argv[0], "set_factory_result") == 0) {
        SetFactoryResult();
    } else {
        printf("presetting cmd err\r\n");
    }
}

