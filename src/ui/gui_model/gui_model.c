#include <string.h>
#include "stdlib.h"
#include "gui.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_api.h"
#include "bip39_english.h"
#include "user_memory.h"
#include "log_print.h"
#include "secret_cache.h"
#include "background_task.h"
#include "bip39.h"
#include "slip39.h"
#include "gui_setting_widgets.h"
#include "account_public_info.h"
#include "user_utils.h"
#include "gui_chain.h"
#include "device_setting.h"
#include "user_memory.h"
#include "presetting.h"
#include "ui_display_task.h"
#include "motor_manager.h"
#include "gui_animating_qrcode.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "qrdecode_task.h"
#include "gui_views.h"
#include "assert.h"
#include "version.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "rust.h"
#include "user_msg.h"
#include "general_msg.h"
#include "cmsis_os.h"
#include "fingerprint_process.h"
#include "user_delay.h"
#include "user_fatfs.h"
#include "mhscpu_qspi.h"
#include "safe_mem_lib.h"
#include "usb_task.h"
#else
#include "simulator_mock_define.h"
#include "simulator_model.h"
#endif

#define SECTOR_SIZE                         4096
#define APP_ADDR                            (0x1001000 + 0x80000)   //108 1000
#define APP_CHECK_START_ADDR                (0x1400000)
#define APP_END_ADDR                        (0x2000000)

#define MODEL_WRITE_SE_HEAD                 do {                                \
        ret = CHECK_BATTERY_LOW_POWER();                                        \
        CHECK_ERRCODE_BREAK("save low power", ret);                             \
        ret = GetBlankAccountIndex(&newAccount);                                \
        CHECK_ERRCODE_BREAK("get blank account", ret);                          \
        ret = GetExistAccountNum(&accountCnt);                                  \
        CHECK_ERRCODE_BREAK("get exist account", ret);                          \
        printf("before accountCnt = %d\n", accountCnt);

#define MODEL_WRITE_SE_END                                                      \
        ret = VerifyPasswordAndLogin(&newAccount, SecretCacheGetNewPassword());    \
        CHECK_ERRCODE_BREAK("login error", ret);                                \
        GetExistAccountNum(&accountCnt);                                        \
        printf("after accountCnt = %d\n", accountCnt);                          \
    } while (0);                                                                \
    if (ret == SUCCESS_CODE) {                                                  \
        ClearSecretCache();                                                     \
        GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));  \
    } else {                                                                            \
        GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &ret, sizeof(ret));     \
    }

