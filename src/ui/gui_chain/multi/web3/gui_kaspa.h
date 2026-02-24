#ifndef _GUI_KASPA_H
#define _GUI_KASPA_H

#include "stdint.h"
#include "stdbool.h"
#include "rust.h"
#include "lvgl.h"

void GuiSetKaspaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);

void *GuiGetParsedKaspaTx(void);

UREncodeResult *GuiGetKaspaSignQrCodeData(void);

TransactionCheckResult *GuiGetKaspaCheckResult(void);

void GuiClearKaspaData(void);

void GuiKaspaTxOverview(lv_obj_t *parent, void *data);

void GuiKaspaTxDetail(lv_obj_t *parent, void *data);

#endif