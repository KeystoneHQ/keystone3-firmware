/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_model.c
 * Description:
 * author     : stone wang
 * data       : 2023-03-23 09:41
**********************************************************************/
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
#ifndef COMPILE_SIMULATOR
#include "rust.h"
#include "user_msg.h"
#include "general_msg.h"
#include "cmsis_os.h"
#include "fingerprint_process.h"
#include "user_delay.h"
#include "user_fatfs.h"
#endif

static PasswordVerifyResult_t g_passwordVerifyResult;

static int32_t ModelSaveWalletDesc(const void *inData, uint32_t inDataLen);
static int32_t ModelDelWallet(const void *inData, uint32_t inDataLen);
static int32_t ModelDelAllWallet(const void *inData, uint32_t inDataLen);
static int32_t ModelWritePassphrase(const void *inData, uint32_t inDataLen);
static int32_t ModelChangeAmountPass(const void *inData, uint32_t inDataLen);
static int32_t ModelVerifyAmountPass(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateEntropy(const void *inData, uint32_t inDataLen);
static int32_t ModelBip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModelWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModelBip39VerifyMnemonic(const void *inData, uint32_t inDataLen);
static int32_t ModelGenerateSlip39Entropy(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen);
static int32_t ModeGetAmount(const void *inData, uint32_t inDataLen);
static int32_t ModeGetWalletDesc(const void *inData, uint32_t inDataLen);
static int32_t ModeLoginWallet(const void *inData, uint32_t inDataLen);
static int32_t ModeControlQrDecode(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39WriteEntropy(const void *inData, uint32_t inDataLen);
static int32_t ModelComparePubkey(bool bip39, uint8_t *ems, uint8_t emsLen, uint16_t id, uint8_t ie, uint8_t *index);
static int32_t ModelBip39ForgetPass(const void *inData, uint32_t inDataLen);
static int32_t ModelSlip39ForgetPass(const void *inData, uint32_t inDataLen);
static int32_t ModelCalculateWebAuthCode(const void *inData, uint32_t inDataLen);
static int32_t ModelWriteLastLockDeviceTime(const void *inData, uint32_t inDataLen);
static int32_t ModelCopySdCardOta(const void *inData, uint32_t inDataLen);
static int32_t ModelURGenerateQRCode(const void *inData, uint32_t inDataLen, void *getUR);
static int32_t ModelURUpdate(const void *inData, uint32_t inDataLen);
static int32_t ModelURClear(const void *inData, uint32_t inDataLen);

#ifndef COMPILE_SIMULATOR
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
#endif

#ifdef COMPILE_SIMULATOR
int32_t AsyncExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen)
{
    func(inData, inDataLen);
    return SUCCESS_CODE;
}

int32_t AsyncExecuteRunnable(BackgroundAsyncFuncWithRunnable_t func, const void *inData, uint32_t inDataLen, BackgroundAsyncRunnable_t runnable)
{
    return SUCCESS_CODE;
}
#endif

void GuiModelWriteSe(void)
{
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    AsyncExecute(ModelWriteEntropyAndSeed, NULL, 0);
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

void GuiModelSlip39WriteSe(void)
{
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    AsyncExecute(ModelSlip39WriteEntropy, NULL, 0);
}

void GuiModelSlip39CalWriteSe(Slip39Data_t slip39)
{
    AsyncExecute(ModelSlip39CalWriteEntropyAndSeed, &slip39, sizeof(slip39));
}

void GuiModelCalculateWebAuthCode(void *webAuthData)
{
    AsyncExecuteWithPtr(ModelCalculateWebAuthCode, webAuthData);
}

void GuiModelSlip39ForgetPassword(Slip39Data_t slip39)
{
    // GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
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

void GuiModelChangeAmountPassWord(uint8_t accountIndex)
{
    AsyncExecute(ModelChangeAmountPass, &accountIndex, sizeof(accountIndex));
}

void GuiModelVerifyAmountPassWord(uint16_t *param)
{
    AsyncExecute(ModelVerifyAmountPass, param, sizeof(*param));
}

void GuiModelBip39UpdateMnemonic(uint8_t wordCnt)
{
    uint32_t mnemonicNum = wordCnt;
    AsyncExecute(ModelGenerateEntropy, &mnemonicNum, sizeof(mnemonicNum));
}

void GuiModelSlip39UpdateMnemonic(Slip39Data_t slip39)
{
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    AsyncExecute(ModelGenerateSlip39Entropy, &slip39, sizeof(slip39));
}

void GuiModeGetAmount(void)
{
    AsyncExecute(ModeGetAmount, NULL, 0);
}

void GuiModeGetWalletDesc(void)
{
    AsyncExecute(ModeGetWalletDesc, NULL, 0);
}

void GuiModeLoginWallet(void)
{
    AsyncExecute(ModeLoginWallet, NULL, 0);
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
#ifndef COMPILE_SIMULATOR
    entropyLen = (mnemonicNum == 24) ? 32 : 16;
    GenerateEntropy(entropy, entropyLen, SecretCacheGetNewPassword());
    SecretCacheSetEntropy(entropy, entropyLen);
    bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
    SecretCacheSetMnemonic(mnemonic);
    retData = SUCCESS_CODE;
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC, &retData, sizeof(retData));
    memset(mnemonic, 0, strlen(mnemonic));
    SRAM_FREE(mnemonic);
#else
    mnemonic = SRAM_MALLOC(256);
    uint16_t buffLen = 0;
    for (int i = 0; i < mnemonicNum; i++) {
        buffLen += sprintf(mnemonic + buffLen, "%s ", wordlist[lv_rand(0, 2047)]);
    }
    mnemonic[strlen(mnemonic) - 1] = '\0';
    SecretCacheSetMnemonic(mnemonic);
    retData = SUCCESS_CODE;
    GuiApiEmitSignal(SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC, NULL, 0);
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// Generate bip39 wallet writes
static int32_t ModelWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret;
#ifndef COMPILE_SIMULATOR
    uint8_t *entropy;
    uint32_t entropyLen;
    uint8_t newAccount;
    uint8_t accountCnt;

    entropy = SecretCacheGetEntropy(&entropyLen);
    MODEL_WRITE_SE_HEAD
    ret = CreateNewAccount(newAccount, entropy, entropyLen, SecretCacheGetNewPassword());
    ClearAccountPassphrase(newAccount);
    CHECK_ERRCODE_BREAK("save entropy error", ret);
    MODEL_WRITE_SE_END
#else
    ret = ERR_KEYSTORE_MNEMONIC_INVALID;
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &ret, sizeof(ret));
    // GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));
#endif
    SetLockScreen(enable);
    return 0;
}

// Import of mnemonic words for bip39
static int32_t ModelBip39CalWriteEntropyAndSeed(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret;
#ifndef COMPILE_SIMULATOR
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
        ret = ModelComparePubkey(true, NULL, 0, 0, 0, &newAccount);
        CHECK_ERRCODE_BREAK("mnemonic not match", !ret);
    } else {
        ret = ModelComparePubkey(true, NULL, 0, 0, 0, NULL);
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
memset(entropy, 0, entropyInLen);
SRAM_FREE(entropy);
#else
    ret = ERR_KEYSTORE_MNEMONIC_REPEAT;
    // GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &ret, sizeof(ret));
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, NULL, 0);
#endif
SetLockScreen(enable);
return 0;
}

