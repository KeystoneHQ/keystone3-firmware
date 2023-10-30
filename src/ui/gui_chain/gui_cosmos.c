#include "gui_analyze.h"
#include "rust.h"
#include "keystore.h"
#include "version.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "cjson/cJSON.h"
#include "user_memory.h"
#include "account_manager.h"

static bool g_isMulti = false;
static void *g_urResult = NULL;
static void *g_parseResult = NULL;
static int8_t g_cosmosListIndex = -1;

static const CosmosChain_t g_cosmosChains[COSMOS_CHAINS_LEN] = {
    {CHAIN_ATOM, "cosmos", 118, XPUB_TYPE_COSMOS, "cosmoshub-4"},
    {CHAIN_OSMO, "osmo", 118, XPUB_TYPE_COSMOS, "osmosis-1"},
    {CHAIN_SCRT, "secret", 529, XPUB_TYPE_SCRT, "secret-4"},
    {CHAIN_AKT, "akash", 118, XPUB_TYPE_COSMOS, "akashnet-2"},
    {CHAIN_CRO, "cro", 394, XPUB_TYPE_CRO, "crypto-org-chain-mainnet-1"},
    {CHAIN_IOV, "star", 234, XPUB_TYPE_IOV, "iov-mainnet-ibc"},
    {CHAIN_ROWAN, "sif", 118, XPUB_TYPE_COSMOS, "sifchain-1"},
    {CHAIN_CTK, "shentu", 118, XPUB_TYPE_COSMOS, "shentu-2.2"},
    {CHAIN_IRIS, "iaa", 118, XPUB_TYPE_COSMOS, "irishub-1"},
    {CHAIN_REGEN, "regen", 118, XPUB_TYPE_COSMOS, "regen-1"},
    {CHAIN_XPRT, "persistence", 118, XPUB_TYPE_COSMOS, "core-1"},
    {CHAIN_DVPN, "sent", 118, XPUB_TYPE_COSMOS, "sentinelhub-2"},
    {CHAIN_IXO, "ixo", 118, XPUB_TYPE_COSMOS, "ixo-4"},
    {CHAIN_NGM, "emoney", 118, XPUB_TYPE_COSMOS, "emoney-3"},
    {CHAIN_BLD, "agoric", 564, XPUB_TYPE_BLD, "agoric-3"},
    {CHAIN_BOOT, "bostrom", 118, XPUB_TYPE_COSMOS, "bostrom"},
    {CHAIN_JUNO, "juno", 118, XPUB_TYPE_COSMOS, "juno-1"},
    {CHAIN_STARS, "stars", 118, XPUB_TYPE_COSMOS, "stargaze-1"},
    {CHAIN_AXL, "axelar", 118, XPUB_TYPE_COSMOS, "axelar-dojo-1"},
    {CHAIN_SOMM, "somm", 118, XPUB_TYPE_COSMOS, "sommelier-3"},
    {CHAIN_UMEE, "umee", 118, XPUB_TYPE_COSMOS, "umee-1"},
    {CHAIN_GRAV, "gravity", 118, XPUB_TYPE_COSMOS, "gravity-bridge-3"},
    {CHAIN_TGD, "tgrade", 118, XPUB_TYPE_COSMOS, "tgrade-mainnet-1"},
    {CHAIN_STRD, "stride", 118, XPUB_TYPE_COSMOS, "stride-1"},
    {CHAIN_EVMOS, "evmos", 60, XPUB_TYPE_ETH_BIP44_STANDARD, "evmos_9001-2"},
    {CHAIN_INJ, "inj", 60, XPUB_TYPE_ETH_BIP44_STANDARD, "injective-1"},
    {CHAIN_KAVA, "kava", 459, XPUB_TYPE_KAVA, "kava_2222-10"},
    {CHAIN_QCK, "quick", 118, XPUB_TYPE_COSMOS, "quicksilver-1"},
    {CHAIN_LUNA, "terra", 330, XPUB_TYPE_TERRA, "phoenix-1"},
    {CHAIN_LUNC, "terra", 330, XPUB_TYPE_TERRA, "columbus-5"},
};

