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
bool GetTrxMemoExist(void *indata, void *param);
bool GetTrxMemoBytesExist(void *indata, void *param);
void GetTrxToken(void *indata, void *param, uint32_t maxLen);
void GetTrxSwapDstAsset(void *indata, void *param, uint32_t maxLen);
void GetTrxSwapDstAddress(void *indata, void *param, uint32_t maxLen);
void GetTrxNetwork(void *indata, void *param, uint32_t maxLen);
void TrxCheckVault(lv_event_t *e);
void GetTrxExpiration(void *indata, void *param, uint32_t maxLen);
void GetTrxValueRaw(void *indata, void *param, uint32_t maxLen);
void GetTrxMemo(void *indata, void *param, uint32_t maxLen);
void GetTrxMemoHex(void *indata, void *param, uint32_t maxLen);
const char *GuiGetTrxMemoText(void *param);
const char *GuiGetTrxMemoHexText(void *param);
int GetTrxMemoLen(void *param);
int GetTrxMemoHexLen(void *param);
void GetTrxMemoHash(void *indata, void *param, uint32_t maxLen);
void GetTrxFeeLimit(void *indata, void *param, uint32_t maxLen);
void GetTrxTimestamp(void *indata, void *param, uint32_t maxLen);
void GetTrxRefBlockNum(void *indata, void *param, uint32_t maxLen);
void GetTrxRefBlockBytes(void *indata, void *param, uint32_t maxLen);
void GetTrxRefBlockHash(void *indata, void *param, uint32_t maxLen);
void GetTrxPermissionId(void *indata, void *param, uint32_t maxLen);
void GetTrxRawDataHash(void *indata, void *param, uint32_t maxLen);

UREncodeResult *GuiGetTrxSignQrCodeData(void);
UREncodeResult *GuiGetTrxSignUrDataUnlimited(void);

void *GuiGetTrxPersonalMessage(void);
void GetTrxPersonalMessageType(void *indata, void *param, uint32_t maxLen);
void GetTrxMessageFrom(void *indata, void *param, uint32_t maxLen);
void GetTrxMessageUtf8(void *indata, void *param, uint32_t maxLen);
void GetTrxMessageRaw(void *indata, void *param, uint32_t maxLen);
void GuiShowTrxMessagePaged(lv_obj_t *parent, void *param, bool raw);
bool GetTrxMessageFromExist(void *indata, void *param);
bool GetTrxMessageFromNotExist(void *indata, void *param);
void GetTrxMessagePos(uint16_t *x, uint16_t *y, void *param);

void GetTrxCallValue(void *indata, void *param, uint32_t maxLen);
void GetTrxCallTokenValue(void *indata, void *param, uint32_t maxLen);
void GetTrxTokenId(void *indata, void *param, uint32_t maxLen);
void GetTrxCallData(void *indata, void *param, uint32_t maxLen);
const char *GuiGetTrxCallDataText(void *param);
void GetTrxContractWarning(void *indata, void *param, uint32_t maxLen);
bool GetTrxAttachedTokenExist(void *indata, void *param);
bool GetTrxContractWarningExist(void *indata, void *param);
int GetTrxCallDataLen(void *param);
