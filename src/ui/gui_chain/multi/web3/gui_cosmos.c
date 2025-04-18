#include "gui_analyze.h"
#include "rust.h"
#include "keystore.h"
#include "version.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "cjson/cJSON.h"
#include "user_memory.h"
#include "account_manager.h"
#include "gui_chain.h"
#define MAX_COSMOS_ADDR_LEN 61

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static int8_t g_cosmosListIndex = -1;
static char g_cosmosAddr[MAX_COSMOS_ADDR_LEN];
static char g_hdPath[26];
static const CosmosChain_t g_cosmosChains[COSMOS_CHAINS_LEN] = {
    {CHAIN_BABYLON, HOME_WALLET_CARD_BABYLON, 118, "bbn", XPUB_TYPE_COSMOS, "baby_3535-1"},
    {CHAIN_NEUTARO, HOME_WALLET_CARD_NEUTARO, 118, "neutaro", XPUB_TYPE_COSMOS, "Neutaro-1"},
    {CHAIN_TIA, HOME_WALLET_CARD_TIA, 118, "celestia", XPUB_TYPE_COSMOS, "celestia"},
    {CHAIN_NTRN, HOME_WALLET_CARD_NTRN, 118, "neutron", XPUB_TYPE_COSMOS, "neutron-1"},
    {CHAIN_DYM, HOME_WALLET_CARD_DYM, 118, "dym", XPUB_TYPE_ETH_BIP44_STANDARD, "dymension_1100-1"},
    {CHAIN_ATOM, HOME_WALLET_CARD_ATOM, 118, "cosmos", XPUB_TYPE_COSMOS, "cosmoshub-4"},
    {CHAIN_OSMO, HOME_WALLET_CARD_OSMO, 118, "osmo", XPUB_TYPE_COSMOS, "osmosis-1"},
    {CHAIN_SCRT, HOME_WALLET_CARD_SCRT, 529, "secret", XPUB_TYPE_SCRT, "secret-4"},
    {CHAIN_AKT, HOME_WALLET_CARD_AKT, 118, "akash", XPUB_TYPE_COSMOS, "akashnet-2"},
    {CHAIN_CRO, HOME_WALLET_CARD_CRO, 394, "cro", XPUB_TYPE_CRO, "crypto-org-chain-mainnet-1"},
    {CHAIN_RUNE, HOME_WALLET_CARD_RUNE, 931, "thor", XPUB_TYPE_THOR, "thorchain-1"},
    {CHAIN_IOV, HOME_WALLET_CARD_IOV, 234, "star", XPUB_TYPE_IOV, "iov-mainnet-ibc"},
    {CHAIN_ROWAN, HOME_WALLET_CARD_ROWAN, 118, "sif", XPUB_TYPE_COSMOS, "sifchain-1"},
    {CHAIN_CTK, HOME_WALLET_CARD_CTK, 118, "shentu", XPUB_TYPE_COSMOS, "shentu-2.2"},
    {CHAIN_IRIS, HOME_WALLET_CARD_IRIS, 118, "iaa", XPUB_TYPE_COSMOS, "irishub-1"},
    {CHAIN_REGEN, HOME_WALLET_CARD_REGEN, 118, "regen", XPUB_TYPE_COSMOS, "regen-1"},
    {CHAIN_XPRT, HOME_WALLET_CARD_XPRT, 118, "persistence", XPUB_TYPE_COSMOS, "core-1"},
    {CHAIN_DVPN, HOME_WALLET_CARD_DVPN, 118, "sent", XPUB_TYPE_COSMOS, "sentinelhub-2"},
    {CHAIN_IXO, HOME_WALLET_CARD_IXO, 118, "ixo", XPUB_TYPE_COSMOS, "ixo-4"},
    {CHAIN_NGM, HOME_WALLET_CARD_NGM, 118, "emoney", XPUB_TYPE_COSMOS, "emoney-3"},
    {CHAIN_BLD, HOME_WALLET_CARD_BLD, 564, "agoric", XPUB_TYPE_BLD, "agoric-3"},
    {CHAIN_BOOT, HOME_WALLET_CARD_BOOT, 118, "bostrom", XPUB_TYPE_COSMOS, "bostrom"},
    {CHAIN_JUNO, HOME_WALLET_CARD_JUNO, 118, "juno", XPUB_TYPE_COSMOS, "juno-1"},
    {CHAIN_STARS, HOME_WALLET_CARD_STARS, 118, "stars", XPUB_TYPE_COSMOS, "stargaze-1"},
    {CHAIN_AXL, HOME_WALLET_CARD_AXL, 118, "axelar", XPUB_TYPE_COSMOS, "axelar-dojo-1"},
    {CHAIN_SOMM, HOME_WALLET_CARD_SOMM, 118, "somm", XPUB_TYPE_COSMOS, "sommelier-3"},
    {CHAIN_UMEE, HOME_WALLET_CARD_UMEE, 118, "umee", XPUB_TYPE_COSMOS, "umee-1"},
    {CHAIN_GRAV, HOME_WALLET_CARD_GRAV, 118, "gravity", XPUB_TYPE_COSMOS, "gravity-bridge-3"},
    {CHAIN_TGD, HOME_WALLET_CARD_TGD, 118, "tgrade", XPUB_TYPE_COSMOS, "tgrade-mainnet-1"},
    {CHAIN_STRD, HOME_WALLET_CARD_STRD, 118, "stride", XPUB_TYPE_COSMOS, "stride-1"},
    {CHAIN_EVMOS, HOME_WALLET_CARD_EVMOS, 60, "evmos", XPUB_TYPE_ETH_BIP44_STANDARD, "evmos_9001-2"},
    {CHAIN_INJ, HOME_WALLET_CARD_INJ, 60, "inj", XPUB_TYPE_ETH_BIP44_STANDARD, "injective-1"},
    {CHAIN_KAVA, HOME_WALLET_CARD_KAVA, 459, "kava", XPUB_TYPE_KAVA, "kava_2222-10"},
    {CHAIN_QCK, HOME_WALLET_CARD_QCK, 118, "quick", XPUB_TYPE_COSMOS, "quicksilver-1"},
    {CHAIN_LUNA, HOME_WALLET_CARD_LUNA, 330, "terra", XPUB_TYPE_TERRA, "phoenix-1"},
    {CHAIN_LUNC, HOME_WALLET_CARD_LUNC, 330, "terra", XPUB_TYPE_TERRA, "columbus-5"}
};

