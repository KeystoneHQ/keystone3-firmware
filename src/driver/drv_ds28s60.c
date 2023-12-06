#include "drv_ds28s60.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "mhscpu.h"
//#include "drv_spi.h"
#include "drv_spi_io.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "log_print.h"
#include "hmac.h"
#include "drv_trng.h"
#include "user_utils.h"
#include "user_delay.h"
#include "drv_otp.h"
#include "err_code.h"
#include "assert.h"

//#define DS28S60_TEST_MODE
//#define DS28S60_FORCE_BINDING

#define DS28S60_HARDWARE_EVB            0
#define DS28S60_HARDWARE_EVT0           1

#define DS28S60_HARDWARE_CFG            DS28S60_HARDWARE_EVT0

//EVB
#if (DS28S60_HARDWARE_CFG == DS28S60_HARDWARE_EVB)
#define DS28S60_CS_PORT                 GPIOD
#define DS28S60_CS_PIN                  GPIO_Pin_7
#define DS28S60_PDWN_PORT               GPIOG
#define DS28S60_PDWN_PIN                GPIO_Pin_0
#define DS28S60_MISO_PORT               GPIOB
#define DS28S60_MISO_PIN                GPIO_Pin_5
#define DS28S60_MOSI_PORT               GPIOB
#define DS28S60_MOSI_PIN                GPIO_Pin_4
#define DS28S60_CLK_PORT                GPIOB
#define DS28S60_CLK_PIN                 GPIO_Pin_2

#elif (DS28S60_HARDWARE_CFG == DS28S60_HARDWARE_EVT0)
//EVT0
#define DS28S60_CS_PORT                 GPIOD
#define DS28S60_CS_PIN                  GPIO_Pin_9
#define DS28S60_PDWN_PORT               GPIOG
#define DS28S60_PDWN_PIN                GPIO_Pin_0
#define DS28S60_MISO_PORT               GPIOD
#define DS28S60_MISO_PIN                GPIO_Pin_11
#define DS28S60_MOSI_PORT               GPIOD
#define DS28S60_MOSI_PIN                GPIO_Pin_10
#define DS28S60_CLK_PORT                GPIOD
#define DS28S60_CLK_PIN                 GPIO_Pin_8

#endif

#define WAIT_DELAY_TICK                 50
#define RETRY_MAX_COUNT                 5

#define DS28S60_CS_SET                  GPIO_SetBits(DS28S60_CS_PORT, DS28S60_CS_PIN)
#define DS28S60_CS_CLR                  GPIO_ResetBits(DS28S60_CS_PORT, DS28S60_CS_PIN)

#define DS28S60_PDWN_SET                GPIO_SetBits(DS28S60_PDWN_PORT, DS28S60_PDWN_PIN)
#define DS28S60_PDWN_CLR                GPIO_ResetBits(DS28S60_PDWN_PORT, DS28S60_PDWN_PIN)

#define DS28S60_OVERTIME                1000
#define BINDING_DATA_PAGE               91

#define VALUE_CHECK(value, expect)          {if (value != expect) {printf("input err!\r\n"); return; }}

DS28S60_Info_t g_ds28s60Info;

static const SPIIO_Cfg_t DS28S60_SPI_CONFIG = {
    .MISO_PORT = DS28S60_MISO_PORT,
    .MISO_PIN = DS28S60_MISO_PIN,
    .MOSI_PORT = DS28S60_MOSI_PORT,
    .MOSI_PIN = DS28S60_MOSI_PIN,
    .CLK_PORT = DS28S60_CLK_PORT,
    .CLK_PIN = DS28S60_CLK_PIN,
};

#ifdef DS28S60_TEST_MODE

static const uint8_t MASTER_SECRET[] = {
    0x3B, 0x01, 0x5F, 0x84, 0x13, 0x1B, 0x10, 0xDB, \
    0x43, 0x52, 0xC3, 0x1F, 0xE6, 0x0A, 0x37, 0x1E, \
    0x58, 0xFB, 0x63, 0x9A, 0x64, 0xC3, 0x13, 0xF2, \
    0x2D, 0x65, 0xE9, 0xB4, 0xC5, 0x91, 0x3E, 0x15
};