static int32_t ModelSaveWalletDesc(const void *inData, uint32_t inDataLen);
static int32_t ModelDelWallet(const void *inData, uint32_t inDataLen);
static int32_t ModelDelAllWallet(const void *inData, uint32_t inDataLen);
static int32_t ModelWritePassphrase(const void *inData, uint32_t inDataLen);
static int32_t ModelChangeAccountPass(const void *inData, uint32_t inDataLen);
static int32_t ModelVerifyAccountPass(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateEntropy(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateEntropyWithDiceRolls(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateTonMnemonic(const void *inData, uint32_t inDataLen);
static int32_t ModelBip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModelTonCalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModelWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModelTonWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModelBip39VerifyMnemonic(const void *inData, uint32_t inDataLen);
static int32_t ModelTonVerifyMnemonic(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateSlip39Entropy(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateSlip39EntropyWithDiceRolls(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModeGetAccount(const void *inData, uint32_t inDataLen);
static int32_t ModeGetWalletDesc(const void *inData, uint32_t inDataLen);
static int32_t ModeControlQrDecode(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39WriteEntropy(const void *inData, uint32_t inDataLen);
static int32_t ModelComparePubkey(MnemonicType mnemonicType, uint8_t *ems, uint8_t emsLen, uint16_t id, uint8_t ie, uint8_t *index);
static int32_t ModelBip39ForgetPass(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39ForgetPass(const void *inData, uint32_t inDataLen);
static int32_t ModelTonForgetPass(const void *inData, uint32_t inDataLen);
static int32_t ModelCalculateWebAuthCode(const void *inData, uint32_t inDataLen);
static int32_t ModelWriteLastLockDeviceTime(const void *inData, uint32_t inDataLen);
static int32_t ModelCopySdCardOta(const void *inData, uint32_t inDataLen);
static int32_t ModelURGenerateQRCode(const void *indata, uint32_t inDataLen, BackgroundAsyncRunnable_t getUR);
static int32_t ModelCalculateCheckSum(const void *indata, uint32_t inDataLen);
static int32_t ModelRsaGenerateKeyPair();
static int32_t ModelCalculateBinSha256(const void *indata, uint32_t inDataLen);
static int32_t ModelURUpdate(const void *inData, uint32_t inDataLen);
static int32_t ModelURClear(const void *inData, uint32_t inDataLen);
static int32_t ModelCheckTransaction(const void *inData, uint32_t inDataLen);
static int32_t ModelTransactionCheckResultClear(const void *inData, uint32_t inDataLen);
static int32_t ModelParseTransaction(const void *indata, uint32_t inDataLen, BackgroundAsyncRunnable_t parseTransactionFunc);
static int32_t ModelFormatMicroSd(const void *indata, uint32_t inDataLen);

static PasswordVerifyResult_t g_passwordVerifyResult;
static bool g_stopCalChecksum = false;

#ifdef COMPILE_SIMULATOR
int32_t AsyncExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen)
{
    func(inData, inDataLen);
    return SUCCESS_CODE;
}

int32_t AsyncExecuteRunnable(BackgroundAsyncFuncWithRunnable_t func, const void *inData, uint32_t inDataLen, BackgroundAsyncRunnable_t runnable)
{
    func(inData, inDataLen, runnable);
    return SUCCESS_CODE;
}
#endif

void GuiModelWriteSe(void)
{
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    AsyncExecute(ModelWriteEntropyAndSeed, NULL, 0);
}

void GuiModelTonWriteSe(void)
{
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    AsyncExecute(ModelTonWriteEntropyAndSeed, NULL, 0);
}

void GuiModelBip39CalWriteSe(Bip39Data_t bip39)
{
    AsyncExecute(ModelBip39CalWriteEntropyAndSeed, &bip39, sizeof(bip39));
}

void GuiModelTonCalWriteSe(TonData_t ton)
{
    AsyncExecute(ModelTonCalWriteEntropyAndSeed, &ton, sizeof(ton));
}

void GuiModelBip39RecoveryCheck(uint8_t wordsCnt)
{
    AsyncExecute(ModelBip39VerifyMnemonic, &wordsCnt, sizeof(wordsCnt));
}

void GuiModelTonRecoveryCheck()
{
    AsyncExecute(ModelTonVerifyMnemonic, NULL, 0);
}

void GuiModelBip39ForgetPassword(uint8_t wordsCnt)
{
    AsyncExecute(ModelBip39ForgetPass, &wordsCnt, sizeof(wordsCnt));
}

void GuiModelTonForgetPassword()
{
    AsyncExecute(ModelTonForgetPass, NULL, 0);
}

void GuiModelSlip39WriteSe(uint8_t wordsCnt)
{
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    AsyncExecute(ModelSlip39WriteEntropy, &wordsCnt, sizeof(wordsCnt));
}

void GuiModelSlip39CalWriteSe(Slip39Data_t slip39)
{
    AsyncExecute(ModelSlip39CalWriteEntropyAndSeed, &slip39, sizeof(slip39));
}

void GuiModelCalculateCheckSum(void)
{
    SetPageLockScreen(false);
    AsyncExecute(ModelCalculateCheckSum, NULL, 0);
}

void GuiModelCalculateBinSha256(void)
{
    SetPageLockScreen(false);
    AsyncExecute(ModelCalculateBinSha256, NULL, 0);
}

void GuiModelFormatMicroSd(void)
{
    SetPageLockScreen(false);
    AsyncExecute(ModelFormatMicroSd, NULL, 0);
}

void GuiModelStopCalculateCheckSum(void)
{
    SetPageLockScreen(true);
    g_stopCalChecksum = true;
}

void GuiModelCalculateWebAuthCode(void *webAuthData)
{
    AsyncExecuteWithPtr(ModelCalculateWebAuthCode, webAuthData);
}

void GuiModelSlip39ForgetPassword(Slip39Data_t slip39)
{
    AsyncExecute(ModelSlip39ForgetPass, &slip39, sizeof(slip39));
}

void GuiModelSettingSaveWalletDesc(WalletDesc_t *wallet)
{
    AsyncExecute(ModelSaveWalletDesc, wallet, sizeof(*wallet));
}

void GuiModelSettingDelWalletDesc(void)
{
    AsyncExecute(ModelDelWallet, NULL, 0);
}

void GuiModelLockedDeviceDelAllWalletDesc(void)
{
    AsyncExecute(ModelDelAllWallet, NULL, 0);
}

void GuiModelSettingWritePassphrase(void)
{
    AsyncExecute(ModelWritePassphrase, NULL, 0);
}

void GuiModelChangeAccountPassWord(void)
{
    AsyncExecute(ModelChangeAccountPass, NULL, 0);
}

void GuiModelVerifyAccountPassWord(uint16_t *param)
{
    AsyncExecute(ModelVerifyAccountPass, param, sizeof(*param));
}

void GuiModelBip39UpdateMnemonic(uint8_t wordCnt)
{
    uint32_t mnemonicNum = wordCnt;
    AsyncExecute(ModelGenerateEntropy, &mnemonicNum, sizeof(mnemonicNum));
}

void GuiModelBip39UpdateMnemonicWithDiceRolls(uint8_t wordCnt)
{
    uint32_t mnemonicNum = wordCnt;
    AsyncExecute(ModelGenerateEntropyWithDiceRolls, &mnemonicNum, sizeof(mnemonicNum));
}

void GuiModelTonUpdateMnemonic(void)
{
    AsyncExecute(ModelGenerateTonMnemonic, NULL, 0);
}

void GuiModelSlip39UpdateMnemonic(Slip39Data_t slip39)
{
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    AsyncExecute(ModelGenerateSlip39Entropy, &slip39, sizeof(slip39));
}

void GuiModelSlip39UpdateMnemonicWithDiceRolls(Slip39Data_t slip39)
{
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    AsyncExecute(ModelGenerateSlip39EntropyWithDiceRolls, &slip39, sizeof(slip39));
}

void GuiModeGetAccount(void)
{
    AsyncExecute(ModeGetAccount, NULL, 0);
}

void GuiModeGetWalletDesc(void)
{
    AsyncExecute(ModeGetWalletDesc, NULL, 0);
}

void GuiModeControlQrDecode(bool en)
{
    AsyncExecute(ModeControlQrDecode, &en, sizeof(en));
}

void GuiModelWriteLastLockDeviceTime(uint32_t time)
{
    AsyncExecute(ModelWriteLastLockDeviceTime, &time, sizeof(time));
}

void GuiModelCopySdCardOta(void)
{
    AsyncExecute(ModelCopySdCardOta, NULL, 0);
}

void GuiModelURGenerateQRCode(GenerateUR func)
{
    AsyncExecuteRunnable(ModelURGenerateQRCode, NULL, 0, (BackgroundAsyncRunnable_t)func);
}

void GuiModelURUpdate(void)
{
    AsyncExecute(ModelURUpdate, NULL, 0);
}

void GuiModelURClear(void)
{
    AsyncExecute(ModelURClear, NULL, 0);
}

void GuiModelCheckTransaction(ViewType viewType)
{
    AsyncExecute(ModelCheckTransaction, &viewType, sizeof(viewType));
}

void GuiModelRsaGenerateKeyPair(void)
{
    AsyncExecute(ModelRsaGenerateKeyPair, NULL, 0);
}

void GuiModelTransactionCheckResultClear(void)
{
    AsyncExecute(ModelTransactionCheckResultClear, NULL, 0);
}

void GuiModelParseTransaction(ReturnVoidPointerFunc func)
{
    AsyncExecuteRunnable(ModelParseTransaction, NULL, 0, (BackgroundAsyncRunnable_t)func);
}

// bip39 generate
static int32_t ModelGenerateEntropy(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t retData;
    char *mnemonic = NULL;
    uint8_t entropy[32];
    uint32_t mnemonicNum, entropyLen;
    mnemonicNum = *((uint32_t *)inData);
    entropyLen = (mnemonicNum == 24) ? 32 : 16;
    GenerateEntropy(entropy, entropyLen, SecretCacheGetNewPassword());
    SecretCacheSetEntropy(entropy, entropyLen);
    bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
    SecretCacheSetMnemonic(mnemonic);
    retData = SUCCESS_CODE;
    GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC, &retData, sizeof(retData));
    memset_s(mnemonic, strnlen_s(mnemonic, MNEMONIC_MAX_LEN), 0, strnlen_s(mnemonic, MNEMONIC_MAX_LEN));
    SRAM_FREE(mnemonic);
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

static int32_t ModelGenerateEntropyWithDiceRolls(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t retData;
    char *mnemonic = NULL;
    uint8_t entropy[32];
    uint8_t *hash;
    uint32_t mnemonicNum, entropyLen;
    mnemonicNum = *((uint32_t *)inData);
    entropyLen = (mnemonicNum == 24) ? 32 : 16;
    // GenerateEntropy(entropy, entropyLen, SecretCacheGetNewPassword());
    hash = SecretCacheGetDiceRollHash();
    memcpy_s(entropy, sizeof(entropy), hash, entropyLen);
    SecretCacheSetEntropy(entropy, entropyLen);
    bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
    SecretCacheSetMnemonic(mnemonic);
    retData = SUCCESS_CODE;
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC, &retData, sizeof(retData));
    memset_s(mnemonic, strnlen_s(mnemonic, MNEMONIC_MAX_LEN), 0, strnlen_s(mnemonic, MNEMONIC_MAX_LEN));
    SRAM_FREE(mnemonic);
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// Generate bip39 wallet writes
static int32_t ModelWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret;
    uint8_t *entropy, *entropyCheck;
    uint32_t entropyLen;
    uint8_t newAccount;
    uint8_t accountCnt;
    size_t entropyOutLen;

    entropy = SecretCacheGetEntropy(&entropyLen);
    entropyCheck = SRAM_MALLOC(entropyLen);
    ret = bip39_mnemonic_to_bytes(NULL, SecretCacheGetMnemonic(), entropyCheck, entropyLen, &entropyOutLen);
    if (memcmp(entropyCheck, entropy, entropyLen) != 0) {
        memset_s(entropyCheck, entropyLen, 0, entropyLen);
        SRAM_FREE(entropyCheck);
        SetLockScreen(enable);
        return 0;
    }
    memset_s(entropyCheck, entropyLen, 0, entropyLen);
    SRAM_FREE(entropyCheck);
    MODEL_WRITE_SE_HEAD
    ret = ModelComparePubkey(MNEMONIC_TYPE_BIP39, NULL, 0, 0, 0, NULL);
    CHECK_ERRCODE_BREAK("duplicated entropy", ret);
    ret = CreateNewAccount(newAccount, entropy, entropyLen, SecretCacheGetNewPassword());
    ClearAccountPassphrase(newAccount);
    CHECK_ERRCODE_BREAK("save entropy error", ret);
    MODEL_WRITE_SE_END
    SetLockScreen(enable);
    return 0;
}

// Import of mnemonic words for bip39
static int32_t ModelBip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret;
    uint8_t *entropy;
    size_t entropyInLen;
    size_t entropyOutLen;
    Bip39Data_t *bip39Data = (Bip39Data_t *)inData;
    uint8_t newAccount;
    uint8_t accountCnt;
    AccountInfo_t accountInfo;

    entropyInLen = bip39Data->wordCnt * 16 / 12;

    entropy = SRAM_MALLOC(entropyInLen);

    MODEL_WRITE_SE_HEAD
    ret = bip39_mnemonic_to_bytes(NULL, SecretCacheGetMnemonic(), entropy, entropyInLen, &entropyOutLen);
    CHECK_ERRCODE_BREAK("mnemonic error", ret);
    if (bip39Data->forget) {
        ret = ModelComparePubkey(MNEMONIC_TYPE_BIP39, NULL, 0, 0, 0, &newAccount);
        CHECK_ERRCODE_BREAK("mnemonic not match", !ret);
    } else {
        ret = ModelComparePubkey(MNEMONIC_TYPE_BIP39, NULL, 0, 0, 0, NULL);
        CHECK_ERRCODE_BREAK("mnemonic repeat", ret);
    }
    if (bip39Data->forget) {
        GetAccountInfo(newAccount, &accountInfo);
    }
    ret = CreateNewAccount(newAccount, entropy, (uint8_t)entropyOutLen, SecretCacheGetNewPassword());
    CHECK_ERRCODE_BREAK("save entropy error", ret);
    ClearAccountPassphrase(newAccount);
    ret = VerifyPasswordAndLogin(&newAccount, SecretCacheGetNewPassword());
    CHECK_ERRCODE_BREAK("login error", ret);
    if (bip39Data->forget) {
        SetWalletName(accountInfo.walletName);
        SetWalletIconIndex(accountInfo.iconIndex);
        LogoutCurrentAccount();
        CloseUsb();
    }
    UpdateFingerSignFlag(GetCurrentAccountIndex(), false);
    GetExistAccountNum(&accountCnt);
    printf("after accountCnt = %d\n", accountCnt);
}
while (0);
if (ret == SUCCESS_CODE)
{
    ClearSecretCache();
    GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));
} else
{
    GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &ret, sizeof(ret));
}
memset_s(entropy, entropyInLen, 0, entropyInLen);
SRAM_FREE(entropy);
SetLockScreen(enable);
return 0;
}

// Auxiliary word verification for bip39
static int32_t ModelBip39VerifyMnemonic(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret = SUCCESS_CODE;
    SimpleResponse_c_char *xPubResult;
    uint8_t seed[64];

    do {
        ret = bip39_mnemonic_to_seed(SecretCacheGetMnemonic(), NULL, seed, 64, NULL);
        xPubResult = get_extended_pubkey_by_seed(seed, 64, "M/49'/0'/0'");
        if (xPubResult->error_code != 0) {
            free_simple_response_c_char(xPubResult);
            break;
        }
        CLEAR_ARRAY(seed);
        char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_BTC);
        if (!strcmp(xpub, xPubResult->data)) {
            ret = SUCCESS_CODE;
        } else {
            ret = ERR_GENERAL_FAIL;
        }
        free_simple_response_c_char(xPubResult);
    } while (0);
    ClearSecretCache();
    if (ret != SUCCESS_CODE) {
        GuiApiEmitSignal(SIG_CREATE_SINGLE_PHRASE_WRITESE_FAIL, NULL, 0);
    } else {
        GuiApiEmitSignal(SIG_CREATE_SINGLE_PHRASE_WRITESE_PASS, NULL, 0);
    }
    SetLockScreen(enable);
    return 0;
}

