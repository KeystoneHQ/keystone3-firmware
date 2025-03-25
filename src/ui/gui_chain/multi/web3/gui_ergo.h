#ifndef _GUI_ERGO_H_
#define _GUI_ERGO_H_

#include "rust.h"
#include "gui.h"

void GuiSetErgoUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetErgoGUIData(void);
PtrT_TransactionCheckResult GuiGetErgoCheckResult(void);
void GuiErgoOverview(lv_obj_t *parent, void *totalData);
UREncodeResult *GuiGetErgoSignQrCodeData(void);
void FreeErgoMemory(void);
#endif