const CosmosChain_t *GuiGetCosmosChain(uint8_t index)
{
    for (int i = 0; i < COSMOS_CHAINS_LEN; i++) {
        if (g_cosmosChains[i].index == index) {
            return &g_cosmosChains[i];
        }
    }
    return NULL;
}

bool IsCosmosChain(uint8_t index)
{
    return GuiGetCosmosChain(index) != NULL;
}

const char* GuiGetCosmosTxTypeName(CosmosMsgType type)
{
    switch (type) {
    case COSMOS_TX_SEND:
        return "Send";
    case COSMOS_TX_DELEGATE:
        return "Delegate";
    case COSMOS_TX_UNDELEGATE:
        return "Undelegate";
    case COSMOS_TX_REDELEGATE:
        return "Redelegate";
    case COSMOS_TX_WITHDRAW_REWARD:
        return "Withdraw Reward";
    case COSMOS_TX_IBC_TRANSFER:
        return "IBC Transfer";
    case COSMOS_TX_VOTE:
        return "Vote";
    case COSMOS_TX_MULTIPLE:
        return "Multiple";
    case COSMOS_MESSAGE:
        return "Message";
    default:
        return "Unknown";
    }
}

void GuiSetCosmosUrData(void *data, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = data;
    g_isMulti = multi;
#endif
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                   \
    if (result != NULL)                                                                                   \
    {                                                                                                     \
        free_TransactionParseResult_DisplayCosmosTx((PtrT_TransactionParseResult_DisplayCosmosTx)result); \
        result = NULL;                                                                                    \
    }

void *GuiGetCosmosData(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    GetMasterFingerPrint(mfp);
    TransactionCheckResult *result = NULL;
    do {
        URType urType = g_isMulti ? ((URParseMultiResult *)g_urResult)->ur_type : ((URParseResult *)g_urResult)->ur_type;
        result = cosmos_check_tx(data, urType, mfp, sizeof(mfp));
        CHECK_CHAIN_BREAK(result);
        PtrT_TransactionParseResult_DisplayCosmosTx parseResult = cosmos_parse_tx(data, urType);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    free_TransactionCheckResult(result);
    return g_parseResult;
#else
    return NULL;
#endif
}

PtrT_TransactionCheckResult GuiGetCosmosCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4];
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    GetMasterFingerPrint(mfp);
    URType urType = g_isMulti ? ((URParseMultiResult *)g_urResult)->ur_type : ((URParseResult *)g_urResult)->ur_type;
    return cosmos_check_tx(data, urType, mfp, sizeof(mfp));
#else
    return NULL;
#endif
}

void FreeCosmosMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, g_isMulti);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

void GuiGetCosmosTmpType(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNKNOWN)) == 0) {
        sprintf((char *)indata, "unknown");
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_MESSAGE)) == 0) {
        sprintf((char *)indata, "msg");
    } else {
        sprintf((char *)indata, "tx");
    }
}

bool IsCosmosMsg(ViewType viewType)
{
    if (viewType != CosmosTx && viewType != CosmosEvmTx) {
        return false;
    }
    DisplayCosmosTx *data = ((PtrT_TransactionParseResult_DisplayCosmosTx)g_parseResult)->data;
    return strcmp(data->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_MESSAGE)) == 0;
}

void GetCosmosValue(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_SEND)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->send_value);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_IBC_TRANSFER)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->transfer_value);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_DELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->delegate_value);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->undelegate_value);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->redelegate_value);
    } else {
        sprintf((char *)indata, "");
    }
}

void GetCosmosMethod(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    sprintf((char *)indata, "%s", tx->overview->method);
}

void GetCosmosProposal(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    // vote_proposal is a string like "#584", but one # is used for color, so need two # to display #.
    sprintf((char *)indata, "#%s", tx->overview->vote_proposal);
}

void GetCosmosVoted(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    sprintf((char *)indata, "%s", tx->overview->vote_voted);
}

void GetCosmosAddress1Value(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_SEND)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->send_from);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_IBC_TRANSFER)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->transfer_from);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_DELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->delegate_from);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->undelegate_validator);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->redelegate_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->withdraw_reward_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->vote_voter);
    } else {
        sprintf((char *)indata, "");
    }
}

