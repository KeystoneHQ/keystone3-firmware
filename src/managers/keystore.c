#include "define.h"
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
// #include "bip39.h"
// #include "slip39.h"
#include "user_memory.h"
#include "drv_otp.h"
#include "librust_c.h"
#include "drv_rtc.h"
#include "se_interface.h"
#include "se_manager.h"
#include "account_manager.h"
#include "librust_c.h"
#include "assert.h"
#include "secret_cache.h"
#include "drv_mpu.h"
#ifdef COMPILE_SIMULATOR
#include "simulator_model.h"
#include "simulator_storage.h"
#else
#endif
#define KEYSTORE_DEBUG          0

#if KEYSTORE_DEBUG == 1
#define KEYSTORE_PRINT_ARRAY(fmt, args...)       PrintArray(fmt, ##args)
#else
#define KEYSTORE_PRINT_ARRAY(fmt, args...)
#endif

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
        HashWithSalt(inputBuffer, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "generate entropy");

        SE_GetTRng(randomBuffer, ENTROPY_MAX_LEN);
        KEYSTORE_PRINT_ARRAY("trng", randomBuffer, ENTROPY_MAX_LEN);
        // set the initial value
        memcpy_s(outputBuffer, sizeof(outputBuffer), randomBuffer, ENTROPY_MAX_LEN);
        hkdf(inputBuffer, randomBuffer, outputBuffer, ITERATION_TIME);
        KEYSTORE_PRINT_ARRAY("outputBuffer", outputBuffer, ENTROPY_MAX_LEN);
        memcpy_s(inputBuffer, sizeof(inputBuffer), outputBuffer, ENTROPY_MAX_LEN);

        ret = SE_GetDS28S60Rng(randomBuffer, ENTROPY_MAX_LEN);
        CHECK_ERRCODE_BREAK("get ds28s60 trng", ret);
        KEYSTORE_PRINT_ARRAY("ds28s60 rng", randomBuffer, ENTROPY_MAX_LEN);
        hkdf(inputBuffer, randomBuffer, outputBuffer, ITERATION_TIME);
        KEYSTORE_PRINT_ARRAY("outputBuffer", outputBuffer, ENTROPY_MAX_LEN);
        memcpy_s(inputBuffer, sizeof(inputBuffer), outputBuffer, ENTROPY_MAX_LEN);

        ret = SE_GetAtecc608bRng(randomBuffer, ENTROPY_MAX_LEN);
        CHECK_ERRCODE_BREAK("get 608b trng", ret);
        KEYSTORE_PRINT_ARRAY("608b rng", randomBuffer, ENTROPY_MAX_LEN);
        hkdf(inputBuffer, randomBuffer, outputBuffer, ITERATION_TIME);

        KEYSTORE_PRINT_ARRAY("finalEntropy", outputBuffer, ENTROPY_MAX_LEN);
        memcpy_s(entropy, entropyLen, outputBuffer, entropyLen);
    } while (0);
    CLEAR_ARRAY(outputBuffer);
    CLEAR_ARRAY(inputBuffer);
    CLEAR_ARRAY(randomBuffer);
    return ret;
}

uint8_t GetCurrentAccountIndex(void)
{
    return 0;
}