static const uint8_t BINDING_PAGE_DATA[] = {
    0x6E, 0x07, 0x52, 0xCB, 0xEB, 0x52, 0xDF, 0x38, \
    0xB9, 0xFE, 0x05, 0x76, 0x67, 0xAE, 0xA5, 0x07, \
    0xF2, 0xEF, 0x4A, 0xCB, 0x6A, 0x1E, 0x92, 0x9A, \
    0xF7, 0xAE, 0xB0, 0x92, 0x0F, 0x25, 0xC3, 0xAF
};

static const uint8_t PARTIAL_SECRET[] = {
    0x1F, 0xE9, 0xE0, 0x11, 0x3F, 0x6C, 0xF0, 0x82, \
    0x08, 0xC5, 0x3F, 0x9B, 0x9E, 0x85, 0x7A, 0xE6, \
    0x88, 0x7D, 0xE1, 0x43, 0xD9, 0x70, 0x05, 0x1C, \
    0x87, 0x83, 0x7B, 0x34, 0xED, 0x03, 0x2E, 0x62
};
#else
#define MASTER_SECRET_ADDR          OTP_ADDR_DS28S60
#define BINDING_PAGE_DATA_ADDR      MASTER_SECRET_ADDR + 32
#define PARTIAL_SECRET_ADDR         BINDING_PAGE_DATA_ADDR + 32
#endif

static int32_t DS28S60_GetInfo(void);
static int32_t DS28S60_GetBlockProtection(DS28S60_BlockProtection_t *blockProtection, uint8_t block);
static int32_t DS28S60_SetBlockProtection(const DS28S60_BlockProtection_t *blockProtection, uint8_t block);
static void DS28S60_GetHmacKey(uint8_t *key, const DS28S60_Info_t *info, uint8_t pg, uint8_t cmd);
static int32_t DS28S60_WriteSecret(void);
static int32_t DS28S60_Setup(void);
static int32_t ConfirmBlockSettings(void);
static int32_t DS28S60_SendCmdAndGetResult(uint8_t cmd, uint8_t *para, uint8_t paraLen, uint8_t expectedLen, uint8_t *resultArray);
static int32_t DS28S60_TrySendCmdAndGetResult(uint8_t cmd, uint8_t *para, uint8_t paraLen, uint8_t expectedLen, uint8_t *resultArray);
static int32_t DS28S60_Binding(void);
static int32_t DS28S60_SetProctection_From_Index(uint8_t index);
static void DS28S60_PrintInfo(void);
static void GetMasterSecret(uint8_t *masterSecret);
static void GetBindingPageData(uint8_t *bindingPageData);
static void GetPartialSecret(uint8_t *partialSecret);

void DS28S60_Init(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    SpiIoInit(&DS28S60_SPI_CONFIG);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = DS28S60_CS_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(DS28S60_CS_PORT, &gpioInit);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = DS28S60_PDWN_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(DS28S60_PDWN_PORT, &gpioInit);
    DS28S60_CS_SET;
    DS28S60_PDWN_SET;
    UserDelay(100);

    DS28S60_Binding();
}

void DS28S60_Open(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    SpiIoInit(&DS28S60_SPI_CONFIG);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = DS28S60_CS_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(DS28S60_CS_PORT, &gpioInit);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = DS28S60_PDWN_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(DS28S60_PDWN_PORT, &gpioInit);
    DS28S60_CS_SET;
    DS28S60_PDWN_SET;
}

static int32_t DS28S60_GetInfo(void)
{
    uint8_t data[32];
    int32_t ret;
    ret = DS28S60_ReadPage(data, ROM_OPTION_PG);
    if (ret != DS28S60_SUCCESS) {
        return ret;
    }
    if (CheckEntropy(&data[24], 8) == false) {
        return ERR_DS28S60_INFO;
    }
    memcpy(g_ds28s60Info.ROMID, data + 24, 8);
    memcpy(g_ds28s60Info.MANID, data + 22, 2);
    g_ds28s60Info.valid = DS28S60_INFO_VALID;
    return DS28S60_SUCCESS;
}