// Auxiliary word verification for bip39
static int32_t ModelBip39ForgetPass(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret = SUCCESS_CODE;
    do {
        ret = CHECK_BATTERY_LOW_POWER();
        CHECK_ERRCODE_BREAK("save low power", ret);
        ret = ModelComparePubkey(MNEMONIC_TYPE_BIP39, NULL, 0, 0, 0, NULL);
        if (ret != SUCCESS_CODE) {
            GuiApiEmitSignal(SIG_FORGET_PASSWORD_SUCCESS, NULL, 0);
            SetLockScreen(enable);
            return ret;
        }
        ret = ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET;
    } while (0);
    GuiApiEmitSignal(SIG_FORGET_PASSWORD_FAIL, &ret, sizeof(ret));
    SetLockScreen(enable);
    return ret;
}

// ton generate
static int32_t ModelGenerateTonMnemonic(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t retData;
    char *mnemonic = SRAM_MALLOC(MNEMONIC_MAX_LEN);
    memset_s(mnemonic, MNEMONIC_MAX_LEN, 0, MNEMONIC_MAX_LEN);
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_TON_GENERATION_START, NULL, 0);
    GenerateTonMnemonic(mnemonic, SecretCacheGetNewPassword());
    SecretCacheSetMnemonic(mnemonic);
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_TON_GENERATION_END, NULL, 0);
    retData = SUCCESS_CODE;
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC, &retData, sizeof(retData));
    memset_s(mnemonic, strnlen_s(mnemonic, MNEMONIC_MAX_LEN), 0, strnlen_s(mnemonic, MNEMONIC_MAX_LEN));
    SRAM_FREE(mnemonic);
    SetLockScreen(enable);
    ClearLockScreenTime();
    return SUCCESS_CODE;
}

// ton generate
static int32_t ModelTonWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret;
    uint8_t newAccount;
    uint8_t accountCnt;
    char *mnemonic;
    mnemonic = SecretCacheGetMnemonic();
    MODEL_WRITE_SE_HEAD
    ret = ModelComparePubkey(MNEMONIC_TYPE_TON, NULL, 0, 0, 0, NULL);
    CHECK_ERRCODE_BREAK("duplicated entropy", ret);
    ret = CreateNewTonAccount(newAccount, mnemonic, SecretCacheGetNewPassword());
    ClearAccountPassphrase(newAccount);
    CHECK_ERRCODE_BREAK("save entropy error", ret);
    MODEL_WRITE_SE_END
    SetLockScreen(enable);
    ClearLockScreenTime();
    return 0;
}

// Import of mnemonic words for ton
static int32_t ModelTonCalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret;
    TonData_t *tonData = (TonData_t *)inData;
    uint8_t newAccount;
    uint8_t accountCnt;
    AccountInfo_t accountInfo;

    MODEL_WRITE_SE_HEAD
    bool isValid = ton_verify_mnemonic(SecretCacheGetMnemonic());
    if (!isValid) {
        printf("invalid ton mnemonic , line=%d\r\n", __LINE__);
        break;
    }
    if (tonData->forget) {
        ret = ModelComparePubkey(MNEMONIC_TYPE_TON, NULL, 0, 0, 0, &newAccount);
        CHECK_ERRCODE_BREAK("mnemonic not match", !ret);
    } else {
        ret = ModelComparePubkey(MNEMONIC_TYPE_TON, NULL, 0, 0, 0, NULL);
        CHECK_ERRCODE_BREAK("mnemonic repeat", ret);
    }
    if (tonData->forget) {
        GetAccountInfo(newAccount, &accountInfo);
    }
    ret = CreateNewTonAccount(newAccount, SecretCacheGetMnemonic(), SecretCacheGetNewPassword());
    CHECK_ERRCODE_BREAK("save entropy error", ret);
    ClearAccountPassphrase(newAccount);
    ret = VerifyPasswordAndLogin(&newAccount, SecretCacheGetNewPassword());
    CHECK_ERRCODE_BREAK("login error", ret);
    if (tonData->forget) {
        SetWalletName(accountInfo.walletName);
        SetWalletIconIndex(accountInfo.iconIndex);
        LogoutCurrentAccount();
        CloseUsb();
    }
    UpdateFingerSignFlag(GetCurrentAccountIndex(), false);
    GetExistAccountNum(&accountCnt);
    printf("after accountCnt = %d\n", accountCnt);
}
while (0);
if (ret == SUCCESS_CODE)
{
    ClearSecretCache();
    GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));
} else
{
    GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &ret, sizeof(ret));
}
SetLockScreen(enable);
ClearLockScreenTime();
return 0;
}

