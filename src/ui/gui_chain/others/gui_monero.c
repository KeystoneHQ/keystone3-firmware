#ifndef BTC_ONLY

#include "gui_monero.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static XmrRequestType g_requestType = OutputRequest;
static uint8_t g_decryptKey[32] = {0};

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

static lv_obj_t *g_hintBox = NULL;

static void CloseAttentionHandler(lv_event_t *e);
static void ShowHintBox(lv_event_t *e);
static void SetContainerDefaultStyle(lv_obj_t *container);
static void SetUpMoneroDecryptKey(void);
static lv_obj_t *CreateNetworkContainer(lv_obj_t *container);

static void CloseAttentionHandler(lv_event_t *e)
{
    if (g_hintBox) {
        lv_obj_add_flag(g_hintBox, LV_OBJ_FLAG_HIDDEN);
        GUI_DEL_OBJ(g_hintBox);
    }
}

static void ShowHintBox(lv_event_t *e)
{
    GuiCreateTooltipHintBox("TXO Total Amount", "This amount represents the total balance of the TXOs included in this QR code for signing. It may not reflect the full balance in your software wallet or the exact transaction amount.", "https://...");
}


static void SetContainerDefaultStyle(lv_obj_t *container)
{
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static lv_obj_t *CreateNetworkContainer(lv_obj_t *container)
{
    lv_obj_t *networkContainer = GuiCreateContainerWithParent(container, 408, 62);
    SetContainerDefaultStyle(networkContainer);
    lv_obj_t *label = GuiCreateIllustrateLabel(networkContainer, "Network");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(networkContainer, "XMR Mainnet");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 120, 16);

    return networkContainer;
}

static void SetUpMoneroDecryptKey(void)
{
    char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
    SimpleResponse_u8 *decryptKeyData = monero_generate_decrypt_key(pvk);
    if (decryptKeyData->error_code == SUCCESS_CODE) {
        memcpy(g_decryptKey, decryptKeyData->data, 32);
    }
    free_simple_response_u8(decryptKeyData);
}

PtrT_TransactionCheckResult GuiGetMoneroOutputCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    g_requestType = OutputRequest;
    SetUpMoneroDecryptKey();
    char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
    return monero_output_request_check(data, g_decryptKey, pvk);
}

PtrT_TransactionCheckResult GuiGetMoneroUnsignedTxCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    g_requestType = UnsignedTxRequest;
    SetUpMoneroDecryptKey();
    char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
    return monero_unsigned_request_check(data, g_decryptKey, pvk);
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
    do {
        char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
        PtrT_TransactionParseResult_DisplayMoneroOutput parseResult = monero_parse_output(data, g_decryptKey, pvk);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void *GuiGetMoneroUnsignedTxData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        char *pvk = GetCurrentAccountPublicKey(XPUB_TYPE_MONERO_PVK_0);
        PtrT_TransactionParseResult_DisplayMoneroUnsignedTx parseResult = monero_parse_unsigned_tx(data, g_decryptKey, pvk);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

UREncodeResult *GuiGetMoneroKeyimagesQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = monero_generate_keyimage(data, seed, len, 0);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

UREncodeResult *GuiGetMoneroSignedTransactionQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = monero_generate_signature(data, seed, len, 0);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

void FreeMoneroMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    memset(g_decryptKey, 0, sizeof(g_decryptKey));
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
    lv_obj_set_height(parent, 200);

    lv_obj_t * infoContainer = GuiCreateContainerWithParent(parent, 408, 100);
    SetContainerDefaultStyle(infoContainer);
    lv_obj_t *label = GuiCreateIllustrateLabel(infoContainer, "Number of TXOs");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(infoContainer, data->txos_num);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 195, 16);

    label = GuiCreateIllustrateLabel(infoContainer, "Total Amount");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 53);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(infoContainer, data->total_amount);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 195, 53);
    lv_obj_set_width(label, 180);

    lv_obj_t *infoIcon = GuiCreateImg(infoContainer, &imgInfo);
    lv_obj_align(infoIcon, LV_ALIGN_DEFAULT, 360, 57);
    lv_obj_add_flag(infoIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(infoIcon, ShowHintBox, LV_EVENT_CLICKED, NULL);

    lv_obj_t * hintContainer = GuiCreateContainerWithParent(parent, 408, 60);
    lv_obj_align(hintContainer, LV_ALIGN_DEFAULT, 0, 116);
    lv_obj_t *hint = GuiCreateIllustrateLabel(hintContainer, "Sign each TXO to generate Key Image for transaction construction.");
    lv_obj_align(hint, LV_ALIGN_DEFAULT, 24, 0);
    lv_obj_set_width(hint, 360);
    lv_obj_set_style_text_opa(hint, 144, LV_PART_MAIN);
}

