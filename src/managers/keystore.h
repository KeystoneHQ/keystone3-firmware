#ifndef _KEYSTORE_H
#define _KEYSTORE_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "secret_cache.h"
#include "account_manager.h"

#define ACCOUNT_INDEX_LOGOUT                    255
#define AES_BLOCK_SIZE                          16

#define KEY_PIECE_LEN                           32

#define AES_KEY_LEN                             32
#define AUTH_KEY_LEN                            32

#define AES_IV_LEN                              32              //Use first 16 bytes for AES key, last 16 bytes reserved for future features.
#define ENTROPY_MAX_LEN                         32
#define TON_ENTROPY_LEN                         64
#define SEED_LEN                                64
#define SLIP39_EMS_LEN                          32
#define SE_DATA_RESERVED_LEN                    32
#define HMAC_LEN                                32
#define ACCOUNT_TOTAL_LEN                       (AES_IV_LEN + ENTROPY_MAX_LEN + SEED_LEN + SLIP39_EMS_LEN + SE_DATA_RESERVED_LEN + HMAC_LEN)
#define PARAM_LEN                               32

#define ITERATION_TIME                          700

typedef struct {
    uint8_t entropy[ENTROPY_MAX_LEN];
    uint8_t seed[SEED_LEN];
    uint8_t slip39EmsOrTonEntropyL32[SLIP39_EMS_LEN];
    uint8_t reservedData[SE_DATA_RESERVED_LEN];
    uint8_t entropyLen;
} AccountSecret_t;

typedef struct {
    char passphrase[PASSPHRASE_MAX_LEN + 1];
    bool passphraseExist;
    uint8_t mfp[4];
} PassphraseInfo_t;

int32_t GenerateEntropy(uint8_t *entropy, uint8_t entropyLen, const char *password);
int32_t GenerateTonMnemonic(char* mnemonic, const char *password);
int32_t SaveNewBip39Entropy(uint8_t accountIndex, const uint8_t *entropy, uint8_t entropyLen, const char *password);
int32_t SaveNewTonMnemonic(uint8_t accountIndex, const char *mnemonic, const char *password);
int32_t SaveNewSlip39Entropy(uint8_t accountIndex, const uint8_t *ems, const uint8_t *entropy, uint8_t entropyLen, const char *password, uint16_t id, uint8_t ie);
int32_t GetAccountEntropy(uint8_t accountIndex, uint8_t *entropy, uint8_t *entropyLen, const char *password);
int32_t GetAccountSeed(uint8_t accountIndex, uint8_t *seed, const char *password);
int32_t GetAccountSlip39Ems(uint8_t accountIndex, uint8_t *slip39Ems, const char *password);
int32_t ChangePassword(uint8_t accountIndex, const char *newPassword, const char *password);
int32_t VerifyPassword(uint8_t *accountIndex, const char *password);

bool CheckPassphraseSame(uint8_t accountIndex, const char *passphrase);
char* GetPassphrase(uint8_t accountIndex);

int32_t CheckPasswordExisted(const char *password, uint8_t excludeIndex);

int32_t SetPassphrase(uint8_t accountIndex, const char *passphrase, const char *password);
void ClearAccountPassphrase(uint8_t accountIndex);
void GetMasterFingerPrint(uint8_t *mfp);

bool PassphraseExist(uint8_t accountIndex);

#ifndef BUILD_PRODUCTION
void KeyStoreTest(int argc, char *argv[]);
#endif

#endif
