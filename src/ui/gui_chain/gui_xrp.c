#include "gui_xrp.h"
#include "gui_chain.h"
#include "account_manager.h"
#include "user_memory.h"
#include "keystore.h"
#include "secret_cache.h"

#define XRP_ROOT_PATH "m/44'/144'/0'"

static char g_xrpAddr[40];
static char g_hdPath[26];

static bool g_isMulti = false;
static void *g_urResult = NULL;
static void *g_parseResult = NULL;
static char g_cachedPubkey[3][80] = {{'\0'}};
static char g_cachedPath[3][32] = {{'\0'}};

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

void GuiSetXrpUrData(void *data, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = data;
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
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    SimpleResponse_c_char *result = NULL;
    do {
        printf("GuiGetXrpData\n");
        PtrT_TransactionParseResult_DisplayXrpTx parseResult = xrp_parse_tx(data);
        CHECK_CHAIN_BREAK(parseResult);

        printf("parseResult->data->signing_pubkey: %s\n", parseResult->data->signing_pubkey);
        if (g_cachedPubkey[GetCurrentAccountIndex()] != NULL) {
            printf("g_cachedPubkey: %s\n", g_cachedPubkey[GetCurrentAccountIndex()]);
        }
        if (g_cachedPubkey[GetCurrentAccountIndex()] == NULL || strcmp(g_cachedPubkey[GetCurrentAccountIndex()], parseResult->data->signing_pubkey) != 0) {
            printf("cache pubkey\n");
            result = xrp_get_pubkey_path(GetCurrentAccountPublicKey(XPUB_TYPE_XRP), parseResult->data->signing_pubkey);
            CHECK_CHAIN_BREAK(result);

            // if (g_cachedPubkey[GetCurrentAccountIndex()] != NULL) {
            //     SRAM_FREE(g_cachedPubkey[GetCurrentAccountIndex()]);
            // }
            // g_cachedPubkey[GetCurrentAccountIndex()] = SRAM_MALLOC(strlen(parseResult->data->signing_pubkey) + 1);
            strcpy(g_cachedPubkey[GetCurrentAccountIndex()], parseResult->data->signing_pubkey);

            int rootLen = strlen(XRP_ROOT_PATH);
            int extLen = strlen(result->data) - 1;
            strncpy(g_hdPath, XRP_ROOT_PATH, rootLen);
            strncpy(g_hdPath + rootLen, result->data + 1, extLen);
            g_hdPath[rootLen + extLen] = '\0';

            // if (g_cachedPath[GetCurrentAccountIndex()] != NULL) {
            //     SRAM_FREE(g_cachedPath[GetCurrentAccountIndex()]);
            // }
            // g_cachedPath[GetCurrentAccountIndex()] = SRAM_MALLOC(strlen(g_hdPath) + 1);
            strcpy(g_cachedPath[GetCurrentAccountIndex()], g_hdPath);
        } else {
            printf("use cache pubkey\n");
            strcpy(g_hdPath, g_cachedPath[GetCurrentAccountIndex()]);
        }

        g_parseResult = (void *)parseResult;
    } while (0);
    if (result != NULL) {
        free_simple_response_c_char(result);
    }
    return g_parseResult;
#else
    TransactionParseResult_DisplayXrpTx *g_parseResult = malloc(sizeof(TransactionParseResult_DisplayXrpTx));
    DisplayXrpTx *data = malloc(sizeof(DisplayXrpTx));
    data->detail = "detail";
    g_parseResult->data = data;
    g_parseResult->error_code = 0;
    // GuiPendingHintBoxRemove();
    return g_parseResult;
#endif
}

void FreeXrpMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, g_isMulti);
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
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
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
