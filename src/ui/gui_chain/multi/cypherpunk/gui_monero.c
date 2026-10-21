#include "gui_monero.h"
#include "gui_chain_components.h"
#include "account_manager.h"
#include "account_public_info.h"
#include "screen_manager.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static XmrRequestType g_requestType = OutputRequest;

#define CHECK_FREE_PARSE_RESULT(result)                                                                         \
  if (result != NULL)                                                                                           \
  {                                                                                                             \
    switch (g_requestType)                                                                                      \
    {                                                                                                           \
    case OutputRequest:                                                                                         \
      free_TransactionParseResult_DisplayMoneroOutput((PtrT_TransactionParseResult_DisplayMoneroOutput)result); \
      break;                                                                                                    \
    case UnsignedTxRequest:                                                                                     \
      free_TransactionParseResult_DisplayMoneroUnsignedTx((PtrT_TransactionParseResult_DisplayMoneroUnsignedTx)result); \
      break;                                                                                                    \
    }                                                                                                           \
    result = NULL;                                                                                              \
  }

static void ShowHintBox(lv_event_t *e);
static void SetUpMoneroDecryptKey(uint8_t key[32]);
static lv_obj_t *CreateNetworkContainer(lv_obj_t *container);

static void CloseAllHintBox(void)
{
    CloseTooltipHintBoxHandler();
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }
}

static void ShowHintBox(lv_event_t *e)
{
    GuiCreateTooltipHintBox(_("xmr_txo_total_amount_title"), _("xmr_txo_total_amount_desc"), _("xmr_txo_total_amount_link"));
}



static lv_obj_t *CreateNetworkContainer(lv_obj_t *container)
{
    lv_obj_t *networkContainer = GuiCreateContainerWithParent(container, 408, 62);
    GuiSetTransactionContainerStyle(networkContainer);
    lv_obj_t *label = GuiCreateIllustrateLabel(networkContainer, "Network");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(networkContainer, "XMR Mainnet");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 120, 16);

    return networkContainer;
}

static void SetUpMoneroDecryptKey(uint8_t key[32])
{
    memset_s(key, 32, 0, 32);
    char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
    SimpleResponse_u8 *decryptKeyData = monero_generate_decrypt_key(pvk);
    if (decryptKeyData == NULL) {
        printf("monero decrypt key unavailable\n");
        return;
    }
    if (decryptKeyData->error_code == SUCCESS_CODE && decryptKeyData->data != NULL) {
        memcpy(key, decryptKeyData->data, 32);
    }
    monero_free_decrypt_key_response(decryptKeyData);
}

PtrT_TransactionCheckResult GuiGetMoneroOutputCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    g_requestType = OutputRequest;
    uint8_t decryptKey[32];
    SetUpMoneroDecryptKey(decryptKey);
    char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
    PtrT_TransactionCheckResult result = monero_output_request_check(data, decryptKey, pvk);
    memset_s(decryptKey, sizeof(decryptKey), 0, sizeof(decryptKey));
    return result;
}

PtrT_TransactionCheckResult GuiGetMoneroUnsignedTxCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    g_requestType = UnsignedTxRequest;
    uint8_t decryptKey[32];
    SetUpMoneroDecryptKey(decryptKey);
    char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
    char *pubSpendKey = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_0);
    PtrT_TransactionCheckResult result = monero_unsigned_request_check(data, decryptKey, pvk, pubSpendKey);
    memset_s(decryptKey, sizeof(decryptKey), 0, sizeof(decryptKey));
    return result;
}

void GuiSetMoneroUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

