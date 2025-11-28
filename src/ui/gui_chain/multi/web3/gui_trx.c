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

void GuiSetTrxUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                              \
    if (result != NULL)                                                                              \
    {                                                                                                \
        free_TransactionParseResult_DisplayTron((PtrT_TransactionParseResult_DisplayTron)result);    \
        result = NULL;                                                                               \
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
        PtrT_TransactionParseResult_DisplayTron parseResult = tron_parse_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetTrxCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    GetMasterFingerPrint(mfp);
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

UREncodeResult *GuiGetTrxSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    uint8_t seed[SEED_LEN];
    do {
        int ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (ret != 0) {
            break;
        }
        encodeResult = tron_sign_keystone(data, urType, mfp, sizeof(mfp), GetCurrentAccountPublicKey(XPUB_TYPE_TRX),
                                          SOFTWARE_VERSION, seed, GetCurrentAccountSeedLen());
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);
    return encodeResult;
}