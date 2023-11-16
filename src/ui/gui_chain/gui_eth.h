#ifndef _GUI_ETH_H
#define _GUI_ETH_H

#include "rust.h"
#include "lvgl.h"

void GuiSetEthUrData(void *data, bool multi);
void *GuiGetEthData(void);
PtrT_TransactionCheckResult GuiGetEthCheckResult(void);
void GetEthValue(void *indata, void *param);
void GetEthTxFee(void *indata, void *param);
void GetEthGasPrice(void *indata, void *param);
void GetEthGasLimit(void *indata, void *param);

void GetEthNetWork(void *indata, void *param);

void GetEthTransType(void *indata, void *param);

void GetEthMaxFee(void *indata, void *param);
void GetEthMaxPriority(void *indata, void *param);
void GetEthMaxFeePrice(void *indata, void *param);
void GetEthMaxPriorityFeePrice(void *indata, void *param);

UREncodeResult *GuiGetEthSignQrCodeData(void);
UREncodeResult *GuiGetEthSignUrDataUnlimited(void);
void GetEthGetFromAddress(void *indata, void *param);
void GetEthGetToAddress(void *indata, void *param);
void GetTxnFeeDesc(void *indata, void *param);
void GetEthToFromSize(uint16_t *width, uint16_t *height, void *param);
void GetEthToLabelPos(uint16_t *x, uint16_t *y, void *param);
void GetEthEnsName(void *indata, void *param);
void GetToEthEnsName(void *indata, void *param);
bool GetEthContractFromInternal(char *address, char* inputData);
bool GetEthContractFromExternal(char *address, char *selectorId, uint64_t chainId, char* inputData);
void GetEthMethodName(void *indata, void *param);
void GetEthContractName(void *indata, void *param);
void GetEthTransactionData(void *indata, void *param);
bool GetEthEnsExist(void *indata, void *param);
bool GetToEthEnsExist(void *indata, void *param);
bool GetEthContractDataExist(void *indata, void *param);
bool GetEthContractDataNotExist(void *indata, void *param);
void GetEthContractDataSize(uint16_t *width, uint16_t *height, void *param);
void *GetEthContractData(uint8_t *row, uint8_t *col, void *param);
bool GetEthInputDataExist(void *indata, void *param);
void *GuiGetEthPersonalMessage(void);
void GetEthPersonalMessageType(void *indata, void *param);
void GetMessageFrom(void *indata, void *param);
void GetMessageUtf8(void *indata, void *param);
void GetMessageRaw(void *indata, void *param);

void *GuiGetEthTypeData(void);
void GetEthTypedDataDomianName(void *indata, void *param);
void GetEthTypedDataDomianVersion(void *indata, void *param);
void GetEthTypedDataDomianChainId(void *indata, void *param);
void GetEthTypedDataDomianVerifyContract(void *indata, void *param);
void GetEthTypedDataDomianSalt(void *indata, void *param);
void GetEthTypedDataPrimayType(void *indata, void *param);
void GetEthTypedDataMessage(void *indata, void *param);
void GetEthTypedDataFrom(void *indata, void *param);
void EthContractLearnMore(lv_event_t *e);
typedef struct {
    uint64_t chainId;
    char *name;
    char *symbol;
} EvmNetwork_t;

EvmNetwork_t _FindNetwork(uint64_t chainId);

void FreeEthMemory(void);

#endif