void *GuiGetMoneroOutputData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t decryptKey[32];
    SetUpMoneroDecryptKey(decryptKey);
    do {
        char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
        PtrT_TransactionParseResult_DisplayMoneroOutput parseResult = monero_parse_output(data, decryptKey, pvk);
        memset_s(decryptKey, sizeof(decryptKey), 0, sizeof(decryptKey));
        if (parseResult == NULL) {
            break;
        }
        if (parseResult->error_code != 0) {
            printf("monero parse error %d: %s\n", (int)parseResult->error_code, parseResult->error_message);
            free_TransactionParseResult_DisplayMoneroOutput(parseResult);
            break;
        }
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void *GuiGetMoneroUnsignedTxData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t decryptKey[32];
    SetUpMoneroDecryptKey(decryptKey);
    do {
        char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
        char *pubSpendKey = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_0);
        PtrT_TransactionParseResult_DisplayMoneroUnsignedTx parseResult = monero_parse_unsigned_tx(data, decryptKey, pvk, pubSpendKey);
        memset_s(decryptKey, sizeof(decryptKey), 0, sizeof(decryptKey));
        if (parseResult == NULL) {
            break;
        }
        if (parseResult->error_code != 0) {
            printf("monero parse error %d: %s\n", (int)parseResult->error_code, parseResult->error_message);
            free_TransactionParseResult_DisplayMoneroUnsignedTx(parseResult);
            break;
        }
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

UREncodeResult *GuiGetMoneroKeyimagesQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t seed[64];
    do {
        int len = GetCurrentAccountSeedLen();
        int ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        CHECK_ERRCODE_BREAK("GetAccountSeed", ret);
        encodeResult = monero_generate_keyimage(data, seed, len, 0);
        if (encodeResult == NULL || encodeResult->error_code != 0) {
            break;
        }
    } while (0);
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);
    return encodeResult;
}

UREncodeResult *GuiGetMoneroSignedTransactionQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult = NULL;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t seed[64];
    do {
        int len = GetCurrentAccountSeedLen();
        int ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        CHECK_ERRCODE_BREAK("GetAccountSeed", ret);
        encodeResult = monero_generate_signature(data, seed, len, 0);
        if (encodeResult == NULL || encodeResult->error_code != 0) {
            break;
        }
    } while (0);
    memset_s(seed, sizeof(seed), 0, sizeof(seed));
    ClearSecretCache();
    SetLockScreen(enable);
    return encodeResult;
}

void GuiClearMoneroSensitiveState(void)
{
    monero_clear_transaction_state();
}

void FreeMoneroMemory(void)
{
    GuiClearMoneroSensitiveState();
#ifndef COMPILE_SIMULATOR
    CloseAllHintBox();
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    g_requestType = OutputRequest;
#endif
}

void GetXmrTxoCount(void *indata, void *param, uint32_t maxLen)
{
    DisplayMoneroOutput *data = (DisplayMoneroOutput *)param;
    if (data->txos_num == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->txos_num);
}

void GetXmrTotalAmount(void *indata, void *param, uint32_t maxLen)
{
    DisplayMoneroOutput *data = (DisplayMoneroOutput *)param;
    if (data->total_amount == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, data->total_amount);
}

