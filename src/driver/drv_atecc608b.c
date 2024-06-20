#include "drv_atecc608b.h"
#include "define.h"
#include "string.h"
#include "cryptoauthlib.h"
#include "host/atca_host.h"
#include "log_print.h"
#include "user_memory.h"
#include "user_utils.h"
#include "drv_trng.h"
#include "sha256.h"
#include "drv_otp.h"
#include "assert.h"
#include "drv_mpu.h"

//#define ATECC608B_TEST_MODE

#define CHECK_ATECC608B_RET(content, ret)   {if (ret != ATCA_SUCCESS) {printf("%s err,0x%X\r\n", content, ret); break; }}

static int32_t Atecc608bBinding(void);
static void GetIoProtectKey(uint8_t *ioProtectKey);
static void GetAuthKey(uint8_t *authKey);
static void GetEncryptKey(uint8_t *encryptKey);
static uint8_t GetAuthSlot(uint8_t keySlot);
static void Atecc608bPrintConfig(const Atecc608bConfig_t *config);
static int32_t Atecc608bWriteConfig(void);
static ATCA_STATUS Atecc608bAuthorize(uint8_t authSlot, const uint8_t *authKey);

#ifdef ATECC608B_TEST_MODE
static const uint8_t g_ateccTestIoProtectKey[] = {
    0x5B, 0xF1, 0xC8, 0xB5, 0x0C, 0x27, 0xBC, 0x60,
    0x1B, 0x72, 0x11, 0x45, 0x6A, 0xA1, 0x17, 0x4D,
    0xCD, 0x19, 0x23, 0xEE, 0xF1, 0xCA, 0x64, 0x56,
    0xD7, 0xD0, 0xB7, 0x92, 0x1F, 0x7B, 0x7E, 0x5F
};

static const uint8_t g_ateccTestAuthKey[] = {
    0x24, 0x6C, 0x3D, 0xEA, 0x53, 0xF1, 0x03, 0xF7,
    0xE0, 0x47, 0xBC, 0x16, 0x19, 0xB0, 0x05, 0xAD,
    0x5C, 0x20, 0xE6, 0xE1, 0xA7, 0x4B, 0x40, 0x0B,
    0xE1, 0x76, 0x65, 0x0C, 0x38, 0xDB, 0x26, 0xAE
};

static const uint8_t g_ateccTestEncryptKey[] = {
    0xFB, 0xCA, 0xE6, 0xD6, 0xB5, 0xA9, 0xEE, 0x42,
    0x60, 0x6C, 0x55, 0x20, 0x7E, 0xFA, 0x6B, 0x4A,
    0x4F, 0x81, 0xA6, 0x6E, 0x38, 0x56, 0x1E, 0x20,
    0x5D, 0x54, 0xC0, 0x61, 0x72, 0xD1, 0xC4, 0x1F
};
#else
#define IO_PROTECT_KEY_ADDR     OTP_ADDR_ATECC608B
#define AUTH_KEY_ADDR           IO_PROTECT_KEY_ADDR + 32
#define ENCRYPT_KEY_ADDR        AUTH_KEY_ADDR + 32
#endif

/// @brief ATECC608B Init.
/// @param
void Atecc608bInit(void)
{
    atcab_init(&cfg_ateccx08a_i2c_default);
    Atecc608bBinding();
}

/// @brief Get random data from ATECC608B.
/// @param[out] rngArray
/// @param[in] num
/// @return err code.
int32_t Atecc608bGetRng(uint8_t *rngArray, uint32_t num)
{
    int32_t ret;
    uint8_t buffer[32];
    uint32_t i = 0, copyNum;

    while (i < num) {
        copyNum = (num - i) > 32 ? 32 : (num - i);
        ret = atcab_random(buffer);
        CHECK_ERRCODE_BREAK("atcab_random", ret);
        memcpy(rngArray + i, buffer, copyNum);
        i += copyNum;
    }
    CLEAR_ARRAY(buffer);

    return ret;
}

