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
#ifndef COMPILE_SIMULATOR
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
#endif
}

#define CHECK_FREE_PARSE_RESULT(result)                                                              \
    if (result != NULL)                                                                              \
    {                                                                                                \
        free_TransactionParseResult_DisplayTron((PtrT_TransactionParseResult_DisplayTron)result);    \
        result = NULL;                                                                               \
    }

void *GuiGetTrxData(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    URType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayTron parseResult = tron_parse_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
#else
    return NULL;
#endif
}

PtrT_TransactionCheckResult GuiGetTrxCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    URType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    char *trxXpub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
    GetMasterFingerPrint(mfp);
    return tron_check_keystone(data, urType, mfp, sizeof(mfp), trxXpub);
#else
    return NULL;
#endif
}
void FreeTrxMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

void GetTrxValue(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    sprintf((char *)indata, "%s", trx->detail->value);
}

void GetTrxMethod(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    sprintf((char *)indata, "%s", trx->detail->method);
}

void GetTrxFromAddress(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    sprintf((char *)indata, "%s", trx->detail->from);
}

void GetTrxToAddress(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    sprintf((char *)indata, "%s", trx->detail->to);
}

bool GetTrxContractExist(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    return strlen(trx->detail->contract_address) > 0;
}

void GetTrxContract(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    sprintf((char *)indata, "%s", trx->detail->contract_address);
}

bool GetTrxTokenExist(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    return strlen(trx->detail->token) > 0 && strcmp(trx->detail->token, "TRX") != 0;
}

void GetTrxToken(void *indata, void *param)
{
    DisplayTron *trx = (DisplayTron *)param;
    sprintf((char *)indata, "%s", trx->detail->token);
}

UREncodeResult *GuiGetTrxSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    URType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    do {
        uint8_t mfp[4];
        GetMasterFingerPrint(mfp);
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        char *xPub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encodeResult = tron_sign_keystone(data, urType, mfp, sizeof(mfp), xPub, SOFTWARE_VERSION, seed, len);
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