// Auxiliary word verification for ton
static int32_t ModelTonVerifyMnemonic(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret = SUCCESS_CODE;
    SimpleResponse_c_char *xPubResult;
    uint8_t seed[64];

    do {
        SimpleResponse_u8 *seedResponse = ton_mnemonic_to_seed(SecretCacheGetMnemonic());
        ret = seedResponse->error_code;
        if (seedResponse->error_code != 0) {
            break;
        }
        memcpy_s(seed, 64, seedResponse->data, 64);
        xPubResult = ton_seed_to_publickey(seed, 64);
        if (xPubResult->error_code != 0) {
            free_simple_response_c_char(xPubResult);
            break;
        }
        CLEAR_ARRAY(seed);
        char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_TON_NATIVE);
        if (!strcmp(xpub, xPubResult->data)) {
            ret = SUCCESS_CODE;
        } else {
            ret = ERR_GENERAL_FAIL;
        }
        free_simple_response_c_char(xPubResult);
    } while (0);
    ClearSecretCache();
    if (ret != SUCCESS_CODE) {
        GuiApiEmitSignal(SIG_CREATE_SINGLE_PHRASE_WRITESE_FAIL, NULL, 0);
    } else {
        GuiApiEmitSignal(SIG_CREATE_SINGLE_PHRASE_WRITESE_PASS, NULL, 0);
    }
    SetLockScreen(enable);
    ClearLockScreenTime();
    return 0;
}

// Auxiliary word verification for ton
static int32_t ModelTonForgetPass(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret = SUCCESS_CODE;
    do {
        ret = CHECK_BATTERY_LOW_POWER();
        CHECK_ERRCODE_BREAK("save low power", ret);
        ret = ModelComparePubkey(MNEMONIC_TYPE_TON, NULL, 0, 0, 0, NULL);
        if (ret != SUCCESS_CODE) {
            GuiApiEmitSignal(SIG_FORGET_PASSWORD_SUCCESS, NULL, 0);
            SetLockScreen(enable);
            return ret;
        }
        ret = ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET;
    } while (0);
    GuiApiEmitSignal(SIG_FORGET_PASSWORD_FAIL, &ret, sizeof(ret));
    SetLockScreen(enable);
    return ret;
}

static UREncodeResult *g_urResult = NULL;

static int32_t ModelURGenerateQRCode(const void *indata, uint32_t inDataLen, BackgroundAsyncRunnable_t getUR)
{
    GenerateUR func = (GenerateUR)getUR;
    g_urResult = func();
    if (g_urResult->error_code == 0) {
        // printf("%s\r\n", g_urResult->data);
        GuiApiEmitSignal(SIG_BACKGROUND_UR_GENERATE_SUCCESS, g_urResult->data, strnlen_s(g_urResult->data, SIMPLERESPONSE_C_CHAR_MAX_LEN) + 1);
    } else {
        printf("error message: %s\r\n", g_urResult->error_message);
        //TODO: deal with error
    }
    return SUCCESS_CODE;
}

static int32_t ModelURUpdate(const void *inData, uint32_t inDataLen)
{
    if (g_urResult == NULL) return SUCCESS_CODE;
    if (g_urResult->is_multi_part) {
        UREncodeMultiResult *result = get_next_part(g_urResult->encoder);
        if (result->error_code == 0) {
            // printf("%s\r\n", result->data);
            GuiApiEmitSignal(SIG_BACKGROUND_UR_UPDATE, result->data, strnlen_s(result->data, SIMPLERESPONSE_C_CHAR_MAX_LEN) + 1);
        } else {
            //TODO: deal with error
        }
        free_ur_encode_muilt_result(result);
    }
    return SUCCESS_CODE;
}

static int32_t ModelURClear(const void *inData, uint32_t inDataLen)
{
    if (g_urResult != NULL) {
        free_ur_encode_result(g_urResult);
        g_urResult = NULL;
    }
    return SUCCESS_CODE;
}

// Compare the generated extended public key
static int32_t ModelComparePubkey(MnemonicType mnemonicType, uint8_t *ems, uint8_t emsLen, uint16_t id, uint8_t ie, uint8_t *index)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    bool bip39 = mnemonicType == MNEMONIC_TYPE_BIP39;
    bool slip39 = mnemonicType == MNEMONIC_TYPE_SLIP39;
    bool ton = mnemonicType == MNEMONIC_TYPE_TON;
    uint8_t seed[64] = {0};
    int ret = 0;
    uint8_t existIndex = 0;
    if (ton) {
        VecFFI_u8 *entropyResult = ton_mnemonic_to_entropy(SecretCacheGetMnemonic());
        uint8_t checksum[32] = {0};
        CalculateTonChecksum(entropyResult->data, checksum);
        free_VecFFI_u8(entropyResult);
        char value[65] = {0};
        for (size_t i = 0; i < 32; i++) {
            snprintf_s(value, 65, "%s%02x", value, checksum[i]);
        }
        existIndex = SpecifiedXPubExist(value, ton);
        if (index != NULL) {
            *index = existIndex;
        }
        if (existIndex != 0xFF) {
            ret = ERR_KEYSTORE_MNEMONIC_REPEAT;
        } else {
            ret = SUCCESS_CODE;
        }
    } else {
        do {
            SimpleResponse_c_char *xPubResult;
            if (bip39) {
                ret = bip39_mnemonic_to_seed(SecretCacheGetMnemonic(), NULL, seed, 64, NULL);
                xPubResult = get_extended_pubkey_by_seed(seed, 64, "M/49'/0'/0'");
            }
            if (slip39) {
                ret = Slip39GetSeed(ems, seed, emsLen, "", ie, id);
                xPubResult = get_extended_pubkey_by_seed(seed, emsLen, "M/49'/0'/0'");
            }

            CHECK_CHAIN_BREAK(xPubResult);
            CLEAR_ARRAY(seed);
            existIndex = SpecifiedXPubExist(xPubResult->data, ton);
            if (index != NULL) {
                *index = existIndex;
            }
            if (existIndex != 0xFF) {
                ret = ERR_KEYSTORE_MNEMONIC_REPEAT;
            } else {
                ret = SUCCESS_CODE;
            }
            free_simple_response_c_char(xPubResult);
        } while (0);
    }
    SetLockScreen(enable);
    return ret;
}

static int32_t Slip39CreateGenerate(Slip39Data_t *slip39, bool isDiceRoll)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    uint8_t entropy[32], ems[32];
    uint32_t entropyLen;
    uint16_t id;
    uint8_t ie;
    entropyLen = (slip39->wordCnt == 20) ? 16 : 32;
    char *wordsList[slip39->memberCnt];
    if (isDiceRoll) {
        memcpy_s(entropy, sizeof(entropy), SecretCacheGetDiceRollHash(), entropyLen);
    } else {
        GenerateEntropy(entropy, entropyLen, SecretCacheGetNewPassword());
    }
    PrintArray("entropy", entropy, entropyLen);
    SecretCacheSetEntropy(entropy, entropyLen);
    GetSlip39MnemonicsWords(entropy, ems, slip39->wordCnt, slip39->memberCnt, slip39->threShold, wordsList, &id, &ie);
    SecretCacheSetEms(ems, entropyLen);
    SecretCacheSetIdentifier(id);
    SecretCacheSetIteration(ie);
    for (int i = 0; i < slip39->memberCnt; i++) {
        SecretCacheSetSlip39Mnemonic(wordsList[i], i);
    }

    for (int i = 0; i < slip39->memberCnt; i++) {
        memset_s(wordsList[i], strlen(wordsList[i]), 0, strlen(wordsList[i]));
        SRAM_FREE(wordsList[i]);
    }
    GuiApiEmitSignal(SIG_CREATE_SHARE_UPDATE_MNEMONIC, NULL, 0);
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// slip39 generate
static int32_t ModelGenerateSlip39Entropy(const void *inData, uint32_t inDataLen)
{
    return Slip39CreateGenerate((Slip39Data_t *)inData, false);
}

// slip39 generate
static int32_t ModelGenerateSlip39EntropyWithDiceRolls(const void *inData, uint32_t inDataLen)
{
    return Slip39CreateGenerate((Slip39Data_t *)inData, true);
}