/// @brief Write data to ATECC608B on encrypted communication.
/// @param[in] slot ATECC608B data zone slot.
/// @param[in] block ATECC608B block.
/// @param[in] data 32 byte data.
/// @return err code.
int32_t Atecc608bEncryptWrite(uint8_t slot, uint8_t block, const uint8_t *data)
{
    int32_t ret;
    uint8_t nonce[20];
    uint8_t encryptKey[32];
    uint8_t authKey[32];

    do {
        GetEncryptKey(encryptKey);
        GetAuthKey(authKey);
        TrngGet(nonce, 20);
        ret = Atecc608bAuthorize(SLOT_AUTH_KEY, authKey);
        CHECK_ATECC608B_RET("auth", ret);
        ret = atcab_write_enc(slot, block, data, encryptKey, SLOT_ENCRYPT_KEY, nonce);
        CHECK_ATECC608B_RET("write_encrypt", ret);
    } while (0);
    CLEAR_ARRAY(nonce);
    CLEAR_ARRAY(encryptKey);
    CLEAR_ARRAY(authKey);
    return ret;
}

/// @brief Read data from ATECC608B on encrypted communication.
/// @param[in] slot ATECC608B data zone slot.
/// @param[in] block ATECC608B block.
/// @param[out] data 32 byte data.
/// @return err code.
int32_t Atecc608bEncryptRead(uint8_t slot, uint8_t block, uint8_t *data)
{
    int32_t ret;
    uint8_t nonce[20];
    uint8_t encryptKey[32];
    uint8_t authKey[32];

    MpuSetOtpProtection(false);
    do {
        GetEncryptKey(encryptKey);
        GetAuthKey(authKey);
        TrngGet(nonce, 20);
        ret = Atecc608bAuthorize(SLOT_AUTH_KEY, authKey);
        CHECK_ATECC608B_RET("auth", ret);
        ret = atcab_read_enc(slot, block, data, encryptKey, SLOT_ENCRYPT_KEY, nonce);
        CHECK_ATECC608B_RET("read_encrypt", ret);
    } while (0);
    MpuSetOtpProtection(true);
    CLEAR_ARRAY(nonce);
    CLEAR_ARRAY(encryptKey);
    CLEAR_ARRAY(authKey);
    return ret;
}

/// @brief Get KDF output.
/// @param[in] slot KDF key slot.
/// @param[in] inData Input data.
/// @param[in] authKey Auth key.
/// @param[in] inLen Input data length.
/// @param[out] outData Output data.
/// @return err code.
int32_t Atecc608bKdf(uint8_t slot, const uint8_t *authKey, const uint8_t *inData, uint32_t inLen, uint8_t *outData)
{
    int32_t ret;
    uint8_t nonce[32];
    uint8_t ioProtectKey[32];
    uint8_t authSlot;

    do {
        authSlot = GetAuthSlot(slot);
        if (authSlot == 255) {
            ret = ERR_ATECC608B_SLOT_NUM_ERR;
            break;
        }
        ret = Atecc608bAuthorize(authSlot, authKey);
        CHECK_ATECC608B_RET("auth", ret);
        ret = atcab_kdf(KDF_MODE_SOURCE_SLOT | KDF_MODE_TARGET_OUTPUT_ENC | KDF_MODE_ALG_HKDF,
                        slot, KDF_DETAILS_HKDF_MSG_LOC_INPUT | (inLen << 24), inData, outData, nonce);
        CHECK_ATECC608B_RET("kdf", ret);
        //PrintArray("outData", outData, 32);
        //PrintArray("nonce", nonce, 32);
        GetIoProtectKey(ioProtectKey);
        atca_io_decrypt_in_out_t io_dec_params = {
            .io_key = ioProtectKey,
            .out_nonce = nonce,
            .data = outData,
            .data_size = 32,
        };
        ret = atcah_io_decrypt(&io_dec_params);
        CHECK_ATECC608B_RET("atcah_io_decrypt", ret);
        //PrintArray("outData", outData, 32);
        //PrintArray("nonce", nonce, 32);
    } while (0);
    CLEAR_ARRAY(nonce);
    CLEAR_ARRAY(ioProtectKey);

    return ret;
}

