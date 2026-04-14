#include "gui_analyze.h"
#include "rust.h"
#include "account_public_info.h"
#include "keystore.h"
#include "gui_chain.h"
#include "version.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "account_manager.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static ViewType g_viewType = ViewTypeUnKnown;

void GuiSetTrxUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_viewType = g_isMulti ? g_urMultiResult->t : g_urResult->t;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                              \
    if (result != NULL)                                                                              \
    {                                                                                                                             \
        switch (g_viewType)                                                                                                       \
        {                                                                                                                         \
        case TronTx:                                                                                                               \
            free_TransactionParseResult_DisplayTron((PtrT_TransactionParseResult_DisplayTron)result);                               \
            break;                                                                                                                \
        case TronPersonalMessage:                                                                                                  \
            free_TransactionParseResult_DisplayTRONPersonalMessage((PtrT_TransactionParseResult_DisplayTRONPersonalMessage)result); \
            break;                                                                                                                \
        default:                                                                                                                  \
            break;                                                                                                                \
        }                                                                                                                         \
        result = NULL;                                                                                                            \
    }

void *GuiGetTrxData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayTron parseResult = NULL;
        if (urType == TronSignRequest) {
            parseResult = tron_parse_sign_request(data);
        } else {
            parseResult = tron_parse_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
        }
        
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetTrxCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    GetMasterFingerPrint(mfp);
    if (urType == TronSignRequest) {
        return tron_check_sign_request(data, trxXpub, mfp, sizeof(mfp));
    }
    return tron_check_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
}

void FreeTrxMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

void GetTrxValue(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->value);
}

void GetTrxMethod(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->method);
}

void GetTrxFromAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->from);
}

void GetTrxToAddress(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->to);
}

bool GetTrxContractExist(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    return strlen(trx->detail->contract_address) > 0;
}

void GetTrxContract(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata, maxLen, trx->detail->contract_address);
}

bool GetTrxTokenExist(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    return strlen(trx->detail->token) > 0 && strcmp(trx->detail->token, "TRX") != 0;
}

void GetTrxToken(void *indata, void *param, uint32_t maxLen)
{
    DisplayTron *trx = (DisplayTron *)param;
    strcpy_s((char *)indata,  maxLen, trx->detail->token);
}

static UREncodeResult *GuiGetTrxSignUrDataDynamic(bool unLimit)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult = NULL;
    
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    uint8_t seed[SEED_LEN];
    uint32_t fragmentLen = unLimit ? FRAGMENT_UNLIMITED_LENGTH : FRAGMENT_MAX_LENGTH_DEFAULT;
    
    do {
        int ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (ret != 0) {
            break;
        }
        if (urType == TronSignRequest) {
            encodeResult = tron_sign_request(data, seed, GetCurrentAccountSeedLen(), fragmentLen);
        } else {
            encodeResult = tron_sign_keystone(data, urType, mfp, sizeof(mfp), GetCurrentAccountPublicKey(XPUB_TYPE_TRX),
                                          SOFTWARE_VERSION, seed, GetCurrentAccountSeedLen());
        }
        
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);

    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);
    
    return encodeResult;
}

UREncodeResult *GuiGetTrxSignQrCodeData(void)
{
    return GuiGetTrxSignUrDataDynamic(false);
}

UREncodeResult *GuiGetTrxSignUrDataUnlimited(void)
{
    return GuiGetTrxSignUrDataDynamic(true);
}

void *GuiGetTrxPersonalMessage(void)
{   
    CHECK_FREE_PARSE_RESULT(g_parseResult);

    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    GetMasterFingerPrint(mfp);

    TransactionCheckResult *result = NULL;
    do {
        result = tron_check_sign_request(data, trxXpub, mfp, sizeof(mfp));
        CHECK_CHAIN_BREAK(result);

        PtrT_TransactionParseResult_DisplayTRONPersonalMessage parseResult = tron_parse_personal_message(data, trxXpub);
        
        CHECK_CHAIN_BREAK(parseResult);
        
        g_parseResult = (void *)parseResult; 
    } while (0);

    free_TransactionCheckResult(result);
    return g_parseResult;
}

void GetTrxPersonalMessageType(void *indata, void *param, uint32_t maxLen)
{   
    if (param == NULL) {
        strcpy_s((char *)indata, maxLen, "raw_message");
        return;
    }
    DisplayTRONPersonalMessage *message = (DisplayTRONPersonalMessage *)param;
    if (message->utf8_message != NULL && strlen(message->utf8_message) > 0) {
        strcpy_s((char *)indata, maxLen, "utf8_message");
    } else {
        strcpy_s((char *)indata, maxLen, "raw_message");
    }
}

static void CopyTrxMessageWithEllipsis(char *dest, uint32_t maxLen, const char *src)
{
    if (dest == NULL || maxLen == 0) {
        return;
    }
    if (src == NULL) {
        dest[0] = '\0';
        return;
    }
    size_t src_len = strlen(src);
    if (src_len < maxLen) {
        snprintf(dest, maxLen, "%s", src);
        return;
    }
    if (maxLen <= 4) {
        snprintf(dest, maxLen, "%.*s", (int)(maxLen - 1), "...");
        return;
    }
    snprintf(dest, maxLen, "%.*s...", (int)(maxLen - 4), src);
}

void GetTrxMessageFrom(void *indata, void *param, uint32_t maxLen)
{
    DisplayTRONPersonalMessage *message = (DisplayTRONPersonalMessage *)param;
    CopyTrxMessageWithEllipsis((char *)indata, maxLen, message ? message->from : NULL);
}

void GetTrxMessageUtf8(void *indata, void *param, uint32_t maxLen)
{
    DisplayTRONPersonalMessage *message = (DisplayTRONPersonalMessage *)param;
    CopyTrxMessageWithEllipsis((char *)indata, maxLen, message ? message->utf8_message : NULL);
}

void GetTrxMessageRaw(void *indata, void *param, uint32_t maxLen)
{
    const char *warning = "\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#";
    size_t warningLen = strlen(warning);
    DisplayTRONPersonalMessage *message = (DisplayTRONPersonalMessage *)param;
    size_t rawLen = message && message->raw_message ? strlen(message->raw_message) : 0;
    if (maxLen == 0) {
        return;
    }
    if (rawLen + warningLen >= (size_t)maxLen) {
        if (maxLen <= 4) {
            snprintf((char *)indata, maxLen, "%s", "");
            return;
        }
        snprintf((char *)indata, maxLen, "%.*s...", (int)(maxLen - 4), message && message->raw_message ? message->raw_message : "");
    } else {
        snprintf((char *)indata, maxLen, "%s%s", message && message->raw_message ? message->raw_message : "", warning);
    }
}

bool GetTrxMessageFromExist(void *indata, void *param)
{   
    if (param == NULL) return false;
    DisplayTRONPersonalMessage *trx = (DisplayTRONPersonalMessage *)param;
    return trx->from != NULL;
}

bool GetTrxMessageFromNotExist(void *indata, void *param)
{
    return !GetTrxMessageFromExist(indata, param);
}

void GetTrxMessagePos(uint16_t *x, uint16_t *y, void *param)
{
    *x = GetTrxMessageFromExist(NULL, param) ? 0 : 24;
    *y = 11;
}