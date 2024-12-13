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
        MnemonicType type = GetMnemonicType();
        uint8_t seed[64];
        int len = 64;
        switch (type) {
        case MNEMONIC_TYPE_BIP39: {
            len = sizeof(seed);
            break;

        }
        case MNEMONIC_TYPE_SLIP39: {
            len = GetCurrentAccountEntropyLen();
            break;
        }
        default:
            break;
        }
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = ton_sign_transaction(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

PtrT_TransactionCheckResult GuiGetAvaxCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    char* publicKey;
    publicKey = GetCurrentAccountPublicKey(XPUB_TYPE_AVAX_BIP44_STANDARD);
    return avax_check_transaction(data, publicKey);
}

void *GuiGetAvaxGUIData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayTonTransaction parseResult = avax_parse_transaction(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void GuiAvaxTxOverview(lv_obj_t *parent, void *totalData)
{
    DisplayAvaxTx *txData = (DisplayAvaxTx *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *container = CreateValueOverviewValue(parent, txData->overview->total_output_amount, txData->overview->fee_amount);
    container = CreateSingleInfoView(parent, _("Network"), txData->overview->network);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    container = CreateSingleInfoView(parent, _("Method"), txData->overview->method);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_update_layout(parent);
}

void GuiAvaxTxRawData(lv_obj_t *parent, void *totalData)
{
    // DisplayAvaxTx *txData = (DisplayAvaxTx *)totalData;
    // lv_obj_set_size(parent, 408, 444);
    // lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_t *lastView = NULL;
    // if (txData->data_view != NULL) {
    //     lastView = CreateDetailsDataViewView(parent, txData, NULL);
    // }
    // lastView = CreateDetailsRawDataView(parent, txData, lastView);
}

// static lv_obj_t *CreateOverviewCommentView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView)
// {
//     return container;
// }

// static lv_obj_t *CreateOverviewContractDataView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView)
// {
//     cJSON *contractData = cJSON_Parse(data->contract_data);
//     int size = cJSON_GetArraySize(contractData);
//     printf("size: %d\n", size);

//     lv_obj_t *tempLastView = NULL;

//     for (size_t i = 0; i < size; i++) {
//         cJSON *data = cJSON_GetArrayItem(contractData, i);
//         char* title = cJSON_GetObjectItem(data, "title")->valuestring;
//         char* value = cJSON_GetObjectItem(data, "value")->valuestring;

//         //100 = 16(padding top) + 16(padding bottom) + 30(title) + 8(margin) + 30(value one line)
//         lv_obj_t *container = CreateContentContainer(parent, 408, 100);
//         lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

//         lv_obj_t *label = GuiCreateIllustrateLabel(container, title);
//         lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
//         lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

//         label = GuiCreateIllustrateLabel(container, value);
//         lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);
//         lv_obj_set_width(label, 360);
//         lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
//         lv_obj_update_layout(label);

//         uint16_t height = lv_obj_get_height(label);

//         lv_obj_set_height(container, height + 70);
//         lv_obj_update_layout(container);

//         tempLastView = container;
//     }

//     return tempLastView;
// }

static lv_obj_t *CreateDetailsDataViewView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView)
{
    // lv_obj_t *container = CreateContentContainer(parent, 408, 244);
    // lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    // lv_obj_t *label = GuiCreateTextLabel(container, _("Data View"));
    // lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    // lv_label_set_recolor(label, true);
    // lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    // label = GuiCreateIllustrateLabel(container, data->data_view);
    // lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 62);
    // lv_obj_set_width(label, 360);
    // lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    // return container;
}
static lv_obj_t *CreateDetailsRawDataView(lv_obj_t *parent, DisplayAvaxTx *data, lv_obj_t *lastView)
{
    // lv_obj_t *container = CreateContentContainer(parent, 408, 244);
    // lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    // if (lastView != NULL) {
    //     lv_obj_align_to(container, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    // }

    // lv_obj_t *label = GuiCreateTextLabel(container, _("Raw Data"));
    // lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    // lv_label_set_recolor(label, true);
    // lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    // label = GuiCreateIllustrateLabel(container, data->raw_data);
    // lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 62);
    // lv_obj_set_width(label, 360);
    // lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    // return container;
}
#endif