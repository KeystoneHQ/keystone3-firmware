#include "account_manager.h"
#include "keystore.h"
#include "se_manager.h"
#include "err_code.h"
#include "se_interface.h"
#include "user_utils.h"
#include "account_public_info.h"
#include "assert.h"
#include "hash_and_salt.h"
#include "secret_cache.h"
#include "log_print.h"
#include "user_memory.h"
#ifdef COMPILE_SIMULATOR
#include "simulator_storage.h"
#include "simulator_mock_define.h"
#else
#include "safe_str_lib.h"
#endif

typedef struct {
    uint8_t loginPasswordErrorCount;
    uint8_t currentPasswordErrorCount;
    uint8_t reserved1[2];
    uint32_t lastLockDeviceTime;
    uint8_t reserved2[24];               //byte 1~31 reserved.
} PublicInfo_t;

static uint8_t g_currentAccountIndex = ACCOUNT_INDEX_LOGOUT;
static uint8_t g_lastAccountIndex = ACCOUNT_INDEX_LOGOUT;
static AccountInfo_t g_currentAccountInfo = {0};
static PublicInfo_t g_publicInfo = {0};

/// @brief Get current account info from SE, and copy info to g_currentAccountInfo.
/// @return err code.
static int32_t ReadCurrentAccountInfo(void)
{
    uint8_t accountIndex, param[32];
    AccountInfo_t *pAccountInfo = (AccountInfo_t *)param;
    int32_t ret;

    accountIndex = GetCurrentAccountIndex();
    ASSERT(accountIndex <= 2);
    ret = SE_HmacEncryptRead(param, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PARAM);
    if (ret == SUCCESS_CODE) {
        memcpy_s(&g_currentAccountInfo, sizeof(AccountInfo_t), pAccountInfo, sizeof(AccountInfo_t));
    }
    return ret;
}

/// @brief Keystore init, secret self test.
/// @return err code.
int32_t AccountManagerInit(void)
{
    int32_t ret;
    ASSERT(sizeof(AccountInfo_t) == 32);
    ASSERT(sizeof(PublicInfo_t) == 32);
    ret = SE_HmacEncryptRead((uint8_t *)&g_publicInfo, PAGE_PUBLIC_INFO);
    return ret;
}

bool AccountManagerIsNeedReset(void)
{
    if (g_publicInfo.loginPasswordErrorCount < 10) {
        return false;
    }
    return true;
}

/// @brief Get passcode type of the current account.
/// @return Passcode type.
PasscodeType GetPasscodeType(void)
{
    return g_currentAccountInfo.passcodeType;
}

uint8_t *GetCurrentAccountMfp()
{
    return g_currentAccountInfo.mfp;
}

/// @brief Get mnemonic type of the current account.
/// @return Mnemonic type.
MnemonicType GetMnemonicType(void)
{
    if (g_currentAccountInfo.isSlip39) {
        return MNEMONIC_TYPE_SLIP39;
    }
    if (g_currentAccountInfo.isTon) {
        return MNEMONIC_TYPE_TON;
    }
    return MNEMONIC_TYPE_BIP39;
}

/// @brief Set passcode type for the current account.
/// @param[in] type Passcode type.
void SetPasscodeType(PasscodeType type)
{
    g_currentAccountInfo.passcodeType = type;
    SaveCurrentAccountInfo();
}
/// @brief Set mnemonic type for the current account.
/// @param[in] type Mnemonic type.
void SetMnemonicType(MnemonicType type)
{
    g_currentAccountInfo.isSlip39 = 0;
    g_currentAccountInfo.isTon = 0;
    switch (type) {
    case MNEMONIC_TYPE_SLIP39:
        g_currentAccountInfo.isSlip39 = 1;
        break;
    case MNEMONIC_TYPE_TON:
        g_currentAccountInfo.isTon = 1;
        break;
    default:
        break;
    }
    SaveCurrentAccountInfo();
}

