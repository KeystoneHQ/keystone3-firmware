#include "define.h"
#include "keystore.h"
#include "string.h"
#include "stdio.h"
#include "user_utils.h"
#include "hash_and_salt.h"
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
#include "account_public_info.h"
#include "drv_rtc.h"
#include "se_interface.h"
#include "se_manager.h"
#include "account_manager.h"
#include "assert.h"
#include "secret_cache.h"
#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#else
#include "simulator_model.h"
#include "simulator_storage.h"
#include "simulator_mock_define.h"
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
}

/// @brief Save a new entropy.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] entropy Entropy to be saved.
/// @param[in] entropyLen
/// @param[in] password Password string.
/// @return err code.
int32_t SaveNewEntropy(uint8_t accountIndex, const uint8_t *entropy, uint8_t entropyLen, const char *password)
{
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
}


/// @brief Get entropy from SE by password.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] entropy Entropy.
/// @param[out] entropyLen
/// @param[in] password Password string.
/// @return err code.
int32_t GetAccountEntropy(uint8_t accountIndex, uint8_t *entropy, uint8_t *entropyLen, const char *password)
{
}


/// @brief Get seed from SE by password, if there is a passphrase before, set a passphrase seed.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] seed Seed.
/// @param[in] password Password string.
/// @return err code.
int32_t GetAccountSeed(uint8_t accountIndex, uint8_t *seed, const char *password)
{
}


/// @brief Get slip39 ems from SE by password.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] slip39Ems slip39Ems.
/// @param[in] password Password string.
/// @return err code.
int32_t GetAccountSlip39Ems(uint8_t accountIndex, uint8_t *slip39Ems, const char *password)
{
}


/// @brief Change password.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] newPassword New password.
/// @param[in] password Old password.
/// @return err code.
int32_t ChangePassword(uint8_t accountIndex, const char *newPassword, const char *password)
{
}


/// @brief Verify password.
/// @param[out] accountIndex If password verify success, account index would be set here. Can be NULL if not needed.
/// @param password Password string.
/// @return err code.
int32_t VerifyPassword(uint8_t *accountIndex, const char *password)
{
}


/// @brief Check if password repeat with existing others.
/// @param[in] password Password string.
/// @param[in] excludeIndex exclude account index, if do not need exclude any account, set excludeIndex to 255.
/// @return err code.
int32_t CheckPasswordExisted(const char *password, uint8_t excludeIndex)
{
}

/// @brief Set passphrase
/// @param[in] accountIndex
/// @param[in] passphrase
/// @param[in] password Password string.
/// @return err code.
int32_t SetPassphrase(uint8_t accountIndex, const char *passphrase, const char *password)
{
}

bool CheckPassphraseSame(uint8_t accountIndex, const char *passphrase)
{
}

/// @brief Get master fingerprint
/// @param[out] mfp
void GetMasterFingerPrint(uint8_t *mfp)
{
}


void ClearAccountPassphrase(uint8_t accountIndex)
{
}

/// @brief Get if the passphrase exist.
/// @param accountIndex
/// @return true-exist, false-not exist.
bool PassphraseExist(uint8_t accountIndex)
{
}

char* GetPassphrase(uint8_t accountIndex)
{
}

/// @brief Save account secret, including entropy/seed/reservedData.
/// @param[in] accountIndex Account index, 0~2.
/// @param[in] accountSecret Account secret data.
/// @param[in] password Password string.
/// @param[in] newAccount is new account.
/// @return err code.
static int32_t SaveAccountSecret(uint8_t accountIndex, const AccountSecret_t *accountSecret, const char *password, bool newAccount)
{
}


/// @brief Load account secret, including entropy/seed/reservedData.
/// @param[in] accountIndex Account index, 0~2.
/// @param[out] accountSecret Account secret data.
/// @param[in] password Password string.
/// @return err code.
static int32_t LoadAccountSecret(uint8_t accountIndex, AccountSecret_t *accountSecret, const char *password)
{
}

/// @brief Combine with the internal AES KEY of MCU.
/// @param[inout] aesKey
/// @return
static void CombineInnerAesKey(uint8_t *aesKey)
{
}


/// @brief Get seed generated by passphrase.
/// @param[in] accountIndex
/// @param[out] seed Seed.
/// @param[in] passphrase Passphrase string.
/// @param[in] password Password string.
/// @return err code.
static int32_t GetPassphraseSeed(uint8_t accountIndex, uint8_t *seed, const char *passphrase, const char *password)
{
}

#ifndef BUILD_PRODUCTION

/// @brief
/// @param argc Test arg count.
/// @param argv Test arg values.
void KeyStoreTest(int argc, char *argv[])
{
}

#endif