// Generate slip39 wallet writes
static int32_t ModelSlip39WriteEntropy(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    uint8_t *entropy;
    uint8_t *ems;
    uint32_t entropyLen;
    uint8_t newAccount;
    uint8_t accountCnt;
    uint16_t id;
    uint8_t ie;
    uint8_t msCheck[32], emsCheck[32];
    uint8_t threShold;
    uint8_t wordCnt = *(uint8_t *)inData;
    int ret;

    ems = SecretCacheGetEms(&entropyLen);
    entropy = SecretCacheGetEntropy(&entropyLen);
    id = SecretCacheGetIdentifier();
    ie = SecretCacheGetIteration();

    MODEL_WRITE_SE_HEAD
    ret = Slip39CheckFirstWordList(SecretCacheGetSlip39Mnemonic(0), wordCnt, &threShold);
    char *words[threShold];
    for (int i = 0; i < threShold; i++) {
        words[i] = SecretCacheGetSlip39Mnemonic(i);
    }
    ret = Sli39GetMasterSecret(threShold, wordCnt, emsCheck, msCheck, words, &id, &ie);
    if ((ret != SUCCESS_CODE) || (memcmp(msCheck, entropy, entropyLen) != 0) || (memcmp(emsCheck, ems, entropyLen) != 0)) {
        ret = ERR_KEYSTORE_MNEMONIC_INVALID;
        break;
    }
    CLEAR_ARRAY(emsCheck);
    CLEAR_ARRAY(msCheck);
    ret = ModelComparePubkey(MNEMONIC_TYPE_SLIP39, ems, entropyLen, id, ie, NULL);
    CHECK_ERRCODE_BREAK("duplicated entropy", ret);
    ret = CreateNewSlip39Account(newAccount, ems, entropy, entropyLen, SecretCacheGetNewPassword(), SecretCacheGetIdentifier(), SecretCacheGetIteration());
    CHECK_ERRCODE_BREAK("save slip39 entropy error", ret);
    ClearAccountPassphrase(newAccount);
    MODEL_WRITE_SE_END

    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// Import of mnemonic words for slip39
static int32_t ModelSlip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    uint8_t *entropy;
    uint8_t entropyLen;
    int32_t ret;
    uint16_t id;
    uint8_t ie;
    uint8_t newAccount;
    uint8_t accountCnt;
    Slip39Data_t *slip39 = (Slip39Data_t *)inData;
    AccountInfo_t accountInfo;

    entropyLen = (slip39->wordCnt == 20) ? 16 : 32;
    entropy = SRAM_MALLOC(entropyLen);
    uint8_t ems[32] = {0};
    uint8_t emsBak[32] = {0};

    char *words[slip39->threShold];
    for (int i = 0; i < slip39->threShold; i++) {
        words[i] = SecretCacheGetSlip39Mnemonic(i);
    }

    MODEL_WRITE_SE_HEAD
    ret = Sli39GetMasterSecret(slip39->threShold, slip39->wordCnt, ems, entropy, words, &id, &ie);
    if (ret != SUCCESS_CODE) {
        printf("get master secret error\n");
        break;
    }
    memcpy_s(emsBak, sizeof(emsBak), ems, entropyLen);

    if (slip39->forget) {
        ret = ModelComparePubkey(MNEMONIC_TYPE_SLIP39, ems, entropyLen, id, ie, &newAccount);
        CHECK_ERRCODE_BREAK("mnemonic not match", !ret);
    } else {
        ret = ModelComparePubkey(MNEMONIC_TYPE_SLIP39, ems, entropyLen, id, ie, NULL);
        CHECK_ERRCODE_BREAK("mnemonic repeat", ret);
    }
    printf("before accountCnt = %d\n", accountCnt);
    if (slip39->forget) {
        GetAccountInfo(newAccount, &accountInfo);
    }
    ret = CreateNewSlip39Account(newAccount, emsBak, entropy, entropyLen, SecretCacheGetNewPassword(), id, ie);
    CHECK_ERRCODE_BREAK("save slip39 entropy error", ret);
    ClearAccountPassphrase(newAccount);
    ret = VerifyPasswordAndLogin(&newAccount, SecretCacheGetNewPassword());
    CHECK_ERRCODE_BREAK("login error", ret);
    if (slip39->forget) {
        SetWalletName(accountInfo.walletName);
        SetWalletIconIndex(accountInfo.iconIndex);
        LogoutCurrentAccount();
        CloseUsb();
    }
    UpdateFingerSignFlag(GetCurrentAccountIndex(), false);
    CLEAR_ARRAY(ems);
    CLEAR_ARRAY(emsBak);
    GetExistAccountNum(&accountCnt);
    printf("after accountCnt = %d\n", accountCnt);
}
while (0);
if (ret == SUCCESS_CODE)
{
    ClearSecretCache();
    GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));
} else
{
    GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &ret, sizeof(ret));
}
SRAM_FREE(entropy);
#endif
SetLockScreen(enable);
return SUCCESS_CODE;
}

static int32_t ModelSlip39ForgetPass(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret = SUCCESS_CODE;
#ifndef COMPILE_SIMULATOR
    uint8_t *entropy;
    uint8_t entropyLen;
    uint16_t id;
    uint8_t ie;
    Slip39Data_t *slip39 = (Slip39Data_t *)inData;

    entropyLen = (slip39->wordCnt == 20) ? 16 : 32;
    entropy = SRAM_MALLOC(entropyLen);
    uint8_t ems[32] = {0};

    char *words[slip39->threShold];
    for (int i = 0; i < slip39->threShold; i++) {
        words[i] = SecretCacheGetSlip39Mnemonic(i);
    }

    do {
        ret = CHECK_BATTERY_LOW_POWER();
        CHECK_ERRCODE_BREAK("save low power", ret);
        ret = Sli39GetMasterSecret(slip39->threShold, slip39->wordCnt, ems, entropy, words, &id, &ie);
        if (ret != SUCCESS_CODE) {
            printf("get master secret error\n");
            break;
        }
        ret = ModelComparePubkey(MNEMONIC_TYPE_SLIP39, ems, entropyLen, id, ie, NULL);
        if (ret != SUCCESS_CODE) {
            GuiApiEmitSignal(SIG_FORGET_PASSWORD_SUCCESS, NULL, 0);
            SetLockScreen(enable);
            return ret;
        }
        ret = ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET;
    } while (0);
    GuiApiEmitSignal(SIG_FORGET_PASSWORD_FAIL, &ret, sizeof(ret));

    SRAM_FREE(entropy);
#else
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));
#endif
    SetLockScreen(enable);
    return ret;
}

