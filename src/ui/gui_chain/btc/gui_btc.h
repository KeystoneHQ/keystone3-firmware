#ifndef _GUI_BTC_H
#define _GUI_BTC_H

#include "rust.h"

UREncodeResult *GuiGetSignQrCodeData(void);
void GuiSetPsbtUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void FreePsbtUxtoMemory(void);
void FreeBtcMsgMemory(void);
void *GuiGetParsedQrData(void);
PtrT_TransactionCheckResult GuiGetPsbtCheckResult(void);
void GetPsbtTotalOutAmount(void *indata, void *param, uint32_t maxLen);
void GetPsbtFeeAmount(void *indata, void *param, uint32_t maxLen);
void GetPsbtTotalOutSat(void *indata, void *param, uint32_t maxLen);
void GetPsbtFeeSat(void *indata, void *param, uint32_t maxLen);

void GetPsbtNetWork(void *indata, void *param, uint32_t maxLen);

void GetPsbtDetailInputValue(void *indata, void *param, uint32_t maxLen);
void GetPsbtDetailOutputValue(void *indata, void *param, uint32_t maxLen);
void GetPsbtDetailFee(void *indata, void *param, uint32_t maxLen);

void *GetPsbtInputData(uint8_t *row, uint8_t *col, void *param);
void *GetPsbtOutputData(uint8_t *row, uint8_t *col, void *param);
void *GetPsbtInputDetailData(uint8_t *row, uint8_t *col, void *param);
void *GetPsbtOutputDetailData(uint8_t *row, uint8_t *col, void *param);

void GetPsbtOverviewSize(uint16_t *width, uint16_t *height, void *param);
void GetPsbtDetailSize(uint16_t *width, uint16_t *height, void *param);

int GetBtcMsgDetailLen(void *param);
void GetBtcMsgDetail(void *indata, void *param, uint32_t maxLen);
void GuiBtcMsg(lv_obj_t *parent, void *totalData);

void GuiBtcTxOverview(lv_obj_t *parent, void *g_totalData);
void GuiBtcTxDetail(lv_obj_t *parent, void *g_totalData);
void GuiSetPsbtStrData(char *psbtBytes, uint32_t psbtBytesLen);

#endif