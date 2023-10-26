#include "gui_chain.h"

#define XRP_ROOT_PATH "m/44'/144'/0'"

static char g_xrpAddr[36];
static char g_hdPath[25];

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
