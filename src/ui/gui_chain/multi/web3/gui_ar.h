#ifndef _GUI_AR_H
#define _GUI_AR_H

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "librust_c.h"
#include "gui_views.h"
#include "gui_chain.h"
#include "gui_lock_widgets.h"
#include "screen_manager.h"
#include "account_public_info.h"

PtrT_TransactionCheckResult GuiGetArCheckResult(void);
void GuiArSetMessageAddress(const char *address);
void GuiSetArUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetArData(void);
void FreeArMemory(void);
void GuiArTxOverview(lv_obj_t *parent, void *totalData);
void GuiArTxDetails(lv_obj_t *parent, void *totalData);
void GuiArMessageOverview(lv_obj_t *parent, void *totalData);
UREncodeResult *GuiGetArweaveSignQrCodeData(void);
void GuiArDataItemOverview(lv_obj_t *parent, void *totalData);
void GuiArDataItemDetail(lv_obj_t *parent, void *totalData);

#endif