void GuiShowXmrTransactionOverview(lv_obj_t *parent, void *totalData)
{
    DisplayMoneroUnsignedTx *data = (DisplayMoneroUnsignedTx *)totalData;

    lv_obj_set_width(parent, 408);
    lv_obj_set_height(parent, 602);

    lv_obj_t * amountContainer = GuiCreateContainerWithParent(parent, 408, 144);
    SetContainerDefaultStyle(amountContainer);

    lv_obj_t *label = GuiCreateIllustrateLabel(amountContainer, "Amount");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateScrollLittleTitleLabel(amountContainer, data->input_amount, 360);
    lv_obj_set_style_text_color(label, lv_color_hex(16090890), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 50);

    label = GuiCreateIllustrateLabel(amountContainer, "Fee");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 98);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(amountContainer, data->fee);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 73, 98);

    lv_obj_t * networkContainer = CreateNetworkContainer(parent);
    lv_obj_align(networkContainer, LV_ALIGN_DEFAULT, 0, 160);

    uint32_t inputsSize = data->inputs->size;
    uint32_t outputsSize = data->outputs->size;
    // 18: top/bottom padding
    // 30: title height(Outputs/Inputs/Change)
    // 16: title bottom padding
    // 90: input height
    // 120: output height
    uint32_t containerHeight = 18 * 2 + 30 * 2 + 30 + 16 + inputsSize * 90 + outputsSize * 120;
    lv_obj_t * detilsContainer = GuiCreateContainerWithParent(parent, 408, containerHeight);
    SetContainerDefaultStyle(detilsContainer);
    lv_obj_align(detilsContainer, LV_ALIGN_DEFAULT, 0, 238);

    label = GuiCreateIllustrateLabel(detilsContainer, "Inputs");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    for (size_t i = 0; i < data->inputs->size; i++)
    {
        char inputIndex[BUFFER_SIZE_16] = {0};
        sprintf(inputIndex, "%d", i + 1);
        label = GuiCreateIllustrateLabel(detilsContainer, inputIndex);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 54 + i * 90);
        lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(detilsContainer, data->inputs->data[i].key);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 54 + i * 90);
        lv_obj_set_width(label, 332);
    }
    
    label = GuiCreateIllustrateLabel(detilsContainer, "Outputs");
    uint32_t outputsLabelY = 18 + 16 + 30 + inputsSize * 90;
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, outputsLabelY);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    uint32_t addressOffset = 0;
    for (size_t i = 0; i < data->outputs->size; i++)
    {
        bool is_change = data->outputs->data[i].is_change;
        uint32_t addressY = outputsLabelY + 38 + i * 120 + addressOffset;
        char outputIndex[BUFFER_SIZE_16] = {0};
        sprintf(outputIndex, "%d", i + 1);
        label = GuiCreateIllustrateLabel(detilsContainer, outputIndex);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, addressY);
        lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

        if (is_change) {
            lv_obj_t *changeContainer = GuiCreateContainerWithParent(detilsContainer, 87, 30);
            lv_obj_align(changeContainer, LV_ALIGN_DEFAULT, 52, addressY);
            lv_obj_set_style_radius(changeContainer, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(changeContainer, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(changeContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
            label = GuiCreateIllustrateLabel(changeContainer, "Change");
            lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_opa(label, 163, LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

            addressY += 30;
            addressOffset = 30;
        }
        label = GuiCreateIllustrateLabel(detilsContainer, data->outputs->data[i].address);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 52, addressY);
        lv_obj_set_width(label, 332);
    }
}

void GuiShowXmrTransactionDetails(lv_obj_t *parent, void *totalData)
{
    DisplayMoneroUnsignedTx *data = (DisplayMoneroUnsignedTx *)totalData;

    lv_obj_set_width(parent, 408);
    lv_obj_set_height(parent, 602);

    lv_obj_t * networkContainer = CreateNetworkContainer(parent);

    lv_obj_t * amountContainer = GuiCreateContainerWithParent(parent, 408, 138);
    SetContainerDefaultStyle(amountContainer);
    lv_obj_align(amountContainer, LV_ALIGN_DEFAULT, 0, 78);

    lv_obj_t *label = GuiCreateIllustrateLabel(amountContainer, "Input Value");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(amountContainer, data->output_amount);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 147, 16);
    lv_obj_set_width(label, 220);

    label = GuiCreateIllustrateLabel(amountContainer, "Output Value");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 54);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(amountContainer, data->input_amount);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 164, 54);
    lv_obj_set_width(label, 220);

    label = GuiCreateIllustrateLabel(amountContainer, "Fee");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 92);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(amountContainer, data->fee);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 73, 92);
    lv_obj_set_width(label, 220);

    uint32_t inputsSize = data->inputs->size;
    uint32_t inputContainerHeight = 16 * 2 + 30 + inputsSize * 120;
    lv_obj_t * inputsContainer = GuiCreateContainerWithParent(parent, 408, inputContainerHeight);
    SetContainerDefaultStyle(inputsContainer);
    lv_obj_align(inputsContainer, LV_ALIGN_DEFAULT, 0, 232);

    label = GuiCreateIllustrateLabel(inputsContainer, "Inputs");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    for (size_t i = 0; i < data->inputs->size; i++)
    {
        char inputIndex[BUFFER_SIZE_16] = {0};
        sprintf(inputIndex, "%d", i + 1);
        label = GuiCreateIllustrateLabel(inputsContainer, inputIndex);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 54 + i * 120);
        lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(inputsContainer, data->inputs->data[i].amount);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 54 + i * 120);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(inputsContainer, data->inputs->data[i].key);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 84 + i * 120);
        lv_obj_set_width(label, 332);
    }

    uint32_t outputsSize = data->outputs->size;
    uint32_t outputContainerHeight = 16 * 2 + 30 + outputsSize * 150;
    lv_obj_t * outputsContainer = GuiCreateContainerWithParent(parent, 408, outputContainerHeight);
    SetContainerDefaultStyle(outputsContainer);
    lv_obj_align(outputsContainer, LV_ALIGN_DEFAULT, 0, 232 + 16 + inputContainerHeight);

    label = GuiCreateIllustrateLabel(outputsContainer, "Outputs");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

    for (size_t i = 0; i < data->outputs->size; i++)
    {
        bool is_change = data->outputs->data[i].is_change;
        char outputIndex[BUFFER_SIZE_16] = {0};
        sprintf(outputIndex, "%d", i + 1);
        label = GuiCreateIllustrateLabel(outputsContainer, outputIndex);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 54 + i * 150);
        lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(outputsContainer, data->outputs->data[i].amount);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 54 + i * 150);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

        if (is_change) {
            lv_obj_t *changeContainer = GuiCreateContainerWithParent(outputsContainer, 87, 30);
            uint32_t changeContainerX = lv_obj_get_self_width(label) + 16;
            lv_obj_align(changeContainer, LV_ALIGN_DEFAULT, 52 + changeContainerX, 54 + i * 150);
            lv_obj_set_style_radius(changeContainer, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(changeContainer, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(changeContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
            label = GuiCreateIllustrateLabel(changeContainer, "Change");
            lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_opa(label, 163, LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        }

        label = GuiCreateIllustrateLabel(outputsContainer, data->outputs->data[i].address);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 84 + i * 150);
        lv_obj_set_width(label, 332);
    }
}

#endif
