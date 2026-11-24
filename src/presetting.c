#include "presetting.h"
#include "stdio.h"
#include "drv_otp.h"
#include "log_print.h"
#include "user_utils.h"
#include "user_memory.h"
#include "err_code.h"
#include "assert.h"
#include "drv_mpu.h"
#include "drv_battery.h"
#include "drv_aw32001.h"

#define FACTORY_RESULT_CHECK_ENABLE         1

static const uint8_t g_newUpdatePublicKey[65] = {
    0x04,
    0xBD, 0x99, 0x79, 0x6F, 0x42, 0xB7, 0xE9, 0xBD,
    0xBC, 0xF4, 0x6D, 0x48, 0xB2, 0x7B, 0x8E, 0x60,
    0x75, 0xB1, 0x7E, 0x04, 0xA9, 0xEC, 0x9C, 0xA4,
    0x54, 0x20, 0x1E, 0x7A, 0x6E, 0x15, 0xC2, 0xE9,
    0xE7, 0x2F, 0xEE, 0x6F, 0x81, 0x2D, 0xE1, 0x92,
    0x91, 0xC5, 0x48, 0x68, 0xFA, 0xE8, 0xED, 0x03,
    0x8E, 0xE5, 0x88, 0xCB, 0x60, 0x15, 0x90, 0xD4,
    0xC5, 0x1D, 0x54, 0x3D, 0x86, 0xD3, 0x7D, 0x29
};

int32_t GetSerialNumber(char *serialNumber)
{
    MpuSetOtpProtection(false);
    char temp[256];
    OTP_PowerOn();
    memcpy(temp, (uint8_t *)OTP_ADDR_SN, 256);
    MpuSetOtpProtection(true);
    if (CheckEntropy((uint8_t *)temp, 256) == false) {
        serialNumber[0] = '\0';
        return ERR_SERIAL_NUMBER_NOT_EXIST;
    }
    if (strnlen_s(temp, SERIAL_NUMBER_MAX_LEN + 1) >= SERIAL_NUMBER_MAX_LEN) {
        serialNumber[0] = '\0';
        return ERR_SERIAL_NUMBER_INVALID;
    }
    strcpy_s(serialNumber, SERIAL_NUMBER_MAX_LEN, temp);
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
    ASSERT(strnlen_s(serialNumber, SERIAL_NUMBER_MAX_LEN - 1) < SERIAL_NUMBER_MAX_LEN);
    OTP_PowerOn();
    CLEAR_ARRAY(temp);
    strcpy_s(temp, SERIAL_NUMBER_MAX_LEN, serialNumber);
    WriteOtpData(OTP_ADDR_SN, (uint8_t *)temp, SERIAL_NUMBER_MAX_LEN);
    return SUCCESS_CODE;
}

int32_t GetWebAuthRsaKey(uint8_t *key)
{
    uint8_t *data;
    MpuSetOtpProtection(false);

    OTP_PowerOn();
    data = SRAM_MALLOC(WEB_AUTH_RSA_KEY_LEN);
    memcpy(data, (uint8_t *)OTP_ADDR_WEB_AUTH_RSA_KEY, WEB_AUTH_RSA_KEY_LEN);
    if (CheckEntropy(data, WEB_AUTH_RSA_KEY_LEN) == false) {
        MpuSetOtpProtection(true);
        SRAM_FREE(data);
        return ERR_WEB_AUTH_KEY_NOT_EXIST;
    }
    MpuSetOtpProtection(true);
    memcpy(key, data, WEB_AUTH_RSA_KEY_LEN);
    SRAM_FREE(data);
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
    memcpy_s(data, WEB_AUTH_RSA_KEY_LEN, key, WEB_AUTH_RSA_KEY_LEN);
    for (uint32_t i = 0; i < WEB_AUTH_RSA_KEY_LEN; i += 256) {
        WriteOtpData(OTP_ADDR_WEB_AUTH_RSA_KEY + i, data + i, 256);
    }
    memset_s(data, WEB_AUTH_RSA_KEY_LEN, 0, WEB_AUTH_RSA_KEY_LEN);
    SRAM_FREE(data);
    return SUCCESS_CODE;
}

