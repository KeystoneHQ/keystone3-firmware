#ifndef BTC_ONLY
#include "gui_avax.h"
#include "rust.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "gui_chain.h"
#include "gui_chain_components.h"

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

static lv_obj_t *CreateOverviewAmountView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView);
static lv_obj_t *CreateOverviewActionView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView);
static lv_obj_t *CreateOverviewDestinationView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView);
static lv_obj_t *CreateOverviewContractDataView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView);
static lv_obj_t *CreateDetailsDataViewView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView);
static lv_obj_t *CreateDetailsRawDataView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView);

void GuiSetAvaxUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_viewType = g_isMulti ? g_urMultiResult->t : g_urResult->t;
}

UREncodeResult *GuiGetAvaxSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = avax_sign(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
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
        ExtendedPublicKey keys[2];
        public_keys->data = keys;
        public_keys->size = NUMBER_OF_ARRAYS(keys);
        keys[0].path = "m/44'/60'/0'";
        keys[0].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_AVAX_BIP44_STANDARD);
        keys[1].path = "m/44'/9000'/0";
        keys[1].xpub = GetCurrentAccountPublicKey(XPUB_TYPE_AVAX_X_P);
        PtrT_TransactionParseResult_DisplayTonTransaction parseResult = avax_parse_transaction(data, mfp, sizeof(mfp), public_keys);
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
} DisplayUtxoFromTo;

lv_obj_t *CreateTxOverviewFromTo(lv_obj_t *parent, void *from, int fromLen, void *to, int toLen)
{
    int height = 16 + 30 + 8 + (60 + 8) * fromLen - 8 + 16 + 30 + 8 + (60 + 8) * toLen + 16;
    lv_obj_t *container = CreateContentContainer(parent, 408, height);

    DisplayUtxoFromTo *ptr = (DisplayUtxoFromTo *)from;
    lv_obj_t *label = GuiCreateNoticeLabel(container, _("From"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    for (int i = 0; i < fromLen; i++) {
        lv_obj_t *label = GuiCreateIllustrateLabel(container, ptr[i].address);
        lv_obj_set_width(label, 360);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54 + 60 * i);
    }

    ptr = (DisplayUtxoFromTo *)to;
    uint16_t offset = 30 + 8 + (30 + 8) * fromLen + 16;
    label = GuiCreateNoticeLabel(container, _("To"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, offset);
    for (int i = 0; i < toLen; i++) {
        lv_obj_t *label = GuiCreateIllustrateLabel(container, ptr[i].address);
        lv_obj_set_width(label, 360);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 38 + offset + 68 * i);
    }

    return container;
}

lv_obj_t *CreateTxDetailsFromTo(lv_obj_t *parent, char *tag, void *fromTo, int len)
{
    int height = 16 + 30 + 8 + (128 + 8) * len - 8 + 16;
    lv_obj_t *container = CreateContentContainer(parent, 408, height);

    DisplayUtxoFromTo *ptr = (DisplayUtxoFromTo *)fromTo;
    lv_obj_t *label = GuiCreateNoticeLabel(container, tag);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    for (int i = 0; i < len; i++) {
        lv_obj_t *label = GuiCreateIllustrateLabel(container, "");
        lv_label_set_recolor(label, true);
        lv_label_set_text_fmt(label, "%d    #F5870A %s#", i, ptr[i].amount);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54 + (128 + 8) * i);

        label = GuiCreateIllustrateLabel(container, ptr[i].address);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 88 + (128 + 8) * i);
        lv_obj_set_width(label, 360);
    }

    return container;
}

void GuiAvaxTxOverview(lv_obj_t *parent, void *totalData)
{
    DisplayAvaxTx *txData = (DisplayAvaxTx *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *container = CreateValueOverviewValue(parent, txData->data->total_output_amount, txData->data->fee_amount);

    if (txData->data->network != NULL) {
        char *key[] = {txData->data->network_key, "Subnet ID"};
        char *value[] = {txData->data->network, txData->data->subnet_id};
        container = CreateDynamicInfoView(parent, key, value, NUMBER_OF_ARRAYS(key) - (txData->data->subnet_id ? 0 : 1));
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    if (txData->data->method != NULL) {
        container = CreateSingleInfoView(parent, txData->data->method->method_key, txData->data->method->method);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    printf("txData->data->from->size->address: %s\n", txData->data->to->data[0].address);
    container = CreateTxOverviewFromTo(parent, NULL, 0, txData->data->to->data, txData->data->to->size);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_update_layout(parent);
}

void GuiAvaxTxRawData(lv_obj_t *parent, void *totalData)
{
    DisplayAvaxTx *txData = (DisplayAvaxTx *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *container = NULL;
    if (txData->data->network != NULL) {
        char *key[] = {txData->data->network_key, "Subnet ID"};
        char *value[] = {txData->data->network, txData->data->subnet_id};
        container = CreateDynamicInfoView(parent, key, value, NUMBER_OF_ARRAYS(key) - (txData->data->subnet_id ? 0 : 1));
    }

    if (txData->data->method != NULL) {
        char startTime[BUFFER_SIZE_64] = {0}, endTime[BUFFER_SIZE_64] = {0};
        uint8_t keyLen = 1;
        if (txData->data->method->start_time != 0 && txData->data->method->end_time != 0) {
            StampTimeToUtcTime(txData->data->method->start_time, startTime, sizeof(startTime));
            StampTimeToUtcTime(txData->data->method->end_time, endTime, sizeof(endTime));
            keyLen = 3;
        }
        char *key[] = {txData->data->method->method_key, "Start time", "End Time"};
        char *value[] = {txData->data->method->method, startTime, endTime};
        container = CreateDynamicInfoView(parent, key, value, keyLen);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }

    container = CreateValueDetailValue(parent, txData->data->total_input_amount, txData->data->total_output_amount, txData->data->fee_amount);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    container = CreateTxDetailsFromTo(parent, "From", NULL, 0);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    container = CreateTxDetailsFromTo(parent, "To", txData->data->to->data, txData->data->to->size);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_update_layout(parent);
}

static lv_obj_t *CreateDetailsDataViewView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView)
{
}
static lv_obj_t *CreateDetailsRawDataView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView)
{
}
#endif