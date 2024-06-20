#include "gui_ton.h"
#include "rust.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "gui_chain.h"

#define CHECK_FREE_PARSE_RESULT(result)                                     \
    if (result != NULL)                                                     \
    {                                                                       \
        free_TransactionParseResult_DisplayTonTransaction(g_parseResult);   \
        g_parseResult = NULL;                                               \
    }

#define CHECK_FREE_PARSE_PROOF_RESULT(result)                                     \
    if (result != NULL)                                                     \
    {                                                                       \
        free_TransactionParseResult_DisplayTonProof(g_proofParseResult);   \
        g_parseResult = NULL;                                               \
    }

static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static void *g_proofParseResult = NULL;
static bool g_isMulti = false;
static ViewType g_viewType = ViewTypeUnKnown;

static lv_obj_t *createContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h);
static lv_obj_t *CreateOverviewAmountView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView);
static lv_obj_t *CreateOverviewActionView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView);
static lv_obj_t *CreateOverviewDestinationView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView);
static lv_obj_t *CreateOverviewCommentView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView);
static lv_obj_t *CreateOverviewContractDataView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView);
static lv_obj_t *CreateDetailsDataViewView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView);
static lv_obj_t *CreateDetailsRawDataView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView);

void GuiSetTonUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_viewType = g_isMulti ? g_urMultiResult->t : g_urResult->t;
}

UREncodeResult *GuiGetTonSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = ton_sign_transaction(data, seed, 64);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

UREncodeResult *GuiGetTonProofSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = ton_sign_proof(data, seed, 64);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

PtrT_TransactionCheckResult GuiGetTonCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    char* publicKey;
    bool isTonNative = GetMnemonicType() == MNEMONIC_TYPE_TON;
    if (isTonNative) {
        publicKey = GetCurrentAccountPublicKey(XPUB_TYPE_TON_NATIVE);
    } else {
        //remains for bip39 ton
        return ton_not_supported_error();
    }
    return ton_check_transaction(data, publicKey);
}

void *GuiGetTonGUIData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayTonTransaction parseResult = ton_parse_transaction(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void *GuiGetTonProofGUIData(void)
{
    CHECK_FREE_PARSE_PROOF_RESULT(g_proofParseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayTonProof parseResult = ton_parse_proof(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_proofParseResult = (void *)parseResult;
    } while (0);
    return g_proofParseResult;
}

void GuiTonTxOverview(lv_obj_t *parent, void *totalData)
{
    DisplayTonTransaction *txData = (DisplayTonTransaction *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *lastView = CreateOverviewAmountView(parent, txData, NULL);
    lastView = CreateOverviewActionView(parent, txData, lastView);
    lastView = CreateOverviewDestinationView(parent, txData, lastView);
    if (txData->comment != NULL) {
        lastView = CreateOverviewCommentView(parent, txData, lastView);
    }
    if (txData->contract_data != NULL) {
        lastView = CreateOverviewContractDataView(parent, txData, lastView);
    }
    lv_obj_update_layout(parent);
}

void GuiTonTxRawData(lv_obj_t *parent, void *totalData)
{
    DisplayTonTransaction *txData = (DisplayTonTransaction *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *lastView = NULL;
    if (txData->data_view != NULL) {
        lastView = CreateDetailsDataViewView(parent, txData, NULL);
    }
    lastView = CreateDetailsRawDataView(parent, txData, lastView);
}

static lv_obj_t *createContentContainer(lv_obj_t *parent, uint16_t w, uint16_t h)
{
    lv_obj_t *container = GuiCreateContainerWithParent(parent, w, h);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(container, LV_OPA_12, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN);
    return container;
}

static lv_obj_t *CreateOverviewAmountView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView)
{
    lv_obj_t *container = createContentContainer(parent, 408, 106);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, _("Amount"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateTextLabel(container, data->amount);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 50);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    return container;
}

static lv_obj_t *CreateOverviewActionView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView)
{
    lv_obj_t *container = createContentContainer(parent, 408, 64);
    lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, _("Action"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, data->action);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 99, 16);
    return container;
}

static lv_obj_t *CreateOverviewDestinationView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView)
{
    lv_obj_t *container = createContentContainer(parent, 408, 244);
    lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    char *xPub = GetCurrentAccountPublicKey(XPUB_TYPE_TON_NATIVE);
    SimpleResponse_c_char *from = ton_get_address(xPub);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, _("From"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    char fromAddress[128] = {0};
    snprintf_s(fromAddress, 128, "%.22s\n%s", from->data, &from->data[22]);

    label = GuiCreateIllustrateLabel(container, fromAddress);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    label = GuiCreateIllustrateLabel(container, _("To"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 130);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    char toAddress[128] = {0};
    snprintf_s(toAddress, 128, "%.22s\n%s", data->to, &data->to[22]);

    label = GuiCreateIllustrateLabel(container, toAddress);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 168);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    return container;
}

static lv_obj_t *CreateOverviewCommentView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView)
{
    lv_obj_t *container = createContentContainer(parent, 408, 62);
    lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, _("Comment"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, data->comment);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(label);

    uint16_t height = lv_obj_get_height(label);

    lv_obj_set_height(container, height + 62);
    lv_obj_update_layout(container);

    return container;
}

