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
#include "gui_chain_components.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static int8_t g_cosmosListIndex = -1;
static const char *g_cosmosLastDetailPtr = NULL;
static cJSON *g_cosmosLastRoot = NULL;
static cJSON *g_cosmosLastCommon = NULL;
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

static void ClearCosmosDetailCache(void);
static cJSON *GetCosmosParsedDetailRoot(DisplayCosmosTx *tx);
static lv_obj_t *CreateCosmosMessageTitle(lv_obj_t *parent, size_t index, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosJsonItem(lv_obj_t *parent, const char *key, const cJSON *value, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosHighlightedJsonItem(lv_obj_t *parent, const char *key,
                                                 const cJSON *value, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosJsonFields(lv_obj_t *parent, const cJSON *object, lv_obj_t *lastView,
                                        bool overview, bool showMessageIndex);
static lv_obj_t *CreateCosmosDetailsMessageCard(
    lv_obj_t *parent, const cJSON *object, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosOverviewCommonFields(lv_obj_t *parent, const cJSON *common, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosBlindSignView(lv_obj_t *parent, const cJSON *object, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosMemoView(lv_obj_t *parent, const cJSON *memo, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosSingleOverview(lv_obj_t *parent, const cJSON *message, const cJSON *common);
static lv_obj_t *CreateCosmosOverviewValue(lv_obj_t *parent, const char *value, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosOverviewVote(lv_obj_t *parent, const cJSON *message, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosOverviewAddresses(lv_obj_t *parent, const cJSON *message, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosVoteDetails(lv_obj_t *parent, const cJSON *message, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosFeeDetails(lv_obj_t *parent, const cJSON *common, lv_obj_t *lastView);
static lv_obj_t *CreateCosmosNetworkDetails(lv_obj_t *parent, const cJSON *common, lv_obj_t *lastView);
static bool IsCosmosBlindSignMessage(const cJSON *object);
static void InitCosmosTxContainer(lv_obj_t *parent);

static inline void* GetCosmosUrData(void)
{
    return g_isMulti ? g_urMultiResult->data : g_urResult->data;
}

static inline QRCodeType GetCosmosUrType(void)
{
    return g_isMulti ? g_urMultiResult->ur_type : g_urResult->ur_type;
}

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
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayCosmosTx parseResult = cosmos_parse_tx(GetCosmosUrData(), GetCosmosUrType());
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetCosmosCheckResult(void)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    return cosmos_check_tx(GetCosmosUrData(), GetCosmosUrType(), mfp, sizeof(mfp));
}

void FreeCosmosMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    ClearCosmosDetailCache();
}

void GuiGetCosmosTmpType(void *indata, void *param, uint32_t maxLen)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)param;
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNKNOWN)) == 0) {
        cJSON *root = GetCosmosParsedDetailRoot(tx);
        cJSON *kind = root == NULL ? NULL : cJSON_GetObjectItem(root, "kind");
        const char *page = cJSON_IsArray(kind) && cJSON_GetArraySize(kind) > 0 ? "tx" : "unknown";
        snprintf_s((char *)indata, maxLen, "%s", page);
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

static void ClearCosmosDetailCache(void)
{
    if (g_cosmosLastRoot != NULL) {
        cJSON_Delete(g_cosmosLastRoot);
        g_cosmosLastRoot = NULL;
    }
    g_cosmosLastCommon = NULL;
    g_cosmosLastDetailPtr = NULL;
}

static cJSON *GetCosmosParsedDetailRoot(DisplayCosmosTx *tx)
{
    if (tx == NULL || tx->detail == NULL) {
        return NULL;
    }
    if (g_cosmosLastDetailPtr == tx->detail && g_cosmosLastRoot != NULL) {
        return g_cosmosLastRoot;
    }
    ClearCosmosDetailCache();
    g_cosmosLastRoot = cJSON_Parse((const char *)tx->detail);
    g_cosmosLastDetailPtr = tx->detail;
    if (g_cosmosLastRoot != NULL) {
        g_cosmosLastCommon = cJSON_GetObjectItem(g_cosmosLastRoot, "common");
    }
    return g_cosmosLastRoot;
}

void GuiCosmosTxOverview(lv_obj_t *parent, void *totalData)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)totalData;
    InitCosmosTxContainer(parent);

    cJSON *root = GetCosmosParsedDetailRoot(tx);
    cJSON *kind = root == NULL ? NULL : cJSON_GetObjectItem(root, "kind");
    cJSON *common = root == NULL ? NULL : cJSON_GetObjectItem(root, "common");
    lv_obj_t *lastView = NULL;

    if (cJSON_IsArray(kind)) {
        int messageCount = cJSON_GetArraySize(kind);
        cJSON *singleMessage = messageCount == 1 ? cJSON_GetArrayItem(kind, 0) : NULL;
        if (cJSON_IsObject(singleMessage) && !IsCosmosBlindSignMessage(singleMessage)) {
            CreateCosmosSingleOverview(parent, singleMessage, common);
            lv_obj_update_layout(parent);
            return;
        }
        for (int i = 0; i < messageCount; i++) {
            cJSON *message = cJSON_GetArrayItem(kind, i);
            if (!cJSON_IsObject(message)) {
                continue;
            }
            if (messageCount > 1) {
                lastView = CreateCosmosMessageTitle(parent, (size_t)i, lastView);
            }
            lastView = CreateCosmosJsonFields(parent, message, lastView, true, messageCount > 1);
        }
    }
    lastView = CreateCosmosOverviewCommonFields(parent, common, lastView);
    lv_obj_update_layout(parent);
}

static const char *GetCosmosJsonString(const cJSON *object, const char *key)
{
    cJSON *value = cJSON_IsObject(object) ? cJSON_GetObjectItem(object, key) : NULL;
    if (!cJSON_IsString(value) || value->valuestring == NULL || value->valuestring[0] == '\0') {
        return NULL;
    }
    return value->valuestring;
}

static lv_obj_t *CreateCosmosSingleOverview(lv_obj_t *parent, const cJSON *message, const cJSON *common)
{
    lv_obj_t *lastView = NULL;
    const char *value = GetCosmosJsonString(message, "Value");
    const char *method = GetCosmosJsonString(message, "Method");
    const char *network = GetCosmosJsonString(common, "Network");

    if (value != NULL) {
        lastView = CreateCosmosOverviewValue(parent, value, lastView);
    } else if (method != NULL && strcmp(method, "Vote") == 0) {
        lastView = CreateCosmosOverviewVote(parent, message, lastView);
    }
    if (network != NULL) {
        lastView = CreateTransactionItemView(parent, _("Network"), network, lastView);
    }
    if (method != NULL) {
        lastView = CreateTransactionItemView(parent, _("Method"), method, lastView);
    }
    lastView = CreateCosmosOverviewAddresses(parent, message, lastView);

    if (network != NULL && strcmp(network, "Unknown Network") == 0) {
        lastView = CreateCosmosJsonItem(parent, "Chain ID",
                                        cJSON_GetObjectItem(common, "Chain ID"), lastView);
    }
    cJSON *memo = cJSON_IsObject(common) ? cJSON_GetObjectItem(common, "Memo") : NULL;
    return CreateCosmosMemoView(parent, memo, lastView);
}

static lv_obj_t *CreateCosmosOverviewValue(lv_obj_t *parent, const char *value, lv_obj_t *lastView)
{
    lv_obj_t *container = CreateContentContainer(parent, 408, 0);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    lv_obj_t *title = GuiCreateIllustrateLabel(container, _("Value"));
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(title, LV_OPA_64, LV_PART_MAIN);

    lv_obj_t *amount = GuiCreateLabelWithFont(container, value, GetOverviewAmountFont(value));
    lv_obj_set_width(amount, 360);
    lv_label_set_long_mode(amount, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(amount, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(amount, LV_ALIGN_TOP_LEFT, 24, 50);
    lv_obj_update_layout(amount);
    lv_obj_set_height(container, 50 + lv_obj_get_height(amount) + 26);
    return container;
}

static lv_obj_t *CreateCosmosOverviewVote(lv_obj_t *parent, const cJSON *message, lv_obj_t *lastView)
{
    const char *proposal = GetCosmosJsonString(message, "Proposal");
    const char *voted = GetCosmosJsonString(message, "Voted");
    if (proposal == NULL && voted == NULL) {
        return lastView;
    }

    lv_obj_t *container = CreateContentContainer(parent, 408, 106);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }
    if (proposal != NULL) {
        lv_obj_t *title = GuiCreateIllustrateLabel(container, _("Proposal"));
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, 16);
        lv_obj_set_style_text_opa(title, LV_OPA_64, LV_PART_MAIN);
        lv_obj_t *text = GuiCreateIllustrateLabel(container, proposal);
        lv_obj_set_style_text_color(text, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(text, title, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    }
    if (voted != NULL) {
        lv_obj_t *title = GuiCreateIllustrateLabel(container, _("Voted"));
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, 54);
        lv_obj_set_style_text_opa(title, LV_OPA_64, LV_PART_MAIN);
        lv_obj_t *text = GuiCreateIllustrateLabel(container, voted);
        lv_obj_set_style_text_color(text, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(text, title, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    }
    return container;
}

static lv_obj_t *CreateCosmosOverviewAddresses(lv_obj_t *parent, const cJSON *message, lv_obj_t *lastView)
{
    static const char *fallbackKeys[] = {
        "Delegator", "From", "Voter", "To", "Validator", "New Validator"
    };
    const char *keys[2] = {NULL, NULL};
    const char *values[2] = {NULL, NULL};
    size_t count = 0;
    const char *method = GetCosmosJsonString(message, "Method");

    if (method != NULL) {
        if (strcmp(method, "Send") == 0 || strcmp(method, "IBC Transfer") == 0) {
            keys[0] = "From";
            keys[1] = "To";
        } else if (strcmp(method, "Delegate") == 0) {
            keys[0] = "Delegator";
            keys[1] = "Validator";
        } else if (strcmp(method, "Undelegate") == 0) {
            keys[0] = "Validator";
            keys[1] = "To";
        } else if (strcmp(method, "Re-delegate") == 0) {
            keys[0] = "To";
            keys[1] = "New Validator";
        } else if (strcmp(method, "Withdraw Reward") == 0) {
            keys[0] = "To";
            keys[1] = "Validator";
        } else if (strcmp(method, "Vote") == 0) {
            keys[0] = "Voter";
        }
    }

    for (size_t i = 0; i < NUMBER_OF_ARRAYS(keys); i++) {
        if (keys[i] == NULL) {
            continue;
        }
        const char *value = GetCosmosJsonString(message, keys[i]);
        if (value != NULL) {
            values[count] = value;
            keys[count] = keys[i];
            count++;
        }
    }
    if (count == 0) {
        for (size_t i = 0; i < NUMBER_OF_ARRAYS(fallbackKeys) && count < NUMBER_OF_ARRAYS(keys); i++) {
            const char *value = GetCosmosJsonString(message, fallbackKeys[i]);
            if (value != NULL) {
                keys[count] = fallbackKeys[i];
                values[count] = value;
                count++;
            }
        }
    }
    if (count == 0) {
        return lastView;
    }

    lv_obj_t *container = CreateContentContainer(parent, 408, 0);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    uint16_t y = 16;
    for (size_t i = 0; i < count; i++) {
        lv_obj_t *title = GuiCreateIllustrateLabel(container, _(keys[i]));
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, y);
        lv_obj_set_style_text_opa(title, LV_OPA_64, LV_PART_MAIN);

        lv_obj_t *text = GuiCreateIllustrateLabel(container, values[i]);
        lv_obj_set_width(text, 360);
        lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
        lv_obj_align(text, LV_ALIGN_TOP_LEFT, 24, y + 38);
        lv_obj_update_layout(text);
        y += 38 + lv_obj_get_height(text) + 16;
    }
    lv_obj_set_height(container, y);
    return container;
}

void GuiCosmosTxDetails(lv_obj_t *parent, void *totalData)
{
    DisplayCosmosTx *tx = (DisplayCosmosTx *)totalData;
    InitCosmosTxContainer(parent);

    cJSON *root = GetCosmosParsedDetailRoot(tx);
    cJSON *kind = root == NULL ? NULL : cJSON_GetObjectItem(root, "kind");
    cJSON *common = root == NULL ? NULL : cJSON_GetObjectItem(root, "common");
    lv_obj_t *lastView = NULL;

    if (cJSON_IsArray(kind)) {
        int messageCount = cJSON_GetArraySize(kind);
        cJSON *singleMessage = messageCount == 1 ? cJSON_GetArrayItem(kind, 0) : NULL;
        const char *singleMethod = GetCosmosJsonString(singleMessage, "Method");
        if (singleMethod != NULL && strcmp(singleMethod, "Vote") == 0) {
            lastView = CreateCosmosVoteDetails(parent, singleMessage, lastView);
            lastView = CreateCosmosFeeDetails(parent, common, lastView);
            cJSON *memo = cJSON_IsObject(common) ? cJSON_GetObjectItem(common, "Memo") : NULL;
            lastView = CreateCosmosMemoView(parent, memo, lastView);
            CreateCosmosNetworkDetails(parent, common, lastView);
            lv_obj_update_layout(parent);
            return;
        }
        for (int i = 0; i < messageCount; i++) {
            cJSON *message = cJSON_GetArrayItem(kind, i);
            if (!cJSON_IsObject(message)) {
                continue;
            }
            if (messageCount > 1) {
                lastView = CreateCosmosMessageTitle(parent, (size_t)i, lastView);
            }
            lastView = CreateCosmosJsonFields(parent, message, lastView, false, messageCount > 1);
        }
    }
    lastView = CreateCosmosFeeDetails(parent, common, lastView);
    cJSON *memo = cJSON_IsObject(common) ? cJSON_GetObjectItem(common, "Memo") : NULL;
    lastView = CreateCosmosMemoView(parent, memo, lastView);
    CreateCosmosNetworkDetails(parent, common, lastView);
    lv_obj_update_layout(parent);
}

static uint16_t CreateCosmosDetailInlineValue(lv_obj_t *container, const char *titleText,
                                              const char *valueText, uint16_t y, bool highlight)
{
    if (valueText == NULL) {
        return y;
    }
    lv_obj_t *title = GuiCreateIllustrateLabel(container, _(titleText));
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, y);
    lv_obj_set_style_text_opa(title, LV_OPA_64, LV_PART_MAIN);
    lv_obj_update_layout(title);

    lv_obj_t *value = GuiCreateIllustrateLabel(container, valueText);
    if (highlight) {
        lv_obj_set_style_text_color(value, ORANGE_COLOR, LV_PART_MAIN);
    }
    int32_t valueWidth = 360 - lv_obj_get_width(title) - 16;
    if (valueWidth < 1) {
        valueWidth = 1;
    }
    lv_obj_set_width(value, valueWidth);
    lv_label_set_long_mode(value, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(value, title, LV_ALIGN_OUT_RIGHT_TOP, 16, 0);
    lv_obj_update_layout(value);

    int32_t rowHeight = lv_obj_get_height(title);
    if (lv_obj_get_height(value) > rowHeight) {
        rowHeight = lv_obj_get_height(value);
    }
    return y + rowHeight + 8;
}

static lv_obj_t *CreateCosmosVoteDetails(lv_obj_t *parent, const cJSON *message, lv_obj_t *lastView)
{
    lv_obj_t *container = CreateContentContainer(parent, 408, 0);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    uint16_t y = 16;
    y = CreateCosmosDetailInlineValue(
        container, "Proposal", GetCosmosJsonString(message, "Proposal"), y, true);
    y = CreateCosmosDetailInlineValue(
        container, "Voted", GetCosmosJsonString(message, "Voted"), y, true);
    y = CreateCosmosDetailInlineValue(
        container, "Method", GetCosmosJsonString(message, "Method"), y, false);

    lv_obj_t *voterTitle = GuiCreateIllustrateLabel(container, _("Voter"));
    lv_obj_align(voterTitle, LV_ALIGN_TOP_LEFT, 24, y);
    lv_obj_set_style_text_opa(voterTitle, LV_OPA_64, LV_PART_MAIN);

    const char *voter = GetCosmosJsonString(message, "Voter");
    lv_obj_t *voterValue = GuiCreateIllustrateLabel(container, voter == NULL ? "" : voter);
    lv_obj_set_width(voterValue, 360);
    lv_label_set_long_mode(voterValue, LV_LABEL_LONG_WRAP);
    lv_obj_align(voterValue, LV_ALIGN_TOP_LEFT, 24, y + 38);
    lv_obj_update_layout(voterValue);
    lv_obj_set_height(container, y + 38 + lv_obj_get_height(voterValue) + 16);
    return container;
}

static lv_obj_t *CreateCosmosFeeDetails(lv_obj_t *parent, const cJSON *common, lv_obj_t *lastView)
{
    const char *maxFee = GetCosmosJsonString(common, "Max Fee");
    const char *fee = GetCosmosJsonString(common, "Fee");
    const char *gasLimit = GetCosmosJsonString(common, "Gas Limit");
    if (maxFee == NULL && fee == NULL && gasLimit == NULL) {
        return lastView;
    }

    lv_obj_t *container = CreateContentContainer(parent, 408, 0);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }
    uint16_t y = 16;
    y = CreateCosmosDetailInlineValue(container, "Max Fee", maxFee, y, false);
    if (maxFee != NULL) {
        lv_obj_t *description = GuiCreateLabelWithFont(
            container, "  ·  Max Fee Price * Gas Limit", &openSansDesc);
        lv_obj_set_style_text_opa(description, LV_OPA_64, LV_PART_MAIN);
        lv_obj_align(description, LV_ALIGN_TOP_LEFT, 24, y);
        lv_obj_update_layout(description);
        y += lv_obj_get_height(description) + 8;
    }
    y = CreateCosmosDetailInlineValue(container, "Fee", fee, y, false);
    y = CreateCosmosDetailInlineValue(container, "Gas Limit", gasLimit, y, false);
    lv_obj_set_height(container, y + 8);
    return container;
}

static lv_obj_t *CreateCosmosNetworkDetails(lv_obj_t *parent, const cJSON *common, lv_obj_t *lastView)
{
    const char *network = GetCosmosJsonString(common, "Network");
    const char *chainId = GetCosmosJsonString(common, "Chain ID");
    if (network == NULL && chainId == NULL) {
        return lastView;
    }

    lv_obj_t *container = CreateContentContainer(parent, 408, 0);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }
    uint16_t y = 16;
    y = CreateCosmosDetailInlineValue(container, "Network", network, y, false);
    y = CreateCosmosDetailInlineValue(container, "Chain ID", chainId, y, false);
    lv_obj_set_height(container, y + 8);
    return container;
}

static void InitCosmosTxContainer(lv_obj_t *parent)
{
    lv_obj_t *tabChild = lv_obj_get_parent(parent);
    if (tabChild != NULL) {
        lv_obj_set_style_pad_all(tabChild, 0, LV_PART_MAIN);
    }
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
}

static lv_obj_t *CreateCosmosMessageTitle(lv_obj_t *parent, size_t index, lv_obj_t *lastView)
{
    lv_obj_t *container = CreateContentContainer(parent, 408, 62);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    char title[BUFFER_SIZE_32] = {0};
    snprintf_s(title, sizeof(title), "%s %u", _("Message"), (unsigned int)(index + 1));

    lv_obj_t *label = GuiCreateTextLabel(container, title);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    return container;
}

static lv_obj_t *CreateCosmosJsonItem(lv_obj_t *parent, const char *key, const cJSON *value, lv_obj_t *lastView)
{
    if (key == NULL || !cJSON_IsString(value) || value->valuestring == NULL || value->valuestring[0] == '\0') {
        return lastView;
    }
    return CreateTransactionItemView(parent, _(key), value->valuestring, lastView);
}

static lv_obj_t *CreateCosmosHighlightedJsonItem(lv_obj_t *parent, const char *key,
                                                 const cJSON *value, lv_obj_t *lastView)
{
    lv_obj_t *view = CreateCosmosJsonItem(parent, key, value, lastView);
    if (view != lastView) {
        lv_obj_t *valueLabel = lv_obj_get_child(view, 1);
        if (valueLabel != NULL) {
            lv_obj_set_style_text_color(valueLabel, ORANGE_COLOR, LV_PART_MAIN);
        }
    }
    return view;
}

static bool IsCosmosInlineDetailField(const char *key)
{
    return strcmp(key, "Value") == 0 || strcmp(key, "Method") == 0 ||
           strcmp(key, "Proposal") == 0 || strcmp(key, "Voted") == 0 ||
           strcmp(key, "Source Channel") == 0;
}

static bool IsCosmosHighlightedDetailField(const char *key)
{
    return strcmp(key, "Value") == 0 || strcmp(key, "Proposal") == 0 ||
           strcmp(key, "Voted") == 0;
}

static bool AppendCosmosDetailsField(lv_obj_t *container, const cJSON *field, uint16_t *y)
{
    if (field == NULL || field->string == NULL || !cJSON_IsString(field) ||
        field->valuestring == NULL || field->valuestring[0] == '\0') {
        return false;
    }

    lv_obj_t *title = GuiCreateIllustrateLabel(container, _(field->string));
    lv_obj_set_style_text_opa(title, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, *y);
    lv_obj_update_layout(title);

    lv_obj_t *value = GuiCreateIllustrateLabel(container, field->valuestring);
    lv_label_set_long_mode(value, LV_LABEL_LONG_WRAP);
    if (IsCosmosHighlightedDetailField(field->string)) {
        lv_obj_set_style_text_color(value, ORANGE_COLOR, LV_PART_MAIN);
    }

    if (IsCosmosInlineDetailField(field->string)) {
        uint16_t titleWidth = lv_obj_get_width(title);
        uint16_t valueX = 24 + titleWidth + 16;
        uint16_t valueWidth = valueX < 384 ? 384 - valueX : 0;
        if (valueWidth >= 96) {
            lv_obj_set_width(value, valueWidth);
            lv_obj_align(value, LV_ALIGN_TOP_LEFT, valueX, *y);
            lv_obj_update_layout(value);
            uint16_t titleHeight = lv_obj_get_height(title);
            uint16_t valueHeight = lv_obj_get_height(value);
            *y += (titleHeight > valueHeight ? titleHeight : valueHeight) + 16;
            return true;
        }
    }

    *y += lv_obj_get_height(title) + 8;
    lv_obj_set_width(value, 360);
    lv_obj_align(value, LV_ALIGN_TOP_LEFT, 24, *y);
    lv_obj_update_layout(value);
    *y += lv_obj_get_height(value) + 16;
    return true;
}

static bool IsCosmosPreferredDetailField(
    const char *key, const char *const *preferredKeys, size_t preferredCount)
{
    for (size_t i = 0; i < preferredCount; i++) {
        if (strcmp(key, preferredKeys[i]) == 0) {
            return true;
        }
    }
    return false;
}

static lv_obj_t *CreateCosmosDetailsMessageCard(
    lv_obj_t *parent, const cJSON *object, lv_obj_t *lastView)
{
    lv_obj_t *container = CreateContentContainer(parent, 408, 0);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    uint16_t y = 16;
    bool hasField = false;
    const char *preferredKeys[6] = {"Value", "Method", NULL, NULL, NULL, NULL};
    size_t preferredCount = 2;
    const char *method = GetCosmosJsonString(object, "Method");
    if (method != NULL && strcmp(method, "Send") == 0) {
        preferredKeys[2] = "From";
        preferredKeys[3] = "To";
        preferredCount = 4;
    } else if (method != NULL && strcmp(method, "IBC Transfer") == 0) {
        preferredKeys[2] = "From";
        preferredKeys[3] = "To";
        preferredKeys[4] = "Source Channel";
        preferredCount = 5;
    } else if (method != NULL && strcmp(method, "Delegate") == 0) {
        preferredKeys[2] = "Delegator";
        preferredKeys[3] = "Validator";
        preferredCount = 4;
    } else if (method != NULL && strcmp(method, "Undelegate") == 0) {
        preferredKeys[2] = "Validator";
        preferredKeys[3] = "To";
        preferredCount = 4;
    } else if (method != NULL && strcmp(method, "Re-delegate") == 0) {
        preferredKeys[2] = "To";
        preferredKeys[3] = "Old Validator";
        preferredKeys[4] = "New Validator";
        preferredCount = 5;
    } else if (method != NULL && strcmp(method, "Withdraw Reward") == 0) {
        preferredKeys[0] = "Method";
        preferredKeys[1] = "To";
        preferredKeys[2] = "Validator";
        preferredCount = 3;
    }

    for (size_t i = 0; i < preferredCount; i++) {
        hasField |= AppendCosmosDetailsField(
            container, cJSON_GetObjectItem(object, preferredKeys[i]), &y);
    }
    for (const cJSON *field = object->child; field != NULL; field = field->next) {
        if (field->string == NULL ||
            IsCosmosPreferredDetailField(field->string, preferredKeys, preferredCount)) {
            continue;
        }
        hasField |= AppendCosmosDetailsField(container, field, &y);
    }

    if (!hasField) {
        lv_obj_del(container);
        return lastView;
    }
    lv_obj_set_height(container, y);
    return container;
}

static lv_obj_t *CreateCosmosJsonFields(lv_obj_t *parent, const cJSON *object, lv_obj_t *lastView,
                                        bool overview, bool showMessageIndex)
{
    static const char *overviewKeys[] = {
        "Value", "Method", "Delegator", "From", "To", "Validator", "New Validator",
        "Proposal", "Voted", "Voter"
    };

    if (!cJSON_IsObject(object)) {
        return lastView;
    }
    if (IsCosmosBlindSignMessage(object)) {
        lastView = CreateCosmosBlindSignView(parent, object, lastView);
        if (showMessageIndex) {
            lastView = CreateCosmosJsonItem(parent, "Message Index",
                                            cJSON_GetObjectItem(object, "Message Index"), lastView);
        }
        lastView = CreateCosmosJsonItem(parent, "Type URL",
                                        cJSON_GetObjectItem(object, "Type URL"), lastView);
        return CreateCosmosJsonItem(parent, "Data Digest",
                                    cJSON_GetObjectItem(object, "Data Digest"), lastView);
    }
    if (overview) {
        for (size_t i = 0; i < NUMBER_OF_ARRAYS(overviewKeys); i++) {
            cJSON *value = cJSON_GetObjectItem(object, overviewKeys[i]);
            if (strcmp(overviewKeys[i], "Proposal") == 0 || strcmp(overviewKeys[i], "Voted") == 0) {
                lastView = CreateCosmosHighlightedJsonItem(parent, overviewKeys[i], value, lastView);
            } else {
                lastView = CreateCosmosJsonItem(parent, overviewKeys[i], value, lastView);
            }
        }
        return lastView;
    }

    return CreateCosmosDetailsMessageCard(parent, object, lastView);
}

static bool IsCosmosBlindSignMessage(const cJSON *object)
{
    cJSON *method = cJSON_IsObject(object) ? cJSON_GetObjectItem(object, "Method") : NULL;
    return cJSON_IsString(method) && method->valuestring != NULL &&
           strcmp(method->valuestring, "Blind Sign") == 0;
}

static lv_obj_t *CreateCosmosBlindSignView(lv_obj_t *parent, const cJSON *object, lv_obj_t *lastView)
{
    cJSON *warning = cJSON_GetObjectItem(object, "Warning");
    const char *warningText = cJSON_IsString(warning) && warning->valuestring != NULL
                              ? warning->valuestring
                              : _("Unknown Data");
    lv_obj_t *container = CreateContentContainer(parent, 408, 0);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    lv_obj_t *title = GuiCreateTextLabel(container, _("Blind Sign"));
    lv_obj_set_style_text_color(title, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, 16);

    lv_obj_t *content = GuiCreateIllustrateLabel(container, warningText);
    lv_obj_set_width(content, 360);
    lv_label_set_long_mode(content, LV_LABEL_LONG_WRAP);
    lv_obj_align(content, LV_ALIGN_TOP_LEFT, 24, 54);
    lv_obj_update_layout(content);
    lv_obj_set_height(container, 54 + lv_obj_get_height(content) + 16);
    return container;
}

static lv_obj_t *CreateCosmosMemoView(lv_obj_t *parent, const cJSON *memo, lv_obj_t *lastView)
{
    if (!cJSON_IsString(memo) || memo->valuestring == NULL || memo->valuestring[0] == '\0') {
        return lastView;
    }

    lv_obj_t *container = CreateContentContainer(parent, 408, 0);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    lv_obj_t *title = GuiCreateIllustrateLabel(container, _("Memo"));
    lv_obj_set_style_text_opa(title, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, 16);

    lv_obj_t *textContainer = GuiCreateContainerWithParent(container, 360, 0);
    lv_obj_set_style_bg_opa(textContainer, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(textContainer, LV_ALIGN_TOP_LEFT, 24, 54);

    lv_obj_t *text = GuiCreateIllustrateLabel(textContainer, memo->valuestring);
    lv_obj_set_width(text, 360);
    lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_update_layout(text);

    uint16_t textHeight = lv_obj_get_height(text);
    uint16_t visibleHeight = textHeight > 150 ? 150 : textHeight;
    lv_obj_set_height(textContainer, visibleHeight);
    if (textHeight > visibleHeight) {
        lv_obj_add_flag(textContainer, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(textContainer, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_scroll_dir(textContainer, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(textContainer, LV_SCROLLBAR_MODE_OFF);
    }
    lv_obj_set_height(container, 54 + visibleHeight + 16);
    return container;
}

static lv_obj_t *CreateCosmosOverviewCommonFields(
    lv_obj_t *parent, const cJSON *common, lv_obj_t *lastView)
{
    if (!cJSON_IsObject(common)) {
        return lastView;
    }
    lastView = CreateCosmosMemoView(parent, cJSON_GetObjectItem(common, "Memo"), lastView);
    cJSON *network = cJSON_GetObjectItem(common, "Network");
    lastView = CreateCosmosJsonItem(parent, "Network", network, lastView);
    if (cJSON_IsString(network) && network->valuestring != NULL &&
        strcmp(network->valuestring, "Unknown Network") == 0) {
        lastView = CreateCosmosJsonItem(parent, "Chain ID",
                                        cJSON_GetObjectItem(common, "Chain ID"), lastView);
    }
    return lastView;
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
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_DELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, "Delegator");
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_UNDELEGATE)) == 0) {
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
    if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_DELEGATE)) == 0) {
        strcpy_s((char *)indata, maxLen, "Validator");
    } else if (strcmp(tx->overview->display_type, GuiGetCosmosTxTypeName(COSMOS_TX_REDELEGATE)) == 0) {
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
    cJSON* root = GetCosmosParsedDetailRoot(tx);
    if (root == NULL) {
        strcpy_s((char *)indata, maxLen, "");
        return;
    }
    cJSON* common = g_cosmosLastCommon;
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
    cJSON* root = GetCosmosParsedDetailRoot(tx);
    if (root == NULL) {
        strcpy_s((char *)indata, maxLen, "");
        return;
    }
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
    cJSON* root = GetCosmosParsedDetailRoot(tx);
    if (root == NULL) {
        *len = 0;
        g_cosmosListIndex = -1;
        return;
    }
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
    cJSON* root = GetCosmosParsedDetailRoot(tx);
    if (root == NULL) {
        strcpy_s((char *)indata, maxLen, "");
        return;
    }
    cJSON* value = cJSON_GetObjectItem(root, indata);
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
    char chain_id[BUFFER_SIZE_64] = {0};
    cJSON *root = GetCosmosParsedDetailRoot(parseResult->data);
    cJSON *common = root == NULL ? NULL : cJSON_GetObjectItem(root, "common");
    cJSON *value = cJSON_IsObject(common) ? cJSON_GetObjectItem(common, "Chain ID") : NULL;
    if (!cJSON_IsString(value)) {
        value = root == NULL ? NULL : cJSON_GetObjectItem(root, "Chain ID");
    }
    if (!cJSON_IsString(value) || value->valuestring == NULL || value->valuestring[0] == '\0') {
        return CHAIN_ATOM;
    }
    snprintf_s(chain_id, BUFFER_SIZE_64, "%s", value->valuestring);
    for (uint8_t i = 0; i < COSMOS_CHAINS_LEN; i++) {
        if (strcmp(chain_id, g_cosmosChains[i].chainId) == 0) {
            return g_cosmosChains[i].index;
        }
    }
    if (strcmp(chain_id, "evmos_9000-4") == 0) {
        return CHAIN_EVMOS;
    }
    return CHAIN_UNKNOWN;
}

UREncodeResult *GuiGetCosmosSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    uint8_t seed[SEED_LEN];
    do {
        int ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (ret != SUCCESS_CODE) {
            break;
        }
        encodeResult = cosmos_sign_tx(GetCosmosUrData(), GetCosmosUrType(), seed, GetCurrentAccountSeedLen());
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);
    return encodeResult;
}