/// @brief Derive key at the specified slot.
/// @param[in] slot Specified slot.
/// @param[in] authKey auth key.
/// @return err code.
int32_t Atecc608bDeriveKey(uint8_t slot, const uint8_t *authKey)
{
    int32_t ret;
    uint8_t nonce[NONCE_NUMIN_SIZE];
    uint8_t authSlot;

    do {
        authSlot = GetAuthSlot(slot);
        if (authSlot == 255) {
            ret = ERR_ATECC608B_SLOT_NUM_ERR;
            break;
        }
        ret = Atecc608bAuthorize(authSlot, authKey);
        CHECK_ATECC608B_RET("auth", ret);
        TrngGet(nonce, NONCE_NUMIN_SIZE);
        ret = atcab_nonce_rand(nonce, NULL);
        CHECK_ATECC608B_RET("nonce rand", ret);
        ret = atcab_derivekey(0, slot, NULL);
        CHECK_ATECC608B_RET("derivekey", ret);
    } while (0);
    CLEAR_ARRAY(nonce);

    return ret;
}

/// @brief Try to bind SE chip, return a err code if SE chip already binded.
/// @return err code.
static int32_t Atecc608bBinding(void)
{
    //OTP key exist, SE unlock, err.
    //OTP key exist, SE lock, check. Happend on non-first startup.
    //OTP key doesn't exist, SE unlock, generate new keys and bind it. Only happend on first startup.
    //OTP key doesn't exist, SE lock, err.

#ifdef ATECC608B_TEST_MODE
    return SUCCESS_CODE;
#else
    int32_t ret;
    bool isLock;
    uint8_t keys[96];
    do {
        ret = atcab_is_config_locked(&isLock);
        CHECK_ATECC608B_RET("get lock", ret);
        OTP_PowerOn();
        memcpy(keys, (uint8_t *)OTP_ADDR_ATECC608B, sizeof(keys));
        if (CheckEntropy(keys, 96)) {
            //OTP key exist
            if (!isLock) {
                printf("err,OTP key exist,SE unlock\r\n");
                ret = ERR_ATECC608B_BIND;
            } else {
                // normal start up, for none frist start.
                ret = 0;
            }
        } else {
            //OTP key doesn't exist
            if (!isLock) {
                // first binding logic
                TrngGet(keys, sizeof(keys));
                WriteOtpData(OTP_ADDR_ATECC608B, keys, sizeof(keys));
                //  return 0 for writing config successfully
                ret = Atecc608bWriteConfig();
            } else {
                printf("err,OTP key doesn't exist,SE lock\r\n");
                ret = ERR_ATECC608B_BIND;
            }
        }
    } while (0);
    CLEAR_ARRAY(keys);

    assert(ret == 0);
    return ret;
#endif
}

/// @brief Get IO protect key from MCU OTP.
/// @param[out] ioProtectKey IO protect key, 32 bytes.
static void GetIoProtectKey(uint8_t *ioProtectKey)
{
#ifdef ATECC608B_TEST_MODE
    memcpy(ioProtectKey, g_ateccTestIoProtectKey, sizeof(g_ateccTestIoProtectKey));
#else
    OTP_PowerOn();
    memcpy(ioProtectKey, (uint8_t *)IO_PROTECT_KEY_ADDR, 32);
#endif
}

/// @brief Get auth key from MCU OTP.
/// @param[out] authKey Auth key, 32 bytes.
static void GetAuthKey(uint8_t *authKey)
{
#ifdef ATECC608B_TEST_MODE
    memcpy(authKey, g_ateccTestAuthKey, sizeof(g_ateccTestAuthKey));
#else
    OTP_PowerOn();
    memcpy(authKey, (uint8_t *)AUTH_KEY_ADDR, 32);
#endif
}