void GetCosmosAddress1Label(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
        sprintf((char *)indata, "Validator");
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        sprintf((char *)indata, "Voter");
    } else if (
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0 ||
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0 ||
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0
    ) {
        sprintf((char *)indata, "To");
    } else {
        sprintf((char *)indata, "From");
    }
}

void GetCosmosAddress2Value(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_SEND)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->send_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_IBC_TRANSFER)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->transfer_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_DELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->delegate_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->undelegate_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->redelegate_new_validator);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        sprintf((char *)indata, "%s", tx->overview->withdraw_reward_validator);
    } else {
        sprintf((char *)indata, "");
    }
}

void GetCosmosAddress2Label(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        sprintf((char *)indata, "New Validator");
    } else if (
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0 ||
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0
    ) {
        sprintf((char *)indata, "Validator");
    } else {
        sprintf((char *)indata, "To");
    }
}

void GetCosmosDetailCommon(void *indata, void *param, const char* key)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    cJSON* root = cJSON_Parse((const char *)tx->detail);
    cJSON* common = cJSON_GetObjectItem(root, "common");
    if (common == NULL) {
        sprintf((char *)indata, "");
        return;
    }
    cJSON* value = cJSON_GetObjectItem(common, key);
    if (value == NULL) {
        sprintf((char *)indata, "");
    } else {
        sprintf((char *)indata, "%s", value->valuestring);
    }
}

void GetCosmosMaxFee(void *indata, void *param)
{
    GetCosmosDetailCommon(indata, param, "Max Fee");
}

void GetCosmosFee(void *indata, void *param)
{
    GetCosmosDetailCommon(indata, param, "Fee");
}

void GetCosmosNetwork(void *indata, void *param)
{
    GetCosmosDetailCommon(indata, param, "Network");
}

void GetCosmosGasLimit(void *indata, void *param)
{
    GetCosmosDetailCommon(indata, param, "Gas Limit");
}

void GetCosmosChainId(void *indata, void *param)
{
    GetCosmosDetailCommon(indata, param, "Chain ID");
}

void GetCosmosDetailNthKind(void *indata, void *param, int n, const char* key)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    cJSON* root = cJSON_Parse((const char *)tx->detail);
    cJSON* kind = cJSON_GetObjectItem(root, "kind");
    cJSON* item = cJSON_GetArrayItem(kind, n);
    cJSON* value = cJSON_GetObjectItem(item, key);
    // one # is used for color, two # is used for display #.
    sprintf((char *)indata, !strcmp(key, "Proposal") ? "#%s" : "%s", value->valuestring);
}

void GetCosmosChannel(void *indata, void *param)
{
    GetCosmosDetailNthKind(indata, param, 0, "Source Channel");
}

void GetCosmosOldValidator(void *indata, void *param)
{
    GetCosmosDetailNthKind(indata, param, 0, "Old Validator");
}

void GetCosmosMsgLen(uint8_t *len, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    cJSON* root = cJSON_Parse((const char *)tx->detail);
    cJSON* kind = cJSON_GetObjectItem(root, "kind");
    *len = (uint8_t)cJSON_GetArraySize(kind);
    g_cosmosListIndex = -1;
}

void GetCosmosMsgKey(void *indata, void *param)
{
    ++g_cosmosListIndex;
    GetCosmosDetailNthKind(indata, param, g_cosmosListIndex, "Method");
}

void GetCosmosIndex(void *indata, void *param)
{
    // one # is used for color, two # is used for display #.
    sprintf((char *)indata, "##%d", g_cosmosListIndex + 1);
}

void GetCosmosTextOfKind(void *indata, void *param)
{
    GetCosmosDetailNthKind(indata, param, g_cosmosListIndex, indata);
}

void GetCosmosDetailItemValue(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    cJSON* detail = cJSON_Parse((const char *)tx->detail);
    cJSON* value = cJSON_GetObjectItem(detail, indata);
    if (value == NULL) {
        sprintf((char *)indata, "");
    } else {
        sprintf((char *)indata, "%s", value->valuestring);
    }
}

bool GetCosmosMsgListExist(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    return strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_MULTIPLE)) == 0;
}

