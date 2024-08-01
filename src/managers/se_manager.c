#include <stdio.h>
#include "stdlib.h"
#include "se_manager.h"
#include "se_interface.h"
#include "user_utils.h"
#include "user_memory.h"
#include "assert.h"
#include "sha256.h"
#include "err_code.h"
#include "drv_trng.h"
#include "drv_atecc608b.h"
#include "log_print.h"
#include "hash_and_salt.h"
#include "secret_cache.h"
#ifndef COMPILE_SIMULATOR
#include "drv_mpu.h"
#endif

#define SHA256_COUNT                            3

static int32_t SetNewKeyPieceToAtecc608b(uint8_t accountIndex, uint8_t *piece, const char *password);
static int32_t SetNewKeyPieceToDs28s60(uint8_t accountIndex, uint8_t *piece, const char *password);

static int32_t GetKeyPieceFromAtecc608b(uint8_t accountIndex, uint8_t *piece, const char *password);
static int32_t GetKeyPieceFromDs28s60(uint8_t accountIndex, uint8_t *piece, const char *password);

static int32_t SetNewKeyPieceToAtecc608b(uint8_t accountIndex, uint8_t *piece, const char *password)
{
    uint8_t authKey[32], hostRandom[32], inData[32], outData[32];
    int32_t ret;
    AccountSlot_t accountSlot;

    ASSERT(accountIndex <= 2);
    do {
        HashWithSalt(authKey, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "auth_key");
        GetAccountSlot(&accountSlot, accountIndex);
        //new kdf
        ret = SE_EncryptWrite(accountSlot.auth, 0, authKey);
        CHECK_ERRCODE_BREAK("write auth", ret);
        ret = SE_DeriveKey(accountSlot.rollKdf, authKey);
        CHECK_ERRCODE_BREAK("derive key", ret);
        TrngGet(hostRandom, 32);
        ret = SE_EncryptWrite(accountSlot.hostKdf, 0, hostRandom);
        CHECK_ERRCODE_BREAK("write kdf", ret);

        HashWithSalt(outData, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "password_atecc608b");
        memcpy(inData, outData, 32);
        ret = SE_Kdf(accountSlot.rollKdf, authKey, inData, 32, outData);
        CHECK_ERRCODE_BREAK("kdf", ret);
        memcpy(inData, outData, 32);
        ret = SE_Kdf(accountSlot.hostKdf, authKey, inData, 32, outData);
        CHECK_ERRCODE_BREAK("kdf", ret);
        for (uint32_t i = 0; i < SHA256_COUNT; i++) {
            memcpy(inData, outData, 32);
            sha256((struct sha256 *)outData, inData, 32);
        }
        memcpy(piece, outData, 32);
    } while (0);
    CLEAR_ARRAY(authKey);
    CLEAR_ARRAY(hostRandom);
    CLEAR_ARRAY(inData);
    CLEAR_ARRAY(outData);

    return ret;
}

static int32_t SetNewKeyPieceToDs28s60(uint8_t accountIndex, uint8_t *piece, const char *password)
{
    uint8_t passwordHash[32], randomKey[32], xData[32];
    int32_t ret;

    ASSERT(accountIndex <= 2);
    HashWithSalt(passwordHash, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "ds28s60 digest");
    TrngGet(randomKey, 32);
    for (uint32_t i = 0; i < 32; i++) {
        xData[i] = passwordHash[i] ^ randomKey[i];
    }
    do {
        ret = SE_HmacEncryptWrite(xData, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_KEY_PIECE);
        CHECK_ERRCODE_BREAK("write xData", ret);
        memcpy(piece, randomKey, 32);
    } while (0);
    CLEAR_ARRAY(passwordHash);
    CLEAR_ARRAY(randomKey);
    CLEAR_ARRAY(xData);

    return ret;
}

static int32_t GetKeyPieceFromAtecc608b(uint8_t accountIndex, uint8_t *piece, const char *password)
{
    uint8_t authKey[32], hostRandom[32], inData[32], outData[32];
    int32_t ret;
    AccountSlot_t accountSlot;

    ASSERT(accountIndex <= 2);
    do {
        HashWithSalt(authKey, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "auth_key");
        HashWithSalt(outData, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "password_atecc608b");
        memcpy(inData, outData, 32);

        GetAccountSlot(&accountSlot, accountIndex);
        ret = SE_Kdf(accountSlot.rollKdf, authKey, inData, 32, outData);
        CHECK_ERRCODE_BREAK("kdf", ret);
        memcpy(inData, outData, 32);
        ret = SE_Kdf(accountSlot.hostKdf, authKey, inData, 32, outData);
        CHECK_ERRCODE_BREAK("kdf", ret);
        for (uint32_t i = 0; i < SHA256_COUNT; i++) {
            memcpy(inData, outData, 32);
            sha256((struct sha256 *)outData, inData, 32);
        }
        memcpy(piece, outData, 32);
    } while (0);
    CLEAR_ARRAY(authKey);
    CLEAR_ARRAY(hostRandom);
    CLEAR_ARRAY(inData);
    CLEAR_ARRAY(outData);

    return ret;
}