char *GetCosmosChainAddressByCoinTypeAndIndex(uint8_t chainType,  uint32_t address_index)
{
    char *xPub;
    char rootPath[BUFFER_SIZE_32];
    char hdPath[BUFFER_SIZE_32];
    const CosmosChain_t *chain = GuiGetCosmosChain(chainType);
    snprintf_s(rootPath, BUFFER_SIZE_32, "M/44'/%u'/0'", chain->coinType);
    snprintf_s(hdPath, BUFFER_SIZE_32, "%s/0/%u", rootPath, address_index);
    xPub = GetCurrentAccountPublicKey(chain->xpubType);
    return (char *) cosmos_get_address(hdPath, xPub, rootPath, (char*)chain->prefix);
}

char *GetKeplrConnectionDisplayAddressByIndex(uint32_t index)
{
    SimpleResponse_c_char *result;
    result = (SimpleResponse_c_char *) GetCosmosChainAddressByCoinTypeAndIndex(CHAIN_ATOM, index);
    if (result->error_code == 0) {
        snprintf_s(g_cosmosAddr, MAX_COSMOS_ADDR_LEN, "%s", result->data);
    }
    free_simple_response_c_char(result);
    return g_cosmosAddr;
}

const CosmosChain_t *GuiGetCosmosChain(uint8_t index)
{
    for (int i = 0; i < COSMOS_CHAINS_LEN; i++) {
        if (g_cosmosChains[i].cardIndex == index) {
            return &g_cosmosChains[i];
        }
    }
    return NULL;
}

bool IsCosmosChain(HOME_WALLET_CARD_ENUM index)
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

void GuiSetCosmosUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                   \
    if (result != NULL)                                                                                   \
    {                                                                                                     \
        free_TransactionParseResult_DisplayCosmosTx((PtrT_TransactionParseResult_DisplayCosmosTx)result); \
        result = NULL;                                                                                    \
    }

void *GuiGetCosmosData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    do {
        QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
        PtrT_TransactionParseResult_DisplayCosmosTx parseResult = cosmos_parse_tx(data, urType);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetCosmosCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    return cosmos_check_tx(data, urType, mfp, sizeof(mfp));
}

void FreeCosmosMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

void GuiGetCosmosTmpType(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNKNOWN)) == 0) {
        snprintf_s((char *)indata,  maxLen, "unknown");
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_MESSAGE)) == 0) {
        snprintf_s((char *)indata,  maxLen, "msg");
    } else {
        snprintf_s((char *)indata,  maxLen, "tx");
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

void GetCosmosValue(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_SEND)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->send_value);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_IBC_TRANSFER)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->transfer_value);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_DELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->delegate_value);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->undelegate_value);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->redelegate_value);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetCosmosMethod(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    strcpy_s((char *)indata, maxLen, tx->overview->method);
}

void GetCosmosProposal(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    // vote_proposal is a string like "#584", but one # is used for color, so need two # to display #.
    snprintf_s((char *)indata,  maxLen, "#%s", tx->overview->vote_proposal);
}

void GetCosmosVoted(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    strcpy_s((char *)indata, maxLen, tx->overview->vote_voted);
}

void GetCosmosAddress1Value(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_SEND)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->send_from);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_IBC_TRANSFER)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->transfer_from);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_DELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->delegate_from);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->undelegate_validator);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->redelegate_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->withdraw_reward_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->vote_voter);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetCosmosAddress1Label(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, "Validator");
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0) {
        strcpy_s((char *)indata, maxLen, "Voter");
    } else if (
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0 ||
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0 ||
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0
    ) {
        strcpy_s((char *)indata, maxLen, "To");
    } else {
        strcpy_s((char *)indata, maxLen, "From");
    }
}

