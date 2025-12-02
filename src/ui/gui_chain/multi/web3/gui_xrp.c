#include "define.h"
#include "gui_xrp.h"
#include "gui_chain.h"
#include "account_manager.h"
#include "user_memory.h"
#include "keystore.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "librust_c.h"
#define XRP_ROOT_PATH               ("m/44'/144'/0'")
#define XRP_ADD_MAX_LEN             (40)
#define HD_PATH_MAX_LEN             (26)
#define XPUB_KEY_LEN                (68)

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static char *g_cachedPubkey[3] = {NULL, NULL, NULL};
static char *g_cachedPath[3] = {NULL, NULL, NULL};
static char g_xrpAddr[XRP_ADD_MAX_LEN];
static char g_hdPath[HD_PATH_MAX_LEN];

char *GuiGetXrpPath(uint16_t index)
{
    snprintf_s(g_hdPath, HD_PATH_MAX_LEN, "%s/0/%u", XRP_ROOT_PATH, index);
    return g_hdPath;
}

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

void GuiSetXrpUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                             \
    if (result != NULL)                                                                             \
    {                                                                                               \
        free_TransactionParseResult_DisplayXrpTx((PtrT_TransactionParseResult_DisplayXrpTx)result); \
        result = NULL;                                                                              \
    }

void *GuiGetXrpData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    enum QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;

    PtrT_TransactionParseResult_DisplayXrpTx parseResult = NULL;
    do {
        if (urType == XrpBatchSignRequest) {
            parseResult = xrp_parse_batch_tx(data);
        } else if (is_keystone_xrp_tx(data)) {
            parseResult = xrp_parse_bytes_tx(data);
        } else {
            parseResult = xrp_parse_tx(data);
        }
        CHECK_CHAIN_BREAK(parseResult);

        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetXrpCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    TransactionCheckResult *result = NULL;
    char pubkey[XPUB_KEY_LEN] = {0};
    if (g_cachedPubkey[GetCurrentAccountIndex()] != NULL) {
        strcpy_s(pubkey, XPUB_KEY_LEN, g_cachedPubkey[GetCurrentAccountIndex()]);
    }
    enum QRCodeType urType = URTypeUnKnown;
    if (g_isMulti) {
        urType = g_urMultiResult->ur_type;
    } else {
        urType = g_urResult->ur_type;
    }
    // keystone hot wallet use urType Bytes
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    if (urType == XrpBatchSignRequest) {
        result = xrp_check_batch_tx(data, GetCurrentAccountPublicKey(XPUB_TYPE_XRP), pubkey);
    } else if (is_keystone_xrp_tx(data)) {
        result = xrp_check_tx_bytes(data, mfp, sizeof(mfp), urType);
        return result;
    } else {
        result = xrp_check_tx(data, GetCurrentAccountPublicKey(XPUB_TYPE_XRP), pubkey);
    }
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
        len = strnlen_s(g_hdPath, HD_PATH_MAX_LEN) + 1;
        g_cachedPath[GetCurrentAccountIndex()] = SRAM_MALLOC(len);
        strcpy_s(g_cachedPath[GetCurrentAccountIndex()], len, g_hdPath);
    } else {
        if (g_cachedPath[GetCurrentAccountIndex()] != NULL) {
            strcpy_s(g_hdPath, HD_PATH_MAX_LEN, g_cachedPath[GetCurrentAccountIndex()]);
        }
    }
    return result;
}

void FreeXrpMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
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

bool GetXrpServiceFeeExist(void *indata, void *param)
{
    DisplayXrpTx *tx = (DisplayXrpTx *)param;
    return tx->service_fee_detail != NULL;
}

int GetXrpServiceFeeDetailLen(void *param)
{
    DisplayXrpTx *tx = (DisplayXrpTx *)param;
    return strlen(tx->detail);
}

void GetXrpServiceFeeDetail(void *indata, void *param, uint32_t maxLen)
{
    DisplayXrpTx *tx = (DisplayXrpTx *)param;
    strcpy_s((char *)indata, maxLen, tx->detail);
}

UREncodeResult *GuiGetXrpSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    enum QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        if (urType == XrpBatchSignRequest) {
            encodeResult = xrp_sign_batch_tx(data, g_hdPath, seed, len);
        } else if (is_keystone_xrp_tx(data)) {
            uint8_t mfp[4] = {0};
            GetMasterFingerPrint(mfp);
            encodeResult = xrp_sign_tx_bytes(data, seed, len, mfp, sizeof(mfp), GetCurrentAccountPublicKey(XPUB_TYPE_XRP));
        } else {
            encodeResult = xrp_sign_tx(data, g_hdPath, seed, len);
        }
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}