#ifndef _GUI_ZCASH_H
#define _GUI_ZCASH_H
#include "rust.h"
#include "gui.h"

void GuiSetZcashUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetZcashGUIData(void);
void GuiZcashOverviewWithDataAndHeight(lv_obj_t *parent, DisplayPczt *data, lv_coord_t height);
void GuiZcashOverview(lv_obj_t *parent, void *totalData);
PtrT_TransactionCheckResult GuiGetZcashCheckResult(void);
#ifdef CYPHERPUNK_VERSION
typedef UREncodeResult *(*ZcashCypherpunkSignFunc)(void *data,
        PtrBytes seedFingerprint,
        uint32_t accountIndex,
        bool disabled,
        PtrBytes seed,
        uint32_t seedLen);
UREncodeResult *GuiSignZcashCypherpunkWithSeed(void *data,
        bool unlimited,
        ZcashCypherpunkSignFunc signFunc,
        ZcashCypherpunkSignFunc unlimitedSignFunc);
#endif
UREncodeResult *GuiGetZcashSignQrCodeData(void);
UREncodeResult *GuiGetZcashSignUrDataUnlimited(void);
void FreeZcashMemory(void);


#endif