static int32_t GetKeyPieceFromDs28s60(uint8_t accountIndex, uint8_t *piece, const char *password)
{
    uint8_t passwordHash[32], xData[32];
    int32_t ret;

    ASSERT(accountIndex <= 2);
    HashWithSalt(passwordHash, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "ds28s60 digest");
    do {
        ret = SE_HmacEncryptRead(xData, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_KEY_PIECE);
        CHECK_ERRCODE_BREAK("write xData", ret);
        for (uint32_t i = 0; i < 32; i++) {
            piece[i] = passwordHash[i] ^ xData[i];
        }
    } while (0);
    CLEAR_ARRAY(passwordHash);
    CLEAR_ARRAY(xData);

    return ret;
}

void GetAccountSlot(AccountSlot_t *accountSlot, uint8_t accountIndex)
{
    ASSERT(accountIndex <= 2);
    switch (accountIndex) {
    case 0: {
        accountSlot->auth = SLOT_AUTH_KEY_1;
        accountSlot->rollKdf = SLOT_ROLL_KDF_KEY_1;
        accountSlot->hostKdf = SLOT_HOST_KDF_KEY_1;
    }
    break;
    case 1: {
        accountSlot->auth = SLOT_AUTH_KEY_2;
        accountSlot->rollKdf = SLOT_ROLL_KDF_KEY_2;
        accountSlot->hostKdf = SLOT_HOST_KDF_KEY_2;
    }
    break;
    case 2: {
        accountSlot->auth = SLOT_AUTH_KEY_3;
        accountSlot->rollKdf = SLOT_ROLL_KDF_KEY_3;
        accountSlot->hostKdf = SLOT_HOST_KDF_KEY_3;
    }
    break;
    default:
        break;
    }
}

int32_t GetKeyPieceFromSE(uint8_t accountIndex, uint8_t *pieces, const char *password)
{
    int32_t ret = GetKeyPieceFromAtecc608b(accountIndex, pieces, password);
    CHECK_ERRCODE_RETURN_INT(ret);
    // KEYSTORE_PRINT_ARRAY("608 piece", pieces, 32);
    ret = GetKeyPieceFromDs28s60(accountIndex, pieces + KEY_PIECE_LEN, password);
    CHECK_ERRCODE_RETURN_INT(ret);
    // KEYSTORE_PRINT_ARRAY("ds28s60 piece", pieces + KEY_PIECE_LEN, 32);
    return ret;
}

int32_t SetNewKeyPieceToSE(uint8_t accountIndex, uint8_t *pieces, const char *password)
{
    //TODO: deal with error code
    int32_t ret;
    ret = SetNewKeyPieceToAtecc608b(accountIndex, pieces, password);
    CHECK_ERRCODE_RETURN_INT(ret);

    // KEYSTORE_PRINT_ARRAY("608 piece", pieces, 32);
    ret = SetNewKeyPieceToDs28s60(accountIndex, pieces + KEY_PIECE_LEN, password);
    CHECK_ERRCODE_RETURN_INT(ret);
    // KEYSTORE_PRINT_ARRAY("ds28s60 piece", pieces + KEY_PIECE_LEN, 32);
    return ret;
}

/// @brief Set the fingerprint encrypted password, store in SE.
/// @param[in] index
/// @param[in] encryptedPassword 32 bytes.
/// @return err code.
int32_t SetFpEncryptedPassword(uint32_t index, const uint8_t *encryptedPassword)
{
    ASSERT(index < 10);
    return SE_HmacEncryptWrite(encryptedPassword, PAGE_PF_ENCRYPTED_PASSWORD + index);
}

/// @brief Set the fingerprint state info.
/// @param[in] info 32 byte info.
/// @return err code.
int32_t SetFpStateInfo(uint8_t *info)
{
    uint8_t data[32] = {0};
    int32_t ret;

    MpuSetOtpProtection(false);
    memcpy(data, info, 32);
    ret = SE_HmacEncryptWrite(data, PAGE_PF_INFO);
    MpuSetOtpProtection(true);
    return ret;
}

/// @brief Get the fingerprint state info.
/// @param[out] info 32 byte info.
/// @return err code.
int32_t GetFpStateInfo(uint8_t *info)
{
    uint8_t data[32];
    int32_t ret;
    MpuSetOtpProtection(false);

    ret = SE_HmacEncryptRead(data, PAGE_PF_INFO);
    CHECK_ERRCODE_RETURN_INT(ret);
    memcpy(info, data, 32);
    MpuSetOtpProtection(true);
    return ret;
}

/// @brief Set the wallet data hash.
/// @param[in] index
/// @param[in] info 32 byte info.
/// @return err code.
int32_t SetWalletDataHash(uint8_t index, uint8_t *info)
{
    uint8_t data[32] = {0};
    int32_t ret;

    ASSERT(index <= 2);

    memcpy(data, info, 32);
    MpuSetOtpProtection(false);
    ret = SE_HmacEncryptWrite(data, PAGE_WALLET1_PUB_KEY_HASH + index);
    MpuSetOtpProtection(true);
    return ret;
}