static int32_t DS28S60_GetBlockProtection(DS28S60_BlockProtection_t *blockProtection, uint8_t block)
{
    int32_t ret;
    uint8_t result[2];

    ret = DS28S60_SendCmdAndGetResult(DS28S60_CMD_READ_PROTECTION, &block, 1, 2, result);
    if (ret != DS28S60_SUCCESS) {
        return ret;
    }
    blockProtection->Secret.byte = result[0];
    blockProtection->Prot.byte = result[1];
    return DS28S60_SUCCESS;
}

static int32_t DS28S60_SetBlockProtection(const DS28S60_BlockProtection_t *blockProtection, uint8_t block)
{
    int32_t ret;
    uint8_t sendBuf[3];

    sendBuf[0] = block;
    sendBuf[1] = blockProtection->Secret.byte & 0x87;
    sendBuf[2] = blockProtection->Prot.byte & 0xCF;
    ret = DS28S60_SendCmdAndGetResult(DS28S60_CMD_SET_PROTECTION, sendBuf, 3, 0, NULL);
    if (ret != DS28S60_SUCCESS) {
        return ret;
    }
    return DS28S60_SUCCESS;
}

static void DS28S60_GetHmacKey(uint8_t *key, const DS28S60_Info_t *info, uint8_t pg, uint8_t cmd)
{
    uint8_t msg[76], masterSecret[32], bindingPageData[32], partialSecret[32];

    GetMasterSecret(masterSecret);
    GetBindingPageData(bindingPageData);
    GetPartialSecret(partialSecret);
    memcpy(&msg[0], info->ROMID, 8);
    memcpy(&msg[8], bindingPageData, 32);
    memcpy(&msg[40], partialSecret, 32);
    msg[72] = pg;
    memcpy(&msg[73], info->MANID, 2);
    msg[75] = cmd;

    GetMasterSecret(masterSecret);
    hmac_sha256((uint8_t *)masterSecret, 32, msg, 76, key);
    CLEAR_ARRAY(msg);
    CLEAR_ARRAY(masterSecret);
    CLEAR_ARRAY(bindingPageData);
    CLEAR_ARRAY(partialSecret);
}

static int32_t DS28S60_WriteSecret(void)
{
    int32_t ret;
    uint8_t sendBuf[34], masterSecret[32], bindingPageData[32], partialSecret[32];
    DS28S60_BlockProtection_t blockProtection;

    GetMasterSecret(masterSecret);
    GetBindingPageData(bindingPageData);
    GetPartialSecret(partialSecret);
    do {
        ret = DS28S60_WritePage(bindingPageData, BINDING_DATA_PAGE);
        CHECK_ERRCODE_BREAK("write binding page", ret);
        ret = DS28S60_WritePage(masterSecret, SECRET_A_PG);
        CHECK_ERRCODE_BREAK("write master secret", ret);
        sendBuf[0] = BINDING_DATA_PAGE;
        sendBuf[1] = 0x00;          //M_SEC:SEC_A, D_SEC:SEC_A
        memcpy(&sendBuf[2], partialSecret, 32);
        ret = DS28S60_SendCmdAndGetResult(DS28S60_CMD_CPT_SECRET, sendBuf, 34, 0, NULL);
        CHECK_ERRCODE_BREAK("cpt secret", ret);
        memset(sendBuf, 0, sizeof(sendBuf));
        ret = DS28S60_WritePage(sendBuf, BINDING_DATA_PAGE);
        CHECK_ERRCODE_BREAK("clr binding page", ret);
        UserDelay(50);
        //set SECRET_A_PG wp and lock
        memset(&blockProtection, 0, sizeof(DS28S60_BlockProtection_t));
        blockProtection.Prot.b.WP = 1;
        blockProtection.Prot.b.RP = 1;
        ret = DS28S60_SetBlockProtection(&blockProtection, SECRET_A_BLOCK);
        CHECK_ERRCODE_BREAK("set serect block wp", ret);
    } while (0);
    CLEAR_ARRAY(sendBuf);
    CLEAR_ARRAY(masterSecret);
    CLEAR_ARRAY(bindingPageData);
    CLEAR_ARRAY(partialSecret);

    return ret;
}

