#ifndef BTC_ONLY

#include "gui_monero.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static XmrRequestType g_requestType = OutputRequest;
static uint8_t g_decryptKey[32] = {0};

#define CHECK_FREE_PARSE_RESULT(result)                                                                         \
  if (result != NULL)                                                                                           \
  {                                                                                                             \
    switch (g_requestType)                                                                                      \
    {                                                                                                           \
    case OutputRequest:                                                                                         \
      free_TransactionParseResult_DisplayMoneroOutput((PtrT_TransactionParseResult_DisplayMoneroOutput)result); \
      break;                                                                                                    \
    case UnsignedTxRequest:                                                                                     \
      free_TransactionParseResult_DisplayMoneroUnsignedTx((PtrT_TransactionParseResult_DisplayMoneroUnsignedTx)result); \
      break;                                                                                                    \
    }                                                                                                           \
    result = NULL;                                                                                              \
  }

static void SetUpMoneroDecryptKey(void)
{
    uint8_t *pvk = SecretCacheGetXmrPrivateViewKey();
    SimpleResponse_u8 *decryptKeyData = monero_generate_decrypt_key(pvk);
    if (decryptKeyData->error_code == SUCCESS_CODE) {
        memcpy(g_decryptKey, decryptKeyData->data, 32);
    }
    free_simple_response_u8(decryptKeyData);
}

PtrT_TransactionCheckResult GuiGetMoneroOutputCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    g_requestType = OutputRequest;
    SetUpMoneroDecryptKey();
    return monero_output_request_check(data, g_decryptKey, SecretCacheGetXmrPrivateViewKey());
}

PtrT_TransactionCheckResult GuiGetMoneroUnsignedTxCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    g_requestType = UnsignedTxRequest;
    SetUpMoneroDecryptKey();
    return monero_unsigned_request_check(data, g_decryptKey, SecretCacheGetXmrPrivateViewKey());
}

void GuiSetMoneroUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

void *GuiGetMoneroOutputData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayMoneroOutput parseResult = monero_parse_output(data, g_decryptKey, SecretCacheGetXmrPrivateViewKey());
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void *GuiGetMoneroUnsignedTxData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayMoneroUnsignedTx parseResult = monero_parse_unsigned_tx(data, g_decryptKey, SecretCacheGetXmrPrivateViewKey());
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

UREncodeResult *GuiGetMoneroKeyimagesQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = monero_generate_keyimage(data, seed, len, 0);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

UREncodeResult *GuiGetMoneroSignedTransactionQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = monero_generate_signature(data, seed, len, 0);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

void FreeMoneroMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    memset(g_decryptKey, 0, sizeof(g_decryptKey));
    g_requestType = OutputRequest;
#endif
}

void GetXmrTxoCount(void *indata, void *param, uint32_t maxLen)
{
    DisplayMoneroOutput *data = (DisplayMoneroOutput *)param;
    if (data->txos_num == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->txos_num);
}

void GetXmrTotalAmount(void *indata, void *param, uint32_t maxLen)
{
    DisplayMoneroOutput *data = (DisplayMoneroOutput *)param;
    if (data->total_amount == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->total_amount);
}

#endif
