#ifndef BTC_ONLY
#include "rust.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

void GuiSetSuiUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
#endif
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                                   \
    if (result != NULL)                                                                                                   \
    {                                                                                                                     \
        free_TransactionParseResult_DisplaySuiIntentMessage((PtrT_TransactionParseResult_DisplaySuiIntentMessage)result); \
        result = NULL;                                                                                                    \
    }

void *GuiGetSuiData(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplaySuiIntentMessage parseResult = sui_parse_intent(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
#else
    return NULL;
#endif
}

PtrT_TransactionCheckResult GuiGetSuiCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return sui_check_request(data, mfp, sizeof(mfp));
#else
    return NULL;
#endif
}

void FreeSuiMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

int GetSuiDetailLen(void *param)
{
    DisplaySuiIntentMessage *tx = (DisplaySuiIntentMessage *)param;
    return strlen(tx->detail);
}

void GetSuiDetail(void *indata, void *param)
{
    DisplaySuiIntentMessage *tx = (DisplaySuiIntentMessage *)param;
    sprintf((char *)indata, "%s", tx->detail);
}

UREncodeResult *GuiGetSuiSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encodeResult = sui_sign_intent(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
#else
    UREncodeResult *encodeResult = NULL;
    encodeResult->is_multi_part = 0;
    encodeResult->data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    encodeResult->encoder = NULL;
    encodeResult->error_code = 0;
    encodeResult->error_message = NULL;
    return encodeResult;
#endif
}
#endif