static int32_t DS28S60_SetProctection_From_Index(uint8_t index)
{
    int32_t ret;
    uint8_t block;
    DS28S60_BlockProtection_t blockProtection;

    do {
        ret = DS28S60_WriteSecret();
        CHECK_ERRCODE_BREAK("write secret", ret);
        memset(&blockProtection, 0, sizeof(DS28S60_BlockProtection_t));
        blockProtection.Secret.b.ID = DS28S60_SECRET_KEY_A;
        blockProtection.Prot.b.APH = 1;
        blockProtection.Prot.b.EPH = 1;
        for (block = index; block <= MAX_USER_BLOCK; block++) {
            ret = DS28S60_SetBlockProtection(&blockProtection, block);
            CHECK_ERRCODE_BREAK("set block", ret);
            UserDelay(50);
        }
    } while (0);

    return ret;
}

static int32_t DS28S60_Setup(void)
{
    return DS28S60_SetProctection_From_Index(0);
}

static int32_t ConfirmBlockSettings(void)
{
    int32_t ret;
    uint8_t block = MAX_USER_BLOCK;
    DS28S60_BlockProtection_t blockProtection;
    // Confirm block settings
    // 99% case, first check the last block
    do {
        UserDelay(50);
        ret = DS28S60_GetBlockProtection(&blockProtection, block);
        CHECK_ERRCODE_BREAK("get block protection", ret);
        if (blockProtection.Prot.b.APH == 1) {
            printf("confirm over,block=%d\r\n", block);
            ret = 0;
            break;
        } else {
            // find the first none protected block
            uint8_t left = 0;
            uint8_t right = MAX_USER_BLOCK;
            uint8_t mid;

            while (left < right) {
                mid = left + (right - left) / 2;

                UserDelay(50);
                ret = DS28S60_GetBlockProtection(&blockProtection, mid);
                CHECK_ERRCODE_BREAK("get block protection", ret);
                if (blockProtection.Prot.b.APH == 1) {
                    printf("confirm over,block=%d\r\n", mid);
                    left = mid + 1;
                } else {
                    right = mid;
                }
            }
            ret = DS28S60_SetProctection_From_Index(left);
        }

    } while (0);

    return ret;
}

//resultArray does not contain len-byte and result-byte.
static int32_t DS28S60_SendCmdAndGetResult(uint8_t cmd, uint8_t *para, uint8_t paraLen, uint8_t expectedLen, uint8_t *resultArray)
{
    uint32_t tryCount = 0;
    int32_t ret = SUCCESS_CODE;
    while (tryCount++ < RETRY_MAX_COUNT) {
        ret = DS28S60_TrySendCmdAndGetResult(cmd, para, paraLen, expectedLen, resultArray);
        if (ret == DS28S60_SUCCESS) {
            break;
        }
        printf("retry %d\r\n", tryCount);
    }
    assert(ret == 0);
    return ret;
}

static int32_t DS28S60_TrySendCmdAndGetResult(uint8_t cmd, uint8_t *para, uint8_t paraLen, uint8_t expectedLen, uint8_t *resultArray)
{
    uint8_t length, result;
    //uint32_t startTick;
    uint32_t tryCount;
    DS28S60_CS_CLR;

    SpiIoSendData(&DS28S60_SPI_CONFIG, &cmd, 1);
    SpiIoSendData(&DS28S60_SPI_CONFIG, &paraLen, 1);
    SpiIoSendData(&DS28S60_SPI_CONFIG, para, paraLen);
    //startTick = osKernelGetTickCount();
    tryCount = 0;
    do {
        UserDelay(WAIT_DELAY_TICK);
        SpiIoRecvData(&DS28S60_SPI_CONFIG, &length, 1);
        if (tryCount > DS28S60_OVERTIME / WAIT_DELAY_TICK) {
            printf("ds28s60 overtime, length=%d\r\n", length);
            DS28S60_CS_SET;
            return ERR_DS28S60_OVERTIME;
        }
        tryCount++;
    } while (length == 0 || length == 255);
    if (length - 1 != expectedLen && length != 1) {
        printf("length=%d,expectedLen=%d\r\n", length, expectedLen);
        DS28S60_CS_SET;
        return ERR_DS28S60_UNEXPECTLEN;
    }
    SpiIoRecvData(&DS28S60_SPI_CONFIG, &result, 1);
    if (resultArray != NULL) {
        SpiIoRecvData(&DS28S60_SPI_CONFIG, resultArray, length - 1);
    }
    //endTick = osKernelGetTickCount();
    DS28S60_CS_SET;
    //printf("used tick=%d\r\n", endTick - startTick);
    if (result == DS28S60_RESULT_BYTE_SUCCESS) {
        return DS28S60_SUCCESS;
    } else {
        printf("length=%d,expectedLen=%d\r\n", length, expectedLen);
        printf("result=%d\r\n", result);
        return result;
    }
}

