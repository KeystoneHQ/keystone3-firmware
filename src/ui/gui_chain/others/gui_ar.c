#ifndef BTC_ONLY
#include "gui_ar.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

#define CHECK_FREE_PARSE_RESULT(result)                                                                 \
    if (result != NULL)                                                                                 \
    {                                                                                                   \
        free_TransactionParseResult_DisplayArweaveTx((PtrT_TransactionParseResult_DisplayArweaveTx)result); \
        result = NULL;                                                                                  \
    }

PtrT_TransactionCheckResult GuiGetArCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return ar_check_tx(data, mfp, sizeof(mfp));
#else
    return NULL;
#endif
}

void GuiSetArUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
#endif
}

void *GuiGetArData(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayArweaveTx parseResult = ar_parse(data);
        printf("ar_parse result: %s\n", parseResult->data->from);
        printf("ar_parse result: %s\n", parseResult->data->to);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
#else
    return NULL;
#endif
}

void FreeArMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

UREncodeResult *GuiGetArweaveSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        Rsa_primes_t *primes = FlashReadRsaPrimes();
        if (primes == NULL) {
            encodeResult = NULL;
            break;
        }
        encodeResult = ar_sign_tx(data, primes->p, 256, primes->q, 256);
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