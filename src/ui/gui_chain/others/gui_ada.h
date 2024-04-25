#ifndef _GUI_ADA_H_
#define _GUI_ADA_H_

#include "stdlib.h"
#include "stdint.h"
#include "librust_c.h"

void GuiSetupAdaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetAdaData(void);
PtrT_TransactionCheckResult GuiGetAdaCheckResult(void);
void GetAdaNetwork(void *indata, void *param, uint32_t maxLen);
void GetAdaTotalInput(void *indata, void *param, uint32_t maxLen);
void GetAdaTotalOutput(void *indata, void *param, uint32_t maxLen);
void GetAdaFee(void *indata, void *param, uint32_t maxLen);
void GetAdaWithdrawalsLabel(void *indata, void *param, uint32_t maxLen);
void GetAdaCertificatesLabel(void *indata, void *param, uint32_t maxLen);

void *GetAdaInputDetail(uint8_t *row, uint8_t *col, void *param);
void GetAdaInputDetailSize(uint16_t *width, uint16_t *height, void *param);

void *GetAdaOutputDetail(uint8_t *row, uint8_t *col, void *param);
void GetAdaOutputDetailSize(uint16_t *width, uint16_t *height, void *param);

bool GetAdaCertificatesExist(void *indata, void *param);
void GetAdaCertificatesSize(uint16_t *width, uint16_t *height, void *param);
void *GetAdaCertificatesData(uint8_t *row, uint8_t *col, void *param);

bool GetAdaWithdrawalsExist(void *indata, void *param);
void GetAdaWithdrawalsSize(uint16_t *width, uint16_t *height, void *param);
void *GetAdaWithdrawalsData(uint8_t *row, uint8_t *col, void *param);

bool GetAdaExtraDataExist(void *indata, void *param);
void GetAdaExtraData(void *indata, void *param, uint32_t maxLen);
int GetAdaExtraDataLen(void *param);

void FreeAdaMemory(void);

UREncodeResult *GuiGetAdaSignQrCodeData(void);

#endif