/// @brief Try to bind SE chip, return a err code if SE chip already binded.
/// @return err code.
static int32_t DS28S60_Binding(void)
{
#ifdef DS28S60_TEST_MODE
    return SUCCESS_CODE;
#else
    int32_t ret;
    uint8_t keys[96];
    DS28S60_BlockProtection_t blockProtection;
    do {
        ret = DS28S60_GetBlockProtection(&blockProtection, SECRET_A_BLOCK);
        CHECK_ERRCODE_BREAK("get block protection", ret);
        OTP_PowerOn();
        memcpy(keys, (uint8_t *)OTP_ADDR_DS28S60, sizeof(keys));
        if (CheckEntropy(keys, 96)) {
            //OTP key exist
            if (!blockProtection.Prot.b.WP) {
#ifdef DS28S60_FORCE_BINDING
                printf("ds28s60 force binding\r\n");
                ret = DS28S60_Setup();
                CHECK_ERRCODE_BREAK("ds28s60 setup", ret);
#else
                printf("err,OTP key exist and ds28s60 unprotected\r\n");
                ret = ERR_DS28S60_BIND;
#endif
            } else {
                printf("confirm block settings\r\n");
                ret = ConfirmBlockSettings();
            }
        } else {
            //OTP key doesn't exist
            if (!blockProtection.Prot.b.WP) {
                TrngGet(keys, sizeof(keys));
                WriteOtpData(OTP_ADDR_DS28S60, keys, sizeof(keys));
                ret = DS28S60_Setup();
                CHECK_ERRCODE_BREAK("ds28s60 setup", ret);
            } else {
                printf("err,OTP key doesn't exist and ds28s60 protect\r\n");
                ret = ERR_DS28S60_BIND;
            }
        }
    } while (0);
    CLEAR_ARRAY(keys);
    // assert if binding error
    assert(ret == 0);

    return ret;
#endif
}

int32_t DS28S60_ReadPage(uint8_t *data, uint8_t page)
{
    return DS28S60_SendCmdAndGetResult(DS28S60_CMD_READ_MEM, &page, 1, 32, data);
}

int32_t DS28S60_WritePage(uint8_t *data, uint8_t page)
{
    uint8_t sendBuf[33];
    sendBuf[0] = page;
    memcpy(&sendBuf[1], data, 32);
    return DS28S60_SendCmdAndGetResult(DS28S60_CMD_WRITE_MEM, sendBuf, 33, 0, NULL);
}

int32_t DS28S60_GetRng(uint8_t *rngArray, uint32_t num)
{
    int32_t ret;
    uint8_t buffer[253], copyNum;
    uint32_t i = 0;

    while (i < num) {
        copyNum = (num - i) > 253 ? 253 : (num - i);
        ret = DS28S60_SendCmdAndGetResult(DS28S60_CMD_READ_RNG, &copyNum, 1, copyNum, buffer);
        CHECK_ERRCODE_BREAK("ds28s60_getrng", ret);
        memcpy(rngArray + i, buffer, copyNum);
        i += copyNum;
    }
    CLEAR_ARRAY(buffer);
    return ret;
}