// save wallet desc
static int32_t ModelSaveWalletDesc(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    WalletDesc_t *wallet = (WalletDesc_t *)inData;
    SetWalletName(wallet->name);
    SetWalletIconIndex(wallet->iconIndex);

    GuiApiEmitSignal(SIG_SETTING_CHANGE_WALLET_DESC_PASS, NULL, 0);
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// del wallet
static int32_t ModelDelWallet(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    int32_t ret;
    uint8_t accountIndex = GetCurrentAccountIndex();
    UpdateFingerSignFlag(accountIndex, false);
    CloseUsb();
    ret = DestroyAccount(accountIndex);
    if (ret == SUCCESS_CODE) {
        // reset address index in receive page
        {
            void GuiResetCurrentUtxoAddressIndex(uint8_t index);
#ifndef BTC_ONLY
            void GuiResetCurrentEthAddressIndex(uint8_t index);
            void GuiResetCurrentStandardAddressIndex(uint8_t index);
            void GuiResetCurrentMultiAccountsCache(uint8_t index);
#endif
            GuiResetCurrentUtxoAddressIndex(accountIndex);
#ifndef BTC_ONLY
            GuiResetCurrentEthAddressIndex(accountIndex);
            GuiResetCurrentStandardAddressIndex(accountIndex);
            GuiResetCurrentMultiAccountsCache(accountIndex);
#endif
        }

        uint8_t accountNum;
        GetExistAccountNum(&accountNum);
        if (accountNum == 0) {
            FpWipeManageInfo();
            SetSetupStep(0);
            SaveDeviceSettings();
            g_reboot = true;
            GuiApiEmitSignal(SIG_SETTING_DEL_WALLET_PASS_SETUP, NULL, 0);
        } else {
            GuiApiEmitSignal(SIG_SETTING_DEL_WALLET_PASS, NULL, 0);
        }
    } else {
        GuiApiEmitSignal(SIG_SETTING_DEL_WALLET_FAIL, &ret, sizeof(ret));
    }
#else
    // GuiEmitSignal(SIG_SETTING_DEL_WALLET_PASS_SETUP, NULL, 0);
    GuiEmitSignal(SIG_SETTING_DEL_WALLET_PASS, NULL, 0);
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// del all wallet
static int32_t ModelDelAllWallet(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    WipeDevice();
    SystemReboot();
#else
    GuiEmitSignal(SIG_SETTING_DEL_WALLET_PASS, NULL, 0);
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// write passphrase
static int32_t ModelWritePassphrase(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    int32_t ret = 0;
    if (CheckPassphraseSame(GetCurrentAccountIndex(), SecretCacheGetPassphrase())) {
        GuiApiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_PASS, NULL, 0);
    } else {
        ret = SetPassphrase(GetCurrentAccountIndex(), SecretCacheGetPassphrase(), SecretCacheGetPassword());
        if (ret == SUCCESS_CODE) {
            GuiApiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_PASS, NULL, 0);
            ClearSecretCache();
        } else {
            GuiApiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_FAIL, NULL, 0);
        }
        ClearSecretCache();
    }
#else
    GuiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_PASS, NULL, 0);
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// reset wallet password
static int32_t ModelChangeAccountPass(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    int32_t ret;

    ret = VerifyCurrentAccountPassword(SecretCacheGetPassword());
    ret = ChangePassword(GetCurrentAccountIndex(), SecretCacheGetNewPassword(), SecretCacheGetPassword());
    UpdateFingerSignFlag(GetCurrentAccountIndex(), false);
    if (ret == SUCCESS_CODE) {
        GuiApiEmitSignal(SIG_SETTING_CHANGE_PASSWORD_PASS, NULL, 0);
    } else {
        GuiApiEmitSignal(SIG_SETTING_CHANGE_PASSWORD_FAIL, NULL, 0);
    }

    ClearSecretCache();
#else
    uint8_t *entropy;
    uint8_t entropyLen;
    int32_t ret;
    uint8_t *accountIndex = (uint8_t *)inData;

    // GuiApiEmitSignal(SIG_SETTING_CHANGE_PASSWORD_FAIL, &ret, sizeof(ret));
    GuiApiEmitSignal(SIG_SETTING_CHANGE_PASSWORD_PASS, &ret, sizeof(ret));
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// calculate auth code
static int32_t ModelCalculateWebAuthCode(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    uint8_t *key = SRAM_MALLOC(WEB_AUTH_RSA_KEY_LEN);
    GetWebAuthRsaKey(key);
    char *authCode = calculate_auth_code(inData, key, 512, &key[512], 512);
    SRAM_FREE(key);
    GuiApiEmitSignal(SIG_WEB_AUTH_CODE_SUCCESS, authCode, strlen(authCode));
#else
    uint8_t *entropy;
    uint8_t entropyLen;
    int32_t ret;
    uint8_t *accountIndex = (uint8_t *)inData;

    // GuiApiEmitSignal(SIG_SETTING_CHANGE_PASSWORD_FAIL, &ret, sizeof(ret));
    char *authCode = "12345Yyq";
    GuiEmitSignal(SIG_WEB_AUTH_CODE_SUCCESS, authCode, strlen(authCode));
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

static void ModelVerifyPassSuccess(uint16_t *param)
{
    int32_t ret = SUCCESS_CODE;
    uint8_t walletAmount;
    switch (*param) {
    case DEVICE_SETTING_ADD_WALLET:
        GetExistAccountNum(&walletAmount);
        if (walletAmount == 3) {
            GuiApiEmitSignal(SIG_SETTING_ADD_WALLET_AMOUNT_LIMIT, NULL, 0);
        } else {
            GuiEmitSignal(SIG_INIT_GET_ACCOUNT_NUMBER, &walletAmount, sizeof(walletAmount));
            GuiApiEmitSignal(SIG_VERIFY_PASSWORD_PASS, param, sizeof(*param));
        }
        break;
    case SIG_INIT_SD_CARD_OTA_COPY:
        GuiApiEmitSignal(SIG_VERIFY_PASSWORD_PASS, param, sizeof(*param));
        GuiApiEmitSignal(SIG_INIT_SD_CARD_OTA_COPY, param, sizeof(*param));
        ModelCopySdCardOta(NULL, 0);
        break;
    case SIG_SETTING_WRITE_PASSPHRASE:
        GuiApiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_VERIFY_PASS, param, sizeof(*param));
        SetPageLockScreen(false);
        if (SecretCacheGetPassphrase() == NULL) {
            SecretCacheSetPassphrase("");
        }
        ret = SetPassphrase(GetCurrentAccountIndex(), SecretCacheGetPassphrase(), SecretCacheGetPassword());
#ifdef BTC_ONLY
        if (strnlen_s(SecretCacheGetPassphrase(), PASSPHRASE_MAX_LEN) == 0) {
            AccountPublicInfoSwitch(GetCurrentAccountIndex(), SecretCacheGetPassword(), false);
        }
#endif
        SetPageLockScreen(true);
        if (ret == SUCCESS_CODE) {
            GuiApiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_PASS, NULL, 0);
            ClearSecretCache();
        } else {
            GuiApiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_FAIL, NULL, 0);
        }
        break;
    case SIG_LOCK_VIEW_SCREEN_ON_VERIFY_PASSPHRASE:
        GuiApiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_PASSPHRASE_PASS, param, sizeof(*param));
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD:
        GuiApiEmitSignal(SIG_SETUP_RSA_PRIVATE_KEY_RSA_VERIFY_PASSWORD_PASS, param, sizeof(*param));
        break;
    default:
        GuiApiEmitSignal(SIG_VERIFY_PASSWORD_PASS, param, sizeof(*param));
        break;
    }
}

static void ModelVerifyPassFailed(uint16_t *param)
{
    uint16_t signal = SIG_VERIFY_PASSWORD_FAIL;
    switch (*param) {
    case SIG_LOCK_VIEW_VERIFY_PIN:
    case SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS:
        g_passwordVerifyResult.errorCount = GetLoginPasswordErrorCount();
        printf("gui model get login error count %d \n", g_passwordVerifyResult.errorCount);
        assert(g_passwordVerifyResult.errorCount <= MAX_LOGIN_PASSWORD_ERROR_COUNT);
        if (g_passwordVerifyResult.errorCount == MAX_LOGIN_PASSWORD_ERROR_COUNT) {
            UnlimitedVibrate(SUPER_LONG);
        } else {
            UnlimitedVibrate(LONG);
        }
        break;
    case SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD:
        signal = SIG_SETUP_RSA_PRIVATE_KEY_RSA_VERIFY_PASSWORD_FAIL;
        g_passwordVerifyResult.errorCount = GetCurrentPasswordErrorCount();
        printf("gui model get current error count %d \n", g_passwordVerifyResult.errorCount);
        assert(g_passwordVerifyResult.errorCount <= MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX);
        if (g_passwordVerifyResult.errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
            UnlimitedVibrate(SUPER_LONG);
        } else {
            UnlimitedVibrate(LONG);
        }
        break;
    case SIG_INIT_SD_CARD_OTA_COPY:
        signal = SIG_FIRMWARE_VERIFY_PASSWORD_FAIL;
    default:
        g_passwordVerifyResult.errorCount = GetCurrentPasswordErrorCount();
        printf("gui model get current error count %d \n", g_passwordVerifyResult.errorCount);
        assert(g_passwordVerifyResult.errorCount <= MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX);
        if (g_passwordVerifyResult.errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
            UnlimitedVibrate(SUPER_LONG);
        } else {
            UnlimitedVibrate(LONG);
        }
        break;
    }
    g_passwordVerifyResult.signal = param;
    GuiApiEmitSignal(signal, (void *)&g_passwordVerifyResult, sizeof(g_passwordVerifyResult));
}

