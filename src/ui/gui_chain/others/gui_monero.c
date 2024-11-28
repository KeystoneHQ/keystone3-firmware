#ifndef BTC_ONLY

#include "gui_monero.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static XmrRequestType g_requestType = OutputRequest;

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

PtrT_TransactionCheckResult GuiGetMoneroOutputCheckResult(void)
{
  void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
  uint8_t *pvk = SecretCacheGetXmrPrivateViewKey();
  g_requestType = OutputRequest;
  return monero_output_request_check(data, pvk);
}

PtrT_TransactionCheckResult GuiGetMoneroUnsignedTxCheckResult(void)
{
  void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
  uint8_t *pvk = SecretCacheGetXmrPrivateViewKey();
  g_requestType = UnsignedTxRequest;
  return monero_unsigned_request_check(data, pvk);
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
  do
  {
    PtrT_TransactionParseResult_DisplayMoneroOutput parseResult = monero_parse_output(data);
    CHECK_CHAIN_BREAK(parseResult);
    g_parseResult = (void *)parseResult;
  } while (0);
  return g_parseResult;
}

void *GuiGetMoneroUnsignedTxData(void)
{
  CHECK_FREE_PARSE_RESULT(g_parseResult);
  void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
  do
  {
    PtrT_TransactionParseResult_DisplayMoneroUnsignedTx parseResult = monero_parse_unsigned_tx(data);
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
  do
  {
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
  do
  {
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
  g_requestType = OutputRequest;
#endif
}

#endif
