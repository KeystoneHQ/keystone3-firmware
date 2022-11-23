#ifndef _GUI_BTC_H
#define _GUI_BTC_H

#include "rust.h"

UREncodeResult *GuiGetSignQrCodeData(void);
void GuiSetPsbtUrData(void *data, bool multi);
void FreePsbtUxtoMemory(void);
void *GuiGetParsedQrData(void);
void GetPsbtTotalOutAmount(void *indata, void *param);
void GetPsbtFeeAmount(void *indata, void *param);
void GetPsbtTotalOutSat(void *indata, void *param);
void GetPsbtFeeSat(void *indata, void *param);

void GetPsbtFeeValue(void *indata, void *param);
void GetPsbtNetWork(void *indata, void *param);
void GetPsbtHashID(void *indata, void *param);

void GetPsbtDetailInputValue(void *indata, void *param);
void GetPsbtDetailOutputValue(void *indata, void *param);
void GetPsbtDetailFee(void *indata, void *param);

void *GetPsbtInputData(uint8_t *row, uint8_t *col, void *param);
void *GetPsbtOutputData(uint8_t *row, uint8_t *col, void *param);
void *GetPsbtInputDetailData(uint8_t *row, uint8_t *col, void *param);
void *GetPsbtOutputDetailData(uint8_t *row, uint8_t *col, void *param);

void GetPsbtOverviewSize(uint16_t *width, uint16_t *height, void *param);
void GetPsbtDetailSize(uint16_t *width, uint16_t *height, void *param);

#endif