void GuiShowXmrOutputsDetails(lv_obj_t *parent, void *totalData)
{
    DisplayMoneroOutput *data = (DisplayMoneroOutput *)totalData;

    lv_obj_set_width(parent, 408);
    lv_obj_set_height(parent, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_bottom(parent, 24, LV_PART_MAIN);

    lv_obj_t * infoContainer = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    GuiSetTransactionContainerStyle(infoContainer);
    lv_obj_set_style_pad_bottom(infoContainer, 16, LV_PART_MAIN);
    lv_obj_t *label = GuiCreateIllustrateLabel(infoContainer, "Number of TXOs");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(infoContainer, data->txos_num);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 195, 16);

    label = GuiCreateIllustrateLabel(infoContainer, "Total Amount");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 53);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    lv_obj_t *infoIcon = GuiCreateImg(infoContainer, &imgInfo);
    lv_obj_align_to(infoIcon, label, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    lv_obj_add_flag(infoIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(infoIcon, ShowHintBox, LV_EVENT_CLICKED, NULL);

    lv_obj_t *amountLabel = GuiCreateIllustrateLabel(infoContainer, data->total_amount);
    lv_obj_set_width(amountLabel, 360);
    lv_label_set_long_mode(amountLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(amountLabel, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * hintContainer = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align_to(hintContainer, infoContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_t *hint = GuiCreateIllustrateLabel(hintContainer, "Sign each TXO to generate Key Image for transaction construction.");
    lv_obj_align(hint, LV_ALIGN_DEFAULT, 24, 0);
    lv_obj_set_width(hint, 360);
    lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_opa(hint, 144, LV_PART_MAIN);
}

void GuiShowXmrTransactionOverview(lv_obj_t *parent, void *totalData)
{
    DisplayMoneroUnsignedTx *data = (DisplayMoneroUnsignedTx *)totalData;

    lv_obj_set_width(parent, 408);
    lv_obj_set_height(parent, 602);

    lv_obj_t * amountContainer = GuiCreateContainerWithParent(parent, 408, 144);
    GuiSetTransactionContainerStyle(amountContainer);

    lv_obj_t *label = GuiCreateIllustrateLabel(amountContainer, "Total Output");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateScrollLittleTitleLabel(amountContainer, data->output_amount, 360);
    lv_obj_set_style_text_color(label, lv_color_hex(16090890), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 50);

    label = GuiCreateIllustrateLabel(amountContainer, "Fee");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 98);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(amountContainer, data->fee);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 73, 98);

    lv_obj_t * networkContainer = CreateNetworkContainer(parent);
    lv_obj_align(networkContainer, LV_ALIGN_DEFAULT, 0, 160);

    uint32_t outputsSize = data->outputs->size;
    uint32_t containerHeight = 18 * 2 + 30 + 16 + outputsSize * 160;
    lv_obj_t * detilsContainer = GuiCreateContainerWithParent(parent, 408, containerHeight);
    GuiSetTransactionContainerStyle(detilsContainer);
    lv_obj_align(detilsContainer, LV_ALIGN_DEFAULT, 0, 238);

    label = GuiCreateIllustrateLabel(detilsContainer, "To");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 18);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    uint32_t addressOffset = 0;
    for (size_t i = 0; i < data->outputs->size; i++) {
        bool is_change = data->outputs->data[i].is_change;
        uint32_t addressY = 18 + 38 + i * 160 + addressOffset;
        char outputIndex[BUFFER_SIZE_16] = {0};
        snprintf(outputIndex, sizeof(outputIndex), "%u", (unsigned int)(i + 1));
        label = GuiCreateIllustrateLabel(detilsContainer, outputIndex);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, addressY);
        lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(detilsContainer, data->outputs->data[i].address);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 52, addressY);
        lv_obj_set_width(label, 332);
        lv_obj_t *addressLabel = label;

        if (is_change) {
            lv_obj_t *changeContainer = GuiCreateContainerWithParent(detilsContainer, 87, 30);
            lv_obj_update_layout(label);
            lv_point_t pos;
            lv_label_get_letter_pos(label, 94, &pos);
            lv_obj_align_to(changeContainer, label, LV_ALIGN_BOTTOM_LEFT, pos.x + 16, 0);
            lv_obj_set_style_radius(changeContainer, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(changeContainer, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(changeContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
            label = GuiCreateIllustrateLabel(changeContainer, "Change");
            lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_opa(label, 163, LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        }

        if (data->outputs->data[i].is_integrated && data->outputs->data[i].payment_id != NULL && data->outputs->data[i].payment_id[0] != '\0') {
            char paymentId[BUFFER_SIZE_32] = {0};
            snprintf(paymentId, sizeof(paymentId), "Payment ID: %s", data->outputs->data[i].payment_id);
            lv_obj_t *paymentIdLabel = GuiCreateIllustrateLabel(detilsContainer, paymentId);
            lv_obj_align_to(paymentIdLabel, addressLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);
            lv_obj_set_width(paymentIdLabel, 332);
            lv_label_set_long_mode(paymentIdLabel, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_text_opa(paymentIdLabel, 144, LV_PART_MAIN);
        }
    }
}

static lv_coord_t AddXmrDetailLabel(lv_obj_t *parent, const char *text, lv_coord_t y,
                                    bool muted, bool amount)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, text);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, y);
    if (muted) {
        lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);
    }
    if (amount) {
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    }
    lv_obj_update_layout(label);
    return y + lv_obj_get_height(label);
}

