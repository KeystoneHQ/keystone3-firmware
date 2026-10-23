#ifndef BTC_ONLY
#include "gui_avax.h"
#include "rust.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "gui_chain.h"
#include "gui_chain_components.h"
#include "keystore.h"

#define AVAX_COMPONENT_WIDTH 376

#define CHECK_FREE_PARSE_RESULT(result)                                     \
    if (result != NULL)                                                     \
    {                                                                       \
        free_TransactionParseResult_DisplayAvaxTx(g_parseResult);           \
        g_parseResult = NULL;                                               \
    }

static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static bool g_isMulti = false;
static ViewType g_viewType = ViewTypeUnKnown;

UREncodeResult *GetAvaxSignDataDynamic(bool isUnlimited);

void GuiSetAvaxUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_viewType = g_isMulti ? g_urMultiResult->t : g_urResult->t;
}

UREncodeResult *GuiGetAvaxSignQrCodeData(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    return SignInternal(avax_sign, data);
}

UREncodeResult *GuiGetAvaxSignUrDataUnlimited(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    return SignInternal(avax_sign_unlimited, data);
}

PtrT_TransactionCheckResult GuiGetAvaxCheckResult(void)
{
    uint8_t mfp[4] = {0};
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return avax_check_transaction(data, mfp, sizeof(mfp));
}

void *GuiGetAvaxGUIData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t mfp[4] = {0};
        GetMasterFingerPrint(mfp);
        PtrT_CSliceFFI_ExtendedPublicKey public_keys = SRAM_MALLOC(sizeof(CSliceFFI_ExtendedPublicKey));
        ExtendedPublicKey keys[11];
        public_keys->data = keys;
        public_keys->size = NUMBER_OF_ARRAYS(keys);
        keys[0].path = "m/44'/60'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_AVAX_BIP44_STANDARD);
        for (int i = 0; i < 10; i++) {
            keys[1 + i].path = GetCurrentAccountPath(XPUB_TYPE_AVAX_X_P_0 + i);
            keys[1 + i].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_AVAX_X_P_0 + i);
        }
        PtrT_TransactionParseResult_DisplayAvaxTx parseResult = avax_parse_transaction(data, mfp, sizeof(mfp), public_keys);
        SRAM_FREE(public_keys);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}
typedef struct {
    char *address;
    char *amount;
    char *path;
    bool isChange;
} DisplayUtxoFromTo;