/// @brief Get encrypt key from MCU OTP.
/// @param[out] ioProtectKey Encrypt key, 32 bytes.
static void GetEncryptKey(uint8_t *encryptKey)
{
#ifdef ATECC608B_TEST_MODE
    memcpy(encryptKey, g_ateccTestEncryptKey, sizeof(g_ateccTestEncryptKey));
#else
    OTP_PowerOn();
    memcpy(encryptKey, (uint8_t *)ENCRYPT_KEY_ADDR, 32);
#endif
}

/// @brief Get auth slot by key slot.
/// @param[in] keySlot
/// @return auth slot, return 255 if error.
static uint8_t GetAuthSlot(uint8_t keySlot)
{
    if (keySlot == 4 || keySlot == 5) {
        return 3;
    }
    if (keySlot == 7 || keySlot == 9) {
        return 6;
    }
    if (keySlot == 11 || keySlot == 12) {
        return 10;
    }
    return 255;
}

static void Atecc608bPrintConfig(const Atecc608bConfig_t *config)
{
    PrintArray("sn1", config->sn1, sizeof(config->sn1));
    PrintArray("revNum", config->revNum, sizeof(config->revNum));
    PrintArray("sn2", config->sn2, sizeof(config->sn2));
    printf("aesEnable=0x%02X\r\n", config->aesEnable);
    printf("i2cEnable=0x%02X\r\n", config->i2cEnable);
    printf("i2cAddress=0x%02X\r\n", config->i2cAddress);
    printf("countMatch=0x%02X\r\n", config->countMatch);
    printf("chipMode=0x%02X\r\n", config->chipMode);
    PrintU16Array("slotConfig", config->slotConfig, sizeof(config->slotConfig) / 2);
    PrintArray("counter0", config->counter0, sizeof(config->counter0));
    PrintArray("counter1", config->counter1, sizeof(config->counter1));
    printf("useLock=0x%02X\r\n", config->useLock);
    printf("volatileKeyPermission=0x%02X\r\n", config->volatileKeyPermission);
    printf("secureBoot=0x%04X\r\n", config->secureBoot);
    printf("kdfivLoc=0x%02X\r\n", config->kdfivLoc);
    printf("kdfivStr=0x%04X\r\n", config->kdfivStr);
    printf("userExtra=0x%02X\r\n", config->userExtra);
    printf("userExtraAdd=0x%02X\r\n", config->userExtraAdd);
    printf("lockValue=0x%02X\r\n", config->lockValue);
    printf("lockConfig=0x%02X\r\n", config->lockConfig);
    printf("slotLocked=0x%04X\r\n", config->slotLocked);
    printf("chipOptions=0x%04X\r\n", config->chipOptions);
    PrintArray("x509Format", config->x509Format, sizeof(config->x509Format));
    PrintU16Array("keyConfig", config->keyConfig, sizeof(config->keyConfig) / 2);
}

