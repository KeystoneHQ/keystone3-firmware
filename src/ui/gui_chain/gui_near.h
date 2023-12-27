#ifndef _GUI_NEAR_H
#define _GUI_NEAR_H

char *GuiGetNearPubkey(uint8_t pathType, uint16_t index);
void GuiSetNearUrData(void *data, bool multi);
PtrT_TransactionCheckResult GuiGetNearCheckResult(void);
void FreeNearMemory(void);
UREncodeResult *GuiGetNearSignQrCodeData(void);
void *GuiGetNearData(void);
void GuiShowNearTxOverview(lv_obj_t *parent, void *totalData);
void GuiShowNearTxDetail(lv_obj_t *parent, void *totalData);

#endif
