#include "gui_ton.h"
#include "rust.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "gui_chain.h"

static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static bool g_isMulti = false;
static ViewType g_viewType = ViewTypeUnKnown;

void GuiSetTonUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi) {
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_viewType = g_isMulti ? g_urMultiResult->t : g_urResult->t;
}

UREncodeResult *GuiGetTonSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = ton_sign_transaction(data, seed, 64);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

PtrT_TransactionCheckResult GuiGetTonCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return ton_check_transaction(data, mfp, sizeof(mfp));
}

void *GuiGetTonGUIData(void) {
    // CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayTon parseResult = ton_parse_transaction(data, NULL);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}