int32_t DS28S60_HmacAuthentication(uint8_t page)
{
    uint8_t hmacDevice[32];
    uint8_t hmacCalc[32];
    uint8_t challenge[34];
    //uint8_t pageData[32];
    uint8_t secretKey[32];
    uint8_t msg[76];
    int32_t ret;

    challenge[0] = page;
    challenge[1] = 0x00;            //Secret A
    TrngGet(&challenge[2], 32);
    if (g_ds28s60Info.valid == false) {
        ret = DS28S60_GetInfo();
        if (ret != DS28S60_SUCCESS) {
            return ret;
        }
    }
    ret = DS28S60_SendCmdAndGetResult(DS28S60_CMD_CRPA, challenge, 34, 32, hmacDevice);
    if (ret != DS28S60_SUCCESS) {
        return ret;
    }
    UserDelay(50);
    ret = DS28S60_ReadPage(&msg[8], page);
    if (ret != DS28S60_SUCCESS) {
        return ret;
    }
    UserDelay(50);
    DS28S60_GetHmacKey(secretKey, &g_ds28s60Info, BINDING_DATA_PAGE, DS28S60_CMD_CPT_SECRET);
    memcpy(&msg[0], g_ds28s60Info.ROMID, 8);
    memcpy(&msg[40], &challenge[2], 32);
    msg[72] = page;
    memcpy(&msg[73], g_ds28s60Info.MANID, 2);
    msg[75] = DS28S60_CMD_CRPA;
    //PrintArray("msg", msg, 76);
    hmac_sha256(secretKey, 32, msg, 76, hmacCalc);
    //PrintArray("hmacCalc", hmacCalc, 32);
    //PrintArray("hmacCalc", hmacDevice, 32);
    if (memcmp(hmacCalc, hmacDevice, 32) != 0) {
        return ERR_DS28S60_AUTH;
    }
    CLEAR_ARRAY(hmacDevice);
    CLEAR_ARRAY(hmacCalc);
    CLEAR_ARRAY(challenge);
    CLEAR_ARRAY(secretKey);
    CLEAR_ARRAY(msg);

    return DS28S60_SUCCESS;
}

int32_t DS28S60_HmacEncryptRead(uint8_t *data, uint8_t page)
{
    uint8_t buf[40];
    uint8_t msg[20];
    uint8_t secretKey[32];
    uint8_t hmac[32];
    uint32_t i;
    int32_t ret;

    do {
        if (g_ds28s60Info.valid == false) {
            ret = DS28S60_GetInfo();
            CHECK_ERRCODE_BREAK("DS28S60_GetInfo", ret);
        }

        ret = DS28S60_SendCmdAndGetResult(DS28S60_CMD_ENC_READ, &page, 1, 40, buf);
        CHECK_ERRCODE_BREAK("DS28S60_SendCmdAndGetResult", ret);
        memcpy(&msg[0], buf, 8);
        memcpy(&msg[8], g_ds28s60Info.ROMID, 8);
        msg[16] = page;
        memcpy(&msg[17], g_ds28s60Info.MANID, 2);
        msg[19] = DS28S60_CMD_ENC_READ;
        DS28S60_GetHmacKey(secretKey, &g_ds28s60Info, BINDING_DATA_PAGE, DS28S60_CMD_CPT_SECRET);
        hmac_sha256(secretKey, 32, msg, 20, hmac);
        for (i = 0; i < 32; i++) {
            data[i] = hmac[i] ^ buf[i + 8];
        }
    } while (0);

    CLEAR_ARRAY(buf);
    CLEAR_ARRAY(msg);
    CLEAR_ARRAY(secretKey);
    CLEAR_ARRAY(hmac);

    return ret;
}

