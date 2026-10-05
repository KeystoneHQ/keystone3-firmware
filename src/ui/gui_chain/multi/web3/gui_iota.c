#include "define.h"
#include "rust.h"
#include "keystore.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "account_public_info.h"
#include "secret_cache.h"
#include "user_memory.h"
#include "gui_chain_components.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static bool g_isSignMessageHash = false;

void GuiSetIotaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_isSignMessageHash = false;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                                   \
    if (result != NULL)                                                                                                   \
    {                                                                                                                     \
        if (g_isSignMessageHash)                                                                                          \
        {                                                                                                                 \
            free_TransactionParseResult_DisplayIotaSignMessageHash((PtrT_TransactionParseResult_DisplayIotaSignMessageHash)result);                                     \
        } else                                                                                                            \
        {                                                                                                                 \
            free_TransactionParseResult_DisplayIotaIntentData((PtrT_TransactionParseResult_DisplayIotaIntentData)result); \
        }                                                                                                                 \
        result = NULL;                                                                                                    \
    }                                                                                                                     \


void *GuiGetIotaData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    ExtendedPublicKey keys[XPUB_TYPE_IOTA_9 - XPUB_TYPE_IOTA_0 + 1];
    for (uint32_t i = 0; i < NUMBER_OF_ARRAYS(keys); i++) {
        ChainType chain = XPUB_TYPE_IOTA_0 + i;
        keys[i].path = GetCurrentAccountPath(chain);
        keys[i].xpub = GetCurrentAccountPublicKey(chain);
    }
    do {
        PtrT_TransactionParseResult_DisplayIotaIntentData parseResult = iota_parse_intent(data, keys, NUMBER_OF_ARRAYS(keys));
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void *GuiGetIotaSignMessageHashData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayIotaSignMessageHash parseResult = iota_parse_sign_message_hash(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_isSignMessageHash = true;
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetIotaCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    PtrT_TransactionCheckResult result = sui_check_request(data, mfp, sizeof(mfp));
    return result;
}

PtrT_TransactionCheckResult GuiGetIotaSignHashCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return sui_check_sign_hash_request(data, mfp, sizeof(mfp));
}

void FreeIotaMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

UREncodeResult *GuiGetIotaSignQrCodeData(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    return SignInternal(iota_sign_intent, data);
}

bool GetIotaIsTransaction(void *indata, void *param)
{
    return !GetIotaIsMessage(indata, param);
}

bool GetIotaShowOverview(void *indata, void *param)
{
    if (GetIotaIsMessage(indata, param)) {
        return false;
    }
    DisplayIotaIntentData *iota = (DisplayIotaIntentData *)param;
    return iota->method != NULL;
}

bool GetIotaIsMessage(void *indata, void *param)
{
    DisplayIotaIntentData *iota = (DisplayIotaIntentData *)param;
    return !strcmp(iota->transaction_type, "Message");
}

bool IsIotaMsg(ViewType viewType)
{
    if (viewType != IotaTx || g_isSignMessageHash || g_parseResult == NULL) {
        return false;
    }
    PtrT_TransactionParseResult_DisplayIotaIntentData result = (PtrT_TransactionParseResult_DisplayIotaIntentData)g_parseResult;
    return result->data != NULL && result->data->transaction_type != NULL &&
           GetIotaIsMessage(NULL, result->data);
}

static lv_obj_t *CreateIotaRawMessageView(lv_obj_t *parent, const char *rawData)
{
    const uint16_t width = 408;
    const uint16_t contentWidth = width - 48;
    const uint16_t rawDataY = 54;

    lv_obj_t *container = CreateContentContainer(parent, width, 1);

    lv_obj_t *label = GuiCreateIllustrateLabel(container, "Raw Data");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, rawData);
    lv_obj_set_width(label, contentWidth);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, rawDataY);
    lv_obj_update_layout(label);

    uint16_t noticeY = rawDataY + lv_obj_get_self_height(label) + 24;
    label = GuiCreateIllustrateLabel(container, _("iota_message_raw_data_notice"));
    lv_obj_set_width(label, contentWidth);
    lv_obj_set_style_text_color(label, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, noticeY);
    lv_obj_update_layout(label);
    lv_obj_set_height(container, noticeY + lv_obj_get_self_height(label) + 24);

    return container;
}

void GuiIotaTxOverviewMessage(lv_obj_t *parent, void *totalData)
{
    DisplayIotaIntentData *txData = (DisplayIotaIntentData *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *container = CreateSingleInfoTwoLineView(parent, "Address", txData->sender);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 0);

    if (txData->details != NULL) {
        container = CreateIotaRawMessageView(parent, txData->details);
    } else {
        container = CreateSingleInfoTwoLineView(parent, "Message", txData->message);
    }
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

void GuiIotaTxOverview(lv_obj_t *parent, void *totalData)
{
    if (GetIotaIsMessage(NULL, totalData)) {
        GuiIotaTxOverviewMessage(parent, totalData);
        return;
    }
    DisplayIotaIntentData *txData = (DisplayIotaIntentData *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);

    bool isStake = txData->method != NULL && !strcmp(txData->method, "Stake");

    lv_obj_t *container = NULL;
    if (txData->gas_sponsored) {
        container = CreateTitledNoticeCard(parent, "Gas Sponsored", _("iota_gas_sponsor_notice"), 408);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }

    bool maxGasFeeShown = false;
    if (txData->amount != NULL) {
        if (strcmp(txData->amount, "max") == 0) {
            container = CreateNoticeView(parent, 408, 212, _("iota_max_amount_notice"));
        } else {
            container = CreateValueOverviewValue(parent, "amount", txData->amount, "Maximum Gas Fee", txData->max_gas_fee);
            maxGasFeeShown = true;
        }
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }

    if (txData->max_gas_fee != NULL && !maxGasFeeShown) {
        container = CreateSingleInfoView(parent, "Maximum Gas Fee", txData->max_gas_fee);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }

    if (txData->method != NULL) {
        container = CreateSingleInfoView(parent, "Method", txData->method);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }

    container = CreateSingleInfoTwoLineView(parent, isStake ? "sender" : "From", txData->sender);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    if (txData->recipient != NULL) {
        container = CreateSingleInfoTwoLineView(parent, isStake ? "Validator" : "To", txData->recipient);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }
}

void GuiIotaTxRawData(lv_obj_t *parent, void *totalData)
{
    DisplayIotaIntentData *txData = (DisplayIotaIntentData *)totalData;
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_size(parent, 408, 444);

    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    if (txData->method == NULL) {
        lv_obj_t *notice = CreateTitledNoticeCard(parent, _("iota_unknown_transaction_title"), _("iota_unknown_transaction_desc"), 408);
        lv_obj_set_style_bg_color(notice, RED_COLOR, LV_PART_MAIN);
        lv_obj_align(notice, LV_ALIGN_TOP_LEFT, 0, 0);
    }

    lv_obj_t *container = CreateContentContainer(parent, 408, 444);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *rawDataLabel = GuiCreateIllustrateLabel(container, txData->details);
    lv_obj_set_width(rawDataLabel, 360);
    lv_obj_align(rawDataLabel, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_pad_bottom(rawDataLabel, 16, LV_PART_MAIN);
}

UREncodeResult *GuiGetIotaSignHashQrCodeData(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    return SignInternal(iota_sign_hash, data);
}
