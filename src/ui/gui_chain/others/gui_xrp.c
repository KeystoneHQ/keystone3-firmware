#ifndef BTC_ONLY
#include "define.h"
#include "gui_xrp.h"
#include "gui_chain.h"
#include "account_manager.h"
#include "user_memory.h"
#include "keystore.h"
#include "secret_cache.h"
#include "screen_manager.h"

#define XRP_ROOT_PATH               ("m/44'/144'/0'")
#define XRP_ADD_MAX_LEN             (40)
#define HD_PATH_MAX_LEN             (26)
#define XPUB_KEY_LEN                (68)

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static char *g_cachedPubkey[3] = {NULL};
static char *g_cachedPath[3] = {NULL};
static char g_xrpAddr[XRP_ADD_MAX_LEN];
static char g_hdPath[HD_PATH_MAX_LEN];

char *GuiGetXrpPath(uint16_t index)
{
    snprintf_s(g_hdPath, HD_PATH_MAX_LEN, "%s/0/%u", XRP_ROOT_PATH, index);
    return g_hdPath;
}

#ifdef COMPILE_SIMULATOR
char *GuiGetXrpAddressByIndex(uint16_t index)
{
    snprintf_s(g_xrpAddr, XRP_ADD_MAX_LEN, "rHsMGQEkVNJmpGWs8XUBoTBiAAbwxZ%d", index);
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
        strcpy_s(g_xrpAddr, XRP_ADD_MAX_LEN, result->data);
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
    char pubkey[XPUB_KEY_LEN] = {0};
    if (g_cachedPubkey[GetCurrentAccountIndex()] != NULL) {
        strcpy_s(pubkey, XPUB_KEY_LEN, g_cachedPubkey[GetCurrentAccountIndex()]);
    }
    result = xrp_check_tx(data, GetCurrentAccountPublicKey(XPUB_TYPE_XRP), pubkey);
    if (result != NULL && result->error_code == 0 && strlen(result->error_message) > 0) {
        if (g_cachedPubkey[GetCurrentAccountIndex()] != NULL) {
            SRAM_FREE(g_cachedPubkey[GetCurrentAccountIndex()]);
        }
        uint32_t len = strlen(result->error_message) + 1;
        char *res_str = SRAM_MALLOC(len);
        strcpy_s(res_str, len, result->error_message);
        char *p = strtok(res_str, ":");
        len = strlen(p) + 1;
        g_cachedPubkey[GetCurrentAccountIndex()] = SRAM_MALLOC(len);
        strcpy_s(g_cachedPubkey[GetCurrentAccountIndex()], len, p);

        p = strtok(NULL, ":");
        int rootLen = strlen(XRP_ROOT_PATH);
        int extLen = strlen(p) - 1;
        snprintf_s(g_hdPath, sizeof(g_hdPath), "%s%s", XRP_ROOT_PATH, p + 1);
        g_hdPath[rootLen + extLen] = '\0';
        SRAM_FREE(res_str);

        if (g_cachedPath[GetCurrentAccountIndex()] != NULL) {
            SRAM_FREE(g_cachedPath[GetCurrentAccountIndex()]);
        }
        len = strnlen_s(g_hdPath, HD_PATH_MAX_LEN);
        g_cachedPath[GetCurrentAccountIndex()] = SRAM_MALLOC(len);
        strcpy_s(g_cachedPath[GetCurrentAccountIndex()], len, g_hdPath);
    } else {
        strcpy_s(g_hdPath, HD_PATH_MAX_LEN, g_cachedPath[GetCurrentAccountIndex()]);
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

void GetXrpDetail(void *indata, void *param, uint32_t maxLen)
{
    DisplayXrpTx *tx = (DisplayXrpTx *)param;
    strcpy_s((char *)indata, maxLen, tx->detail);
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