static int32_t Atecc608bWriteConfig(void)
{
    //shared keys
    //slot 0        ioprotect key,                  slot config=0x8080, key config=0x007C, lockable=1.
    //slot 1        auth key,                       slot config=0x8080, key config=0x007C, lockable=1.
    //slot 2        encryption key,                 slot config=0x0083, key config=0x01FC, lockable=1, need authorization(slot1).
    //slot 15       tamper flag

    // Device Key
    //slot 14       Device unique Key P256          slot config=0x2083, key config=0x0013,  lockable=0, ECC, PuInfo set (public key always allowed to be generated), contains a private Key
    // EXT signatures, INT signatures, IsSecret, Write config never

    //individual keys
    //slot 3        user1 auth key,                 slot config=0x42A0, key config=0x005C, lockable=0, encrypt write, limited use.
    //slot 4        user1 roll kdf key,             slot config=0x20A0, key config=0x03DC, lockable=0, need authorization(slot3), inner derivable, limited use.
    //slot 5        user1 host random kdf key,      slot config=0x42A0, key config=0x03DC, lockable=0, need authorization(slot3), encrypt write(slot2 as key), limited use.

    //slot 6        user2 auth key,                 slot config=0x42A0, key config=0x005C, lockable=0, encrypt write, limited use.
    //slot 7        user2 roll kdf key,             slot config=0x20A0, key config=0x06DC, lockable=0, need authorization(slot6), inner derivable, limited use.
    //slot 9        user2 host random kdf key,      slot config=0x42A0, key config=0x06DC, lockable=0, need authorization(slot6), encrypt write(slot2 as key), limited use.

    //slot 10       user3 auth key,                 slot config=0x42A0, key config=0x005C, lockable=0, encrypt write, limited use.
    //slot 11       user3 roll kdf key,             slot config=0x20A0, key config=0x0ADC, lockable=0, need authorization(slot10), inner derivable, limited use.
    //slot 12       user3 host random kdf key,      slot config=0x42A0, key config=0x0ADC, lockable=0, need authorization(slot10), encrypt write(slot2 as key), limited use.

    //slot
    //8/13/  user data,currently not used.   slot config=0x42C2, key config=0x005C, lockable=0, encrypt write/read.

    const uint16_t slotConfig[] = {  0x8080, 0x8080, 0x8080, 0x42A0, 0x20A0, 0x42A0, 0x42A0, 0x20A0,
                                     0x42C2, 0x42A0, 0x42A0, 0x20A0, 0x42A0, 0x42C2, 0x0083, 0x42C2
                                  };
    const uint16_t keyConfig[] = {   0x007C, 0x007C, 0x01FC, 0x005C, 0x03DC, 0x03DC, 0x005C, 0x06DC,
                                     0x005C, 0x06DC, 0x005C, 0x0ADC, 0x0ADC, 0x005C, 0x0013, 0x005C
                                 };
    int32_t ret;
    Atecc608bConfig_t config;
    bool isLock;
    uint8_t tempKey[32];

    do {
        ret = atcab_is_config_locked(&isLock);
        CHECK_ATECC608B_RET("get lock", ret);
        if (isLock == true) {
            printf("already locked\r\n");
            ret = ERR_ATECC608B_UNEXPECT_LOCK;
            break;
        }
        ret = atcab_read_config_zone((uint8_t *)&config);
        printf("atcab_read_config_zone=%d\r\n", ret);
        CHECK_ATECC608B_RET("read config zone", ret);
        memcpy(config.slotConfig, slotConfig, sizeof(slotConfig));
        memcpy(config.keyConfig, keyConfig, sizeof(keyConfig));
        config.chipOptions = 0x0402;
        ret = atcab_write_config_zone((uint8_t *)&config);
        CHECK_ATECC608B_RET("write config zone", ret);
        ret = atcab_lock_config_zone();
        CHECK_ATECC608B_RET("lock config zone", ret);
        printf("lock config zone ok\r\n");
        GetIoProtectKey(tempKey);
        ret = atcab_write_zone(ATCA_ZONE_DATA, SLOT_IO_PROTECT_KEY, 0, 0, tempKey, 32);
        CHECK_ATECC608B_RET("write io protect key", ret);
        GetAuthKey(tempKey);
        ret = atcab_write_zone(ATCA_ZONE_DATA, SLOT_AUTH_KEY, 0, 0, tempKey, 32);
        CHECK_ATECC608B_RET("write auth key", ret);
        GetEncryptKey(tempKey);
        ret = atcab_write_zone(ATCA_ZONE_DATA, SLOT_ENCRYPT_KEY, 0, 0, tempKey, 32);
        CHECK_ATECC608B_RET("write encrypt key", ret);
        printf("write key ok\r\n");
        // generate the unique key for the device;
        ret = atcab_genkey(SLOT_DEVICE_KEY, NULL);
        CHECK_ATECC608B_RET("generate unique key", ret);
        printf("generate device unique key ok\r\n");
        ret = atcab_lock_data_zone();
        printf("lock data zone ok\r\n");
        printf("config done\r\n");
    } while (0);

    CLEAR_ARRAY(tempKey);
    return ret;
}