bool GetCosmosChannelExist(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    return strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_IBC_TRANSFER)) == 0;
}

bool GetCosmosOldValidatorExist(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    return strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0;
}

bool GetCosmosValueExist(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    return strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) != 0 &&
           strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) != 0 &&
           strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_MULTIPLE)) != 0;
}

bool GetCosmosVoteExist(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    return strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0;
}

bool GetCosmosMethodExist(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    return strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_MULTIPLE)) != 0;
}

bool GetCosmosAddrExist(void *indata, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    return strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_MULTIPLE)) != 0;
}

bool GetCosmosAddress2Exist(void *indata, void *param)
{
    return !GetCosmosVoteExist(indata, param);
}

void GetCosmosOverviewAddrSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        *height = 130;
    } else {
        *height = 244;
    }
    *width = 408;
}

void GetCosmosDetailMsgSize(uint16_t *width, uint16_t *height, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_IBC_TRANSFER)) == 0) {
        *height = 382;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        *height = 442;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        *height = 268;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        *height = 290;
    } else {
        *height = 336;
    }
    *width = 408;
}

void GetCosmosDetailMethodLabelPos(uint16_t *x, uint16_t *y, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        *y = 16;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        *y = 108;
    } else {
        *y = 62;
    }
    *x = 24;
}

void GetCosmosDetailMethodValuePos(uint16_t *x, uint16_t *y, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        *y = 16;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        *y = 108;
    } else {
        *y = 62;
    }
    *x = 120;
}

void GetCosmosDetailAddress1LabelPos(uint16_t *x, uint16_t *y, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        *y = 62;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        *y = 154;
    } else {
        *y = 108;
    }
    *x = 24;
}

void GetCosmosDetailAddress1ValuePos(uint16_t *x, uint16_t *y, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        *y = 108;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        *y = 192;
    } else {
        *y = 154;
    }
    *x = 24;
}

void GetCosmosDetailAddress2LabelPos(uint16_t *x, uint16_t *y, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        *y = 328;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        *y = 176;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        *y = 268;
    } else {
        *y = 222;
    }
    *x = 24;
}

void GetCosmosDetailAddress2ValuePos(uint16_t *x, uint16_t *y, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        *y = 366;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        *y = 214;
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        *y = 308;
    } else {
        *y = 260;
    }
    *x = 24;
}

uint8_t GuiGetCosmosTxChain(void)
{
    PtrT_TransactionParseResult_DisplayCosmosTx parseResult = (PtrT_TransactionParseResult_DisplayCosmosTx)GuiGetCosmosData();
    if (parseResult == NULL) {
        return CHAIN_ATOM;
    }
    char* chain_id = SRAM_MALLOC(100);
    if (strcmp(parseResult->data->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_MESSAGE)) == 0 || strcmp(parseResult->data->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNKNOWN)) == 0) {
        cJSON* detail = cJSON_Parse(parseResult->data->detail);
        cJSON* value = cJSON_GetObjectItem(detail, "Chain ID");
        chain_id = value->valuestring;
    } else {
        GetCosmosDetailCommon(chain_id, parseResult->data, "Chain ID");
    }
    if (chain_id != NULL) {
        for (uint8_t i = 0; i < COSMOS_CHAINS_LEN; i++) {
            if (strcmp(chain_id, g_cosmosChains[i].chainId) == 0) {
                return g_cosmosChains[i].index;
            }
        }
        if (strcmp(chain_id, "evmos_9000-4") == 0) {
            return CHAIN_EVMOS;
        }
    }
    SRAM_FREE(chain_id);
    return CHAIN_ATOM;
}

UREncodeResult *GuiGetCosmosSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
    URType urType = g_isMulti ? ((URParseMultiResult *)g_urResult)->ur_type : ((URParseResult *)g_urResult)->ur_type;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = cosmos_sign_tx(data, urType, seed, sizeof(seed));
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
#else
    UREncodeResult *encodeResult = NULL;
    encodeResult->is_multi_part = 0;
    encodeResult->data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    encodeResult->encoder = NULL;
    encodeResult->error_code = 0;
    encodeResult->error_message = NULL;
    return encodeResult;
#endif
}