int32_t DS28S60_HmacEncryptWrite(const uint8_t *data, uint8_t page)
{
    uint8_t oldData[32];
    uint8_t secretKey[32];
    uint8_t msg[76];
    uint8_t challenge[8];
    uint8_t hmac[32];
    uint8_t sendBuf[73];
    int32_t ret;
    uint32_t i;

    do {
        ret = DS28S60_HmacEncryptRead(oldData, page);
        CHECK_ERRCODE_BREAK("DS28S60_HmacEncryptRead", ret);
        sendBuf[0] = page;
        DS28S60_GetHmacKey(secretKey, &g_ds28s60Info, BINDING_DATA_PAGE, DS28S60_CMD_CPT_SECRET);
        //PrintArray("secretKey", secretKey, 32);
        TrngGet(challenge, 8);
        //PrintArray("challenge", challenge, 8);
        memcpy(&msg[0], challenge, 8);
        memcpy(&msg[8], g_ds28s60Info.ROMID, 8);
        msg[16] = page;
        memcpy(&msg[17], g_ds28s60Info.MANID, 2);
        msg[19] = DS28S60_CMD_AUTH_WRITE;
        hmac_sha256(secretKey, 32, msg, 20, hmac);
        //PrintArray("hmac", hmac, 32);
        for (i = 0; i < 32; i ++) {
            sendBuf[i + 1] = data[i] ^ hmac[i];
        }
        //PrintArray("encrypted data", sendBuf + 1, 32);
        memcpy(&msg[0], g_ds28s60Info.ROMID, 8);
        memcpy(&msg[8], oldData, 32);
        memcpy(&msg[40], data, 32);
        msg[72] = page;
        memcpy(&msg[73], g_ds28s60Info.MANID, 2);
        msg[75] = DS28S60_CMD_AUTH_WRITE;
        //PrintArray("msg", msg, 76);
        hmac_sha256(secretKey, 32, msg, 76, hmac);
        //PrintArray("hmac", hmac, 32);
        memcpy(&sendBuf[33], hmac, 32);
        memcpy(&sendBuf[65], challenge, 8);
        //PrintArray("sendBuf", sendBuf, 73);
        ret = DS28S60_SendCmdAndGetResult(DS28S60_CMD_AUTH_WRITE, sendBuf, 73, 0, NULL);
        CHECK_ERRCODE_BREAK("DS28S60_SendCmdAndGetResult", ret);
    } while (0);

    CLEAR_ARRAY(oldData);
    CLEAR_ARRAY(secretKey);
    CLEAR_ARRAY(msg);
    CLEAR_ARRAY(challenge);
    CLEAR_ARRAY(hmac);
    CLEAR_ARRAY(sendBuf);

    return ret;
}

static void DS28S60_PrintInfo(void)
{
    int32_t ret;
    DS28S60_BlockProtection_t blockProtection;

    DS28S60_GetInfo();
    PrintArray("ROMID", g_ds28s60Info.ROMID, 8);
    PrintArray("MANID", g_ds28s60Info.MANID, 2);
    printf("valid=0x%02X\r\n", g_ds28s60Info.valid);
    for (int32_t i = 0; i < 36; i++) {
        ret = DS28S60_GetBlockProtection(&blockProtection, i);
        UserDelay(50);
        if (ret != DS28S60_SUCCESS) {
            printf("get block protection err,block=%d,ret=%d\r\n", i, ret);
            continue;
        }
        if (blockProtection.Secret.byte != 0x00 || blockProtection.Prot.byte != 0x00) {
            printf("block %d protection info:\r\n", i);
            printf("LOCK=%d,SEC_ID=0x%03X\r\n", blockProtection.Secret.b.LOCK, blockProtection.Secret.b.ID);
            printf("ECW=%d,ECH=%d,EPH=%d,APH=%d,WP=%d,RP=%d\r\n", blockProtection.Prot.b.ECW, \
                   blockProtection.Prot.b.ECH, blockProtection.Prot.b.EPH, blockProtection.Prot.b.APH, \
                   blockProtection.Prot.b.WP, blockProtection.Prot.b.RP);
        }
    }
}

/// @brief Get master secret from MCU OTP.
/// @param[out] masterSecret master secret, 32 bytes.
static void GetMasterSecret(uint8_t *masterSecret)
{
#ifdef DS28S60_TEST_MODE
    memcpy(masterSecret, MASTER_SECRET, sizeof(MASTER_SECRET));
#else
    OTP_PowerOn();
    memcpy(masterSecret, (uint8_t *)MASTER_SECRET_ADDR, 32);
#endif
}

/// @brief Get binding page data from MCU OTP.
/// @param[out] bindingPageData binding page data, 32 bytes.
static void GetBindingPageData(uint8_t *bindingPageData)
{
#ifdef DS28S60_TEST_MODE
    memcpy(bindingPageData, BINDING_PAGE_DATA, sizeof(BINDING_PAGE_DATA));
#else
    OTP_PowerOn();
    memcpy(bindingPageData, (uint8_t *)BINDING_PAGE_DATA_ADDR, 32);
#endif
}

/// @brief Get partial secret from MCU OTP.
/// @param[out] partialSecret partial secret, 32 bytes.
static void GetPartialSecret(uint8_t *partialSecret)
{
#ifdef DS28S60_TEST_MODE
    memcpy(partialSecret, PARTIAL_SECRET, sizeof(PARTIAL_SECRET));
#else
    OTP_PowerOn();
    memcpy(partialSecret, (uint8_t *)PARTIAL_SECRET_ADDR, 32);
#endif
}