static lv_obj_t *CreateOverviewContractDataView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView)
{
    cJSON *contractData = cJSON_Parse(data->contract_data);
    int size = cJSON_GetArraySize(contractData);
    printf("size: %d\n", size);

    lv_obj_t *tempLastView = NULL;

    for (size_t i = 0; i < size; i++) {
        cJSON *data = cJSON_GetArrayItem(contractData, i);
        char* title = cJSON_GetObjectItem(data, "title")->valuestring;
        char* value = cJSON_GetObjectItem(data, "value")->valuestring;

        //100 = 16(padding top) + 16(padding bottom) + 30(title) + 8(margin) + 30(value one line)
        lv_obj_t *container = createContentContainer(parent, 408, 100);
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

        lv_obj_t *label = GuiCreateIllustrateLabel(container, title);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
        lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(container, value);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);
        lv_obj_set_width(label, 360);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(label);

        uint16_t height = lv_obj_get_height(label);

        lv_obj_set_height(container, height + 70);
        lv_obj_update_layout(container);

        tempLastView = container;
    }

    return tempLastView;
}

static lv_obj_t *CreateDetailsDataViewView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView)
{
    lv_obj_t *container = createContentContainer(parent, 408, 244);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *label = GuiCreateTextLabel(container, _("Data View"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, data->data_view);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 62);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    return container;
}
static lv_obj_t *CreateDetailsRawDataView(lv_obj_t *parent, DisplayTonTransaction *data, lv_obj_t *lastView)
{
    lv_obj_t *container = createContentContainer(parent, 408, 244);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    if (lastView != NULL) {
        lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    lv_obj_t *label = GuiCreateTextLabel(container, _("Raw Data"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, data->raw_data);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 62);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    return container;
}

void GuiTonProofOverview(lv_obj_t *parent, void *totalData)
{
    DisplayTonProof *txData = (DisplayTonProof *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *container = createContentContainer(parent, 408, 382);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *label = GuiCreateTextLabel(container, _("Domain"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, txData->domain);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 62);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    char address[128] = {0};
    snprintf_s(address, 128, "%.22s\n%s", txData->address, &txData->address[22]);

    // todo translate
    label = GuiCreateTextLabel(container, "Address");
    // label = GuiCreateTextLabel(container, _("Address"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 100);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, address);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 138);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    label = GuiCreateTextLabel(container, _("Payload"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 214);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, txData->payload);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 252);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

}
void GuiTonProofRawData(lv_obj_t *parent, void *totalData)
{
    DisplayTonProof *txData = (DisplayTonProof *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *container = createContentContainer(parent, 408, 382);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, _("Raw Data"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    label = GuiCreateTextLabel(container, txData->raw_message);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 62);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
}