void GuiShowXmrTransactionDetails(lv_obj_t *parent, void *totalData)
{
    DisplayMoneroUnsignedTx *data = (DisplayMoneroUnsignedTx *)totalData;
    lv_obj_set_width(parent, 408);
    lv_obj_set_height(parent, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_bottom(parent, 24, LV_PART_MAIN);

    lv_obj_t *amountContainer = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    GuiSetTransactionContainerStyle(amountContainer);
    lv_coord_t y = 16;
    y = AddXmrDetailLabel(amountContainer, "Input Value", y, true, false);
    y = AddXmrDetailLabel(amountContainer, data->input_amount, y + 6, false, false);
    y = AddXmrDetailLabel(amountContainer, "Output Value", y + 16, true, false);
    y = AddXmrDetailLabel(amountContainer, data->output_amount, y + 6, false, false);
    y = AddXmrDetailLabel(amountContainer, "Fee", y + 16, true, false);
    y = AddXmrDetailLabel(amountContainer, data->fee, y + 6, false, false);
    lv_obj_set_height(amountContainer, y + 16);
    lv_coord_t containerY = y + 32;

    lv_obj_t *inputsContainer = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    GuiSetTransactionContainerStyle(inputsContainer);
    lv_obj_align(inputsContainer, LV_ALIGN_TOP_LEFT, 0, containerY);
    y = AddXmrDetailLabel(inputsContainer, "Inputs", 16, true, false);
    for (size_t i = 0; i < data->inputs->size; i++) {
        char inputIndex[BUFFER_SIZE_32] = {0};
        snprintf(inputIndex, sizeof(inputIndex), "Pubkey %u", (unsigned int)(i + 1));
        y = AddXmrDetailLabel(inputsContainer, inputIndex, y + 16, true, false);
        y = AddXmrDetailLabel(inputsContainer, data->inputs->data[i].amount, y + 6, false, true);
        y = AddXmrDetailLabel(inputsContainer, data->inputs->data[i].key, y + 6, false, false);
    }
    lv_obj_set_height(inputsContainer, y + 16);
    containerY += y + 32;

    lv_obj_t *outputsContainer = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    GuiSetTransactionContainerStyle(outputsContainer);
    lv_obj_align(outputsContainer, LV_ALIGN_TOP_LEFT, 0, containerY);
    y = AddXmrDetailLabel(outputsContainer, "Outputs", 16, true, false);
    for (size_t i = 0; i < data->outputs->size; i++) {
        char outputIndex[BUFFER_SIZE_32] = {0};
        snprintf(outputIndex, sizeof(outputIndex), "Address %u", (unsigned int)(i + 1));
        y = AddXmrDetailLabel(outputsContainer, outputIndex, y + 16, true, false);
        if (data->outputs->data[i].is_change) {
            lv_obj_t *changeContainer = GuiCreateContainerWithParent(outputsContainer, 87, 30);
            lv_obj_align(changeContainer, LV_ALIGN_TOP_RIGHT, -24, y - 30);
            lv_obj_set_style_radius(changeContainer, 16, LV_PART_MAIN);
            lv_obj_set_style_bg_color(changeContainer, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(changeContainer, 30, LV_PART_MAIN);
            lv_obj_t *label = GuiCreateIllustrateLabel(changeContainer, "Change");
            lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_opa(label, 163, LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        }
        y = AddXmrDetailLabel(outputsContainer, data->outputs->data[i].amount, y + 6, false, true);
        y = AddXmrDetailLabel(outputsContainer, data->outputs->data[i].address, y + 6, false, false);
        if (data->outputs->data[i].is_integrated && data->outputs->data[i].payment_id != NULL && data->outputs->data[i].payment_id[0] != '\0') {
            char paymentId[BUFFER_SIZE_32] = {0};
            snprintf(paymentId, sizeof(paymentId), "Payment ID: %s", data->outputs->data[i].payment_id);
            y = AddXmrDetailLabel(outputsContainer, paymentId, y + 6, true, false);
        }
    }
    lv_obj_set_height(outputsContainer, y + 16);
}