/// @brief Before using a slot as a key, authorizion is required.
/// @param
/// @return err code.
static ATCA_STATUS Atecc608bAuthorize(uint8_t authSlot, const uint8_t *authKey)
{
    uint8_t numIn[NONCE_NUMIN_SIZE];
    uint8_t randOut[32];
    uint8_t otherData[13];
    atca_temp_key_t tempKey;
    uint8_t response[32];
    uint8_t sn[9];
    ATCA_STATUS ret;
    atca_nonce_in_out_t nonceParams = {
        .mode = NONCE_MODE_SEED_UPDATE,
        .zero = 0,
        .num_in = numIn,
        .rand_out = randOut,
        .temp_key = &tempKey,
    };
    atca_check_mac_in_out_t checkmacParams = {
        // First SHA block from slot key, Second SHA block from TempKey.
        .mode = CHECKMAC_MODE_BLOCK2_TEMPKEY,
        .key_id = authSlot,
        .sn = sn,
        .client_chal = NULL, // unused in this mode
        .client_resp = response,
        .other_data = otherData,
        .otp = NULL, // unused in this mode,
        .slot_key = authKey,
        .target_key = NULL,
        .temp_key = &tempKey,
    };

    do {
        TrngGet(numIn, sizeof(numIn));
        ret = atcab_nonce_rand(nonceParams.num_in, randOut);
        CHECK_ATECC608B_RET("nonce rand", ret);
        //Calculate contents of TempKey.
        ret = atcah_nonce(&nonceParams);
        CHECK_ATECC608B_RET("nonce", ret);
        TrngGet(otherData, sizeof(otherData));
        ret = atcab_read_serial_number(sn);
        CHECK_ATECC608B_RET("read sn", ret);
        // Compute client response.
        ret = atcah_check_mac(&checkmacParams);
        CHECK_ATECC608B_RET("host check mac", ret);
        ret = atcab_checkmac(
                  checkmacParams.mode,
                  checkmacParams.key_id,
                  checkmacParams.client_chal,
                  checkmacParams.client_resp,
                  checkmacParams.other_data);
        CHECK_ATECC608B_RET("check mac", ret);
    } while (0);
    CLEAR_ARRAY(numIn);
    CLEAR_ARRAY(randOut);
    CLEAR_ARRAY(otherData);
    CLEAR_ARRAY(response);
    CLEAR_OBJECT(tempKey);

    return ret;
}

/// @brief Get the device P256 unique key from 608B
/// @param arg point for the public key
/// @return return
int32_t Atecc608bGenDevicePubkey(uint8_t *pubkey)
{
    int32_t ret;
    do {
        ret = atcab_get_pubkey(SLOT_DEVICE_KEY, pubkey);
        CHECK_ATECC608B_RET("genKey", ret);
    } while (0);
    return ret;
}

int32_t Atecc608bSignMessageWithDeviceKey(uint8_t *messageHash, uint8_t *signature)
{
    int32_t ret;
    do {
        ret = atcab_sign(SLOT_DEVICE_KEY, messageHash, signature);
        CHECK_ATECC608B_RET("signDeviceKey", ret);
    } while (0);
    return ret;
}

