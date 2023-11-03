#include "gui_chain.h"

#define XRP_ROOT_PATH "m/44'/144'/0'"

static char g_xrpAddr[36];
static char g_hdPath[25];

static bool g_isMulti = false;
static void *g_urResult = NULL;
static void *g_parseResult = NULL;

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
    TransactionCheckResult *result = NULL;
    do {
        PtrT_TransactionParseResult_DisplayXrpTx parseResult = xrp_parse_tx(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
        result = xrp_check_pubkey(parseResult->data->signing_pubkey, GetCurrentAccountPublicKey(XPUB_TYPE_XRP));
        CHECK_CHAIN_BREAK(result);
    } while (0);
    free_TransactionCheckResult(result);
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