static void GuiAvaxPrepareComponentParent(lv_obj_t *parent)
{
    lv_obj_set_size(parent, AVAX_COMPONENT_WIDTH, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
}

static lv_obj_t *GuiAvaxCreateDetailsAddressCard(
    lv_obj_t *parent,
    lv_obj_t *lastView,
    const char *title,
    const DisplayUtxoFromTo *item)
{
    lv_obj_t *card = CreateRelativeTransactionContentContainer(
                         parent, AVAX_COMPONENT_WIDTH, 0, lastView);
    uint16_t height = 16;

    lv_obj_t *label = GuiCreateIllustrateLabel(card, title);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    height += 34;

    label = GuiCreateIllustrateLabel(card, item->amount);
    lv_obj_set_width(label, AVAX_COMPONENT_WIDTH - 48);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_update_layout(label);
    height += lv_obj_get_height(label) + 4;

    label = GuiCreateIllustrateLabel(card, item->address);
    lv_obj_set_width(label, AVAX_COMPONENT_WIDTH - 48);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_update_layout(label);
    height += lv_obj_get_height(label);

    if (item->path != NULL && item->path[0] != '\0') {
        height += 4;
        label = GuiCreateNoticeLabel(card, item->path);
        lv_obj_set_width(label, AVAX_COMPONENT_WIDTH - 48);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
        lv_obj_update_layout(label);
        height += lv_obj_get_height(label);
    }

    lv_obj_set_height(card, height + 16);
    lv_obj_update_layout(card);
    return card;
}

static lv_obj_t *GuiAvaxAppendAddresses(
    lv_obj_t *parent,
    lv_obj_t *lastView,
    const char *tag,
    void *fromTo,
    int len,
    bool showDetails)
{
    DisplayUtxoFromTo *ptr = (DisplayUtxoFromTo *)fromTo;
    for (int i = 0; i < len; i++) {
        char title[BUFFER_SIZE_64] = {0};
        if (len > 1) {
            snprintf_s(title, sizeof(title), "%s #%d%s", tag, i + 1,
                       ptr[i].isChange && !strcmp(tag, "To") ? " (Change)" : "");
        } else {
            snprintf_s(title, sizeof(title), "%s%s", tag,
                       ptr[i].isChange && !strcmp(tag, "To") ? " (Change)" : "");
        }

        if (showDetails) {
            lastView = GuiAvaxCreateDetailsAddressCard(parent, lastView, title, &ptr[i]);
        } else {
            lastView = CreateTransactionItemViewWithWidth(
                           parent,
                           title,
                           ptr[i].address,
                           lastView,
                           AVAX_COMPONENT_WIDTH);
        }
    }
    return lastView;
}

void GuiAvaxTxOverview(lv_obj_t *parent, void *totalData)
{
    DisplayAvaxTx *txData = (DisplayAvaxTx *)totalData;
    GuiAvaxPrepareComponentParent(parent);

    lv_obj_t *lastView = CreateTransactionOverviewCardWithWidth(
                             parent,
                             _("Value"),
                             txData->data->amount,
                             _("Fee"),
                             txData->data->fee_amount,
                             AVAX_COMPONENT_WIDTH);

    if (txData->data->network != NULL) {
        lastView = CreateTransactionItemViewWithWidth(
                       parent,
                       txData->data->network_key,
                       txData->data->network,
                       lastView,
                       AVAX_COMPONENT_WIDTH);
        if (txData->data->subnet_id != NULL) {
            lastView = CreateTransactionItemViewWithWidth(
                           parent, _("Subnet ID"), txData->data->subnet_id, lastView, AVAX_COMPONENT_WIDTH);
        }
    }

    if (txData->data->method != NULL) {
        lastView = CreateTransactionItemViewWithWidth(
                       parent,
                       txData->data->method->method_key,
                       txData->data->method->method,
                       lastView,
                       AVAX_COMPONENT_WIDTH);
    }

    lastView = GuiAvaxAppendAddresses(
                   parent, lastView, _("From"), txData->data->from->data, txData->data->from->size, false);
    GuiAvaxAppendAddresses(
        parent, lastView, _("To"), txData->data->to->data, txData->data->to->size, false);
}

void GuiAvaxTxRawData(lv_obj_t *parent, void *totalData)
{
    DisplayAvaxTx *txData = (DisplayAvaxTx *)totalData;
    GuiAvaxPrepareComponentParent(parent);

    lv_obj_t *lastView = NULL;
    if (txData->data->network != NULL) {
        lastView = CreateTransactionItemViewWithWidth(
                       parent,
                       txData->data->network_key,
                       txData->data->network,
                       lastView,
                       AVAX_COMPONENT_WIDTH);
        if (txData->data->subnet_id != NULL) {
            lastView = CreateTransactionItemViewWithWidth(
                           parent, _("Subnet ID"), txData->data->subnet_id, lastView, AVAX_COMPONENT_WIDTH);
        }
    }

    if (txData->data->method != NULL) {
        char startTime[BUFFER_SIZE_64] = {0}, endTime[BUFFER_SIZE_64] = {0};
        lastView = CreateTransactionItemViewWithWidth(
                       parent,
                       txData->data->method->method_key,
                       txData->data->method->method,
                       lastView,
                       AVAX_COMPONENT_WIDTH);
        if (txData->data->method->start_time != 0 && txData->data->method->end_time != 0) {
            StampTimeToUtcTime(txData->data->method->start_time, startTime, sizeof(startTime));
            StampTimeToUtcTime(txData->data->method->end_time, endTime, sizeof(endTime));
            lastView = CreateTransactionItemViewWithWidth(
                           parent, _("Start time"), startTime, lastView, AVAX_COMPONENT_WIDTH);
            lastView = CreateTransactionItemViewWithWidth(
                           parent, _("End Time"), endTime, lastView, AVAX_COMPONENT_WIDTH);
        }
    }

    lastView = CreateTransactionItemViewWithWidth(
                   parent, _("Total Input"), txData->data->total_input_amount, lastView, AVAX_COMPONENT_WIDTH);
    lastView = CreateTransactionItemViewWithWidth(
                   parent, _("Total Output"), txData->data->total_output_amount, lastView, AVAX_COMPONENT_WIDTH);
    lastView = CreateTransactionItemViewWithWidth(
                   parent, _("Fee"), txData->data->fee_amount, lastView, AVAX_COMPONENT_WIDTH);

    if (txData->data->reward_address != NULL) {
        lastView = CreateTransactionItemViewWithWidth(
                       parent, _("Reward Address"), txData->data->reward_address, lastView, AVAX_COMPONENT_WIDTH);
    }

    lastView = GuiAvaxAppendAddresses(
                   parent, lastView, _("From"), txData->data->from->data, txData->data->from->size, true);
    GuiAvaxAppendAddresses(
        parent, lastView, _("To"), txData->data->to->data, txData->data->to->size, true);
}

void FreeAvaxMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}
#endif