int32_t CreateNewAccount(uint8_t accountIndex, const uint8_t *entropy, uint8_t entropyLen, const char *password)
{
    ASSERT(accountIndex <= 2);
    DestroyAccount(accountIndex);
    CLEAR_OBJECT(g_currentAccountInfo);
    g_currentAccountIndex = accountIndex;
    SetWalletName(SecretCacheGetWalletName());
    SetWalletIconIndex(SecretCacheGetWalletIconIndex());

    int32_t ret = SaveNewBip39Entropy(accountIndex, entropy, entropyLen, password);
    CHECK_ERRCODE_RETURN_INT(ret);

    ret = SaveCurrentAccountInfo();
    CHECK_ERRCODE_RETURN_INT(ret);
    ret = AccountPublicInfoSwitch(g_currentAccountIndex, password, true);
    CHECK_ERRCODE_RETURN_INT(ret);
    return ret;
}

int32_t CreateNewTonAccount(uint8_t accountIndex, const char *mnemonic, const char *password)
{
    ASSERT(accountIndex <= 2);
    DestroyAccount(accountIndex);
    CLEAR_OBJECT(g_currentAccountInfo);
    g_currentAccountIndex = accountIndex;
    g_currentAccountInfo.isTon = true;
    SetWalletName(SecretCacheGetWalletName());
    SetWalletIconIndex(SecretCacheGetWalletIconIndex());

    int32_t ret = SaveNewTonMnemonic(accountIndex, mnemonic, password);
    CHECK_ERRCODE_RETURN_INT(ret);

    ret = SaveCurrentAccountInfo();
    CHECK_ERRCODE_RETURN_INT(ret);
    ret = AccountPublicInfoSwitch(g_currentAccountIndex, password, true);
    CHECK_ERRCODE_RETURN_INT(ret);
    return ret;
}

int32_t CreateNewSlip39Account(uint8_t accountIndex, const uint8_t *ems, const uint8_t *entropy, uint8_t entropyLen, const char *password, uint16_t id, uint8_t ie)
{
    ASSERT(accountIndex <= 2);
    DestroyAccount(accountIndex);
    CLEAR_OBJECT(g_currentAccountInfo);
    g_currentAccountIndex = accountIndex;
    g_currentAccountInfo.isSlip39 = true;
    SetWalletName(SecretCacheGetWalletName());
    SetWalletIconIndex(SecretCacheGetWalletIconIndex());

    int32_t ret = SaveNewSlip39Entropy(accountIndex, ems, entropy, entropyLen, password, id, ie);
    CHECK_ERRCODE_RETURN_INT(ret);
    memcpy_s(g_currentAccountInfo.slip39Id, sizeof(g_currentAccountInfo.slip39Id), &id, sizeof(id));
    memcpy_s(g_currentAccountInfo.slip39Ie, sizeof(g_currentAccountInfo.slip39Ie), &ie, sizeof(ie));
    ret = SaveCurrentAccountInfo();
    CHECK_ERRCODE_RETURN_INT(ret);
    ret = AccountPublicInfoSwitch(g_currentAccountIndex, password, true);
    CHECK_ERRCODE_RETURN_INT(ret);
    return ret;
}