void DS28S60_Test(int argc, char *argv[])
{
    uint8_t *data, pageData[32];
    DS28S60_BlockProtection_t blockProtection;
    int32_t ret, num, page, len, block;

    if (strcmp(argv[0], "random") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &num);
        printf("ds28s60 get rng,num=%d\r\n", num);
        data = SRAM_MALLOC(num);
        ret = DS28S60_GetRng(data, num);
        if (ret == DS28S60_SUCCESS) {
            PrintArray("data", data, num);
        } else {
            printf("ds28s60 err=%d\r\n", ret);
        }
        SRAM_FREE(data);
    } else if (strcmp(argv[0], "read") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &page);
        printf("ds28s60 read page %d\r\n", page);
        ret = DS28S60_ReadPage(pageData, page);
        if (ret == DS28S60_SUCCESS) {
            PrintArray("pageData", pageData, 32);
        } else {
            printf("ds28s60 err=%d\r\n", ret);
        }
    } else if (strcmp(argv[0], "write") == 0) {
        VALUE_CHECK(argc, 3);
        sscanf(argv[1], "%d", &page);
        len = strlen(argv[2]) / 2;
        if (len != 32) {
            printf("data len must be 32, now is %d\r\n", len);
        }
        StrToHex(pageData, argv[2]);
        printf("ds28s60 write page %d\r\n", page);
        ret = DS28S60_WritePage(pageData, page);
        if (ret == DS28S60_SUCCESS) {
            printf("write success\r\n");
        } else {
            printf("ds28s60 err=%d\r\n", ret);
        }
    } else if (strcmp(argv[0], "en_read") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &page);
        printf("ds28s60 en_read page %d\r\n", page);
        ret = DS28S60_HmacEncryptRead(pageData, page);
        if (ret == DS28S60_SUCCESS) {
            PrintArray("pageData", pageData, 32);
        } else {
            printf("ds28s60 err=%d\r\n", ret);
        }
    } else if (strcmp(argv[0], "en_write") == 0) {
        VALUE_CHECK(argc, 3);
        sscanf(argv[1], "%d", &page);
        len = strlen(argv[2]) / 2;
        if (len != 32) {
            printf("data len must be 32, now is %d\r\n", len);
        }
        StrToHex(pageData, argv[2]);
        printf("ds28s60 en_write page %d\r\n", page);
        ret = DS28S60_HmacEncryptWrite(pageData, page);
        if (ret == DS28S60_SUCCESS) {
            printf("write success\r\n");
        } else {
            printf("ds28s60 err=%d\r\n", ret);
        }
    } else if (strcmp(argv[0], "info") == 0) {
        DS28S60_PrintInfo();
    } else if (strcmp(argv[0], "write_secret") == 0) {
        ret = DS28S60_WriteSecret();
        if (ret == DS28S60_SUCCESS) {
            printf("write secret succ\r\n");
        } else {
            printf("write secret err=%d\r\n", ret);
        }
    } else if (strcmp(argv[0], "auth") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &page);
        ret = DS28S60_HmacAuthentication(page);
        if (ret == DS28S60_SUCCESS) {
            printf("hmac auth succ\r\n");
        } else {
            printf("hmac auth err=%d\r\n", ret);
        }
    } else if (strcmp(argv[0], "setup_block") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &block);
        memset(&blockProtection, 0, sizeof(DS28S60_BlockProtection_t));
        blockProtection.Secret.b.ID = DS28S60_SECRET_KEY_A;
        blockProtection.Prot.b.APH = 1;
        blockProtection.Prot.b.EPH = 1;
        ret = DS28S60_SetBlockProtection(&blockProtection, block);
        if (ret == DS28S60_SUCCESS) {
            printf("set block protection succ\r\n");
        } else {
            printf("set block protection err=%d\r\n", ret);
        }
    } else if (strcmp(argv[0], "setup_device") == 0) {
        printf("ds28s60 setup\r\n");
        ret = DS28S60_Setup();
        printf("DS28S60_Setup=%d\r\n", ret);
    }
}
