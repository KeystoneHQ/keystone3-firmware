#ifndef BTC_ONLY
#include "gui_chain_components.h"
#include "gui_chain.h"
#include "gui_ergo.h"
#include "keystore.h"
#include "rust.h"
#include "account_public_info.h"

#define CHECK_FREE_PARSE_RESULT(result)                                     \
    if (result != NULL)                                                     \
    {                                                                       \
        free_TransactionParseResult_DisplayErgoTx(g_parseResult);           \
        g_parseResult = NULL;                                               \
    }

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static DisplayErgoTx *g_ergoData;

void GuiSetErgoUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

void *GuiGetErgoGUIData(void) {
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ERG);
    do {
        TransactionParseResult_DisplayErgoTx *parseResult = ergo_parse_tx(data, xpub);
        CHECK_CHAIN_BREAK(parseResult);
        g_ergoData = parseResult->data;
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetErgoCheckResult(void) {
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    char *xpub = GetCurrentAccountPublicKey(XPUB_TYPE_ERG);
    return ergo_check_tx(data, xpub);
}

static lv_obj_t* GuiErgoOverviewFrom(lv_obj_t *parent, VecFFI_DisplayErgoFrom *from, lv_obj_t *last_view);
static lv_obj_t* GuiErgoOverviewTo(lv_obj_t *parent, VecFFI_DisplayErgoTo *to, lv_obj_t *last_view);

void GuiErgoOverview(lv_obj_t *parent, void *totalData) {
    lv_obj_set_size(parent, 408, 480);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* container = GuiCreateContainerWithParent(parent, 408, 480);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* last_view = NULL;

    last_view = CreateTransactionItemView(container, _("Total Input"), g_ergoData->total_input, last_view);
    last_view = CreateTransactionItemView(container, _("Total Output"), g_ergoData->total_output, last_view);
    last_view = CreateTransactionItemView(container, _("Fee"), g_ergoData->fee, last_view);

    last_view = GuiErgoOverviewFrom(container, g_ergoData->from, last_view);
    last_view = GuiErgoOverviewTo(container, g_ergoData->to, last_view);
}

static lv_obj_t* GuiErgoOverviewFrom(lv_obj_t *parent, VecFFI_DisplayErgoFrom *from, lv_obj_t *last_view) {
   lv_obj_t* label, *container;
    uint16_t height = 0;

    container = CreateTransactionContentContainer(parent, 408, height);
    lv_obj_align_to(container, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    // top padding
    height += 16;

    label = GuiCreateIllustrateLabel(container, _("From"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);

    height += 30;

    lv_obj_t *innerContainer;

    for (size_t i = 0; i < from->size; i++)
    {
        lv_obj_t *valueLabel, *indexLabel, *addressLabel;
        uint16_t innerHeight = 0;
        if(i > 0) {
            //add margin
            innerHeight += 16;
        }
        innerContainer = GuiCreateContainerWithParent(container, 360, innerHeight);
        lv_obj_set_style_bg_opa(innerContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(innerContainer, LV_ALIGN_TOP_LEFT, 24, height);

        char *order = (char *)malloc(5);
        snprintf_s(order, 5, "#%d", i + 1);
        indexLabel = GuiCreateIllustrateLabel(innerContainer, order);
        lv_obj_align(indexLabel, LV_ALIGN_TOP_LEFT, 0, innerHeight);

        valueLabel = GuiCreateIllustrateLabel(innerContainer, from->data[i].amount);
        lv_obj_set_style_text_color(valueLabel, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(valueLabel, indexLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

        lv_obj_t *assetLabel;
        if (from->data[i].has_assets) {
            innerHeight += 16;
            for (int x = 0; x < from->data[i].assets->size; x++) {
                innerHeight += 16;
                assetLabel = GuiCreateIllustrateLabel(innerContainer, from->data[i].assets->data[x]);
                lv_obj_align(assetLabel, LV_ALIGN_TOP_LEFT, 0, innerHeight);
                lv_obj_set_style_text_color(assetLabel, WHITE_COLOR, LV_PART_MAIN);
                lv_obj_set_style_text_opa(assetLabel, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);
                innerHeight += lv_obj_get_height(assetLabel);
            }
        }

        if(from->data[i].is_mine) {
            lv_obj_t *tagContainer = GuiCreateContainerWithParent(innerContainer, 87, 30);
            lv_obj_set_style_radius(tagContainer, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(tagContainer, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(tagContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_t *tagLabel = lv_label_create(tagContainer);
            lv_label_set_text(tagLabel, "Mine");
            lv_obj_set_style_text_font(tagLabel, g_defIllustrateFont, LV_PART_MAIN);
            lv_obj_set_style_text_color(tagLabel, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_opa(tagLabel, 163, LV_PART_MAIN);
            lv_obj_align(tagLabel, LV_ALIGN_CENTER, 0, 0);

            lv_obj_align_to(tagContainer, valueLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
        }

        innerHeight += 30;

        addressLabel = GuiCreateIllustrateLabel(innerContainer, from->data[i].address);
        lv_obj_set_width(addressLabel, 360);
        lv_label_set_long_mode(addressLabel, LV_LABEL_LONG_WRAP);
        lv_obj_align(addressLabel, LV_ALIGN_TOP_LEFT, 0, innerHeight);
        lv_obj_update_layout(addressLabel);
        innerHeight += lv_obj_get_height(addressLabel);

        lv_obj_set_height(innerContainer, innerHeight);
        lv_obj_update_layout(innerContainer);

        height += innerHeight;
    }

    // bottom padding
    height += 16;

    lv_obj_set_height(container, height);
    lv_obj_update_layout(container);

    return container;
}

static lv_obj_t* GuiErgoOverviewTo(lv_obj_t *parent, VecFFI_DisplayErgoTo *to, lv_obj_t *last_view) {
   lv_obj_t* label, *container;
    uint16_t height = 0;

    container = CreateTransactionContentContainer(parent, 408, height);
    lv_obj_align_to(container, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    // top padding
    height += 16;

    label = GuiCreateIllustrateLabel(container, _("To"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);

    height += 30;

    lv_obj_t *innerContainer;

    for (size_t i = 0; i < to->size; i++)
    {
        lv_obj_t *valueLabel, *indexLabel, *addressLabel;
        uint16_t innerHeight = 0;
        if(i > 0) {
            //add margin
            innerHeight += 16;
        }
        innerContainer = GuiCreateContainerWithParent(container, 360, innerHeight);
        lv_obj_set_style_bg_opa(innerContainer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(innerContainer, LV_ALIGN_TOP_LEFT, 24, height);

        char *order = (char *)malloc(5);
        snprintf_s(order, 5, "#%d", i + 1);
        indexLabel = GuiCreateIllustrateLabel(innerContainer, order);
        lv_obj_align(indexLabel, LV_ALIGN_TOP_LEFT, 0, innerHeight);

        valueLabel = GuiCreateIllustrateLabel(innerContainer, to->data[i].amount);
        lv_obj_set_style_text_color(valueLabel, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(valueLabel, indexLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

        lv_obj_t *assetLabel;
        if (to->data[i].has_assets) {
            innerHeight += 16;
            for (int x = 0; x < to->data[i].assets->size; x++) {
                innerHeight += 16;
                assetLabel = GuiCreateIllustrateLabel(innerContainer, to->data[i].assets->data[x]);
                lv_obj_align(assetLabel, LV_ALIGN_TOP_LEFT, 0, innerHeight);
                lv_obj_set_style_text_color(assetLabel, WHITE_COLOR, LV_PART_MAIN);
                lv_obj_set_style_text_opa(assetLabel, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);
                innerHeight += lv_obj_get_height(assetLabel);
            }
        }

        if(to->data[i].is_change || to->data[i].is_fee) {
            lv_obj_t *tagContainer = GuiCreateContainerWithParent(innerContainer, 87, 30);
            lv_obj_set_style_radius(tagContainer, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(tagContainer, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(tagContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_t *tagLabel = lv_label_create(tagContainer);
            if (to->data[i].is_change) {
                lv_label_set_text(tagLabel, "Change");
            } else {
                lv_label_set_text(tagLabel, "Fee");
            }
            lv_obj_set_style_text_font(tagLabel, g_defIllustrateFont, LV_PART_MAIN);
            lv_obj_set_style_text_color(tagLabel, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_opa(tagLabel, 163, LV_PART_MAIN);
            lv_obj_align(tagLabel, LV_ALIGN_CENTER, 0, 0);

            lv_obj_align_to(tagContainer, valueLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
        }

        innerHeight += 30;

        addressLabel = GuiCreateIllustrateLabel(innerContainer, to->data[i].address);
        lv_obj_set_width(addressLabel, 360);
        lv_label_set_long_mode(addressLabel, LV_LABEL_LONG_WRAP);
        lv_obj_align(addressLabel, LV_ALIGN_TOP_LEFT, 0, innerHeight);
        lv_obj_update_layout(addressLabel);
        innerHeight += lv_obj_get_height(addressLabel);

        lv_obj_set_height(innerContainer, innerHeight);
        lv_obj_update_layout(innerContainer);

        height += innerHeight;
    }

    // bottom padding
    height += 16;

    lv_obj_set_height(container, height);
    lv_obj_update_layout(container);

    return container;
}

UREncodeResult *GuiGetErgoSignQrCodeData(void) {
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        uint32_t seedLen = sizeof(seed);
        uint8_t entropy[32];
        uint32_t entropyLen = 32;
        GenerateEntropy(entropy, 32, SecretCacheGetPassword());
        encodeResult = ergo_sign_tx(data, seed, seedLen, entropy, entropyLen);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;


}

void FreeErgoMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}
#endif