/// @brief
/// @param argc Test arg count.
/// @param argv Test arg values.
void Atecc608bTest(int argc, char *argv[])
{
    int32_t ret;
    uint8_t *pData, *pDataOut, *nonce, *authKey;
    uint32_t slot, offset, len, keyBlock, temp1, temp2, block;
    bool bRet;

    printf("crypto auth lib test(ATECC608B):\r\n");
    if (strcmp(argv[0], "random") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &len);
        pData = SRAM_MALLOC(len);
        ret = Atecc608bGetRng(pData, len);
        printf("Atecc608bGetRng=%d\r\n", ret);
        PrintArray("data", pData, len);
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "readconfig") == 0) {
        pData = SRAM_MALLOC(BUFFER_SIZE_128);
        ret = atcab_read_config_zone(pData);
        printf("atcab_read_config_zone=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            PrintArray("data", pData, 128);
        }
        Atecc608bPrintConfig((Atecc608bConfig_t *)pData);
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "read") == 0) {
        VALUE_CHECK(argc, 4);
        sscanf(argv[1], "%d", &slot);
        sscanf(argv[2], "%d", &offset);
        sscanf(argv[3], "%d", &len);
        pData = SRAM_MALLOC(len);
        ret = atcab_read_bytes_zone(ATCA_ZONE_DATA, slot, offset, pData, len);
        printf("atcab_read_bytes_zone=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            PrintArray("data", pData, len);
        }
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "write") == 0) {
        VALUE_CHECK(argc, 4);
        sscanf(argv[1], "%d", &slot);
        sscanf(argv[2], "%d", &offset);
        pData = SRAM_MALLOC(strlen(argv[3]) / 2 + 1);
        len = StrToHex(pData, argv[3]);
        printf("writing %d bytes,slot=%d,offset=%d\r\n", len, slot, offset);
        ret = atcab_write_bytes_zone(ATCA_ZONE_DATA, slot, offset, pData, len);
        printf("atcab_write_bytes_zone=%d\r\n", ret);
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "serial") == 0) {
        pData = SRAM_MALLOC(32);
        memset_s(pData, 9, 0, 9);
        ret = atcab_read_serial_number(pData);
        printf("atcab_read_serial_number=%d\r\n", ret);
        PrintArray("serial number", pData, 9);
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "isslotlock") == 0) {
        sscanf(argv[1], "%d", &slot);
        ret = atcab_is_slot_locked(slot, &bRet);
        printf("atcab_is_slot_locked=%d,result=%d\r\n", ret, bRet);
        ret = atcab_is_private(slot, &bRet);
        printf("atcab_is_private=%d,result=%d\r\n", ret, bRet);
    } else if (strcmp(argv[0], "islock") == 0) {
        ret = atcab_is_locked(LOCK_ZONE_CONFIG, &bRet);
        printf("config ret=%d,result=%d\r\n", ret, bRet);
        ret = atcab_is_locked(LOCK_ZONE_DATA, &bRet);
        printf("data ret=%d,result=%d\r\n", ret, bRet);
    } else if (strcmp(argv[0], "lockconfigzone") == 0) {
        ret = atcab_lock_config_zone();
        printf("atcab_lock_config_zone=%d\r\n", ret);
    } else if (strcmp(argv[0], "lockdatazone") == 0) {
        ret = atcab_lock_data_zone();
        printf("atcab_lock_data_zone=%d\r\n", ret);
    } else if (strcmp(argv[0], "aes_encrypt") == 0) {
        VALUE_CHECK(argc, 4);
        sscanf(argv[1], "%d", &slot);
        sscanf(argv[2], "%d", &keyBlock);
        pData = SRAM_MALLOC(strlen(argv[3]) / 2 + 1);
        len = StrToHex(pData, argv[3]);
        printf("aes encrypt %d bytes,keySlot=%d,keyBlock=%d\r\n", len, slot, keyBlock);
        //ret = atcab_write_bytes_zone(ATCA_ZONE_CONFIG, slot, offset, pData, len);
        pDataOut = SRAM_MALLOC(16);
        ret = atcab_aes_encrypt(slot, keyBlock, pData, pDataOut);
        printf("atcab_aes_encrypt=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            PrintArray("pDataOut", pDataOut, 16);
        }
        SRAM_FREE(pData);
        SRAM_FREE(pDataOut);
    } else if (strcmp(argv[0], "read_counter") == 0) {
        temp1 = 0;
        ret = atcab_counter_read(temp1, &temp2);
        //printf("atcab_counter_read=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            printf("counter0=%d\r\n", temp2);
        }
        temp1 = 1;
        ret = atcab_counter_read(temp1, &temp2);
        //printf("atcab_counter_read=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            printf("counter1=%d\r\n", temp2);
        }
    } else if (strcmp(argv[0], "increase_counter") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &temp1);
        ret = atcab_counter_increment(temp1, &temp2);
        printf("calib_counter_increment=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            printf("new counter=%d\r\n", temp2);
        }
    } else if (strcmp(argv[0], "gen_key") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &slot);
        pData = SRAM_MALLOC(64);
        ret = atcab_genkey(slot, pData);
        printf("atcab_genkey=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            PrintArray("public key", pData, 64);
        }
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "get_pubkey") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &slot);
        pData = SRAM_MALLOC(64);
        ret = atcab_get_pubkey(slot, pData);
        printf("atcab_get_pubkey=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            PrintArray("public key", pData, 64);
        }
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "info") == 0) {
        pData = SRAM_MALLOC(4);
        ret = atcab_info(pData);
        printf("atcab_info=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            PrintArray("revision", pData, 4);
        }
        ret = atcab_info_get_latch(&bRet);
        printf("atcab_info_get_latch=%d\r\n", ret);
        if (ret == ATCA_SUCCESS) {
            printf("bRet=%d\r\n", bRet);
        }
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "write_pri_key") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &slot);
        pData = SRAM_MALLOC(36);
        memset_s(pData, 4, 0, 4);
        TrngGet(pData + 4, 32);
        nonce = SRAM_MALLOC(20);
        PrintArray("prikey", pData, 36);
        TrngGet(nonce, 20);
        PrintArray("nonce", nonce, 20);
        ret = atcab_priv_write(slot, pData, 0, NULL, nonce);
        printf("atcab_priv_write=%d\r\n", ret);
        SRAM_FREE(pData);
        SRAM_FREE(nonce);
    } else if (strcmp(argv[0], "write_config") == 0) {
        printf("Atecc608bWriteConfig\r\n");
        Atecc608bWriteConfig();
    } else if (strcmp(argv[0], "en_read") == 0) {
        VALUE_CHECK(argc, 3);
        sscanf(argv[1], "%d", &slot);
        sscanf(argv[2], "%d", &block);
        pData = SRAM_MALLOC(32);
        ret = Atecc608bEncryptRead(slot, block, pData);
        if (ret == ATCA_SUCCESS) {
            PrintArray("data", pData, 32);
        }
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "en_write") == 0) {
        VALUE_CHECK(argc, 4);
        sscanf(argv[1], "%d", &slot);
        sscanf(argv[2], "%d", &block);
        pData = SRAM_MALLOC(strlen(argv[3]) / 2 + 1);
        len = StrToHex(pData, argv[3]);
        ret = Atecc608bEncryptWrite(slot, block, pData);
        if (ret == ATCA_SUCCESS) {
            printf("en write succ\r\n");
        }
        SRAM_FREE(pData);
    } else if (strcmp(argv[0], "kdf") == 0) {
        VALUE_CHECK(argc, 4);
        if (strlen(argv[2]) != 64) {
            printf("auth input err\r\n");
            return;
        }
        sscanf(argv[1], "%d", &slot);
        authKey = SRAM_MALLOC(32);
        pData = SRAM_MALLOC(strlen(argv[3]) / 2 + 1);
        pDataOut = SRAM_MALLOC(32);
        StrToHex(authKey, argv[2]);
        len = StrToHex(pData, argv[3]);
        ret = Atecc608bKdf(slot, authKey, pData, len, pDataOut);
        if (ret == ATCA_SUCCESS) {
            PrintArray("kdf output", pDataOut, 32);
            printf("kdf succ\r\n");
        } else {
            printf("kdf err=%d\r\n", ret);
        }
        SRAM_FREE(pData);
        SRAM_FREE(pDataOut);
        SRAM_FREE(authKey);
    } else if (strcmp(argv[0], "derivekey") == 0) {
        VALUE_CHECK(argc, 3);
        if (strlen(argv[2]) != 64) {
            printf("auth input err\r\n");
            return;
        }
        authKey = SRAM_MALLOC(32);
        StrToHex(authKey, argv[2]);
        sscanf(argv[1], "%d", &slot);
        printf("derive key at slot %d\r\n", slot);
        ret = Atecc608bDeriveKey(slot, authKey);
        if (ret == ATCA_SUCCESS) {
            printf("derivekey succ\r\n");
        } else {
            printf("derivekey err=%d\r\n", ret);
        }
    }
}