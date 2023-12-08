#include "keystore.h"
#include "string.h"
#include "stdio.h"
#include "user_utils.h"
#include "hash_and_salt.h"
#include "hkdf.h"
#include "drv_atecc608b.h"
#include "drv_ds28s60.h"
#include "drv_trng.h"
#include "sha256.h"
#include "hmac.h"
#include "ctaes.h"
#include "sha512.h"
#include "log_print.h"
#include "bip39.h"
#include "slip39.h"
#include "user_memory.h"
#include "drv_otp.h"
#include "librust_c.h"
#include "account_public_info.h"
#include "drv_rtc.h"
#include "se_interface.h"
#include "se_manager.h"
#include "account_manager.h"
#include "librust_c.h"
#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#else
#define memset_s(p, s, c, l) memset(p, c, l)
#endif

#define KEYSTORE_DEBUG          0


#if KEYSTORE_DEBUG == 1
#define KEYSTORE_PRINT_ARRAY(fmt, args...)       PrintArray(fmt, ##args)
#else
#define KEYSTORE_PRINT_ARRAY(fmt, args...)
#endif

#define AES_BLOCK_SIZE                          16

#define KEY_PIECE_LEN                           32

#define AES_KEY_LEN                             32
#define AUTH_KEY_LEN                            32

#define AES_IV_LEN                              32              //Use first 16 bytes for AES key, last 16 bytes reserved for future features.
#define ENTROPY_MAX_LEN                         32
#define SEED_LEN                                64
#define SLIP39_EMS_LEN                          32
#define SE_DATA_RESERVED_LEN                    32
#define HMAC_LEN                                32
#define ACCOUNT_TOTAL_LEN                       (AES_IV_LEN + ENTROPY_MAX_LEN + SEED_LEN + SLIP39_EMS_LEN + SE_DATA_RESERVED_LEN + HMAC_LEN)
#define PARAM_LEN                               32

#define PASSPHRASE_MAX_LEN                      128
#define ITERATION_TIME                          700

typedef struct {
    uint8_t entropy[ENTROPY_MAX_LEN];
    uint8_t seed[SEED_LEN];
    uint8_t slip39Ems[SLIP39_EMS_LEN];
    uint8_t reservedData[SE_DATA_RESERVED_LEN];
    uint8_t entropyLen;
} AccountSecret_t;

typedef struct {
    char passphrase[PASSPHRASE_MAX_LEN + 1];
    bool passphraseExist;
    uint8_t mfp[4];
} PassphraseInfo_t;

static PassphraseInfo_t g_passphraseInfo[3] = {0};

static int32_t SaveAccountSecret(uint8_t accountIndex, const AccountSecret_t *accountSecret, const char *password, bool newAccount);
static int32_t LoadAccountSecret(uint8_t accountIndex, AccountSecret_t *accountSecret, const char *password);

static void CombineInnerAesKey(uint8_t *aesKey);
static int32_t GetPassphraseSeed(uint8_t accountIndex, uint8_t *seed, const char *passphrase, const char *password);

/// @brief Generate 32 byte entropy from SE and mcu TRNG.
/// @param[out] entropy
/// @param[in] entropyLen
/// @param[in] password Password hash is part of the entropy sources.
/// @return err code.
int32_t GenerateEntropy(uint8_t *entropy, uint8_t entropyLen, const char *password)
{
    uint8_t randomBuffer[ENTROPY_MAX_LEN], inputBuffer[ENTROPY_MAX_LEN], outputBuffer[ENTROPY_MAX_LEN];
    int32_t ret;

    do {
        HashWithSalt(inputBuffer, (uint8_t *)password, strlen(password), "generate entropy");
        
        SE_GetTRng(randomBuffer, ENTROPY_MAX_LEN);
        KEYSTORE_PRINT_ARRAY("trng", randomBuffer, ENTROPY_MAX_LEN);
        // set the initial value
        memcpy(outputBuffer, randomBuffer, ENTROPY_MAX_LEN);
        hkdf(inputBuffer, randomBuffer, outputBuffer, ITERATION_TIME);
        KEYSTORE_PRINT_ARRAY("outputBuffer", outputBuffer, ENTROPY_MAX_LEN);
        memcpy(inputBuffer, outputBuffer, ENTROPY_MAX_LEN);
        
        ret = SE_GetDS28S60Rng(randomBuffer, ENTROPY_MAX_LEN);
        CHECK_ERRCODE_BREAK("get ds28s60 trng", ret);
        KEYSTORE_PRINT_ARRAY("ds28s60 rng", randomBuffer, ENTROPY_MAX_LEN);
        hkdf(inputBuffer, randomBuffer, outputBuffer, ITERATION_TIME);
        KEYSTORE_PRINT_ARRAY("outputBuffer", outputBuffer, ENTROPY_MAX_LEN);
        memcpy(inputBuffer, outputBuffer, ENTROPY_MAX_LEN);
        
        ret = SE_GetAtecc608bRng(randomBuffer, ENTROPY_MAX_LEN);
        CHECK_ERRCODE_BREAK("get 608b trng", ret);
        KEYSTORE_PRINT_ARRAY("608b rng", randomBuffer, ENTROPY_MAX_LEN);
        hkdf(inputBuffer, randomBuffer, outputBuffer, ITERATION_TIME);
        
        KEYSTORE_PRINT_ARRAY("finalEntropy", outputBuffer, ENTROPY_MAX_LEN);
        memcpy(entropy, outputBuffer, entropyLen);
    } while (0);
    CLEAR_ARRAY(outputBuffer);
    CLEAR_ARRAY(inputBuffer);
    CLEAR_ARRAY(randomBuffer);
    return ret;
}

