#include "gui_ar.h"
#include "gui_chain_components.h"
#include "rsa.h"
#include "user_utils.h"
#include "gui_api.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static ArweaveRequestType g_requestType = ArweaveRequestTypeTransaction;
static void *g_parseResult = NULL;
static char g_messageAddress[44];
static bool g_isAoTransfer = false;

#define AR_COMPONENT_WIDTH 376
#define AR_COMPONENT_CONTENT_WIDTH (AR_COMPONENT_WIDTH - 48)

#define CHECK_FREE_PARSE_RESULT(result)                                                                                           \
    if (result != NULL)                                                                                                           \
    {                                                                                                                             \
        switch (g_requestType)                                                                                                    \
        {                                                                                                                         \
        case ArweaveRequestTypeTransaction:                                                                                       \
            free_TransactionParseResult_DisplayArweaveTx((PtrT_TransactionParseResult_DisplayArweaveTx)result);                   \
            break;                                                                                                                \
        case ArweaveRequestTypeMessage:                                                                                           \
            free_TransactionParseResult_DisplayArweaveMessage((PtrT_TransactionParseResult_DisplayArweaveMessage)result);         \
            break;                                                                                                                \
        default:                                                                                                                  \
            break;                                                                                                                \
        }                                                                                                                         \
        result = NULL;                                                                                                            \
    }

static void ParseRequestType();
static void GuiArPrepareComponentParent(lv_obj_t *parent, uint16_t height);
static lv_obj_t *GuiArCreatePagedMessageView(lv_obj_t *parent, const char *title, const char *value, bool utf8, lv_obj_t *lastView);
static lv_obj_t *GuiArCreateTxDetailSummary(lv_obj_t *parent, DisplayArweaveTx *txData);
static void GuiArCreateTxTagsCard(lv_obj_t *parent, cJSON *root, lv_obj_t *lastView);

static void ParseRequestType()
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    SimpleResponse_ArweaveRequestType *requestType = ar_request_type(data);
    if (requestType->error_code != 0) {
        g_requestType = ArweaveRequestTypeUnknown;
    }
    g_requestType = *requestType->data;
}

PtrT_TransactionCheckResult GuiGetArCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return ar_check_tx(data, mfp, sizeof(mfp));
}

void GuiArSetMessageAddress(const char *address)
{
    CLEAR_ARRAY(g_messageAddress);
    if (address != NULL) {
        strcpy_s(g_messageAddress, sizeof(g_messageAddress), address);
    }
}

static void GuiArGetMessageAddress(char *address, uint32_t maxLen)
{
    SimpleResponse_c_char *result = fix_arweave_address(g_messageAddress);
    if (result == NULL) {
        return;
    }
    if (result->error_code == SUCCESS_CODE && result->data != NULL) {
        strcpy_s(address, maxLen, result->data);
    }
    free_simple_response_c_char(result);
}

void GuiSetArUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    GuiArSetMessageAddress(NULL);
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    ParseRequestType();
}

void *GuiGetArData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    do {
        if (g_requestType == ArweaveRequestTypeUnknown) {
            printf("Unknown request type\n");
            break;
        }
        if (g_requestType == ArweaveRequestTypeTransaction) {
            PtrT_TransactionParseResult_DisplayArweaveTx parseResult = ar_parse(data);
            CHECK_CHAIN_BREAK(parseResult);
            g_parseResult = (void *)parseResult;
        } else if (g_requestType == ArweaveRequestTypeMessage) {
            PtrT_TransactionParseResult_DisplayArweaveMessage parseResult = ar_message_parse(data);
            CHECK_CHAIN_BREAK(parseResult);
            g_parseResult = (void *)parseResult;
        } else if (g_requestType == ArweaveRequestTypeDataItem) {
            bool isAoTransfer = ar_is_ao_transfer(data);
            g_isAoTransfer = isAoTransfer;
            if (g_isAoTransfer) {
                PtrT_TransactionParseResult_DisplayArweaveAOTransfer parseResult = ar_parse_ao_transfer(data);
                CHECK_CHAIN_BREAK(parseResult);
                g_parseResult = (void *)parseResult;
            } else {
                PtrT_TransactionParseResult_DisplayArweaveDataItem parseResult = ar_parse_data_item(data);
                CHECK_CHAIN_BREAK(parseResult);
                g_parseResult = (void *)parseResult;
            }
        }
    } while (0);
    return g_parseResult;
}