void GetCosmosAddress2Value(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_SEND)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->send_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_IBC_TRANSFER)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->transfer_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_DELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->delegate_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->undelegate_to);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->redelegate_new_validator);
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0) {
        strcpy_s((char *)indata, maxLen, tx->overview->withdraw_reward_validator);
    } else {
        strcpy_s((char *)indata, maxLen, "");
    }
}

void GetCosmosAddress2Label(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
        snprintf_s((char *)indata,  maxLen, "New Validator");
    } else if (
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_WITHDRAW_REWARD)) == 0 ||
        strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_VOTE)) == 0
    ) {
        strcpy_s((char *)indata, maxLen, "Validator");
    } else {
        strcpy_s((char *)indata, maxLen, "To");
    }
}

void GetCosmosDetailCommon(void *indata, void *param, const char* key, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    cJSON* root = cJSON_Parse((const char *)tx->detail);
    cJSON* common = cJSON_GetObjectItem(root, "common");
    if (common == NULL) {
        strcpy_s((char *)indata, maxLen, "");
        return;
    }
    cJSON* value = cJSON_GetObjectItem(common, key);
    if (value == NULL) {
        strcpy_s((char *)indata, maxLen, "");
    } else {
        strcpy_s((char *)indata, maxLen, value->valuestring);
    }
}

void GetCosmosMaxFee(void *indata, void *param, uint32_t maxLen)
{
    GetCosmosDetailCommon(indata, param, "Max Fee", maxLen);
}

void GetCosmosFee(void *indata, void *param, uint32_t maxLen)
{
    GetCosmosDetailCommon(indata, param, "Fee", maxLen);
}

void GetCosmosNetwork(void *indata, void *param, uint32_t maxLen)
{
    GetCosmosDetailCommon(indata, param, "Network", maxLen);
}

void GetCosmosGasLimit(void *indata, void *param, uint32_t maxLen)
{
    GetCosmosDetailCommon(indata, param, "Gas Limit", maxLen);
}

void GetCosmosChainId(void *indata, void *param, uint32_t maxLen)
{
    GetCosmosDetailCommon(indata, param, "Chain ID", maxLen);
}

static void GetCosmosDetailNthKind(void *indata, void *param, int n, const char* key, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    cJSON* root = cJSON_Parse((const char *)tx->detail);
    cJSON* kind = cJSON_GetObjectItem(root, "kind");
    cJSON* item = cJSON_GetArrayItem(kind, n);
    cJSON* value = cJSON_GetObjectItem(item, key);
    // one # is used for color, two # is used for display #.
    snprintf_s((char *)indata,  maxLen, !strcmp(key, "Proposal") ? "#%s" : "%s", value->valuestring);
}

void GetCosmosChannel(void *indata, void *param, uint32_t maxLen)
{
    GetCosmosDetailNthKind(indata, param, 0, "Source Channel", maxLen);
}

void GetCosmosOldValidator(void *indata, void *param, uint32_t maxLen)
{
    GetCosmosDetailNthKind(indata, param, 0, "Old Validator", maxLen);
}

void GetCosmosMsgLen(uint8_t *len, void *param)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    cJSON* root = cJSON_Parse((const char *)tx->detail);
    cJSON* kind = cJSON_GetObjectItem(root, "kind");
    *len = (uint8_t)cJSON_GetArraySize(kind);
    g_cosmosListIndex = -1;
}

void GetCosmosMsgKey(void *indata, void *param, uint32_t maxLen)
{
    ++g_cosmosListIndex;
    GetCosmosDetailNthKind(indata, param, g_cosmosListIndex, "Method", maxLen);
}

void GetCosmosIndex(void *indata, void *param, uint32_t maxLen)
{
    // one # is used for color, two # is used for display #.
    snprintf_s((char *)indata,  maxLen, "##%d", g_cosmosListIndex + 1, maxLen);
}

void GetCosmosTextOfKind(void *indata, void *param, uint32_t maxLen)
{
    GetCosmosDetailNthKind(indata, param, g_cosmosListIndex, indata, maxLen);
}

void GetCosmosDetailItemValue(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    cJSON* detail = cJSON_Parse((const char *)tx->detail);
    cJSON* value = cJSON_GetObjectItem(detail, indata);
    if (value == NULL) {
        strcpy_s((char *)indata, maxLen, "");
    } else {
        strcpy_s((char *)indata, maxLen, value->valuestring);
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
    char* chain_id = SRAM_MALLOC(BUFFER_SIZE_64);
    if (strcmp(parseResult->data->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_MESSAGE)) == 0 || strcmp(parseResult->data->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNKNOWN)) == 0) {
        cJSON* detail = cJSON_Parse(parseResult->data->detail);
        cJSON* value = cJSON_GetObjectItem(detail, "Chain ID");
        snprintf_s(chain_id, BUFFER_SIZE_64, "%s", value->valuestring);
    } else {
        GetCosmosDetailCommon(chain_id, parseResult->data, "Chain ID", BUFFER_SIZE_64);
    }
    printf("chain_id: %s\n", chain_id);
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
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    QRCodeType urType = g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = cosmos_sign_tx(data, urType, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}