/// @brief Save a new entropy.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] entropy Entropy to be saved.
/// @param[in] entropyLen
/// @param[in] password Password string.
/// @return err code.
int32_t SaveNewBip39Entropy(uint8_t accountIndex, const uint8_t *entropy, uint8_t entropyLen, const char *password)
{
    int32_t ret = SUCCESS_CODE;
    // AccountSecret_t accountSecret = {0};
    // char *mnemonic = NULL;
    // uint8_t passwordHash[32];

    // ASSERT(accountIndex <= 2);
    // do {
    //     ret = CheckPasswordExisted(password, 255);
    //     CHECK_ERRCODE_BREAK("check repeat password", ret);
    //     memcpy_s(accountSecret.entropy, sizeof(accountSecret.entropy), entropy, entropyLen);
    //     accountSecret.entropyLen = entropyLen;
    //     // bip39 entropy->mnemonic->seed
    //     ret = bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
    //     CHECK_ERRCODE_BREAK("bip39_mnemonic_from_bytes", ret);
    //     ret = bip39_mnemonic_to_seed(mnemonic, NULL, accountSecret.seed, SEED_LEN, NULL);
    //     CHECK_ERRCODE_BREAK("bip39_mnemonic_to_seed", ret);
    //     SRAM_FREE(mnemonic);

    //     ret = SaveAccountSecret(accountIndex, &accountSecret, password, true);
    //     CHECK_ERRCODE_BREAK("SaveAccountSecret", ret);
    //     HashWithSalt(passwordHash, (const uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "password hash");
    //     ret = SE_HmacEncryptWrite(passwordHash, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
    //     CHECK_ERRCODE_BREAK("write password hash", ret);

    // } while (0);

    // CLEAR_ARRAY(passwordHash);
    // CLEAR_OBJECT(accountSecret);
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
        memcpy_s(accountSecret.entropy, sizeof(accountSecret.entropy), entropy, entropyLen);
        accountSecret.entropyLen = entropyLen;

        // The length of ems is the same as the length of entropy
        memcpy_s(accountSecret.seed, sizeof(accountSecret.seed), entropy, entropyLen);
        memcpy_s(accountSecret.slip39EmsOrTonEntropyL32, sizeof(accountSecret.slip39EmsOrTonEntropyL32), ems, entropyLen);
        ret = SaveAccountSecret(accountIndex, &accountSecret, password, true);
        CHECK_ERRCODE_BREAK("SaveAccountSecret", ret);
        HashWithSalt(passwordHash, (const uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "password hash");
        ret = SE_HmacEncryptWrite(passwordHash, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("write password hash", ret);

    } while (0);

    CLEAR_ARRAY(passwordHash);
    CLEAR_OBJECT(accountSecret);
    return ret;
}

/// @brief Get entropy from SE by password.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] entropy Entropy, ton entropy should be 64 bytes.
/// @param[out] entropyLen
/// @param[in] password Password string.
/// @return err code.
int32_t GetAccountEntropy(uint8_t accountIndex, uint8_t *entropy, uint8_t *entropyLen, const char *password)
{
    int32_t ret;
    AccountSecret_t accountSecret;

    ASSERT(accountIndex <= 2);
    ret = LoadAccountSecret(accountIndex, &accountSecret, password);
    MnemonicType mnemonicType = GetMnemonicType();
    if (ret == SUCCESS_CODE) {
        if (mnemonicType == MNEMONIC_TYPE_TON) {
            memcpy_s(entropy, ENTROPY_MAX_LEN, accountSecret.entropy, ENTROPY_MAX_LEN);
            memcpy_s(entropy + 32, ENTROPY_MAX_LEN, accountSecret.slip39EmsOrTonEntropyL32, ENTROPY_MAX_LEN);
            *entropyLen = 64;
        } else {
            memcpy_s(entropy, ENTROPY_MAX_LEN, accountSecret.entropy, ENTROPY_MAX_LEN);
            *entropyLen = accountSecret.entropyLen;
        }
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
            // no passphrase
            seedLen = GetMnemonicType() == MNEMONIC_TYPE_SLIP39 ? accountSecret.entropyLen : sizeof(accountSecret.seed);
            memcpy_s(seed, seedLen, accountSecret.seed, seedLen);
        } else {
            // recalculate seed with passphrase
            ret = GetPassphraseSeed(accountIndex, tempSeed, g_passphraseInfo[accountIndex].passphrase, password);
            CHECK_ERRCODE_BREAK("GetPassphraseSeed", ret);
            memcpy_s(seed, SEED_LEN, tempSeed, SEED_LEN);
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
        memcpy_s(slip39Ems, SLIP39_EMS_LEN, accountSecret.slip39EmsOrTonEntropyL32, SLIP39_EMS_LEN);
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
        HashWithSalt(passwordHash, (const uint8_t *)newPassword, strnlen_s(newPassword, PASSWORD_MAX_LEN), "password hash");
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
#ifdef COMPILE_SIMULATOR
    return SimulatorVerifyPassword(accountIndex, password);
#endif

    for (i = 0; i < 3; i++) {
        ret = SE_HmacEncryptRead(passwordHashStore, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("read password hash", ret);
        HashWithSalt(passwordHashClac, (const uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "password hash");
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
        // password existed
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
    return 0;
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
        memset_s(mfp, 4, 0, 4);
    } else {
        if (PassphraseExist(accountIndex)) {
            memcpy_s(mfp, 4, g_passphraseInfo[accountIndex].mfp, 4);
        } else {
            memcpy_s(mfp, 4, GetCurrentAccountMfp(), 4);
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

    assert(g_passphraseInfo[accountIndex].passphraseExist == (strnlen_s(g_passphraseInfo[accountIndex].passphrase, PASSPHRASE_MAX_LEN) > 0));
    return (strnlen_s(g_passphraseInfo[accountIndex].passphrase, PASSPHRASE_MAX_LEN) > 0);
}

char *GetPassphrase(uint8_t accountIndex)
{
    if (accountIndex > 2) {
        return false;
    }

    return g_passphraseInfo[accountIndex].passphrase;
}

int32_t DestroyAccount(uint8_t accountIndex)
{
    return 0;
}

/// @brief Save account secret, including entropy/seed/reservedData.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] accountSecret Account secret data.
/// @param[in] password Password string.
/// @param[in] newAccount is new account.
/// @return err code.
static int32_t SaveAccountSecret(uint8_t accountIndex, const AccountSecret_t *accountSecret, const char *password, bool newAccount)
{
    return 0;
}

/// @brief Load account secret, including entropy/seed/reservedData.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] accountSecret Account secret data.
/// @param[in] password Password string.
/// @return err code.
static int32_t LoadAccountSecret(uint8_t accountIndex, AccountSecret_t *accountSecret, const char *password)
{
    return 0;
}

/// @brief Combine with the internal AES KEY of MCU.
/// @param[inout] aesKey
/// @return
static void CombineInnerAesKey(uint8_t *aesKey)
{
    uint8_t aesPiece[AES_KEY_LEN];
#ifndef COMPILE_SIMULATOR

    MpuSetOtpProtection(false);
    OTP_PowerOn();
    memcpy_s(aesPiece, sizeof(aesPiece), (uint8_t *)OTP_ADDR_AES_KEY, AES_KEY_LEN);
    if (CheckEntropy(aesPiece, AES_KEY_LEN) == false) {
        printf("need generate inner aes piece\r\n");
        TrngGet(aesPiece, AES_KEY_LEN);
        WriteOtpData(OTP_ADDR_AES_KEY, aesPiece, AES_KEY_LEN);
    }
    MpuSetOtpProtection(true);
#else
    TrngGet(aesPiece, AES_KEY_LEN);
#endif
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
    UNUSED(accountIndex);
    UNUSED(seed);
    UNUSED(passphrase);
    UNUSED(password);
    return SUCCESS_CODE;
}

#ifdef FORGEBOX
/// @brief Save a new ton mnemonic.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] entropy Entropy to be saved.
/// @param[in] entropyLen
/// @param[in] password Password string.
/// @return err code.
int32_t SaveNewTonMnemonic(uint8_t accountIndex, const char *mnemonic, const char *password)
{
    UNUSED(accountIndex);
    UNUSED(mnemonic);
    UNUSED(password);
    return SUCCESS_CODE;
}

/// @brief Generate a valid ton mnemonic from SE and mcu TRNG.
/// We do not consider ton mnemonic with password currently.
/// The password here is actually the user's password.
/// @param[out] entropy
/// @param[in] entropyLen
/// @param[in] password Password hash is part of the entropy sources.
/// @return err code.
int32_t GenerateTonMnemonic(char *mnemonic, const char *password)
{
    return 0;
}
#endif

int32_t GenerateTRNGRandomness(uint8_t *randomness, uint8_t len)
{
    return GenerateEntropy(randomness, len, "generate trng randomness");
}