void FreeArMemory(void)
{
    GuiArSetMessageAddress(NULL);
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

static void GuiArPrepareComponentParent(lv_obj_t *parent, uint16_t height)
{
    lv_obj_set_size(parent, AR_COMPONENT_WIDTH, height);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
}

void GuiArTxOverview(lv_obj_t *parent, void *totalData)
{
    DisplayArweaveTx *txData = (DisplayArweaveTx *)totalData;
    GuiArPrepareComponentParent(parent, 444);

    lv_obj_t *lastView = CreateTransactionOverviewCardWithWidth(
        parent,
        _("Value"),
        txData->value,
        _("Fee"),
        txData->fee,
        AR_COMPONENT_WIDTH);
    lastView = CreateTransactionItemViewWithWidth(parent, _("From"), txData->from, lastView, AR_COMPONENT_WIDTH);
    CreateTransactionItemViewWithWidth(parent, _("Destination"), txData->to, lastView, AR_COMPONENT_WIDTH);
}

void GuiArTxDetails(lv_obj_t *parent, void *totalData)
{
    DisplayArweaveTx *txData = (DisplayArweaveTx *)totalData;
    GuiArPrepareComponentParent(parent, 444);

    lv_obj_t *lastView = GuiArCreateTxDetailSummary(parent, txData);

    cJSON *root = txData->detail == NULL ? NULL : cJSON_Parse((const char *)txData->detail);
    if (!cJSON_IsArray(root)) {
        cJSON_Delete(root);
        return;
    }

    GuiArCreateTxTagsCard(parent, root, lastView);
    cJSON_Delete(root);
}

static lv_obj_t *GuiArCreateDetailLabel(lv_obj_t *parent, const char *text, int16_t x, int16_t y, lv_opa_t opa)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, text == NULL ? "" : text);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, x, y);
    lv_obj_set_style_text_opa(label, opa, LV_PART_MAIN);
    return label;
}

