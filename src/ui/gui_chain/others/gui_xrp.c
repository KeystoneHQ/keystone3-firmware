#ifndef BTC_ONLY
#include "gui_xrp.h"
#include "gui_chain.h"
#include "account_manager.h"
#include "user_memory.h"
#include "keystore.h"
#include "secret_cache.h"
#include "screen_manager.h"

#define XRP_ROOT_PATH "m/44'/144'/0'"

static char g_xrpAddr[40];
static char g_hdPath[26];

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static char *g_cachedPubkey[3] = {NULL};
static char *g_cachedPath[3] = {NULL};

char *GuiGetXrpPath(uint16_t index)
{
    sprintf(g_hdPath, "%s/0/%u", XRP_ROOT_PATH, index);
    return g_hdPath;
}

#ifdef COMPILE_SIMULATOR
char *GuiGetXrpAddressByIndex(uint16_t index)
{
    sprintf(g_xrpAddr, "rHsMGQEkVNJmpGWs8XUBoTBiAAbwxZ%d", index);
    return g_xrpAddr;
}
#else
char *GuiGetXrpAddressByIndex(uint16_t index)
{
    char *xPub;
    char *hdPath = GuiGetXrpPath(index);
    SimpleResponse_c_char *result;

    xPub = GetCurrentAccountPublicKey(XPUB_TYPE_XRP);
    result = xrp_get_address(hdPath, xPub, XRP_ROOT_PATH);

    if (result->error_code == 0) {
        strcpy(g_xrpAddr, result->data);
    }

    free_simple_response_c_char(result);
    return g_xrpAddr;
}
#endif

void GuiSetXrpUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
#endif
}

#define CHECK_FREE_PARSE_RESULT(result)                                                             \
    if (result != NULL)                                                                             \
    {                                                                                               \
        free_TransactionParseResult_DisplayXrpTx((PtrT_TransactionParseResult_DisplayXrpTx)result); \
        result = NULL;                                                                              \
    }

void *GuiGetXrpData(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayXrpTx parseResult = xrp_parse_tx(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
#else
    TransactionParseResult_DisplayXrpTx *g_parseResult = SRAM_MALLOC(sizeof(TransactionParseResult_DisplayXrpTx));
    DisplayXrpTx *data = SRAM_MALLOC(sizeof(DisplayXrpTx));
    data->detail = "detail";
    g_parseResult->data = data;
    g_parseResult->error_code = 0;
    return g_parseResult;
#endif
}

PtrT_TransactionCheckResult GuiGetXrpCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    TransactionCheckResult *result = NULL;
    char pubkey[68] = {0};
    if (g_cachedPubkey[GetCurrentAccountIndex()] != NULL) {
        strcpy(pubkey, g_cachedPubkey[GetCurrentAccountIndex()]);
    }
    result = xrp_check_tx(data, GetCurrentAccountPublicKey(XPUB_TYPE_XRP), pubkey);
    if (result != NULL && result->error_code == 0 && strlen(result->error_message) > 0) {
        if (g_cachedPubkey[GetCurrentAccountIndex()] != NULL) {
            SRAM_FREE(g_cachedPubkey[GetCurrentAccountIndex()]);
        }
        char *res_str = SRAM_MALLOC(strlen(result->error_message) + 1);
        strcpy(res_str, result->error_message);
        char *p = strtok(res_str, ":");
        g_cachedPubkey[GetCurrentAccountIndex()] = SRAM_MALLOC(strlen(p) + 1);
        strcpy(g_cachedPubkey[GetCurrentAccountIndex()], p);

        p = strtok(NULL, ":");
        int rootLen = strlen(XRP_ROOT_PATH);
        int extLen = strlen(p) - 1;
        strcpy(g_hdPath, XRP_ROOT_PATH);
        strncpy(g_hdPath + rootLen, p + 1, extLen);
        g_hdPath[rootLen + extLen] = '\0';
        SRAM_FREE(res_str);

        if (g_cachedPath[GetCurrentAccountIndex()] != NULL) {
            SRAM_FREE(g_cachedPath[GetCurrentAccountIndex()]);
        }
        g_cachedPath[GetCurrentAccountIndex()] = SRAM_MALLOC(strlen(g_hdPath) + 1);
        strcpy(g_cachedPath[GetCurrentAccountIndex()], g_hdPath);
    } else {
        strcpy(g_hdPath, g_cachedPath[GetCurrentAccountIndex()]);
    }

    return result;
#else
    return NULL;
#endif
}

void FreeXrpMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

int GetXrpDetailLen(void *param)
{
    DisplayXrpTx *tx = (DisplayXrpTx *)param;
    return strlen(tx->detail);
}

void GetXrpDetail(void *indata, void *param)
{
    DisplayXrpTx *tx = (DisplayXrpTx *)param;
    sprintf((char *)indata, "%s", tx->detail);
}

UREncodeResult *GuiGetXrpSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encodeResult = xrp_sign_tx(data, g_hdPath, seed, len);
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