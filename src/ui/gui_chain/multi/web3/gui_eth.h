#ifndef _GUI_ETH_H
#define _GUI_ETH_H

#include "rust.h"
#include "lvgl.h"

void GuiSetEthUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetEthData(void);
PtrT_TransactionCheckResult GuiGetEthCheckResult(void);
void GetEthValue(void *indata, void *param, uint32_t maxLen);
void GetEthTxFee(void *indata, void *param, uint32_t maxLen);
void GetEthGasPrice(void *indata, void *param, uint32_t maxLen);
void GetEthGasLimit(void *indata, void *param, uint32_t maxLen);

void GetEthNetWork(void *indata, void *param, uint32_t maxLen);

void GetEthTransType(void *indata, void *param, uint32_t maxLen);

void GetEthMaxFee(void *indata, void *param, uint32_t maxLen);
void GetEthMaxPriority(void *indata, void *param, uint32_t maxLen);
void GetEthMaxFeePrice(void *indata, void *param, uint32_t maxLen);
void GetEthMaxPriorityFeePrice(void *indata, void *param, uint32_t maxLen);

UREncodeResult *GuiGetEthSignQrCodeData(void);
UREncodeResult *GuiGetEthSignUrDataUnlimited(void);
void GetEthGetFromAddress(void *indata, void *param, uint32_t maxLen);
void GetEthGetToAddress(void *indata, void *param, uint32_t maxLen);
void GetEthGetDetailPageToAddress(void *indata, void *param, uint32_t maxLen);
void GetTxnFeeDesc(void *indata, void *param, uint32_t maxLen);
void GetEthToFromSize(uint16_t *width, uint16_t *height, void *param);
void GetEthToLabelPos(uint16_t *x, uint16_t *y, void *param);
void GetEthTypeDomainPos(uint16_t *x, uint16_t *y, void *param);
void GetEthEnsName(void *indata, void *param, uint32_t maxLen);
void GetToEthEnsName(void *indata, void *param, uint32_t maxLen);
void GetEthInputData(void *indata, void *param, uint32_t maxLen);
void GetEthNonce(void *indata, void *param, uint32_t maxLen);
int GetEthInputDataLen(void *param);
bool GetEthTypeDataHashExist(void *indata, void *param);
bool GetEthContractFromInternal(char *address, char *inputData);
bool GetEthTypeDataChainExist(void *indata, void *param);
bool GetEthTypeDataVersionExist(void *indata, void *param);
bool GetEthContractFromExternal(char *address, char *selectorId, uint64_t chainId, char *inputData);
void GetEthMethodName(void *indata, void *param, uint32_t maxLen);
void GetEthContractName(void *indata, void *param, uint32_t maxLen);
void GetEthTransactionData(void *indata, void *param, uint32_t maxLen);
bool GetEthEnsExist(void *indata, void *param);
bool GetToEthEnsExist(void *indata, void *param);
bool GetEthContractDataExist(void *indata, void *param);
bool GetEthContractDataNotExist(void *indata, void *param);
void GetEthGetSignerAddress(void *indata, void *param, uint32_t maxLen);
void GetEthContractDataSize(uint16_t *width, uint16_t *height, void *param);
void GetEthTypeDomainSize(uint16_t *width, uint16_t *height, void *param);
void *GetEthContractData(uint8_t *row, uint8_t *col, void *param);
bool GetEthInputDataExist(void *indata, void *param);
bool EthInputExistContractNot(void *indata, void *param);
bool GetEthPermitWarningExist(void *indata, void *param);
bool GetEthPermitCantSign(void *indata, void *param);
bool GetEthOperationWarningExist(void *indata, void *param);
void *GuiGetEthPersonalMessage(void);
void GetEthPersonalMessageType(void *indata, void *param, uint32_t maxLen);
void GetMessageFrom(void *indata, void *param, uint32_t maxLen);
void GetMessageUtf8(void *indata, void *param, uint32_t maxLen);
void GetMessageRaw(void *indata, void *param, uint32_t maxLen);
void EthContractCheckRawDataCallback(void);

void *GuiGetEthTypeData(void);
void GetEthTypedDataDomianName(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataDomainHash(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataMessageHash(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataSafeTxHash(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataDomianVersion(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataDomianChainId(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataDomianVerifyContract(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataDomianSalt(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataPrimayType(void *indata, void *param, uint32_t maxLen);
void GetEthTypedDataMessage(void *indata, void *param, uint32_t maxLen);
int GetEthTypedDataMessageLen(void *param);
void GetEthTypedDataFrom(void *indata, void *param, uint32_t maxLen);
void EthContractLearnMore(lv_event_t *e);
void EthContractCheckRawData(lv_event_t *e);

typedef struct {
    uint64_t chainId;
    char *name;
    char *symbol;
} EvmNetwork_t;

typedef struct {
    char *symbol;
    char *contract_address;
    uint8_t decimals;
} Erc20Contract_t;

typedef struct {
    char *recipient;
    char *value;
} Erc20Transfer_t;

EvmNetwork_t FindEvmNetwork(uint64_t chainId);
void *FindErc20Contract(char *contract_address);


void FreeEthMemory(void);

#endif