static lv_obj_t *GuiArCreateTxDetailSummary(lv_obj_t *parent, DisplayArweaveTx *txData)
{
    lv_obj_t *card = CreateRelativeTransactionContentContainer(parent, AR_COMPONENT_WIDTH, 358, NULL);
    lv_obj_t *section = GuiArCreateDetailLabel(card, "#1", 24, 16, LV_OPA_COVER);
    lv_obj_set_style_text_color(section, lv_color_hex(16090890), LV_PART_MAIN);

    lv_obj_t *title = GuiArCreateDetailLabel(card, _("Value"), 24, 62, LV_OPA_64);
    lv_obj_t *value = GuiArCreateDetailLabel(card, txData->value, 0, 62, LV_OPA_COVER);
    lv_obj_set_style_text_color(value, lv_color_hex(16090890), LV_PART_MAIN);
    lv_obj_align_to(value, title, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    title = GuiArCreateDetailLabel(card, _("Fee"), 24, 100, LV_OPA_64);
    value = GuiArCreateDetailLabel(card, txData->fee, 0, 100, LV_OPA_COVER);
    lv_obj_align_to(value, title, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    GuiArCreateDetailLabel(card, _("From"), 24, 138, LV_OPA_64);
    value = GuiArCreateDetailLabel(card, txData->from, 24, 176, LV_OPA_COVER);
    lv_obj_set_width(value, AR_COMPONENT_CONTENT_WIDTH);
    lv_label_set_long_mode(value, LV_LABEL_LONG_WRAP);

    GuiArCreateDetailLabel(card, _("To"), 24, 244, LV_OPA_64);
    value = GuiArCreateDetailLabel(card, txData->to, 24, 282, LV_OPA_COVER);
    lv_obj_set_width(value, AR_COMPONENT_CONTENT_WIDTH);
    lv_label_set_long_mode(value, LV_LABEL_LONG_WRAP);
    return card;
}

static void GuiArCreateTxTagsCard(lv_obj_t *parent, cJSON *root, lv_obj_t *lastView)
{
    int size = cJSON_GetArraySize(root);
    if (size <= 0) {
        return;
    }

    lv_obj_t *card = CreateRelativeTransactionContentContainer(parent, AR_COMPONENT_WIDTH, 62, lastView);
    lv_obj_t *section = GuiArCreateDetailLabel(card, "#2", 24, 16, LV_OPA_COVER);
    lv_obj_set_style_text_color(section, lv_color_hex(16090890), LV_PART_MAIN);
    int16_t y = 62;

    for (int i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");
        cJSON *value = cJSON_GetObjectItemCaseSensitive(item, "value");
        if (!cJSON_IsString(name) || !cJSON_IsString(value)) {
            continue;
        }

        GuiArCreateDetailLabel(card, _("Name"), 24, y, LV_OPA_64);
        lv_obj_t *text = GuiArCreateDetailLabel(card, name->valuestring, 96, y, LV_OPA_COVER);
        lv_obj_set_style_text_color(text, lv_color_hex(16090890), LV_PART_MAIN);
        lv_obj_set_width(text, AR_COMPONENT_WIDTH - 120);
        lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(text);
        y += LV_MAX(30, lv_obj_get_height(text)) + 8;

        GuiArCreateDetailLabel(card, _("Value"), 24, y, LV_OPA_64);
        text = GuiArCreateDetailLabel(card, value->valuestring, 96, y, LV_OPA_COVER);
        lv_obj_set_width(text, AR_COMPONENT_WIDTH - 120);
        lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(text);
        y += LV_MAX(30, lv_obj_get_height(text)) + 16;
    }

    lv_obj_set_height(card, y);
}

static lv_obj_t *GuiArCreatePagedMessageView(lv_obj_t *parent, const char *title, const char *value, bool utf8, lv_obj_t *lastView)
{
    lv_obj_t *container = CreateRelativeTransactionContentContainer(parent, AR_COMPONENT_WIDTH, 420, lastView);

    lv_obj_t *titleLabel = GuiCreateIllustrateLabel(container, title);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(16090890), LV_PART_MAIN);
    lv_obj_set_style_text_opa(titleLabel, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_t *content = GuiCreateContainerWithParent(container, AR_COMPONENT_CONTENT_WIDTH, 350);
    lv_obj_align(content, LV_ALIGN_TOP_LEFT, 24, 54);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, LV_PART_MAIN);
    GuiShowPagedMessageText(content, value, utf8, NULL, NULL);
    return container;
}

void GuiArMessageOverview(lv_obj_t *parent, void *totalData)
{
    DisplayArweaveMessage *messageData = (DisplayArweaveMessage *)totalData;
    GuiArPrepareComponentParent(parent, 542);

    char address[128] = {0};
    GuiArGetMessageAddress(address, sizeof(address));

    lv_obj_t *lastView = NULL;
    lastView = CreateTransactionItemViewWithWidth(parent, _("Address"), address, lastView, AR_COMPONENT_WIDTH);
    lastView = GuiArCreatePagedMessageView(parent, _("Message (UTF-8)"), messageData->message, true, lastView);
    GuiArCreatePagedMessageView(parent, _("Raw Message"), messageData->raw_message, false, lastView);
}

UREncodeResult *GuiGetArweaveSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult = NULL;
    Rsa_primes_t *primes = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        int32_t ret = LoadAndValidateArKey(SecretCacheGetPassword(), &primes, NULL);
        if (ret != SUCCESS_CODE) {
            GuiApiEmitSignal(SIG_SETUP_RSA_PRIVATE_KEY_WRITE_FAIL, &ret, sizeof(ret));
            break;
        }
        encodeResult = ar_sign_tx(data, primes->p, SPI_FLASH_RSA_PRIME_SIZE, primes->q, SPI_FLASH_RSA_PRIME_SIZE);
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);

    if (primes) {
        memset_s(primes->p, SPI_FLASH_RSA_PRIME_SIZE, 0, SPI_FLASH_RSA_PRIME_SIZE);
        memset_s(primes->q, SPI_FLASH_RSA_PRIME_SIZE, 0, SPI_FLASH_RSA_PRIME_SIZE);
        memset_s(primes, sizeof(Rsa_primes_t), 0, sizeof(Rsa_primes_t));
        SRAM_FREE(primes);
    }
    ClearSecretCache();
    SetLockScreen(enable);
    return encodeResult;
}

