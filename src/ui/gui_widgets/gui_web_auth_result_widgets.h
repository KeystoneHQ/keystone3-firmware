#ifndef _GUI_WEB_AUTH_RESULT_WIDGETS_H
#define _GUI_WEB_AUTH_RESULT_WIDGETS_H

typedef void (*WebAuthSuccessCb)(void);
#include "stdint.h"
#include "librust_c.h"

void GuiWebAuthResultAreaInit();
void GuiWebAuthResultAreaDeInit();
void GuiWebAuthResultAreaRefresh();
void GuiWebAuthResultAreaRestart();
uint32_t GuiWebAuthResultPrevTile();
uint32_t GuiWebAuthResultNextTile();
void GuiSetWebAuthResultData(URParseResult *data, URParseMultiResult *multiResult, bool multi);
void GuiWebAuthResultSetSuccessCb(WebAuthSuccessCb cb);
void GuiWebAuthShowAuthCode(char* authCode);
#endif