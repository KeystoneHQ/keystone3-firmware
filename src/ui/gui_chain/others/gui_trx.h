#include "rust.h"

void GuiSetTrxUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetTrxData(void);
PtrT_TransactionCheckResult GuiGetTrxCheckResult(void);
void FreeTrxMemory(void);
void GetTrxValue(void *indata, void *param, uint32_t maxLen);
void GetTrxMethod(void *indata, void *param, uint32_t maxLen);
void GetTrxFromAddress(void *indata, void *param, uint32_t maxLen);
void GetTrxToAddress(void *indata, void *param, uint32_t maxLen);
bool GetTrxContractExist(void *indata, void *param);
void GetTrxContract(void *indata, void *param, uint32_t maxLen);
bool GetTrxTokenExist(void *indata, void *param);
void GetTrxToken(void *indata, void *param, uint32_t maxLen);
UREncodeResult *GuiGetTrxSignQrCodeData(void);
