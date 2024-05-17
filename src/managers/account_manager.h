#ifndef _ACCOUNT_MANAGER_H_
#define _ACCOUNT_MANAGER_H_

#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"

#define WALLET_NAME_MAX_LEN                 16

typedef enum {
    PASSCODE_TYPE_PIN,
    PASSCODE_TYPE_PASSWORD,
} PasscodeType;

typedef enum {
    MNEMONIC_TYPE_BIP39,
    MNEMONIC_TYPE_SLIP39,
    MNEMONIC_TYPE_TON,
} MnemonicType;

// // define the error code by module
// typedef enum {
//     ERR_KEYSTORE_608B_GENPUBERROR = 0x0100,
// } Keystone_ERROR_CODE;

// IMPORTANT!
// DO NOT CHANGE ORDER OF THE MEMBERS IN THE STRUCTURE
typedef struct {
    uint8_t entropyLen;
    PasscodeType passcodeType       : 1;
    uint8_t isSlip39                : 1;    //true if slip39 wallet.
    uint8_t passphraseQuickAccess   : 1;
    uint8_t passphraseMark          : 1;    //true if Passphrase set, non-volatile storage.
    uint8_t isTon                   : 1;    //true if TON wallet.
    //still remain 3 bits for flags.
    //uint8_t flag1                  : 1;
    //uint8_t flag2                  : 1;
    //uint8_t flag3                  : 1;
    uint8_t slip39Id[2];                    //slip39 Idrandom identifier.
    uint8_t mfp[4];
    uint8_t slip39Ie[1];                    //slip39 Iteration exponent.
    uint8_t reserved2[5];                   //byte 9~13 reserved.
    uint8_t iconIndex;
    char walletName[WALLET_NAME_MAX_LEN + 1];
} AccountInfo_t;

int32_t AccountManagerInit(void);
bool AccountManagerIsNeedReset(void);

PasscodeType GetPasscodeType(void);
MnemonicType GetMnemonicType(void);
void SetPasscodeType(PasscodeType type);
void SetMnemonicType(MnemonicType type);

int32_t CreateNewAccount(uint8_t accountIndex, const uint8_t *entropy, uint8_t entropyLen, const char *password);
int32_t CreateNewTonAccount(uint8_t accountIndex, const char *mnemonic, const char *password);
int32_t CreateNewSlip39Account(uint8_t accountIndex, const uint8_t *ems, const uint8_t *entropy, uint8_t entropyLen, const char *password, uint16_t id, uint8_t ie);
int32_t ClearCurrentPasswordErrorCount(void);
int32_t VerifyCurrentAccountPassword(const char *password);
int32_t VerifyPasswordAndLogin(uint8_t *accountIndex, const char *password);
void LogoutCurrentAccount(void);
uint8_t GetCurrentAccountIndex(void);
void SetCurrentAccountIndex(void);
int32_t GetExistAccountNum(uint8_t *accountNum);
int32_t GetBlankAccountIndex(uint8_t *accountIndex);
int32_t DestroyAccount(uint8_t accountIndex);
int32_t SaveCurrentAccountInfo(void);
void SetCurrentAccountMfp(uint8_t* mfp);
void SetCurrentAccountEntropyLen(uint8_t entropyLen);
void SetPassphraseQuickAccess(bool exist);
bool GetPassphraseQuickAccess(void);
bool GetPassphraseMark(void);
uint8_t GetWalletIconIndex(void);
char *GetWalletName(void);
void SetPassphraseMark(bool enable);
void SetWalletIconIndex(uint8_t iconIndex);
void SetWalletName(const char *walletName);
uint8_t GetLoginPasswordErrorCount(void);
uint8_t GetCurrentPasswordErrorCount(void);
uint32_t GetLastLockDeviceTime(void);
void SetLastLockDeviceTime(uint32_t timeStamp);
uint32_t GetCurrentAccountEntropyLen(void);

uint8_t *GetCurrentAccountMfp(void);
int32_t GetAccountInfo(uint8_t accountIndex, AccountInfo_t *pInfo);
int32_t ErasePublicInfo(void);
int32_t ClearCurrentPasswordErrorCount(void);

uint16_t GetSlip39Id(void);
uint8_t GetSlip39Ie(void);

void AccountsDataCheck(void);

#endif