#include "rust.h"
#include "account_public_info.h"

#define COSMOS_CHAINS_LEN 32

typedef struct {
    uint8_t index;
    const char *prefix;
    int16_t coinType;
    ChainType xpubType;
    const char *chainId;
} CosmosChain_t;

typedef enum {
    COSMOS_TX_SEND,
    COSMOS_TX_DELEGATE,
    COSMOS_TX_UNDELEGATE,
    COSMOS_TX_REDELEGATE,
    COSMOS_TX_WITHDRAW_REWARD,
    COSMOS_TX_IBC_TRANSFER,
    COSMOS_TX_VOTE,
    COSMOS_TX_MULTIPLE,
    COSMOS_TX_UNKNOWN,
    COSMOS_MESSAGE,
} CosmosMsgType;

const char* GuiGetCosmosTxTypeName(CosmosMsgType type);
void GuiSetCosmosUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi);
void *GuiGetCosmosData(void);
PtrT_TransactionCheckResult GuiGetCosmosCheckResult(void);
void FreeCosmosMemory(void);
void GuiGetCosmosTmpType(void *indata, void *param, uint32_t maxLen);
UREncodeResult *GuiGetCosmosSignQrCodeData(void);
const CosmosChain_t *GuiGetCosmosChain(uint8_t index);
bool IsCosmosChain(uint8_t index);
uint8_t GuiGetCosmosTxChain(void);
void GetCosmosValue(void *indata, void *param, uint32_t maxLen);
void GetCosmosNetwork(void *indata, void *param, uint32_t maxLen);
void GetCosmosMethod(void *indata, void *param, uint32_t maxLen);
void GetCosmosAddress1Value(void *indata, void *param, uint32_t maxLen);
void GetCosmosAddress1Label(void *indata, void *param, uint32_t maxLen);
void GetCosmosAddress2Value(void *indata, void *param, uint32_t maxLen);
void GetCosmosAddress2Label(void *indata, void *param, uint32_t maxLen);
void GetCosmosMaxFee(void *indata, void *param, uint32_t maxLen);
void GetCosmosFee(void *indata, void *param, uint32_t maxLen);
void GetCosmosGasLimit(void *indata, void *param, uint32_t maxLen);
void GetCosmosChainId(void *indata, void *param, uint32_t maxLen);
void GetCosmosChannel(void *indata, void *param, uint32_t maxLen);
bool GetCosmosChannelExist(void *indata, void *param);
void GetCosmosDetailMsgSize(uint16_t *width, uint16_t *height, void *param);
void GetCosmosOldValidator(void *indata, void *param, uint32_t maxLen);
bool GetCosmosOldValidatorExist(void *indata, void *param);
void GetCosmosDetailAddress2LabelPos(uint16_t *x, uint16_t *y, void *param);
void GetCosmosDetailAddress2ValuePos(uint16_t *x, uint16_t *y, void *param);
bool GetCosmosValueExist(void *indata, void *param);
void GetCosmosDetailMethodLabelPos(uint16_t *x, uint16_t *y, void *param);
void GetCosmosDetailMethodValuePos(uint16_t *x, uint16_t *y, void *param);
void GetCosmosDetailAddress1LabelPos(uint16_t *x, uint16_t *y, void *param);
void GetCosmosDetailAddress1ValuePos(uint16_t *x, uint16_t *y, void *param);
bool GetCosmosVoteExist(void *indata, void *param);
void GetCosmosProposal(void *indata, void *param, uint32_t maxLen);
void GetCosmosVoted(void *indata, void *param, uint32_t maxLen);
void GetCosmosOverviewAddrSize(uint16_t *width, uint16_t *height, void *param);
bool GetCosmosAddress2Exist(void *indata, void *param);
void GetCosmosMsgLen(uint8_t *len, void *param);
void GetCosmosMsgKey(void *indata, void *param, uint32_t maxLen);
void GetCosmosIndex(void *indata, void *param, uint32_t maxLen);
bool GetCosmosMsgListExist(void *indata, void *param);
bool GetCosmosMethodExist(void *indata, void *param);
bool GetCosmosAddrExist(void *indata, void *param);
void GetCosmosTextOfKind(void *indata, void *param, uint32_t maxLen);
void GetCosmosDetailItemValue(void *indata, void *param, uint32_t maxLen);
bool IsCosmosMsg(ViewType viewType);