/// @brief Save a new entropy.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] entropy Entropy to be saved.
/// @param[in] entropyLen
/// @param[in] password Password string.
/// @return err code.
int32_t SaveNewEntropy(uint8_t accountIndex, const uint8_t *entropy, uint8_t entropyLen, const char *password)
{
    int32_t ret;
    AccountSecret_t accountSecret = {0};
    char *mnemonic = NULL;
    uint8_t passwordHash[32];

    ASSERT(accountIndex <= 2);
    do {
        ret = CheckPasswordExisted(password, 255);
        CHECK_ERRCODE_BREAK("check repeat password", ret);
        memcpy(accountSecret.entropy, entropy, entropyLen);
        accountSecret.entropyLen = entropyLen;
        //bip39 entropy->mnemonic->seed
        ret = bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
        CHECK_ERRCODE_BREAK("bip39_mnemonic_from_bytes", ret);
        ret = bip39_mnemonic_to_seed(mnemonic, NULL, accountSecret.seed, SEED_LEN, NULL);
        CHECK_ERRCODE_BREAK("bip39_mnemonic_to_seed", ret);
        SRAM_FREE(mnemonic);

        ret = SaveAccountSecret(accountIndex, &accountSecret, password, true);
        CHECK_ERRCODE_BREAK("SaveAccountSecret", ret);
        HashWithSalt(passwordHash, (const uint8_t *)password, strlen(password), "password hash");
        ret = SE_HmacEncryptWrite(passwordHash, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("write password hash", ret);

    } while (0);

    CLEAR_ARRAY(passwordHash);
    CLEAR_OBJECT(accountSecret);
    return ret;
}


/// @brief Save a new slip39 entropy.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] entropy Entropy to be saved.
/// @param[in] entropyLen
/// @param[in] password Password string.
/// @param[in] id Idrandom identifier..
/// @param[in] ie Iteration exponent.
/// @return err code.
int32_t SaveNewSlip39Entropy(uint8_t accountIndex, const uint8_t *ems, const uint8_t *entropy, uint8_t entropyLen, const char *password, uint16_t id, uint8_t ie)
{
    int32_t ret;
    AccountSecret_t accountSecret = {0};
    uint8_t passwordHash[32];

    ASSERT(accountIndex <= 2);
    do {
        ret = CheckPasswordExisted(password, 255);
        CHECK_ERRCODE_BREAK("check repeat password", ret);
        memcpy(accountSecret.entropy, entropy, entropyLen);
        accountSecret.entropyLen = entropyLen;

        memcpy(accountSecret.seed, entropy, entropyLen);
        memcpy(accountSecret.slip39Ems, ems, entropyLen);
        ret = SaveAccountSecret(accountIndex, &accountSecret, password, true);
        CHECK_ERRCODE_BREAK("SaveAccountSecret", ret);
        HashWithSalt(passwordHash, (const uint8_t *)password, strlen(password), "password hash");
        ret = SE_HmacEncryptWrite(passwordHash, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("write password hash", ret);

    } while (0);

    CLEAR_ARRAY(passwordHash);
    CLEAR_OBJECT(accountSecret);
    return ret;
}


/// @brief Get entropy from SE by password.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] entropy Entropy.
/// @param[out] entropyLen
/// @param[in] password Password string.
/// @return err code.
int32_t GetAccountEntropy(uint8_t accountIndex, uint8_t *entropy, uint8_t *entropyLen, const char *password)
{
    int32_t ret;
    AccountSecret_t accountSecret;

    ASSERT(accountIndex <= 2);
    ret = LoadAccountSecret(accountIndex, &accountSecret, password);
    if (ret == SUCCESS_CODE) {
        memcpy(entropy, accountSecret.entropy, ENTROPY_MAX_LEN);
        *entropyLen = accountSecret.entropyLen;
    }
    CLEAR_OBJECT(accountSecret);
    return ret;
}


/// @brief Get seed from SE by password, if there is a passphrase before, set a passphrase seed.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] seed Seed.
/// @param[in] password Password string.
/// @return err code.
int32_t GetAccountSeed(uint8_t accountIndex, uint8_t *seed, const char *password)
{
    int32_t ret;
    AccountSecret_t accountSecret;
    uint8_t tempSeed[SEED_LEN];
    uint32_t seedLen;

    ASSERT(accountIndex <= 2);
    do {
        ret = LoadAccountSecret(accountIndex, &accountSecret, password);
        CHECK_ERRCODE_BREAK("LoadAccountSecret", ret);
        if (PassphraseExist(accountIndex) == false) {
            //no passphrase
            seedLen = GetMnemonicType() == MNEMONIC_TYPE_SLIP39 ? accountSecret.entropyLen : sizeof(accountSecret.seed);
            memcpy(seed, accountSecret.seed, seedLen);
        } else {
            //recalculate seed with passphrase
            ret = GetPassphraseSeed(accountIndex, tempSeed, g_passphraseInfo[accountIndex].passphrase, password);
            CHECK_ERRCODE_BREAK("GetPassphraseSeed", ret);
            memcpy(seed, tempSeed, SEED_LEN);
        }
    } while (0);

    CLEAR_OBJECT(accountSecret);
    CLEAR_ARRAY(tempSeed);
    return ret;
}


/// @brief Get slip39 ems from SE by password.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] slip39Ems slip39Ems.
/// @param[in] password Password string.
/// @return err code.
int32_t GetAccountSlip39Ems(uint8_t accountIndex, uint8_t *slip39Ems, const char *password)
{
    int32_t ret;
    AccountSecret_t accountSecret;

    ASSERT(accountIndex <= 2);
    ret = LoadAccountSecret(accountIndex, &accountSecret, password);
    if (ret == SUCCESS_CODE) {
        memcpy(slip39Ems, accountSecret.slip39Ems, SLIP39_EMS_LEN);
    }
    CLEAR_OBJECT(accountSecret);
    return ret;
}


/// @brief Change password.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] newPassword New password.
/// @param[in] password Old password.
/// @return err code.
int32_t ChangePassword(uint8_t accountIndex, const char *newPassword, const char *password)
{
    int32_t ret;
    AccountSecret_t accountSecret;
    uint8_t passwordHash[32];

    ASSERT(accountIndex <= 2);
    do {
        ret = CheckPasswordExisted(newPassword, accountIndex);
        CHECK_ERRCODE_BREAK("check repeat password", ret);
        ret = LoadAccountSecret(accountIndex, &accountSecret, password);
        CHECK_ERRCODE_BREAK("load account secret", ret);
        ret = SaveAccountSecret(accountIndex, &accountSecret, newPassword, false);
        CHECK_ERRCODE_BREAK("save account secret", ret);
        HashWithSalt(passwordHash, (const uint8_t *)newPassword, strlen(newPassword), "password hash");
        ret = SE_HmacEncryptWrite(passwordHash, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("write password hash", ret);
    } while (0);
    CLEAR_OBJECT(accountSecret);
    return ret;
}


/// @brief Verify password.
/// @param[out] accountIndex If password verify success, account index would be set here. Can be NULL if not needed.
/// @param password Password string.
/// @return err code.
int32_t VerifyPassword(uint8_t *accountIndex, const char *password)
{
    uint8_t passwordHashClac[32], passwordHashStore[32];
    int32_t ret, i;

    for (i = 0; i < 3; i++) {
        ret = SE_HmacEncryptRead(passwordHashStore, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("read password hash", ret);
        HashWithSalt(passwordHashClac, (const uint8_t *)password, strlen(password), "password hash");
        if (memcmp(passwordHashStore, passwordHashClac, 32) == 0) {
            if (accountIndex != NULL) {
                *accountIndex = i;
            }
            ret = SUCCESS_CODE;
            break;
        } else {
            ret = ERR_KEYSTORE_PASSWORD_ERR;
        }
    }

    CLEAR_ARRAY(passwordHashStore);
    CLEAR_ARRAY(passwordHashClac);
    return ret;
}


/// @brief Check if password repeat with existing others.
/// @param[in] password Password string.
/// @param[in] excludeIndex exclude account index, if do not need exclude any account, set excludeIndex to 255.
/// @return err code.
int32_t CheckPasswordExisted(const char *password, uint8_t excludeIndex)
{
    int32_t ret;
    uint8_t accountIndex;

    ret = VerifyPassword(&accountIndex, password);
    if (ret == SUCCESS_CODE && excludeIndex != accountIndex) {
        //password existed
        ret = ERR_KEYSTORE_REPEAT_PASSWORD;
    } else if (ret == ERR_KEYSTORE_PASSWORD_ERR) {
        ret = SUCCESS_CODE;
    }
    return ret;
}

/// @brief Set passphrase
/// @param[in] accountIndex
/// @param[in] passphrase
/// @param[in] password Password string.
/// @return err code.
int32_t SetPassphrase(uint8_t accountIndex, const char *passphrase, const char *password)
{
    uint8_t seed[SEED_LEN];
    int32_t ret;
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();

    ASSERT(strlen(password) >= 6);
    ASSERT(accountIndex <= 2);
    do {
        ret = GetPassphraseSeed(accountIndex, seed, passphrase, password);
        CHECK_ERRCODE_BREAK("GetPassphraseSeed", ret);
        SimpleResponse_u8 *simpleResponse = get_master_fingerprint(seed, len);
        if (simpleResponse == NULL) {
            printf("get_master_fingerprint return NULL\r\n");
            break;
        }
        if (simpleResponse->error_code != 0) {
            printf("get_master_fingerprint error\r\n");
            if (simpleResponse->error_message != NULL) {
                printf("error code = %d\r\nerror msg is: %s\r\n", simpleResponse->error_code, simpleResponse->error_message);
            }
        }
        CHECK_ERRCODE_BREAK("get_master_fingerprint", simpleResponse->error_code);
        uint8_t *masterFingerprint = simpleResponse->data;
        memcpy(g_passphraseInfo[accountIndex].mfp, masterFingerprint, 4);
        free_simple_response_u8(simpleResponse);
        if (strlen(passphrase) > 0) {
            strcpy(g_passphraseInfo[accountIndex].passphrase, passphrase);
            g_passphraseInfo[accountIndex].passphraseExist = true;
        } else {
            ClearAccountPassphrase(accountIndex);
        }
        ret = TempAccountPublicInfo(accountIndex, password, true);
        SetPassphraseMark(passphrase[0] != '\0');
    } while (0);
    CLEAR_ARRAY(seed);

    return ret;
}

bool CheckPassphraseSame(uint8_t accountIndex, const char *passphrase)
{
    ASSERT(accountIndex <= 2);
    return !strcmp(passphrase, g_passphraseInfo[accountIndex].passphrase);
}

/// @brief Get master fingerprint
/// @param[out] mfp
void GetMasterFingerPrint(uint8_t *mfp)
{
    uint8_t accountIndex = GetCurrentAccountIndex();
    if (accountIndex > 2) {
        memset(mfp, 0, 4);
    } else {
        if (PassphraseExist(accountIndex)) {
            memcpy(mfp, g_passphraseInfo[accountIndex].mfp, 4);
        } else {
            memcpy(mfp, GetCurrentAccountMfp(), 4);
        }
    }
}


void ClearAccountPassphrase(uint8_t accountIndex)
{
    if (accountIndex > 2) {
        return;
    }
    memset_s(g_passphraseInfo[accountIndex].passphrase, sizeof(g_passphraseInfo[accountIndex].passphrase), 0, sizeof(g_passphraseInfo[accountIndex].passphrase));
    memset_s(g_passphraseInfo[accountIndex].mfp, sizeof(g_passphraseInfo[accountIndex].mfp), 0, sizeof(g_passphraseInfo[accountIndex].mfp));
    g_passphraseInfo[accountIndex].passphraseExist = false;
}

/// @brief Get if the passphrase exist.
/// @param accountIndex
/// @return true-exist, false-not exist.
bool PassphraseExist(uint8_t accountIndex)
{
    if (accountIndex > 2) {
        return false;
    }

    assert(g_passphraseInfo[accountIndex].passphraseExist == (strlen(g_passphraseInfo[accountIndex].passphrase) > 0));
    return (strlen(g_passphraseInfo[accountIndex].passphrase) > 0);
}

char* GetPassphrase(uint8_t accountIndex)
{
    if (accountIndex > 2) {
        return false;
    }

    return g_passphraseInfo[accountIndex].passphrase;
}

/// @brief Save account secret, including entropy/seed/reservedData.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] accountSecret Account secret data.
/// @param[in] password Password string.
/// @param[in] newAccount is new account.
/// @return err code.
static int32_t SaveAccountSecret(uint8_t accountIndex, const AccountSecret_t *accountSecret, const char *password, bool newAccount)
{
    uint8_t pieces[KEY_PIECE_LEN * 2], hash[32], sha512Hash[64];
    uint8_t *enKey, *authKey;
    uint8_t *iv, *encryptEntropy, *encryptSeed, *slip39Ems, *encryptReservedData, *hmac;
    uint8_t accountEncryptData[ACCOUNT_TOTAL_LEN];
    int32_t ret;
    AES256_CBC_ctx ctx;
    uint32_t seedLen;

    ASSERT(accountIndex <= 2);
    enKey = sha512Hash;
    authKey = sha512Hash + AES_KEY_LEN;
    iv = accountEncryptData;
    encryptEntropy = iv + AES_IV_LEN;
    encryptSeed = encryptEntropy + ENTROPY_MAX_LEN;
    slip39Ems = encryptSeed + SEED_LEN;
    encryptReservedData = slip39Ems + SLIP39_EMS_LEN;
    hmac = encryptReservedData + SE_DATA_RESERVED_LEN;
    do {
        ret = SetNewKeyPieceToSE(accountIndex, pieces, password);
        CHECK_ERRCODE_BREAK("set key to se", ret);
        HashWithSalt(hash, pieces, sizeof(pieces), "combine two pieces");
        KEYSTORE_PRINT_ARRAY("pieces hash", hash, sizeof(hash));
        sha512((struct sha512 *)sha512Hash, hash, sizeof(hash));
        KEYSTORE_PRINT_ARRAY("sha512Hash", sha512Hash, 64);
        TrngGet(iv, AES_IV_LEN);
        CombineInnerAesKey(enKey);
        AES256_CBC_init(&ctx, enKey, iv);
        AES256_CBC_encrypt(&ctx, ENTROPY_MAX_LEN / AES_BLOCK_SIZE, encryptEntropy, accountSecret->entropy);
        AES256_CBC_encrypt(&ctx, SEED_LEN / AES_BLOCK_SIZE, encryptSeed, accountSecret->seed);
        AES256_CBC_encrypt(&ctx, SLIP39_EMS_LEN / AES_BLOCK_SIZE, slip39Ems, accountSecret->slip39Ems);
        AES256_CBC_encrypt(&ctx, SE_DATA_RESERVED_LEN / AES_BLOCK_SIZE, encryptReservedData, accountSecret->reservedData);
        KEYSTORE_PRINT_ARRAY("iv", iv, AES_IV_LEN);
        KEYSTORE_PRINT_ARRAY("enKey", enKey, AES_KEY_LEN);
        KEYSTORE_PRINT_ARRAY("encryptEntropy", encryptEntropy, ENTROPY_MAX_LEN);
        KEYSTORE_PRINT_ARRAY("encryptSeed", encryptSeed, SEED_LEN);
        KEYSTORE_PRINT_ARRAY("slip39Ems", slip39Ems, SLIP39_EMS_LEN);
        KEYSTORE_PRINT_ARRAY("encryptReservedData", encryptReservedData, SE_DATA_RESERVED_LEN);
        hmac_sha256(authKey, AUTH_KEY_LEN, accountEncryptData, ACCOUNT_TOTAL_LEN - HMAC_LEN, hmac);
        KEYSTORE_PRINT_ARRAY("accountEncryptData", accountEncryptData, ACCOUNT_TOTAL_LEN);
        KEYSTORE_PRINT_ARRAY("authKey", authKey, AUTH_KEY_LEN);
        //param[0] = accountSecret->entropyLen;
        ret = SE_HmacEncryptWrite(iv, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV);
        CHECK_ERRCODE_BREAK("write iv", ret);
        ret = SE_HmacEncryptWrite(encryptEntropy, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_ENTROPY);
        CHECK_ERRCODE_BREAK("write encrypt entropy", ret);
        ret = SE_HmacEncryptWrite(encryptSeed, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SEED_H32);
        CHECK_ERRCODE_BREAK("write encrypt seed h32", ret);
        ret = SE_HmacEncryptWrite(encryptSeed + 32, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SEED_L32);
        CHECK_ERRCODE_BREAK("write encrypt seed l32", ret);
        ret = SE_HmacEncryptWrite(slip39Ems, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SLIP39_EMS);
        CHECK_ERRCODE_BREAK("write slip39 ems", ret);
        ret = SE_HmacEncryptWrite(encryptReservedData, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_RESERVED);
        CHECK_ERRCODE_BREAK("write encrypt reserved data", ret);
        ret = SE_HmacEncryptWrite(hmac, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_HMAC);
        CHECK_ERRCODE_BREAK("write hmac", ret);
        if (newAccount) {
            SetCurrentAccountEntropyLen(accountSecret->entropyLen);
            seedLen = GetMnemonicType() == MNEMONIC_TYPE_SLIP39 ? accountSecret->entropyLen : sizeof(accountSecret->seed);
            SimpleResponse_u8 *simpleResponse = get_master_fingerprint((PtrBytes)accountSecret->seed, seedLen);
            if (simpleResponse == NULL) {
                printf("get_master_fingerprint return NULL\r\n");
                break;
            }
            if (simpleResponse->error_code != 0) {
                printf("get_master_fingerprint error\r\n");
                if (simpleResponse->error_message != NULL) {
                    printf("error code = %d\r\nerror msg is: %s\r\n", simpleResponse->error_code, simpleResponse->error_message);
                }
            }
            CHECK_ERRCODE_BREAK("get_master_fingerprint", simpleResponse->error_code);
            uint8_t *masterFingerprint = simpleResponse->data;
            PrintArray("masterFingerprint", masterFingerprint, 4);
            SetCurrentAccountMfp(masterFingerprint);
            free_simple_response_u8(simpleResponse);
            SaveCurrentAccountInfo();
            CHECK_ERRCODE_BREAK("write param", ret);
        }
    } while (0);

    CLEAR_OBJECT(ctx);
    CLEAR_ARRAY(pieces);
    CLEAR_ARRAY(hash);
    CLEAR_ARRAY(sha512Hash);
    CLEAR_ARRAY(accountEncryptData);
    return ret;
}


/// @brief Load account secret, including entropy/seed/reservedData.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] accountSecret Account secret data.
/// @param[in] password Password string.
/// @return err code.
static int32_t LoadAccountSecret(uint8_t accountIndex, AccountSecret_t *accountSecret, const char *password)
{
    uint8_t pieces[KEY_PIECE_LEN * 2], hash[32], sha512Hash[64], hmacCalc[32];
    uint8_t *enKey, *authKey;
    uint8_t *iv, *encryptEntropy, *encryptSeed, *slip39Ems, *encryptReservedData, *hmac;
    uint8_t accountEncryptData[ACCOUNT_TOTAL_LEN], param[32];
    AccountInfo_t *pAccountInfo = (AccountInfo_t *)param;
    int32_t ret;
    AES256_CBC_ctx ctx;

    ASSERT(accountIndex <= 2);
    enKey = sha512Hash;
    authKey = sha512Hash + AES_KEY_LEN;
    iv = accountEncryptData;
    encryptEntropy = iv + AES_IV_LEN;
    encryptSeed = encryptEntropy + ENTROPY_MAX_LEN;
    slip39Ems = encryptSeed + SEED_LEN;
    encryptReservedData = slip39Ems + SLIP39_EMS_LEN;
    hmac = encryptReservedData + SE_DATA_RESERVED_LEN;
    do {
        ret = GetKeyPieceFromSE(accountIndex, pieces, password);
        HashWithSalt(hash, pieces, sizeof(pieces), "combine two pieces");
        KEYSTORE_PRINT_ARRAY("pieces hash", hash, sizeof(hash));
        sha512((struct sha512 *)sha512Hash, hash, sizeof(hash));
        KEYSTORE_PRINT_ARRAY("sha512Hash", sha512Hash, 64);

        ret = SE_HmacEncryptRead(iv, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV);
        CHECK_ERRCODE_BREAK("read iv", ret);
        ret = SE_HmacEncryptRead(encryptEntropy, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_ENTROPY);
        CHECK_ERRCODE_BREAK("read encrypt entropy", ret);
        ret = SE_HmacEncryptRead(encryptSeed, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SEED_H32);
        CHECK_ERRCODE_BREAK("read encrypt seed h32", ret);
        ret = SE_HmacEncryptRead(encryptSeed + 32, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SEED_L32);
        CHECK_ERRCODE_BREAK("read encrypt seed l32", ret);
        ret = SE_HmacEncryptRead(slip39Ems, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SLIP39_EMS);
        CHECK_ERRCODE_BREAK("read slip39 ems", ret);
        ret = SE_HmacEncryptRead(encryptReservedData, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_RESERVED);
        CHECK_ERRCODE_BREAK("read encrypt reserved data", ret);
        ret = SE_HmacEncryptRead(hmac, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_HMAC);
        CHECK_ERRCODE_BREAK("read hmac", ret);
        ret = SE_HmacEncryptRead(param, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PARAM);
        CHECK_ERRCODE_BREAK("read param", ret);
        hmac_sha256(authKey, AUTH_KEY_LEN, accountEncryptData, ACCOUNT_TOTAL_LEN - HMAC_LEN, hmacCalc);
        KEYSTORE_PRINT_ARRAY("accountEncryptData", accountEncryptData, ACCOUNT_TOTAL_LEN);
        KEYSTORE_PRINT_ARRAY("authKey", authKey, AUTH_KEY_LEN);
        ret = ((memcmp(hmacCalc, hmac, HMAC_LEN) == 0) ? SUCCESS_CODE : ERR_KEYSTORE_AUTH);
        CHECK_ERRCODE_BREAK("check hmac", ret);
        accountSecret->entropyLen = (pAccountInfo->entropyLen == 0) ? 32 : pAccountInfo->entropyLen;    //32 bytes as default.
        CombineInnerAesKey(enKey);
        AES256_CBC_init(&ctx, enKey, iv);
        AES256_CBC_decrypt(&ctx, ENTROPY_MAX_LEN / AES_BLOCK_SIZE, accountSecret->entropy, encryptEntropy);
        AES256_CBC_decrypt(&ctx, SEED_LEN / AES_BLOCK_SIZE, accountSecret->seed, encryptSeed);
        AES256_CBC_decrypt(&ctx, SLIP39_EMS_LEN / AES_BLOCK_SIZE, accountSecret->slip39Ems, slip39Ems);
        AES256_CBC_decrypt(&ctx, SE_DATA_RESERVED_LEN / AES_BLOCK_SIZE, accountSecret->reservedData, encryptReservedData);
        KEYSTORE_PRINT_ARRAY("iv", iv, AES_IV_LEN);
        KEYSTORE_PRINT_ARRAY("enKey", enKey, AES_KEY_LEN);
        KEYSTORE_PRINT_ARRAY("encryptEntropy", encryptEntropy, ENTROPY_MAX_LEN);
        KEYSTORE_PRINT_ARRAY("encryptSeed", encryptSeed, SEED_LEN);
        KEYSTORE_PRINT_ARRAY("slip39Ems", slip39Ems, SLIP39_EMS_LEN);
        KEYSTORE_PRINT_ARRAY("encryptReservedData", encryptReservedData, SE_DATA_RESERVED_LEN);
    } while (0);

    CLEAR_OBJECT(ctx);
    CLEAR_ARRAY(pieces);
    CLEAR_ARRAY(hash);
    CLEAR_ARRAY(sha512Hash);
    CLEAR_ARRAY(accountEncryptData);
    CLEAR_ARRAY(hmacCalc);
    return ret;
}

/// @brief Combine with the internal AES KEY of MCU.
/// @param[inout] aesKey
/// @return
static void CombineInnerAesKey(uint8_t *aesKey)
{
    uint8_t aesPiece[AES_KEY_LEN];
    OTP_PowerOn();
    memcpy(aesPiece, (uint8_t *)OTP_ADDR_AES_KEY, AES_KEY_LEN);
    if (CheckEntropy(aesPiece, AES_KEY_LEN) == false) {
        printf("need generate inner aes piece\r\n");
        TrngGet(aesPiece, AES_KEY_LEN);
        WriteOtpData(OTP_ADDR_AES_KEY, aesPiece, AES_KEY_LEN);
    }
    for (uint32_t i = 0; i < AES_KEY_LEN; i++) {
        aesKey[i] ^= aesPiece[i];
    }
}


/// @brief Get seed generated by passphrase.
/// @param[in] accountIndex
/// @param[out] seed Seed.
/// @param[in] passphrase Passphrase string.
/// @param[in] password Password string.
/// @return err code.
static int32_t GetPassphraseSeed(uint8_t accountIndex, uint8_t *seed, const char *passphrase, const char *password)
{
    uint8_t entropy[ENTROPY_MAX_LEN], entropyLen;
    int32_t ret;
    char *mnemonic = NULL;

    ASSERT(accountIndex <= 2);
    do {
        if (GetMnemonicType() == MNEMONIC_TYPE_SLIP39) {
            uint8_t slip39Ems[SLIP39_EMS_LEN];
            GetAccountSlip39Ems(accountIndex, slip39Ems, password);
            ret = Slip39GetSeed(slip39Ems, seed, GetCurrentAccountEntropyLen(), passphrase, GetSlip39Ie(), GetSlip39Id());
            CHECK_ERRCODE_BREAK("slip39_mnemonic_to_seed", ret);
        } else {
            ret = GetAccountEntropy(accountIndex, entropy, &entropyLen, password);
            CHECK_ERRCODE_BREAK("GetAccountEntropy", ret);
            ret = bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
            CHECK_ERRCODE_BREAK("bip39_mnemonic_from_bytes", ret);
            ret = bip39_mnemonic_to_seed(mnemonic, passphrase, seed, SEED_LEN, NULL);
            CHECK_ERRCODE_BREAK("bip39_mnemonic_to_seed", ret);
            SRAM_FREE(mnemonic);
        }
    } while (0);

    return ret;
}

#ifndef BUILD_PRODUCTION

/// @brief
/// @param argc Test arg count.
/// @param argv Test arg values.
void KeyStoreTest(int argc, char *argv[])
{
    uint8_t entropy[ENTROPY_MAX_LEN], seed[SEED_LEN], accountIndex, entropyLen, key[32], slip39Ems[SLIP39_EMS_LEN];
    int32_t index, ret, tempI32;
    uint8_t byte32[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    char tempStr[32];
    uint8_t ems[32];
    if (strcmp(argv[0], "new_entropy") == 0) {
        VALUE_CHECK(argc, 4);
        sscanf(argv[1], "%d", &index);
        sscanf(argv[2], "%d", &tempI32);
        entropyLen = tempI32;
        GenerateEntropy(entropy, entropyLen, argv[3]);
        PrintArray("entropy", entropy, entropyLen);
        ret = CreateNewAccount(index, entropy, entropyLen, argv[3]);
        printf("CreateNewAccount=%d\r\n", ret);
    } else if (strcmp(argv[0], "new_slip39_entropy") == 0) {
        VALUE_CHECK(argc, 4);
        sscanf(argv[1], "%d", &index);
        sscanf(argv[2], "%d", &tempI32);
        entropyLen = tempI32;
        GenerateEntropy(entropy, entropyLen, argv[3]);
        PrintArray("entropy", entropy, entropyLen);
        ret = CreateNewSlip39Account(index, ems, entropy, entropyLen, argv[3], 4543, 0);
        printf("CreateNewSlip39Account=%d\r\n", ret);
    } else if (strcmp(argv[0], "save_slip39_entropy") == 0) {
        VALUE_CHECK(argc, 5);
        sscanf(argv[1], "%d", &index);
        sscanf(argv[2], "%d", &tempI32);
        entropyLen = tempI32;
        printf("entropylen = %d\n", entropyLen);
        if (StrToHex(entropy, argv[3]) != entropyLen) {
            printf("input length err\r\n");
        }
        PrintArray("entropy", entropy, entropyLen);
        uint8_t ems[32] = {0};
        ret = CreateNewSlip39Account(index, ems, entropy, entropyLen, argv[4], 1234, 0);
        printf("CreateNewSlip39Account=%d\r\n", ret);
    } else if (strcmp(argv[0], "get_entropy") == 0) {
        VALUE_CHECK(argc, 3);
        sscanf(argv[1], "%d", &index);
        ret = GetAccountEntropy(index, entropy, &entropyLen, argv[2]);
        printf("GetAccountEntropy=%d\r\n", ret);
        if (ret == SUCCESS_CODE) {
            PrintArray("entropy", entropy, entropyLen);
        }
    } else if (strcmp(argv[0], "save_new_entropy") == 0) {
        VALUE_CHECK(argc, 5);
        sscanf(argv[1], "%d", &index);
        sscanf(argv[2], "%d", &tempI32);
        entropyLen = tempI32;
        printf("entropylen = %d\n", entropyLen);
        if (StrToHex(entropy, argv[3]) != entropyLen) {
            printf("input length err\r\n");
        }
        PrintArray("entropy", entropy, 32);
        ret = CreateNewAccount(index, entropy, entropyLen, argv[4]);
        printf("CreateNewAccount=%d\r\n", ret);
    } else if (strcmp(argv[0], "get_seed") == 0) {
        VALUE_CHECK(argc, 3);
        sscanf(argv[1], "%d", &index);
        ret = GetAccountSeed(index, seed, argv[2]);
        printf("GetAccountSeed=%d\r\n", ret);
        if (ret == SUCCESS_CODE) {
            PrintArray("seed", seed, SEED_LEN);
        }
    } else if (strcmp(argv[0], "get_slip39_ems") == 0) {
        VALUE_CHECK(argc, 3);
        sscanf(argv[1], "%d", &index);
        ret = GetAccountSlip39Ems(index, slip39Ems, argv[2]);
        printf("GetAccountSlip39Ems=%d\r\n", ret);
        if (ret == SUCCESS_CODE) {
            PrintArray("slip39Ems", slip39Ems, SLIP39_EMS_LEN);
        }
    } else if (strcmp(argv[0], "verify_password") == 0) {
        VALUE_CHECK(argc, 2);
        ret = VerifyCurrentAccountPassword(argv[1]);
        if (ret == SUCCESS_CODE) {
            printf("VerifyCurrentAccountPassword ok\r\n");
        } else {
            printf("VerifyCurrentAccountPassword err=%d\r\n", ret);
        }
        ret = VerifyPassword(&accountIndex, argv[1]);
        if (ret == SUCCESS_CODE) {
            printf("password verify ok,accountIndex=%d\r\n", accountIndex);
        } else {
            printf("password verify err\r\n");
        }
    } else if (strcmp(argv[0], "login") == 0) {
        VALUE_CHECK(argc, 2);
        ret = VerifyPasswordAndLogin(&accountIndex, argv[1]);
        if (ret == SUCCESS_CODE) {
            printf("login ok,accountIndex=%d\r\n", accountIndex);
        } else {
            printf("login err=%d\r\n", ret);
        }
    } else if (strcmp(argv[0], "change_password") == 0) {
        VALUE_CHECK(argc, 4);
        sscanf(argv[1], "%d", &index);
        ret = ChangePassword(index, argv[2], argv[3]);
        printf("ChangePassword=%d\r\n", ret);
    } else if (strcmp(argv[0], "get_account") == 0) {
        ret = GetBlankAccountIndex(&accountIndex);
        if (ret == SUCCESS_CODE) {
            printf("next blank account=%d\r\n", accountIndex);
        } else {
            printf("get blank account err\r\n");
        }
        ret = GetExistAccountNum(&accountIndex);
        if (ret == SUCCESS_CODE) {
            printf("existing account num=%d\r\n", accountIndex);
        } else {
            printf("get exist account err\r\n");
        }
    } else if (strcmp(argv[0], "destroy_account") == 0) {
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%d", &index);
        ret = DestroyAccount(index);
        printf("DestroyAccount=%d\r\n", ret);
    } else if (strcmp(argv[0], "get_info") == 0) {
        uint8_t mfp[4];
        AccountInfo_t accountInfo;
        printf("current account index=%d\r\n", GetCurrentAccountIndex());
        printf("GetPasscodeType=%d,GetPassphraseQuickAccess=%d,GetWalletIconIndex=%d\r\n", GetPasscodeType(), GetPassphraseQuickAccess(), GetWalletIconIndex());
        printf("GetMnemonicType=%d,slip39_ID=%d,slip39_IE=%d\r\n", GetMnemonicType(), GetSlip39Id(), GetSlip39Ie());
        printf("wallet name=%s\r\n", GetWalletName());
        GetMasterFingerPrint(mfp);
        PrintArray("mfp", mfp, 4);
        for (accountIndex = 0; accountIndex < 3; accountIndex++) {
            GetAccountInfo(accountIndex, &accountInfo);
            PrintArray("accountInfo", (uint8_t *)&accountInfo, sizeof(AccountInfo_t));
        }
    } else if (strcmp(argv[0], "set_info") == 0) {
        PasscodeType passcodeType;
        MnemonicType mnemonicType;
        VALUE_CHECK(argc, 3);
        if (strcmp(argv[1], "passcode") == 0) {
            sscanf(argv[2], "%d", &tempI32);
            passcodeType = tempI32 ? PASSCODE_TYPE_PASSWORD : PASSCODE_TYPE_PIN;
            SetPasscodeType(passcodeType);
            printf("set passcode type %d\r\n", passcodeType);
        } else if (strcmp(argv[1], "mnemonic") == 0) {
            sscanf(argv[2], "%d", &tempI32);
            mnemonicType = tempI32 ? MNEMONIC_TYPE_SLIP39 : MNEMONIC_TYPE_BIP39;
            SetMnemonicType(mnemonicType);
            printf("set mnemonic type %d\r\n", mnemonicType);
        } else if (strcmp(argv[1], "passphrase") == 0) {
            sscanf(argv[2], "%d", &tempI32);
            SetPassphraseQuickAccess(tempI32 != 0);
            printf("set passphrase exist %d\r\n", tempI32 != 0);
        } else if (strcmp(argv[1], "icon") == 0) {
            sscanf(argv[2], "%d", &tempI32);
            SetWalletIconIndex(tempI32);
            printf("set icon %d\r\n", tempI32);
        } else if (strcmp(argv[1], "name") == 0) {
            SetWalletName(argv[2]);
            printf("set name %s\r\n", argv[2]);
        } else {
            printf("set_info input err\r\n");
        }
    } else if (strcmp(argv[0], "public_info") == 0) {
        printf("login password err count=%d\r\n", GetLoginPasswordErrorCount());
        printf("current password err count=%d\r\n", GetCurrentPasswordErrorCount());
        printf("last lock device time=%d\r\n", GetLastLockDeviceTime());
    } else if (strcmp(argv[0], "set_last_lock_device_time") == 0) {
        SetLastLockDeviceTime(GetCurrentStampTime());
        printf("set last lock device time done\n");
    } else if (strcmp(argv[0], "set_passphrase") == 0) {
        VALUE_CHECK(argc, 3);
        ret = SetPassphrase(GetCurrentAccountIndex(), argv[1], argv[2]);
        printf("SetPassphrase=%d\r\n", ret);
    } else if (strcmp(argv[0], "get_passphrase") == 0) {
        for (accountIndex = 0; accountIndex < 3; accountIndex++) {
            printf("accountIndex %d:\r\n", accountIndex);
            printf("passphrase=%s\r\n", g_passphraseInfo[accountIndex].passphrase);
            printf("mfp=0x%08X\r\n", *((uint32_t *)(g_passphraseInfo[accountIndex].mfp)));
        }
    } else if (strcmp(argv[0], "set_fp_info") == 0) {
        printf("set fp info test\r\n");
        TrngGet(key, 32);
        PrintArray("fp aes key", key, 32);
        SetFpCommAesKey(key);
        TrngGet(key, 32);
        PrintArray("fp rest key", key, 32);
        SetFpResetKey(key);
        for (index = 0; index < 10; index++) {
            memset(key, index + 1, 32);
            SetFpEncryptedPassword(index, key);
        }
        SetFpStateInfo(byte32);
        printf("set done\r\n");
    } else if (strcmp(argv[0], "get_fp_info") == 0) {
        printf("get fp info test\r\n");
        if (FpAesKeyExist()) {
            GetFpCommAesKey(key);
            PrintArray("fp aes key", key, 16);
        } else {
            printf("fp aes does not exist\r\n");
        }
        if (FpResetKeyExist()) {
            GetFpResetKey(key);
            PrintArray("fp reset key", key, 16);
        } else {
            printf("fp reset does not exist\r\n");
        }
        for (index = 0; index < 10; index++) {
            GetFpEncryptedPassword(index, key);
            sprintf(tempStr, "encrypted password %d", index);
            PrintArray(tempStr, key, 32);
        }
        GetFpStateInfo(byte32);
        PrintArray("fp state info", byte32, 32);
    } else if (strcmp(argv[0], "clear_fp_info") == 0) {
        printf("clear fp info test\r\n");
        memset(key, 0, 32);
        SetFpCommAesKey(key);
        SetFpResetKey(key);
        for (index = 0; index < 10; index++) {
            SetFpEncryptedPassword(index, key);
        }
        uint8_t tempByte32[32] = {0};
        SetFpStateInfo(tempByte32);
        printf("clear done\r\n");
    } else if (strcmp(argv[0], "device_pub_key") == 0) {
        printf("get device pubkey test\r\n");
        uint8_t pubkey[65] = {0};
        GetDevicePublicKey(pubkey);
        PrintArray("device pubkey", pubkey, 65);
        printf("get device pubkey clear done\r\n");
    } else if (strcmp(argv[0], "sign_device_key") == 0) {
        printf("sign with device pubkey test\r\n");
        uint8_t message[32] = {0};
        uint8_t signature[64] = {0};
        SignMessageWithDeviceKey(message, signature);
        PrintArray("signature is using device key", signature, 64);
        printf("signature with device key done\r\n");
    } else {
        printf("keystore cmd err\r\n");
    }
}

#endif