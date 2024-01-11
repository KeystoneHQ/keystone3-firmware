#include "rust.h"

void GuiSetTrxUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetTrxData(void);
PtrT_TransactionCheckResult GuiGetTrxCheckResult(void);
void FreeTrxMemory(void);
void GetTrxValue(void *indata, void *param);
void GetTrxMethod(void *indata, void *param);
void GetTrxFromAddress(void *indata, void *param);
void GetTrxToAddress(void *indata, void *param);
bool GetTrxContractExist(void *indata, void *param);
void GetTrxContract(void *indata, void *param);
bool GetTrxTokenExist(void *indata, void *param);
void GetTrxToken(void *indata, void *param);
UREncodeResult *GuiGetTrxSignQrCodeData(void);
