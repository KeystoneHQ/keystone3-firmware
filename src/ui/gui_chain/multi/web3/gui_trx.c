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
    do {
        PtrT_TransactionParseResult_DisplayTron parseResult = NULL;
        if( urType == 7) {
            parseResult = tron_parse_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
        }else{
            parseResult = tron_parse_sign_request(data);
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
    printf("GuiGetTrxCheckResult, urType: %d\n", urType);
    GetMasterFingerPrint(mfp);
    printf("Trx check sign request, mfp: %02x%02x%02x%02x, xpub: %s\n", mfp[0], mfp[1], mfp[2], mfp[3], trxXpub);
    if( urType == 7) {
        return tron_check_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
    }
    return tron_check_sign_request(data, trxXpub, mfp, sizeof(mfp));
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
    uint8_t seed[64];
    uint32_t fragmentLen = unLimit ? FRAGMENT_UNLIMITED_LENGTH : FRAGMENT_MAX_LENGTH_DEFAULT;
    
    do {
        int ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (ret != 0) {
            break;
        }
        if( urType == 7) {
            encodeResult = tron_sign_keystone(data, urType, mfp, sizeof(mfp), GetCurrentAccountPublicKey(XPUB_TYPE_TRX),
                                          SOFTWARE_VERSION, seed, GetCurrentAccountSeedLen());
        } else {
            encodeResult = tron_sign_request(data, seed, GetCurrentAccountSeedLen(), fragmentLen);
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
    printf("DEBUG: GetTrxPersonalMessageType param: %p\n", param);
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

void GetTrxMessageFrom(void *indata, void *param, uint32_t maxLen)
{
    DisplayTRONPersonalMessage *message = (DisplayTRONPersonalMessage *)param;
    if (message->from == NULL) {
        strcpy_s((char *)indata, maxLen, "");
        return;
    }
    if (strlen(message->from) >= maxLen) {
        snprintf((char *)indata, maxLen - 3, "%s", message->from);
        strcat((char *)indata, "...");
    } else {
        strcpy_s((char *)indata, maxLen, message->from);
    }
}
void GetTrxMessageUtf8(void *indata, void *param, uint32_t maxLen)
{
    DisplayTRONPersonalMessage *message = (DisplayTRONPersonalMessage *)param;
    if (strlen(message->utf8_message) >= maxLen) {
        snprintf((char *)indata, maxLen - 3, "%s", message->utf8_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, maxLen, "%s", message->utf8_message);
    }
}

void GetTrxMessageRaw(void *indata, void *param, uint32_t maxLen)
{
    int len = strlen("\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    DisplayTRONPersonalMessage *message = (DisplayTRONPersonalMessage *)param;
    if (strlen(message->raw_message) >= maxLen - len) {
        snprintf((char *)indata, maxLen - 3 - len, "%s", message->raw_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, maxLen, "%s%s", message->raw_message, "\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
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