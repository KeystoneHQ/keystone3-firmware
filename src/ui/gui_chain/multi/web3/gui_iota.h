#ifndef _GUI_IOTA_H
#define _GUI_IOTA_H

void GuiSetIotaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetIotaData(void);
void *GuiGetIotaSignMessageHashData(void);
void GuiShowIotaSignMessageHashOverview(lv_obj_t *parent, void *totalData);
void GuiShowIotaSignMessageHashDetails(lv_obj_t *parent, void *totalData);
PtrT_TransactionCheckResult GuiGetIotaCheckResult(void);
PtrT_TransactionCheckResult GuiGetIotaSignHashCheckResult(void);
void FreeIotaMemory(void);
int GetIotaDetailLen(void *param);
void GetIotaDetail(void *indata, void *param, uint32_t maxLen);
UREncodeResult *GuiGetIotaSignQrCodeData(void);
UREncodeResult *GuiGetIotaSignHashQrCodeData(void);
void GuiIotaTxOverview(lv_obj_t *parent, void *totalData);
void GuiIotaTxRawData(lv_obj_t *parent, void *totalData);
bool GetIotaIsMessage(void *indata, void *param);
bool GetIotaIsTransaction(void *indata, void *param);
#endif