// Auxiliary word verification for bip39
static int32_t ModelBip39VerifyMnemonic(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret = SUCCESS_CODE;
#ifndef COMPILE_SIMULATOR
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
#else
    GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS, &ret, sizeof(ret));
#endif
    SetLockScreen(enable);
    return 0;
}

// Auxiliary word verification for bip39
static int32_t ModelBip39ForgetPass(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    int32_t ret = SUCCESS_CODE;
#ifndef COMPILE_SIMULATOR
    do {
        ret = CHECK_BATTERY_LOW_POWER();
        CHECK_ERRCODE_BREAK("save low power", ret);
        ret = ModelComparePubkey(true, NULL, 0, 0, 0, NULL);
        if (ret != SUCCESS_CODE) {
            GuiApiEmitSignal(SIG_FORGET_PASSWORD_SUCCESS, NULL, 0);
            SetLockScreen(enable);
            return ret;
        }
        ret = ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET;
    } while (0);
    GuiApiEmitSignal(SIG_FORGET_PASSWORD_FAIL, &ret, sizeof(ret));
#else
    ret = ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET;
    // GuiEmitSignal(SIG_FORGET_PASSWORD_FAIL, &ret, sizeof(ret));
    GuiEmitSignal(SIG_FORGET_PASSWORD_SUCCESS, NULL, 0);
#endif
    SetLockScreen(enable);
    return ret;
}