/// @brief verify the wallet data hash.
/// @param[in] index
/// @param[in] info 32 byte info.
/// @return result of verify.
bool VerifyWalletDataHash(uint8_t index, uint8_t *info)
{
    uint8_t data[32];
    int32_t ret;

    ASSERT(index <= 2);

    MpuSetOtpProtection(false);
    ret = SE_HmacEncryptRead(data, PAGE_WALLET1_PUB_KEY_HASH + index);
    if (ret == SUCCESS_CODE && !memcmp(data, info, 32)) {
        return true;
    } else {
        if (CheckAllFF(data, 32) || CheckAllZero(data, 32)) {
            SetWalletDataHash(index, data);
            return true;
        } else {
            return false;
        }
    }
    MpuSetOtpProtection(true);
}

int32_t SetMultisigDataHash(uint8_t index, uint8_t *info)
{
    uint8_t data[32] = {0};
    int32_t ret;

    ASSERT(index <= 2);

    memcpy(data, info, 32);
    ret = SE_HmacEncryptWrite(data, index * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_MULTISIG_CONFIG_HASH);
    return ret;
}

bool VerifyMultisigWalletDataHash(uint8_t index, uint8_t *info)
{
    uint8_t data[32];
    int32_t ret;

    ASSERT(index <= 2);

    ret = SE_HmacEncryptRead(data, index * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_MULTISIG_CONFIG_HASH);
    if (ret == SUCCESS_CODE && !memcmp(data, info, 32)) {
        return true;
    } else {
        if (CheckAllFF(data, 32) || CheckAllZero(data, 32)) {
            SetMultisigDataHash(index, data);
            return true;
        } else {
            return false;
        }
    }
}

/// @brief Get the fingerprint encrypted password which stored in SE.
/// @param[in] index
/// @param[out] encryptedPassword 32 bytes.
/// @return err code.
int32_t GetFpEncryptedPassword(uint32_t index, uint8_t *encryptedPassword)
{
    ASSERT(index < 10);
    return SE_HmacEncryptRead(encryptedPassword, PAGE_PF_ENCRYPTED_PASSWORD + index);
}

/// @brief Set the fingerprint communication AES key.
/// @param[in] aesKey length 16bytes.
/// @return err code.
int32_t SetFpCommAesKey(const uint8_t *aesKey)
{
    return SE_HmacEncryptWrite(aesKey, PAGE_PF_AES_KEY);
}

/// @brief Get the fingerprint communication AES key.
/// @param[out] aesKey length 16bytes.
/// @return err code.
int32_t GetFpCommAesKey(uint8_t *aesKey)
{
    MpuSetOtpProtection(false);
    int32_t ret = SE_HmacEncryptRead(aesKey, PAGE_PF_AES_KEY);
    MpuSetOtpProtection(true);
    return ret;
}

/// @brief Set the fingerprint reset AES key.
/// @param[in] resetKey length 16bytes.
/// @return err code.
int32_t SetFpResetKey(const uint8_t *resetKey)
{
    return SE_HmacEncryptWrite(resetKey, PAGE_PF_RESET_KEY);
}

/// @brief Get the fingerprint reset AES key.
/// @param[out] resetKey length 16bytes.
/// @return err code.
int32_t GetFpResetKey(uint8_t *resetKey)
{
    return SE_HmacEncryptRead(resetKey, PAGE_PF_RESET_KEY);
}

/// @brief Get whether the fingerprint AES key exists.
/// @return true - exists.
bool FpAesKeyExist()
{
    uint8_t key[32];
    bool ret;

    MpuSetOtpProtection(false);
    if (SE_HmacEncryptRead(key, PAGE_PF_AES_KEY) != SUCCESS_CODE) {
        return false;
    }
    MpuSetOtpProtection(true);
    ret = CheckEntropy(key, 32);
    CLEAR_ARRAY(key);
    return ret;
}

int32_t SignMessageWithDeviceKey(uint8_t *messageHash, uint8_t *signaure)
{
    int32_t ret;
    do {
        ret = Atecc608bSignMessageWithDeviceKey(messageHash, signaure);
        CHECK_ERRCODE_BREAK("get device pubkey error", ret);
    } while (0);
    return ret;
}

/// @brief Get the device public key
/// @param pubkey
/// @return
int32_t GetDevicePublicKey(uint8_t *pubkey)
{
    int32_t ret;
    uint8_t pubkeyXY[64] = {0};
    do {
        ret = Atecc608bGenDevicePubkey(pubkeyXY);
        PrintArray("608B pubkey1", pubkeyXY, 64);
        CHECK_ERRCODE_BREAK("get device pubkey error", ret);
        pubkey[0] = 0x04;
        memcpy(pubkey + 1, pubkeyXY, 64);
        PrintArray("608B pubkey2", pubkey, 65);
    } while (0);
    return ret;
}
