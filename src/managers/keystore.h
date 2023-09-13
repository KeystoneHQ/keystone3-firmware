/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Seeds are stored in ATECC608B and DS28S60.
 * Author: leon sun
 * Create: 2023-3-7
 ************************************************************************************************/


#ifndef _KEYSTORE_H
#define _KEYSTORE_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

#define ACCOUNT_INDEX_LOGOUT                    255

typedef enum {
    PASSCODE_TYPE_PIN,
    PASSCODE_TYPE_PASSWORD,
} PasscodeType;

typedef enum {
    MNEMONIC_TYPE_BIP39,
    MNEMONIC_TYPE_SLIP39,
} MnemonicType;


// // define the error code by module
// typedef enum {
//     ERR_KEYSTORE_608B_GENPUBERROR = 0x0100,
// } Keystone_ERROR_CODE;


typedef struct {
    uint8_t entropyLen;
    PasscodeType passcodeType       : 1;
    MnemonicType mnemonicType       : 1;
    uint8_t passphraseQuickAccess   : 1;
    uint8_t passphraseMark          : 1;    //true if Passphrase set, non-volatile storage.
    uint8_t slip39Id[2];                    //slip39 Idrandom identifier.
    uint8_t mfp[4];
    uint8_t slip39Ie[1];                    //slip39 Iteration exponent.
    uint8_t reserved2[5];                   //byte 9~13 reserved.
    uint8_t iconIndex;
    char walletName[17];
} AccountInfo_t;


int32_t KeystoreInit(void);
int32_t GenerateEntropy(uint8_t *entropy, uint8_t entropyLen, const char *password);
int32_t SaveNewEntropy(uint8_t accountIndex, const uint8_t *entropy, uint8_t entropyLen, const char *password);
int32_t SaveNewSlip39Entropy(uint8_t accountIndex, const uint8_t *ems, const uint8_t *entropy, uint8_t entropyLen, const char *password, uint16_t id, uint8_t ie);
int32_t GetAccountEntropy(uint8_t accountIndex, uint8_t *entropy, uint8_t *entropyLen, const char *password);
int32_t GetAccountSeed(uint8_t accountIndex, uint8_t *seed, const char *password);
int32_t GetAccountSlip39Ems(uint8_t accountIndex, uint8_t *slip39Ems, const char *password);
int32_t ChangePassword(uint8_t accountIndex, const char *newPassword, const char *password);
int32_t VerifyPassword(uint8_t *accountIndex, const char *password);
int32_t VerifyCurrentAccountPassword(const char *password);
int32_t VerifyPasswordAndLogin(uint8_t *accountIndex, const char *password);
void LogoutCurrentAccount(void);
int32_t CheckPasswordExisted(const char *password, uint8_t excludeIndex);
uint8_t GetCurrentAccountIndex(void);
void SetCurrentAccountIndex(void);
int32_t GetExistAccountNum(uint8_t *accountNum);
int32_t GetBlankAccountIndex(uint8_t *accountIndex);
int32_t DestroyAccount(uint8_t accountIndex);
int32_t SetPassphrase(uint8_t accountIndex, const char *passphrase, const char *password);
void ClearAccountPassphrase(uint8_t accountIndex);
void GetMasterFingerPrint(uint8_t *mfp);
PasscodeType GetPasscodeType(void);
MnemonicType GetMnemonicType(void);
uint16_t GetSlip39Id(void);
uint8_t GetSlip39Ie(void);
bool GetPassphraseQuickAccess(void);
bool GetPassphraseMark(void);
uint8_t GetWalletIconIndex(void);
char *GetWalletName(void);
void SetPasscodeType(PasscodeType type);
void SetMnemonicType(MnemonicType type);
void SetPassphraseQuickAccess(bool exist);
void SetPassphraseMark(bool enable);
void SetWalletIconIndex(uint8_t iconIndex);
void SetWalletName(const char *walletName);
uint8_t GetLoginPasswordErrorCount(void);
uint8_t GetCurrentPasswordErrorCount(void);
uint32_t GetLastLockDeviceTime(void);
void SetLastLockDeviceTime(uint32_t timeStamp);
uint32_t GetCurrentAccountEntropyLen(void);
bool PassphraseExist(uint8_t accountIndex);

int32_t GetAccountInfo(uint8_t accountIndex, AccountInfo_t *pInfo);
int32_t ErasePublicInfo(void);
int32_t ClearCurrentPasswordErrorCount(void);

#ifndef BUILD_PRODUCTION
void KeyStoreTest(int argc, char *argv[]);
#endif

#endif