/// @brief Verify password for the account that was login. PasswordErrorCount++ if err.
/// @param[in] password Password string.
/// @return err code.
int32_t VerifyCurrentAccountPassword(const char *password)
{
    uint8_t accountIndex, passwordHashClac[32], passwordHashStore[32];
    int32_t ret;

    do {
        accountIndex = GetCurrentAccountIndex();
        if (accountIndex > 2) {
            ret = ERR_KEYSTORE_NOT_LOGIN;
            break;
        }
#ifdef COMPILE_SIMULATOR
        ret = SimulatorVerifyCurrentPassword(accountIndex, password);
#else
        ret = SE_HmacEncryptRead(passwordHashStore, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("read password hash", ret);
        HashWithSalt(passwordHashClac, (const uint8_t *)password, strlen(password), "password hash");
        ret = memcmp(passwordHashStore, passwordHashClac, 32);
#endif
        if (ret == SUCCESS_CODE) {
            g_publicInfo.currentPasswordErrorCount = 0;
        } else {
            g_publicInfo.currentPasswordErrorCount++;
            printf("password error count=%d\r\n", g_publicInfo.currentPasswordErrorCount);
            ret = ERR_KEYSTORE_PASSWORD_ERR;
        }
        SE_HmacEncryptWrite((uint8_t *)&g_publicInfo, PAGE_PUBLIC_INFO);
    } while (0);

    CLEAR_ARRAY(passwordHashStore);
    CLEAR_ARRAY(passwordHashClac);
    return ret;
}

int32_t ClearCurrentPasswordErrorCount(void)
{
    printf("clear current password error count\r\n");
    g_publicInfo.loginPasswordErrorCount = 0;
    g_publicInfo.currentPasswordErrorCount = 0;
    SE_HmacEncryptWrite((uint8_t *)&g_publicInfo, PAGE_PUBLIC_INFO);
    return SUCCESS_CODE;
}

/// @brief Verify password, if password verify success, set current account id. PasswordErrorCount++ if err.
/// @param[out] accountIndex If password verify success, account index would be set here. Can be NULL if not needed.
/// @param password Password string.
/// @return err code.
int32_t VerifyPasswordAndLogin(uint8_t *accountIndex, const char *password)
{
    int32_t ret;
    uint8_t tempIndex;

    ret = VerifyPassword(&tempIndex, password);
    if (ret == SUCCESS_CODE) {
        g_currentAccountIndex = tempIndex;
        g_lastAccountIndex = tempIndex;
        if (accountIndex) {
            *accountIndex = tempIndex;
        }
        ret = ReadCurrentAccountInfo();
        g_publicInfo.loginPasswordErrorCount = 0;
        g_publicInfo.currentPasswordErrorCount = 0;
        if (PassphraseExist(g_currentAccountIndex)) {
            //passphrase exist.
            printf("passphrase exist\r\n");
            TempAccountPublicInfo(g_currentAccountIndex, password, false);
        } else {
            printf("passphrase not exist, info switch\r\n");
            ret = AccountPublicInfoSwitch(g_currentAccountIndex, password, false);
        }
    } else {
        g_publicInfo.loginPasswordErrorCount++;
    }
    SE_HmacEncryptWrite((uint8_t *)&g_publicInfo, PAGE_PUBLIC_INFO);

    return ret;
}

/// @brief Get current account index.
/// @return current account index.return 255 if not currently authenticated.
uint8_t GetCurrentAccountIndex(void)
{
    return g_currentAccountIndex;
}

/// @brief Set last account index.
void SetCurrentAccountIndex(void)
{
    g_currentAccountIndex = g_lastAccountIndex;
}

void SetCurrentAccountMfp(uint8_t* mfp)
{
    memcpy_s(g_currentAccountInfo.mfp, sizeof(g_currentAccountInfo.mfp), mfp, 4);
}

void SetCurrentAccountEntropyLen(uint8_t entropyLen)
{
    g_currentAccountInfo.entropyLen = entropyLen;
}

/// @brief Get exist account number.
/// @param[out] accountNum 1~3
/// @return err code.
int32_t GetExistAccountNum(uint8_t *accountNum)
{
    int32_t ret;
    uint8_t data[32], count = 0;

#ifdef COMPILE_SIMULATOR
    *accountNum = SimulatorGetAccountNum();
    return SUCCESS_CODE;
#endif

    for (uint8_t i = 0; i < 3; i++) {
        ret = SE_HmacEncryptRead(data, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV);
        CHECK_ERRCODE_BREAK("read iv", ret);
        if (CheckEntropy(data, 32)) {
            count++;
        }
    }
    CLEAR_ARRAY(data);
    if (ret == SUCCESS_CODE) {
        *accountNum = count;
    }

    return ret;
}

/// @brief Logout current account.
void LogoutCurrentAccount(void)
{
    g_currentAccountIndex = ACCOUNT_INDEX_LOGOUT;
}

/// @brief Get the specific account info.
/// @param[in] accountIndex
/// @param[out] pInfo
/// @return err code.
int32_t GetAccountInfo(uint8_t accountIndex, AccountInfo_t *pInfo)
{
    int32_t ret;
    ASSERT(accountIndex <= 2);
    ret = SE_HmacEncryptRead((uint8_t *)pInfo, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PARAM);
    return ret;
}

/// @brief Erase public info in SE.
/// @return err code.
int32_t ErasePublicInfo(void)
{
    CLEAR_OBJECT(g_publicInfo);
    return SE_HmacEncryptWrite((uint8_t *)&g_publicInfo, PAGE_PUBLIC_INFO);
}

/// @brief Get slip39 idrandom identifier of the current account.
/// @return idrandom identifier.
uint16_t GetSlip39Id(void)
{
    uint16_t id;
    memcpy(&id, g_currentAccountInfo.slip39Id, 2);
    return id;
}

/// @brief Get slip39 iteration exponent of the current account.
/// @return iteration exponent.
uint8_t GetSlip39Ie(void)
{
    uint8_t ie;
    memcpy(&ie, g_currentAccountInfo.slip39Ie, 1);
    return ie;
}

/// @brief Get a boolean representing whether passphrase exists for the current account.
/// @return passphrase exists.
bool GetPassphraseQuickAccess(void)
{
    return g_currentAccountInfo.passphraseQuickAccess != 0;
}

/// @brief Get a boolean representing whether passphrase mark enable for the current account.
/// @return passphrase exists.
bool GetPassphraseMark(void)
{
    return g_currentAccountInfo.passphraseMark != 0;
}

/// @brief Get wallet icon index of the current account.
/// @return wallet icon index.
uint8_t GetWalletIconIndex(void)
{
    return g_currentAccountInfo.iconIndex;
}

/// @brief Get wallet name of the current account.
/// @return wallet name string.
char *GetWalletName(void)
{
    return g_currentAccountInfo.walletName;
}

/// @brief Set passphrase quick access enable for the current account.
/// @param[in] enable Quick access enable.
void SetPassphraseQuickAccess(bool enable)
{
    g_currentAccountInfo.passphraseQuickAccess = enable ? 1 : 0;
    // should only show quick access after restart.
    if (PassphraseExist(GetCurrentAccountIndex()) == true) {
        SetPassphraseMark(true);
    }
    SaveCurrentAccountInfo();
}

/// @brief Set passphrase mark enable for the current account.
/// @param[in] enable Mark enable.
void SetPassphraseMark(bool enable)
{
    g_currentAccountInfo.passphraseMark = enable ? 1 : 0;
    SaveCurrentAccountInfo();
}

/// @brief Set wallet icon index for the current account.
/// @param[in] iconIndex
void SetWalletIconIndex(uint8_t iconIndex)
{
    g_currentAccountInfo.iconIndex = iconIndex;
    SaveCurrentAccountInfo();
}

/// @brief Set wallet name for the current account.
/// @param[in] walletName
void SetWalletName(const char *walletName)
{
    memset_s(g_currentAccountInfo.walletName, sizeof(g_currentAccountInfo.walletName), 0, sizeof(g_currentAccountInfo.walletName));
    strcpy_s(g_currentAccountInfo.walletName, WALLET_NAME_MAX_LEN + 1, walletName);
    SaveCurrentAccountInfo();
}

/// @brief Get password err count for login.
/// @return password err count.
uint8_t GetLoginPasswordErrorCount(void)
{
    return g_publicInfo.loginPasswordErrorCount;
}

/// @brief Get password err count for current account verify.
/// @return password err count.
uint8_t GetCurrentPasswordErrorCount(void)
{
    return g_publicInfo.currentPasswordErrorCount;
}

/// @brief
/// @param
/// @return
uint32_t GetLastLockDeviceTime(void)
{
    return g_publicInfo.lastLockDeviceTime;
}

/// @brief
/// @param timeStamp
void SetLastLockDeviceTime(uint32_t timeStamp)
{
    g_publicInfo.lastLockDeviceTime = timeStamp;
    SE_HmacEncryptWrite((uint8_t *)&g_publicInfo, PAGE_PUBLIC_INFO);
}

/// @brief Get the current account entropy length.
/// @return Entropy length, also represents the seed length when SLIP39 used.
uint32_t GetCurrentAccountEntropyLen(void)
{
    return g_currentAccountInfo.entropyLen;
}

/// @brief Save g_currentAccountInfo to SE.
/// @return err code.
int32_t SaveCurrentAccountInfo(void)
{
    uint8_t accountIndex, param[32];
    AccountInfo_t *pAccountInfo = (AccountInfo_t *)param;
    int32_t ret;

    accountIndex = GetCurrentAccountIndex();
    ASSERT(accountIndex <= 2);
    memcpy(pAccountInfo, &g_currentAccountInfo, sizeof(AccountInfo_t));
    ret = SE_HmacEncryptWrite(param, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PARAM);
    return ret;
}

/// @brief Get a blank account index.
/// @param[out] accountIndex If there is a blank account, it would be set here.
/// @return err code.
int32_t GetBlankAccountIndex(uint8_t *accountIndex)
{
    int32_t ret;
    uint8_t data[32] = {0};

    for (uint8_t i = 0; i < 3; i++) {
        CLEAR_ARRAY(data);
#ifndef COMPILE_SIMULATOR
        ret = SE_HmacEncryptRead(data, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV);
#else
        ret = SE_HmacEncryptRead(data, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_ENTROPY_OR_TON_ENTROPY_H32);
#endif
        CHECK_ERRCODE_BREAK("read iv", ret);
        if (CheckEntropy(data, 32) == false) {
            *accountIndex = i;
            break;
        }
        if (i == 2) {
            *accountIndex = 255;
        }
    }
    CLEAR_ARRAY(data);

    return ret;
}

/// @brief Destroy the specified account.
/// @param[in] accountIndex Account index, 0~2.
/// @return err code.
int32_t DestroyAccount(uint8_t accountIndex)
{
    int32_t ret;
    uint8_t data[32] = {0};

    ASSERT(accountIndex <= 2);
    for (uint8_t i = 0; i < PAGE_NUM_PER_ACCOUNT; i++) {
        printf("erase index=%d\n", i);
        ret = SE_HmacEncryptWrite(data, accountIndex * PAGE_NUM_PER_ACCOUNT + i);
        CHECK_ERRCODE_BREAK("ds28s60 write", ret);
    }
    DeleteAccountPublicInfo(accountIndex);
    ClearAccountPassphrase(accountIndex);
    SetWalletDataHash(accountIndex, data);
    LogoutCurrentAccount();

    CLEAR_OBJECT(g_currentAccountInfo);
    CLEAR_ARRAY(data);

    return ret;
}

void AccountsDataCheck(void)
{
    int32_t ret;
    uint8_t data[32], accountIndex, validCount, i;

    for (accountIndex = 0; accountIndex < 3; accountIndex++) {
        validCount = 0;
        ret = SE_HmacEncryptRead(data, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV);
        CHECK_ERRCODE_BREAK("read iv", ret);
        if (CheckEntropy(data, 32)) {
            validCount++;
        }
        ret = SE_HmacEncryptRead(data, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("read pwd hash", ret);
        if (CheckEntropy(data, 32)) {
            validCount++;
        }
        if (validCount == 1) {
            printf("illegal data:%d\n", accountIndex);
            memset_s(data, sizeof(data), 0, sizeof(data));
            for (i = 0; i < PAGE_NUM_PER_ACCOUNT; i++) {
                printf("erase index=%d\n", i);
                ret = SE_HmacEncryptWrite(data, accountIndex * PAGE_NUM_PER_ACCOUNT + i);
                CHECK_ERRCODE_BREAK("ds28s60 write", ret);
            }
        }
    }
}