bool GetFactoryResult(void)
{
    MpuSetOtpProtection(false);
#if (FACTORY_RESULT_CHECK_ENABLE == 1)
    uint32_t data;
    OTP_PowerOn();
    memcpy(&data, (uint32_t *)OTA_ADDR_FACTORY_BASE, 4);
    MpuSetOtpProtection(true);
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
    const uint32_t slot = UPDATE_PUB_KEY_MIGRATION_SLOT;
    uint32_t addr = OTP_ADDR_UPDATE_PUB_KEY + slot * UPDATE_PUB_KEY_LEN;

    if (pubKey == NULL) {
        return ERR_UPDATE_PUB_KEY_NOT_EXIST;
    }
    memset_s(pubKey, UPDATE_PUB_KEY_LEN + 1, 0, UPDATE_PUB_KEY_LEN + 1);
    MpuSetOtpProtection(false);
    OTP_PowerOn();
    memcpy_s(data, sizeof(data), (const void *)(uintptr_t)addr, sizeof(data));
    MpuSetOtpProtection(true);

    if (CheckAllFF(data, sizeof(data))) {
        return ERR_UPDATE_PUB_KEY_NOT_EXIST;
    }
    bool hasTargetKey = memcmp(data, g_newUpdatePublicKey + 1, sizeof(data)) == 0;
    if (!hasTargetKey) {
        printf("[factory] unexpected update key in OTP slot %lu\r\n", (unsigned long)slot);
        ASSERT(hasTargetKey);
        return ERR_GENERAL_FAIL;
    }
    pubKey[0] = 0x04;
    memcpy_s(pubKey + 1, UPDATE_PUB_KEY_LEN, data, sizeof(data));
    return SUCCESS_CODE;
}

void GetNewUpdatePubKey(uint8_t pubKey[65])
{
    memcpy(pubKey, g_newUpdatePublicKey, sizeof(g_newUpdatePublicKey));
}



int32_t SetUpdatePubKey(const uint8_t *pubKey, uint32_t slot)
{
    if (slot != UPDATE_PUB_KEY_MIGRATION_SLOT) {
        ASSERT(slot == UPDATE_PUB_KEY_MIGRATION_SLOT);
        return ERR_GENERAL_FAIL;
    }
    if (pubKey == NULL || pubKey[0] != 0x04 ||
            memcmp(pubKey, g_newUpdatePublicKey, sizeof(g_newUpdatePublicKey)) != 0) {
        return ERR_UPDATE_PUB_KEY_NOT_EXIST;
    }

    uint8_t current[UPDATE_PUB_KEY_LEN + 1];
    int32_t result = GetUpdatePubKey(current);
    if (result != ERR_UPDATE_PUB_KEY_NOT_EXIST) {
        return result;
    }

    uint32_t milliVolt = GetBatteryMilliVolt();
    uint8_t batteryPercent = GetBatteryPercentByMilliVolt(milliVolt, GetUsbPowerState() == USB_POWER_STATE_DISCONNECT);
    if (batteryPercent <= 40) {
        return ERR_KEYSTORE_SAVE_LOW_POWER;
    }

    uint32_t addr = OTP_ADDR_UPDATE_PUB_KEY + slot * UPDATE_PUB_KEY_LEN;
    MpuSetOtpProtection(false);
    result = WriteOtpData(addr, pubKey + 1, UPDATE_PUB_KEY_LEN);
    MpuSetOtpProtection(true);
    if (result != 0 || GetUpdatePubKey(current) != SUCCESS_CODE ||
            memcmp(current, pubKey, sizeof(current)) != 0) {
        return ERR_GENERAL_FAIL;
    }
    printf("[update-key] OTP write and readback verified\r\n");
    return SUCCESS_CODE;
}

int32_t PrepareUpdatePubKey(void)
{
    uint8_t updateKey[UPDATE_PUB_KEY_LEN + 1];
    int32_t status = GetUpdatePubKey(updateKey);
    if (status == ERR_UPDATE_PUB_KEY_NOT_EXIST) {
        GetNewUpdatePubKey(updateKey);
        status = SetUpdatePubKey(updateKey, UPDATE_PUB_KEY_MIGRATION_SLOT);
    } else if (status == SUCCESS_CODE) {
        printf("[update-key] target key already present and verified\r\n");
    }
    return status;
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
        len = strnlen_s(argv[1], 1024);
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
        len = strnlen_s(argv[1], 1024);
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
        ret = SetUpdatePubKey(data, UPDATE_PUB_KEY_MIGRATION_SLOT);
        printf("SetUpdatePubKey ret=%d\n", ret);
        SRAM_FREE(data);
    } else if (strcmp(argv[0], "set_factory_result") == 0) {
        SetFactoryResult();
    } else {
        printf("presetting cmd err\r\n");
    }
}
