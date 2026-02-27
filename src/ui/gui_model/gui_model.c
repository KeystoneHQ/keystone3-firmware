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
#include "fetch_sensitive_data_task.h"
#include "gui_setting_widgets.h"
#include "user_utils.h"
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
#include "user_delay.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "rust.h"
#include "user_msg.h"
#include "general_msg.h"
#include "cmsis_os.h"
#include "fingerprint_process.h"
#include "user_fatfs.h"
#include "mhscpu_qspi.h"
#include "safe_mem_lib.h"
#include "usb_task.h"
#include "drv_mpu.h"
#else
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
static int32_t ModelBip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModelWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModelBip39VerifyMnemonic(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateSlip39Entropy(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateSlip39EntropyWithDiceRolls(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModeGetAccount(const void *inData, uint32_t inDataLen);
static int32_t ModeGetWalletDesc(const void *inData, uint32_t inDataLen);
static int32_t ModeControlQrDecode(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39WriteEntropy(const void *inData, uint32_t inDataLen);
static int32_t ModelComparePubkey(MnemonicType mnemonicType, uint8_t *ems, uint8_t emsLen, uint16_t id, bool eb, uint8_t ie, uint8_t *index);
static int32_t ModelBip39ForgetPass(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39ForgetPass(const void *inData, uint32_t inDataLen);
static int32_t ModelCalculateWebAuthCode(const void *inData, uint32_t inDataLen);
static int32_t ModelWriteLastLockDeviceTime(const void *inData, uint32_t inDataLen);
static int32_t ModelCopySdCardOta(const void *inData, uint32_t inDataLen);
static int32_t ModelURGenerateQRCode(const void *indata, uint32_t inDataLen, BackgroundAsyncRunnable_t getUR);
static int32_t ModelCalculateCheckSum(const void *indata, uint32_t inDataLen);
static int32_t ModelCalculateBinSha256(const void *indata, uint32_t inDataLen);
static int32_t ModelURUpdate(const void *inData, uint32_t inDataLen);
static int32_t ModelURClear(const void *inData, uint32_t inDataLen);
static int32_t ModelCheckTransaction(const void *inData, uint32_t inDataLen);
static int32_t ModelTransactionCheckResultClear(const void *inData, uint32_t inDataLen);
static int32_t ModelParseTransaction(const void *indata, uint32_t inDataLen, BackgroundAsyncRunnable_t parseTransactionFunc);
static int32_t ModelFormatMicroSd(const void *indata, uint32_t inDataLen);
static int32_t ModelParseTransactionRawData(const void *inData, uint32_t inDataLen);
static int32_t ModelTransactionParseRawDataDelay(const void *inData, uint32_t inDataLen);

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

void GuiModelParseTransactionRawData(void)
{
    AsyncExecute(ModelParseTransactionRawData, NULL, 0);
}

void GuiModelTransactionParseRawDataDelay(void)
{
    AsyncExecute(ModelTransactionParseRawDataDelay, NULL, 0);
}

void GuiModelBip39CalWriteSe(Bip39Data_t bip39)
{
    AsyncExecute(ModelBip39CalWriteEntropyAndSeed, &bip39, sizeof(bip39));
}


void GuiModelBip39RecoveryCheck(uint8_t wordsCnt)
{
    AsyncExecute(ModelBip39VerifyMnemonic, &wordsCnt, sizeof(wordsCnt));
}


void GuiModelBip39ForgetPassword(uint8_t wordsCnt)
{
    AsyncExecute(ModelBip39ForgetPass, &wordsCnt, sizeof(wordsCnt));
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
    return SUCCESS_CODE;
}

static int32_t ModelGenerateEntropyWithDiceRolls(const void *inData, uint32_t inDataLen)
{
    return SUCCESS_CODE;
}

static int32_t ModelParseTransactionRawData(const void *inData, uint32_t inDataLen)
{
    UserDelay(100);
    GuiApiEmitSignal(SIG_SHOW_TRANSACTION_LOADING_DELAY, NULL, 0);
}

static int32_t ModelTransactionParseRawDataDelay(const void *inData, uint32_t inDataLen)
{
    GuiApiEmitSignal(SIG_HIDE_TRANSACTION_PARSE_LOADING_DELAY, NULL, 0);
}

// Generate bip39 wallet writes
static int32_t ModelWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    return 0;
}

// Import of mnemonic words for bip39
static int32_t ModelBip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    return 0;
}

// Auxiliary word verification for bip39
static int32_t ModelBip39VerifyMnemonic(const void *inData, uint32_t inDataLen)
{
    return 0;
}

// Auxiliary word verification for bip39
static int32_t ModelBip39ForgetPass(const void *inData, uint32_t inDataLen)
{
    return 0;
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

static bool ShouldUseCyclicPart(void)
{
    if (g_urResult == NULL) return false;
    if (strnlen_s(g_urResult->data, SIMPLERESPONSE_C_CHAR_MAX_LEN) < 6) return false;
    if (strncmp(g_urResult->data, "ur:xmr", 6) == 0 || strncmp(g_urResult->data, "UR:XMR", 6) == 0) {
        return true;
    }
    return false;
}

static int32_t ModelURUpdate(const void *inData, uint32_t inDataLen)
{
    if (g_urResult == NULL) return SUCCESS_CODE;
    if (g_urResult->is_multi_part) {
        UREncodeMultiResult *result = NULL;
        if (ShouldUseCyclicPart()) {
            result = get_next_cyclic_part(g_urResult->encoder);
        } else {
            result = get_next_part(g_urResult->encoder);
        }
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
static int32_t ModelComparePubkey(MnemonicType mnemonicType, uint8_t *ems, uint8_t emsLen, uint16_t id, bool eb, uint8_t ie, uint8_t *index)
{
    int ret = 0;
    return ret;
}

static int32_t Slip39CreateGenerate(Slip39Data_t *slip39, bool isDiceRoll)
{
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
    return SUCCESS_CODE;
}

// Import of mnemonic words for slip39
static int32_t ModelSlip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);

//     uint8_t *entropy;
//     uint8_t entropyLen;
//     int32_t ret;
//     uint16_t id;
//     uint8_t ie;
//     bool eb;
//     uint8_t newAccount;
//     uint8_t accountCnt;
//     Slip39Data_t *slip39 = (Slip39Data_t *)inData;
//     AccountInfo_t accountInfo;

//     entropyLen = (slip39->wordCnt == 20) ? 16 : 32;
//     entropy = SRAM_MALLOC(entropyLen);
//     uint8_t ems[32] = {0};
//     uint8_t emsBak[32] = {0};

//     char *words[slip39->threShold];
//     for (int i = 0; i < slip39->threShold; i++) {
//         words[i] = SecretCacheGetSlip39Mnemonic(i);
//     }

//     MODEL_WRITE_SE_HEAD
//     ret = Slip39GetMasterSecret(slip39->threShold, slip39->wordCnt, ems, entropy, words, &id, &eb, &ie);
//     if (ret != SUCCESS_CODE) {
//         printf("get master secret error\n");
//         break;
//     }
//     memcpy_s(emsBak, sizeof(emsBak), ems, entropyLen);

//     if (slip39->forget) {
//         ret = ModelComparePubkey(MNEMONIC_TYPE_SLIP39, ems, entropyLen, id, eb, ie, &newAccount);
//         CHECK_ERRCODE_BREAK("mnemonic not match", !ret);
//     } else {
//         ret = ModelComparePubkey(MNEMONIC_TYPE_SLIP39, ems, entropyLen, id, eb, ie, NULL);
//         CHECK_ERRCODE_BREAK("mnemonic repeat", ret);
//     }
//     printf("before accountCnt = %d\n", accountCnt);
//     if (slip39->forget) {
//         GetAccountInfo(newAccount, &accountInfo);
//     }
//     ret = CreateNewSlip39Account(newAccount, emsBak, entropy, entropyLen, SecretCacheGetNewPassword(), id, eb, ie);
//     CHECK_ERRCODE_BREAK("save slip39 entropy error", ret);
//     ClearAccountPassphrase(newAccount);
//     ret = VerifyPasswordAndLogin(&newAccount, SecretCacheGetNewPassword());
//     CHECK_ERRCODE_BREAK("login error", ret);
//     UpdateFingerSignFlag(GetCurrentAccountIndex(), false);
//     if (slip39->forget) {
//         SetWalletName(accountInfo.walletName);
//         SetWalletIconIndex(accountInfo.iconIndex);
//         LogoutCurrentAccount();
//         CloseUsb();
//     }
//     CLEAR_ARRAY(ems);
//     CLEAR_ARRAY(emsBak);
//     GetExistAccountNum(&accountCnt);
//     printf("after accountCnt = %d\n", accountCnt);
// }
// while (0);
// if (ret == SUCCESS_CODE)
// {
//     ClearSecretCache();
//     GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));
// } else
// {
//     GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &ret, sizeof(ret));
// }
// SRAM_FREE(entropy);
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

static int32_t ModelSlip39ForgetPass(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret = SUCCESS_CODE;
// #ifndef COMPILE_SIMULATOR
//     uint8_t *entropy;
//     uint8_t entropyLen;
//     uint16_t id;
//     uint8_t ie;
//     bool eb;
//     Slip39Data_t *slip39 = (Slip39Data_t *)inData;

//     entropyLen = (slip39->wordCnt == 20) ? 16 : 32;
//     entropy = SRAM_MALLOC(entropyLen);
//     uint8_t ems[32] = {0};

//     char *words[slip39->threShold];
//     for (int i = 0; i < slip39->threShold; i++) {
//         words[i] = SecretCacheGetSlip39Mnemonic(i);
//     }

//     do {
//         ret = CHECK_BATTERY_LOW_POWER();
//         CHECK_ERRCODE_BREAK("save low power", ret);
//         ret = Slip39GetMasterSecret(slip39->threShold, slip39->wordCnt, ems, entropy, words, &id, &eb, &ie);
//         if (ret != SUCCESS_CODE) {
//             printf("get master secret error\n");
//             break;
//         }
//         ret = ModelComparePubkey(MNEMONIC_TYPE_SLIP39, ems, entropyLen, id, eb, ie, NULL);
//         if (ret != SUCCESS_CODE) {
//             GuiApiEmitSignal(SIG_FORGET_PASSWORD_SUCCESS, NULL, 0);
//             SetLockScreen(enable);
//             return ret;
//         }
//         ret = ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET;
//     } while (0);
//     GuiApiEmitSignal(SIG_FORGET_PASSWORD_FAIL, &ret, sizeof(ret));

//     SRAM_FREE(entropy);
// #else
//     GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));
// #endif
    SetLockScreen(enable);
    return ret;
}

// save wallet desc
static int32_t ModelSaveWalletDesc(const void *inData, uint32_t inDataLen)
{
    return SUCCESS_CODE;
}

// del wallet
static int32_t ModelDelWallet(const void *inData, uint32_t inDataLen)
{
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
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// reset wallet password
static int32_t ModelChangeAccountPass(const void *inData, uint32_t inDataLen)
{
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
}

static void ModelVerifyPassFailed(uint16_t *param)
{
}

// verify wallet password
static int32_t ModelVerifyAccountPass(const void *inData, uint32_t inDataLen)
{
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
    // assert(walletAmount == 0);
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
    if (en) {
        read_qrcode();
    }
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

static int32_t ModelWriteLastLockDeviceTime(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);

    uint32_t time = *(uint32_t*)inData;
    // SetLastLockDeviceTime(time);

    SetLockScreen(enable);
    return SUCCESS_CODE;
}

static int32_t ModelCopySdCardOta(const void *inData, uint32_t inDataLen)
{
#ifndef COMPILE_SIMULATOR
    static uint8_t walletAmount;
    SetPageLockScreen(false);
    int32_t ret = FatfsFileCopy("0:/forgebox.bin", "1:/pillar.bin");
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
    UNUSED(inData);
    UNUSED(inDataLen);
    // g_checkResult = CheckUrResult(viewType);
    if (g_checkResult != NULL && g_checkResult->error_code == 0) {
        GuiApiEmitSignal(SIG_TRANSACTION_CHECK_PASS, NULL, 0);
    } else {
        printf("transaction check fail, error code: %d, error msg: %s\r\n", g_checkResult->error_code, g_checkResult->error_message);
        GuiApiEmitSignal(SIG_HIDE_TRANSACTION_LOADING, NULL, 0);
        GuiApiEmitSignal(SIG_TRANSACTION_CHECK_FAIL, g_checkResult, sizeof(g_checkResult));
    }

    return SUCCESS_CODE;
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
    return SUCCESS_CODE;
}

static const uint8_t APP_END_MAGIC_NUMBER[] = {'m', 'h', '1', '9', '0', '3', 'a', 'p', 'p', 'e', 'n', 'd'};
uint32_t BinarySearchLastNonFFSector(void)
{
    size_t APP_END_MAGIC_NUMBER_SIZE = sizeof(APP_END_MAGIC_NUMBER);
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
        if (memcmp(buffer, APP_END_MAGIC_NUMBER, APP_END_MAGIC_NUMBER_SIZE) == 0) {
            if (CheckAllFF(&buffer[APP_END_MAGIC_NUMBER_SIZE], SECTOR_SIZE - APP_END_MAGIC_NUMBER_SIZE)) {
                SRAM_FREE(buffer);
                return i;
            }
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
    return false;
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