// verify wallet password
static int32_t ModelVerifyAccountPass(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    static bool firstVerify = true;
    SetLockScreen(false);
    uint8_t accountIndex;
    int32_t ret;
    uint16_t *param = (uint16_t *)inData;

    // Unlock screen
    if (SIG_LOCK_VIEW_VERIFY_PIN == *param || SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS == *param) {
        ret = VerifyPasswordAndLogin(&accountIndex, SecretCacheGetPassword());
        if (ret == ERR_KEYSTORE_EXTEND_PUBLIC_KEY_NOT_MATCH) {
            GuiApiEmitSignal(SIG_EXTENDED_PUBLIC_KEY_NOT_MATCH, NULL, 0);
            return ret;
        } else if (ret == SUCCESS_CODE) {
            ModeGetWalletDesc(NULL, 0);
        }
    } else {
        ret = VerifyCurrentAccountPassword(SecretCacheGetPassword());
    }

    if (SIG_LOCK_VIEW_VERIFY_PIN == *param && firstVerify && ModelGetPassphraseQuickAccess()) {
        *param = SIG_LOCK_VIEW_SCREEN_ON_VERIFY_PASSPHRASE;
        firstVerify = false;
    }

    // some scene would need clear secret after check
    if (*param != SIG_SETTING_CHANGE_PASSWORD &&
            *param != SIG_SETTING_WRITE_PASSPHRASE &&
            *param != SIG_LOCK_VIEW_SCREEN_ON_VERIFY_PASSPHRASE &&
            *param != SIG_FINGER_SET_SIGN_TRANSITIONS &&
            *param != SIG_FINGER_REGISTER_ADD_SUCCESS &&
            *param != SIG_SIGN_TRANSACTION_WITH_PASSWORD &&
            *param != SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD &&
            *param != SIG_MULTISIG_WALLET_IMPORT_VERIFY_PASSWORD &&
            *param != SIG_MULTISIG_WALLET_DELETE_VERIFY_PASSWORD &&
            !strnlen_s(SecretCacheGetPassphrase(), PASSPHRASE_MAX_LEN) &&
            !GuiCheckIfViewOpened(&g_createWalletView) &&
            !ModelGetPassphraseQuickAccess()) {
        ClearSecretCache();
    }
    SetLockScreen(enable);
    if (ret == SUCCESS_CODE) {
        ModelVerifyPassSuccess(param);
    } else {
        ModelVerifyPassFailed(param);
    }
    return SUCCESS_CODE;
}

