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
void GetTrxSwapDstAsset(void *indata, void *param, uint32_t maxLen);
void GetTrxSwapDstAddress(void *indata, void *param, uint32_t maxLen);
void GetTrxNetwork(void *indata, void *param, uint32_t maxLen);
void TrxCheckVault(lv_event_t *e);
void GetTrxExpiration(void *indata, void *param, uint32_t maxLen);
void GetTrxValueRaw(void *indata, void *param, uint32_t maxLen);
void GetTrxMemo(void *indata, void *param, uint32_t maxLen);

UREncodeResult *GuiGetTrxSignQrCodeData(void);
UREncodeResult *GuiGetTrxSignUrDataUnlimited(void);

void *GuiGetTrxPersonalMessage(void);
void GetTrxPersonalMessageType(void *indata, void *param, uint32_t maxLen);
void GetTrxMessageFrom(void *indata, void *param, uint32_t maxLen);
void GetTrxMessageUtf8(void *indata, void *param, uint32_t maxLen);
void GetTrxMessageRaw(void *indata, void *param, uint32_t maxLen);
bool GetTrxMessageFromExist(void *indata, void *param);
bool GetTrxMessageFromNotExist(void *indata, void *param);
void GetTrxMessagePos(uint16_t *x, uint16_t *y, void *param);

