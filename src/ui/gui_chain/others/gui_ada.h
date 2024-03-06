#ifndef _GUI_ADA_H_
#define _GUI_ADA_H_

#include "stdlib.h"
#include "stdint.h"
#include "librust_c.h"

void GuiSetupAdaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetAdaData(void);
PtrT_TransactionCheckResult GuiGetAdaCheckResult(void);
void GetAdaNetwork(void *indata, void *param);
void GetAdaTotalInput(void *indata, void *param);
void GetAdaTotalOutput(void *indata, void *param);
void GetAdaFee(void *indata, void *param);
void GetAdaWithdrawalsLabel(void *indata, void *param);
void GetAdaCertificatesLabel(void *indata, void *param);

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
void GetAdaExtraData(void *data, void *param);
int GetAdaExtraDataLen(void *param);

char *GuiGetYoroiBaseAddressByIndex(uint16_t index);

void FreeAdaMemory(void);

UREncodeResult *GuiGetAdaSignQrCodeData(void);

#endif