// get wallet amount
static int32_t ModeGetAccount(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    static uint8_t walletAmount;
    int32_t ret;

    ret = GetExistAccountNum(&walletAmount);
    if (ret != SUCCESS_CODE) {
        walletAmount = 0xFF;
    }
    GuiApiEmitSignal(SIG_INIT_GET_ACCOUNT_NUMBER, &walletAmount, sizeof(walletAmount));
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// get wallet desc
static int32_t ModeGetWalletDesc(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    static WalletDesc_t wallet;
    if (GetCurrentAccountIndex() > 2) {
        SetLockScreen(enable);
        return SUCCESS_CODE;
    }
    wallet.iconIndex = GetWalletIconIndex();
    strcpy_s(wallet.name, WALLET_NAME_MAX_LEN + 1, GetWalletName());
    GuiApiEmitSignal(SIG_INIT_GET_CURRENT_WALLET_DESC, &wallet, sizeof(wallet));
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// stop/start qr decode
static int32_t ModeControlQrDecode(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    bool *en = (bool *)inData;
#ifndef COMPILE_SIMULATOR
    UserDelay(100);
    if (en) {
        PubValueMsg(QRDECODE_MSG_START, 0);
    } else {
        PubValueMsg(QRDECODE_MSG_STOP, 0);
    }
#else
    read_qrcode();
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

static int32_t ModelWriteLastLockDeviceTime(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);

    uint32_t time = *(uint32_t *)inData;
    SetLastLockDeviceTime(time);

    SetLockScreen(enable);
    return SUCCESS_CODE;
}

static int32_t ModelCopySdCardOta(const void *inData, uint32_t inDataLen)
{
#ifndef COMPILE_SIMULATOR
    static uint8_t walletAmount;
    SetPageLockScreen(false);
    int32_t ret = FatfsFileCopy("0:/keystone3.bin", "1:/pillar.bin");
    if (ret == SUCCESS_CODE) {
        GetExistAccountNum(&walletAmount);
        if (walletAmount == 0) {
            SetSetupStep(4);
            SaveDeviceSettings();
        }
        NVIC_SystemReset();
    } else {
        SetPageLockScreen(true);
        GuiApiEmitSignal(SIG_INIT_SD_CARD_OTA_COPY_FAIL, NULL, 0);
    }
#else
    GuiApiEmitSignal(SIG_INIT_SD_CARD_OTA_COPY_FAIL, NULL, 0);
#endif
    return SUCCESS_CODE;
}

static PtrT_TransactionCheckResult g_checkResult = NULL;

static int32_t ModelCheckTransaction(const void *inData, uint32_t inDataLen)
{
    GuiApiEmitSignal(SIG_SHOW_TRANSACTION_LOADING, NULL, 0);
    ViewType viewType = *((ViewType *)inData);
    g_checkResult = CheckUrResult(viewType);
    if (g_checkResult != NULL && g_checkResult->error_code == 0) {
        GuiApiEmitSignal(SIG_TRANSACTION_CHECK_PASS, NULL, 0);
    } else {
        printf("transaction check fail, error code: %d, error msg: %s\r\n", g_checkResult->error_code, g_checkResult->error_message);
        GuiApiEmitSignal(SIG_HIDE_TRANSACTION_LOADING, NULL, 0);
        GuiApiEmitSignal(SIG_TRANSACTION_CHECK_FAIL, g_checkResult, sizeof(g_checkResult));
    }

    return SUCCESS_CODE;
}

int32_t RsaGenerateKeyPair(bool needEmitSignal)
{
    printf("RsaGenerate RsaGenerate RsaGenerate");
    bool lockState = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    if (needEmitSignal) {
        GuiApiEmitSignal(SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD_START, NULL, 0);
    }
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    int32_t ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    ASSERT(ret == 0);
    SimpleResponse_u8 *secret = generate_arweave_secret(seed, len);
    ASSERT(secret != NULL && secret->error_code == 0);
    FlashWriteRsaPrimes(secret->data);
    free_simple_response_u8(secret);
    GuiApiEmitSignal(SIG_SETUP_RSA_PRIVATE_KEY_GENERATE_ADDRESS, NULL, 0);
    AccountPublicInfoSwitch(GetCurrentAccountIndex(), SecretCacheGetPassword(), true);
    RecalculateManageWalletState();
    ClearLockScreenTime();
    SetLockScreen(lockState);
    if (needEmitSignal) {
        GuiApiEmitSignal(SIG_SETUP_RSA_PRIVATE_KEY_WITH_PASSWORD_PASS, NULL, 0);
        GuiApiEmitSignal(SIG_SETUP_RSA_PRIVATE_KEY_HIDE_LOADING, NULL, 0);
    }
    return SUCCESS_CODE;
}

static int32_t ModelRsaGenerateKeyPair()
{
    return RsaGenerateKeyPair(true);
}

static int32_t ModelTransactionCheckResultClear(const void *inData, uint32_t inDataLen)
{
    if (g_checkResult != NULL) {
        free_TransactionCheckResult(g_checkResult);
        g_checkResult = NULL;
    }
    return SUCCESS_CODE;
}

static int32_t ModelParseTransaction(const void *indata, uint32_t inDataLen, BackgroundAsyncRunnable_t parseTransactionFunc)
{
    ReturnVoidPointerFunc func = (ReturnVoidPointerFunc)parseTransactionFunc;
    // There is no need to release here, the parsing results will be released when exiting the details page.
    TransactionParseResult_DisplayTx *parsedResult = (TransactionParseResult_DisplayTx *)func();
    if (parsedResult != NULL && parsedResult->error_code == 0 && parsedResult->data != NULL) {
        GuiApiEmitSignal(SIG_TRANSACTION_PARSE_SUCCESS, parsedResult, sizeof(parsedResult));
    } else {
        GuiApiEmitSignal(SIG_TRANSACTION_PARSE_FAIL, parsedResult, sizeof(parsedResult));
    }
    GuiApiEmitSignal(SIG_HIDE_TRANSACTION_LOADING, NULL, 0);
    return SUCCESS_CODE;
}

static uint32_t BinarySearchLastNonFFSector(void)
{
    uint8_t *buffer = SRAM_MALLOC(SECTOR_SIZE);
    uint32_t startIndex = (APP_CHECK_START_ADDR - APP_ADDR) / SECTOR_SIZE;
    uint32_t endIndex = (APP_END_ADDR - APP_ADDR) / SECTOR_SIZE;

    uint8_t percent = 1;
    GuiApiEmitSignal(SIG_SETTING_CHECKSUM_PERCENT, &percent, sizeof(percent));

    for (int i = startIndex + 1; i < endIndex; i++) {
        if (g_stopCalChecksum == true) {
            SRAM_FREE(buffer);
            return SUCCESS_CODE;
        }
        memcpy_s(buffer, SECTOR_SIZE, (uint32_t *)(APP_ADDR + i * SECTOR_SIZE), SECTOR_SIZE);
        if ((i - startIndex) % 200 == 0) {
            percent++;
            GuiApiEmitSignal(SIG_SETTING_CHECKSUM_PERCENT, &percent, sizeof(percent));
        }
        if (CheckAllFF(&buffer[2], SECTOR_SIZE - 2) && ((buffer[0] * 256 + buffer[1]) < 4096)) {
            SRAM_FREE(buffer);
            return i;
        }
    }
    SRAM_FREE(buffer);
    return -1;
}

static int32_t ModelCalculateCheckSum(const void *indata, uint32_t inDataLen)
{
#ifndef COMPILE_SIMULATOR
    g_stopCalChecksum = false;
    uint8_t buffer[4096] = {0};
    uint8_t hash[32] = {0};
    int num = BinarySearchLastNonFFSector();
    ASSERT(num >= 0);
    if (g_stopCalChecksum == true) {
        return SUCCESS_CODE;
    }
    struct sha256_ctx ctx;
    sha256_init(&ctx);
    uint8_t percent = 0;
    for (int i = 0; i <= num; i++) {
        if (g_stopCalChecksum == true) {
            return SUCCESS_CODE;
        }
        memset_s(buffer, SECTOR_SIZE, 0, SECTOR_SIZE);
        memcpy_s(buffer, sizeof(buffer), (uint32_t *)(APP_ADDR + i * SECTOR_SIZE), SECTOR_SIZE);
        sha256_update(&ctx, buffer, SECTOR_SIZE);
        if (percent != i * 100 / num) {
            percent = i * 100 / num;
            if (percent != 100 && percent >= 10) {
                GuiApiEmitSignal(SIG_SETTING_CHECKSUM_PERCENT, &percent, sizeof(percent));
            }
        }
    }
    sha256_done(&ctx, (struct sha256 *)hash);
    memset_s(buffer, SECTOR_SIZE, 0, SECTOR_SIZE);
    percent = 100;
    SetPageLockScreen(true);
    SecretCacheSetChecksum(hash);
    GuiApiEmitSignal(SIG_SETTING_CHECKSUM_PERCENT, &percent, sizeof(percent));
#else
    uint8_t percent = 100;
    char *hash = "131b3a1e9314ba076f8e459a1c4c6713eeb38862f3eb6f9371360aa234cdde1f";
    SecretCacheSetChecksum(hash);
    GuiEmitSignal(SIG_SETTING_CHECKSUM_PERCENT, &percent, sizeof(percent));
#endif
    return SUCCESS_CODE;
}

#define FATFS_SHA256_BUFFER_SIZE              4096
static int32_t ModelCalculateBinSha256(const void *indata, uint32_t inDataLen)
{
    uint8_t percent;
#ifndef COMPILE_SIMULATOR
    g_stopCalChecksum = false;
    FIL fp;
    uint8_t *data = NULL;
    uint32_t fileSize, actualSize, copyOffset, totalSize = 0;
    uint8_t oldPercent = 0;
    FRESULT res;
    struct sha256_ctx ctx;
    sha256_init(&ctx);
    unsigned char hash[32];
    do {
        res = f_open(&fp, SD_CARD_OTA_BIN_PATH, FA_OPEN_EXISTING | FA_READ);
        if (res) {
            return res;
        }
        fileSize = f_size(&fp);
        data = SRAM_MALLOC(FATFS_SHA256_BUFFER_SIZE);
        for (copyOffset = 0; copyOffset <= fileSize; copyOffset += FATFS_SHA256_BUFFER_SIZE) {
            if (!SdCardInsert() || (g_stopCalChecksum == true)) {
                res = ERR_GENERAL_FAIL;
                break;
            }

            res = f_read(&fp, data, FATFS_SHA256_BUFFER_SIZE, &actualSize);
            if (res) {
                FatfsError(res);
                break;
            }
            sha256_update(&ctx, data, actualSize);
            totalSize += actualSize;

            uint8_t percent = totalSize * 100 / fileSize;
            if (oldPercent != percent) {
                printf("==========copy %d%%==========\n", percent);
                oldPercent = percent;
                if (percent != 100 && percent >= 2) {
                    GuiApiEmitSignal(SIG_SETTING_SHA256_PERCENT, &percent, sizeof(percent));
                }
            }
        }
    } while (0);

    if (res == FR_OK) {
        sha256_done(&ctx, (struct sha256 *)hash);
        for (int i = 0; i < sizeof(hash); i++) {
            printf("%02x", hash[i]);
        }
        SecretCacheSetChecksum(hash);
        percent = 100;
        GuiApiEmitSignal(SIG_SETTING_SHA256_PERCENT, &percent, sizeof(percent));
    } else {
        GuiApiEmitSignal(SIG_SETTING_SHA256_PERCENT_ERROR, NULL, 0);
    }
    SRAM_FREE(data);
    f_close(&fp);
    SetPageLockScreen(true);
#else
    percent = 100;
    char *hash = "131b3a1e9314ba076f8e459a1c4c6713eeb38862f3eb6f9371360aa234cdde1f";
    SecretCacheSetChecksum(hash);
    GuiEmitSignal(SIG_SETTING_SHA256_PERCENT, &percent, sizeof(percent));
#endif
    return SUCCESS_CODE;
}

bool ModelGetPassphraseQuickAccess(void)
{
#ifdef COMPILE_SIMULATOR
    return false;
#else
    if (PassphraseExist(GetCurrentAccountIndex()) == false && GetPassphraseQuickAccess() == true && GetPassphraseMark() == true) {
        return true;
    } else {
        return false;
    }
#endif
}

static int32_t ModelFormatMicroSd(const void *indata, uint32_t inDataLen)
{
    int ret = FormatSdFatfs();
    if (ret != SUCCESS_CODE) {
        GuiApiEmitSignal(SIG_SETTING_MICRO_CARD_FORMAT_FAILED, NULL, 0);
    } else {
        GuiApiEmitSignal(SIG_SETTING_MICRO_CARD_FORMAT_SUCCESS, NULL, 0);
    }
    SetPageLockScreen(true);

    return SUCCESS_CODE;
}
