#include "account_manager.h"
#include "keystore.h"
#include "se_manager.h"
#include "err_code.h"
#include "se_interface.h"
#include "user_utils.h"
#include "assert.h"
#include "hash_and_salt.h"
#include "secret_cache.h"
#include "log_print.h"
#include "user_memory.h"
#include "librust_c.h"
#ifdef COMPILE_SIMULATOR
#include "simulator_storage.h"
#else
#include "drv_otp.h"
#include "drv_mpu.h"
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