static UREncodeResult *g_urResult = NULL;

static int32_t ModelURGenerateQRCode(const void *indata, uint32_t inDataLen, void *getUR)
{
    GenerateUR func = (GenerateUR)getUR;
    g_urResult = func();
    if (g_urResult->error_code == 0) {
        GuiApiEmitSignal(SIG_BACKGROUND_UR_GENERATE_SUCCESS, g_urResult->data, strlen(g_urResult->data) + 1);
    } else {
        //TODO: deal with error
    }
    return SUCCESS_CODE;
}

static int32_t ModelURUpdate(const void *inData, uint32_t inDataLen)
{
    if (g_urResult->is_multi_part) {
        UREncodeMultiResult *result = get_next_part(g_urResult->encoder);
        if (result->error_code == 0) {
            GuiApiEmitSignal(SIG_BACKGROUND_UR_UPDATE, result->data, strlen(result->data) + 1);
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
static int32_t ModelComparePubkey(bool bip39, uint8_t *ems, uint8_t emsLen, uint16_t id, uint8_t ie, uint8_t *index)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    SimpleResponse_c_char *xPubResult;
    uint8_t seed[64] = {0};
    int ret = 0;
    uint8_t existIndex = 0;
    do {
        if (bip39) {
            ret = bip39_mnemonic_to_seed(SecretCacheGetMnemonic(), NULL, seed, 64, NULL);
            xPubResult = get_extended_pubkey_by_seed(seed, 64, "M/49'/0'/0'");
        } else {
            ret = Slip39GetSeed(ems, seed, emsLen, "", ie, id);
            xPubResult = get_extended_pubkey_by_seed(seed, emsLen, "M/49'/0'/0'");
        }
        CHECK_CHAIN_BREAK(xPubResult);
        CLEAR_ARRAY(seed);
        existIndex = SpecifiedXPubExist(xPubResult->data);
        if (index != NULL) {
            *index = existIndex;
        }
        if (existIndex != 0xFF) {
            ret = ERR_KEYSTORE_MNEMONIC_REPEAT;
        } else {
            ret = SUCCESS_CODE;
        }
    } while (0);
    free_simple_response_c_char(xPubResult);
    SetLockScreen(enable);
    return ret;
#endif
}

// slip39 generate
static int32_t ModelGenerateSlip39Entropy(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#define SRAM_MNEMONIC_LEN       33 * 10
    int32_t retData;
    uint8_t entropy[32], ems[32];
    uint32_t memberCnt, threShold, entropyLen = 32;
#ifndef COMPILE_SIMULATOR
    uint16_t id;
    uint8_t ie;
    Slip39Data_t *slip39 = (Slip39Data_t *)inData;
    memberCnt = slip39->memberCnt;
    threShold = slip39->threShold;
    char *wordsList[memberCnt];
    GenerateEntropy(entropy, 32, SecretCacheGetNewPassword());
    SecretCacheSetEntropy(entropy, entropyLen);
    GetSlip39MnemonicsWords(entropy, ems, 33, memberCnt, threShold, wordsList, &id, &ie);
    SecretCacheSetEms(ems, entropyLen);
    SecretCacheSetIdentifier(id);
    SecretCacheSetIteration(ie);
    for (int i = 0; i < memberCnt; i++) {
        SecretCacheSetSlip39Mnemonic(wordsList[i], i);
    }

    for (int i = 0; i < memberCnt; i++) {
        memset(wordsList[i], 0, SRAM_MNEMONIC_LEN);
        // todo 这里用SRAM_FREE有问题
        free(wordsList[i]);
    }
    retData = SUCCESS_CODE;
    GuiApiEmitSignal(SIG_CREATE_SHARE_UPDATE_MNEMONIC, &retData, sizeof(retData));
#else
    memberCnt = 3;
    char *mnemonic = NULL;
    mnemonic = SRAM_MALLOC(SRAM_MNEMONIC_LEN);
    memset(mnemonic, 0, SRAM_MNEMONIC_LEN);
    uint16_t buffLen = 0;
    for (int i = 0; i < memberCnt; i++) {
        for (int j = 0; j < 33; j++) {
            strcat(mnemonic, wordlist[lv_rand(0, 2047)]);
            if (j == 32) {
                break;
            }
            mnemonic[strlen(mnemonic)] = ' ';
        }
        SecretCacheSetSlip39Mnemonic(mnemonic, i);
        memset(mnemonic, 0, SRAM_MNEMONIC_LEN);
    }
    retData = SUCCESS_CODE;
    GuiEmitSignal(SIG_CREATE_SHARE_UPDATE_MNEMONIC, NULL, 0);
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// Generate slip39 wallet writes
static int32_t ModelSlip39WriteEntropy(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    uint8_t *entropy;
    uint8_t *ems;
    uint32_t entropyLen;
    uint8_t newAccount;
    uint8_t accountCnt;
    int ret;

    ems = SecretCacheGetEms(&entropyLen);
    entropy = SecretCacheGetEntropy(&entropyLen);

    MODEL_WRITE_SE_HEAD
    ret = CreateNewSlip39Account(newAccount, ems, entropy, 32, SecretCacheGetNewPassword(), SecretCacheGetIdentifier(), SecretCacheGetIteration());
    CHECK_ERRCODE_BREAK("save slip39 entropy error", ret);
    ClearAccountPassphrase(newAccount);
    MODEL_WRITE_SE_END

#endif
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
    memcpy(emsBak, ems, entropyLen);

    if (slip39->forget) {
        ret = ModelComparePubkey(false, ems, entropyLen, id, ie, &newAccount);
        CHECK_ERRCODE_BREAK("mnemonic not match", !ret);
    } else {
        ret = ModelComparePubkey(false, ems, entropyLen, id, ie, NULL);
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
        ret = ModelComparePubkey(false, ems, entropyLen, id, ie, NULL);
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
#ifndef COMPILE_SIMULATOR
    WalletDesc_t *wallet = (WalletDesc_t *)inData;
    SetWalletName(wallet->name);
    SetWalletIconIndex(wallet->iconIndex);

    GuiApiEmitSignal(SIG_SETTING_CHANGE_WALLET_DESC_PASS, NULL, 0);
#else
    WalletDesc_t *wallet = (char *)inData;
    GuiEmitSignal(SIG_SETTING_CHANGE_WALLET_DESC_PASS, NULL, 0);
#endif
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
    UpdateFingerSignFlag(GetCurrentAccountIndex(), false);
    ret = DestroyAccount(GetCurrentAccountIndex());
    if (ret == SUCCESS_CODE) {
        // reset address index in receive page
        {
            void GuiResetCurrentUtxoAddressIndex(void);
            void GuiResetCurrentEthAddressIndex(void);
            void GuiResetCurrentStandardAddressIndex(void);

            GuiResetCurrentUtxoAddressIndex();
            GuiResetCurrentEthAddressIndex();
            GuiResetCurrentStandardAddressIndex();
        }

        uint8_t accountNum;
        GetExistAccountNum(&accountNum);
        if (accountNum == 0) {
            {
                FpWipeManageInfo();
                SetSetupStep(0);
                SaveDeviceSettings();
                g_reboot = true;
            }
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
    // GuiEmitSignal(SIG_SETTING_DEL_WALLET_PASS_SETUP, NULL, 0);
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
    int32_t ret = SetPassphrase(GetCurrentAccountIndex(), SecretCacheGetPassphrase(), SecretCacheGetPassword());
    if (ret == SUCCESS_CODE) {
        GuiApiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_PASS, NULL, 0);
    } else {
        GuiApiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_FAIL, NULL, 0);
    }
#else
    GuiEmitSignal(SIG_SETTING_WRITE_PASSPHRASE_PASS, NULL, 0);
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// reset wallet password
static int32_t ModelChangeAmountPass(const void *inData, uint32_t inDataLen)
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
    char* authCode = calculate_auth_code(inData, key, 512, &key[512], 512);
    GuiApiEmitSignal(SIG_WEB_AUTH_CODE_SUCCESS, authCode, strlen(authCode));
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

// verify wallet password
static int32_t ModelVerifyAmountPass(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    static uint8_t walletAmount;
    uint8_t accountIndex;
#ifndef COMPILE_SIMULATOR
    int32_t ret;
    uint16_t *param = (uint16_t *)inData;

    if (SIG_LOCK_VIEW_VERIFY_PIN == *param || SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS == *param) {
        ret = VerifyPasswordAndLogin(&accountIndex, SecretCacheGetPassword());
        if (ret == ERR_KEYSTORE_EXTEND_PUBLIC_KEY_NOT_MATCH) {
            GuiApiEmitSignal(SIG_EXTENDED_PUBLIC_KEY_NOT_MATCH, NULL, 0);
            return ret;
        }
    } else {
        ret = VerifyCurrentAccountPassword(SecretCacheGetPassword());
    }
    SetLockScreen(enable);
    if (ret == SUCCESS_CODE) {
        if (*param == DEVICE_SETTING_ADD_WALLET) {
            GetExistAccountNum(&walletAmount);
            if (walletAmount == 3) {
                GuiApiEmitSignal(SIG_SETTING_ADD_WALLET_AMOUNT_LIMIT, NULL, 0);
            } else {
                GuiEmitSignal(SIG_INIT_GET_ACCOUNT_NUMBER, &walletAmount, sizeof(walletAmount));
                GuiApiEmitSignal(SIG_VERIFY_PASSWORD_PASS, param, sizeof(*param));
            }
        } else if (*param == SIG_INIT_SD_CARD_OTA_COPY) {
            GuiApiEmitSignal(SIG_VERIFY_PASSWORD_PASS, param, sizeof(*param));
            GuiApiEmitSignal(SIG_INIT_SD_CARD_OTA_COPY, param, sizeof(*param));
            ModelCopySdCardOta(inData, inDataLen);
        } else {
            GuiApiEmitSignal(SIG_VERIFY_PASSWORD_PASS, param, sizeof(*param));
        }
    } else {
        if (SIG_LOCK_VIEW_VERIFY_PIN == *param || SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS == *param) {
            g_passwordVerifyResult.errorCount = GetLoginPasswordErrorCount();
            printf("gui model get login error count %d \n", g_passwordVerifyResult.errorCount);
            assert(g_passwordVerifyResult.errorCount <= MAX_LOGIN_PASSWORD_ERROR_COUNT);
            if (g_passwordVerifyResult.errorCount == MAX_LOGIN_PASSWORD_ERROR_COUNT) {
                UnlimitedVibrate(SUPER_LONG);
            } else {
                UnlimitedVibrate(LONG);
            }
        } else {
            g_passwordVerifyResult.errorCount = GetCurrentPasswordErrorCount();
            printf("gui model get current error count %d \n", g_passwordVerifyResult.errorCount);
            assert(g_passwordVerifyResult.errorCount <= MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX);
            if (g_passwordVerifyResult.errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
                UnlimitedVibrate(SUPER_LONG);
            } else {
                UnlimitedVibrate(LONG);
            }
        }
        g_passwordVerifyResult.signal = param;
        GuiApiEmitSignal(SIG_VERIFY_PASSWORD_FAIL, (void*)&g_passwordVerifyResult, sizeof(g_passwordVerifyResult));
    }
#else
    uint8_t *entropy;
    uint8_t entropyLen;
    int32_t ret;
    uint16_t *param = (uint16_t *)inData;

    if (param == NULL) {
        GuiEmitSignal(SIG_VERIFY_PASSWORD_PASS, NULL, 0);
        return SUCCESS_CODE;
    }
    walletAmount = 2;
    if (*param == DEVICE_SETTING_ADD_WALLET) {
        if (walletAmount == 3) {
            GuiEmitSignal(SIG_SETTING_ADD_WALLET_AMOUNT_LIMIT, NULL, 0);
        } else {
            GuiEmitSignal(SIG_INIT_GET_ACCOUNT_NUMBER, &walletAmount, sizeof(walletAmount));
            GuiApiEmitSignal(SIG_VERIFY_PASSWORD_PASS, param, sizeof(*param));
        }
    } else {
        if (!strcmp(SecretCacheGetPassword(), "999999")) {
            GuiApiEmitSignal(SIG_VERIFY_PASSWORD_FAIL, param, sizeof(*param));
        } else {
            GuiEmitSignal(SIG_VERIFY_PASSWORD_PASS, param, sizeof(*param));
        }
    }
#endif
    return SUCCESS_CODE;
}

// get wallet amount
static int32_t ModeGetAmount(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    static uint8_t walletAmount;
#ifndef COMPILE_SIMULATOR
    int32_t ret;

    ret = GetExistAccountNum(&walletAmount);
    if (ret != SUCCESS_CODE) {
        walletAmount = 0xFF;
    }
    GuiApiEmitSignal(SIG_INIT_GET_ACCOUNT_NUMBER, &walletAmount, sizeof(walletAmount));
#else
    walletAmount = SIMULATOR_WALLET_AMOUNT;
    GuiEmitSignal(SIG_INIT_GET_ACCOUNT_NUMBER, &walletAmount, sizeof(walletAmount));
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// get wallet desc
static int32_t ModeGetWalletDesc(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    static WalletDesc_t wallet;
#ifndef COMPILE_SIMULATOR
    if (GetCurrentAccountIndex() > 2) {
        SetLockScreen(enable);
        return SUCCESS_CODE;
    }
    wallet.iconIndex = GetWalletIconIndex();
    strcpy(wallet.name, GetWalletName());
    GuiApiEmitSignal(SIG_INIT_GET_CURRENT_WALLET_DESC, &wallet, sizeof(wallet));
#else
    wallet.iconIndex = 0;
    GuiEmitSignal(SIG_INIT_GET_CURRENT_WALLET_DESC, &wallet, sizeof(wallet));
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}

// login wallet
static int32_t ModeLoginWallet(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    static WalletDesc_t wallet;
#ifndef COMPILE_SIMULATOR
    wallet.iconIndex = GetWalletIconIndex();
    strcpy(wallet.name, GetWalletName());
    GuiApiEmitSignal(SIG_INIT_GET_CURRENT_WALLET_DESC, &wallet, sizeof(wallet));
#else
    wallet.iconIndex = 0;
    GuiEmitSignal(SIG_INIT_GET_CURRENT_WALLET_DESC, &wallet, sizeof(wallet));
#endif
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
    static uint8_t urRet = 0;
    UrViewType_t urViewType = { KeyDerivationRequest, QRHardwareCall };
    printf("here\r\n");
    GuiEmitSignal(SIG_QRCODE_VIEW_SCAN_PASS, &urViewType, sizeof(urViewType));
    // GuiEmitSignal(SIG_QRCODE_VIEW_SCAN_FAIL, &urRet, sizeof(urRet));
#endif
    SetLockScreen(enable);
    return SUCCESS_CODE;
}


static int32_t ModelWriteLastLockDeviceTime(const void *inData, uint32_t inDataLen)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);

    uint32_t time = *(uint32_t*)inData;
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
#endif
    return SUCCESS_CODE;
}