static void GuiArRenderAOTransferOverview(lv_obj_t *parent, DisplayArweaveAOTransfer *txData);
static void GuiArRenderAOTransferDetail(lv_obj_t *parent, DisplayArweaveAOTransfer *txData);
static void GuiArRenderDataItemOverview(lv_obj_t *parent, DisplayArweaveDataItem *txData);
static void GuiArRenderDataItemDetail(lv_obj_t *parent, DisplayArweaveDataItem *txData);

void GuiArDataItemOverview(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, AR_COMPONENT_WIDTH, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    if (g_isAoTransfer) {
        DisplayArweaveAOTransfer *txData = (DisplayArweaveAOTransfer *)totalData;
        GuiArRenderAOTransferOverview(parent, txData);
    } else {
        DisplayArweaveDataItem *txData = (DisplayArweaveDataItem *)totalData;
        GuiArRenderDataItemOverview(parent, txData);
    }
}
void GuiArDataItemDetail(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, AR_COMPONENT_WIDTH, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    if (g_isAoTransfer) {
        DisplayArweaveAOTransfer *txData = (DisplayArweaveAOTransfer *)totalData;
        GuiArRenderAOTransferDetail(parent, txData);
    } else {
        DisplayArweaveDataItem *txData = (DisplayArweaveDataItem *)totalData;
        GuiArRenderDataItemDetail(parent, txData);
    }
}

static void GuiArRenderAOTransferOverview(lv_obj_t *parent, DisplayArweaveAOTransfer *txData)
{
    lv_obj_t *lastView = NULL;
    lastView = CreateTransactionItemViewWithWidth(parent, _("Action"), _("AO Transfer"), lastView, AR_COMPONENT_WIDTH);
    lastView = CreateTransactionItemViewWithWidth(parent, _("From"), txData->from, lastView, AR_COMPONENT_WIDTH);
    lastView = CreateTransactionItemViewWithWidth(parent, _("Destination"), txData->to, lastView, AR_COMPONENT_WIDTH);
    lastView = CreateTransactionItemViewWithWidth(parent, _("Quantity"), txData->quantity, lastView, AR_COMPONENT_WIDTH);
    lastView = CreateTransactionItemViewWithWidth(parent, _("Token ID"), txData->token_id, lastView, AR_COMPONENT_WIDTH);
}

static void GuiArRenderAOTransferDetail(lv_obj_t *parent, DisplayArweaveAOTransfer *txData)
{
    lv_obj_t *lastView = NULL;
    for (size_t i = 0; i < txData->other_info->size; i++) {
        lastView = CreateTransactionItemViewWithWidth(parent, txData->other_info->data[i].name, txData->other_info->data[i].value, lastView, AR_COMPONENT_WIDTH);
    }
}

static void GuiArRenderDataItemOverview(lv_obj_t *parent, DisplayArweaveDataItem *txData)
{
    lv_obj_t *lastView = NULL;
    lastView = CreateTransactionItemViewWithWidth(parent, _("Action"), _("Sign DataItem"), lastView, AR_COMPONENT_WIDTH);
    lastView = CreateTransactionItemViewWithWidth(parent, _("Owner"), txData->owner, lastView, AR_COMPONENT_WIDTH);
    if (txData->target != NULL) {
        lastView = CreateTransactionItemViewWithWidth(parent, _("Target"), txData->target, lastView, AR_COMPONENT_WIDTH);
    }
    if (txData->anchor != NULL) {
        lastView = CreateTransactionItemViewWithWidth(parent, _("Anchor"), txData->anchor, lastView, AR_COMPONENT_WIDTH);
    }
    lastView = CreateTransactionItemViewWithWidth(parent, _("Data"), txData->data, lastView, AR_COMPONENT_WIDTH);
}
static void GuiArRenderDataItemDetail(lv_obj_t *parent, DisplayArweaveDataItem *txData)
{
    lv_obj_t *lastView = NULL;
    for (size_t i = 0; i < txData->tags->size; i++) {
        lastView = CreateTransactionItemViewWithWidth(parent, txData->tags->data[i].name, txData->tags->data[i].value, lastView, AR_COMPONENT_WIDTH);
    }
}
