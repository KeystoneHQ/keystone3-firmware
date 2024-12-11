#include "gui_zcash.h"
#include "gui_chain_components.h"
#include "user_memory.h"
#include "account_manager.h"
#include "gui_chain.h"

#define MAX_MEMO_LENGTH 1024

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static DisplayPczt *g_zcashData;

#define CHECK_FREE_PARSE_RESULT(result)                                                             \
    if (result != NULL)                                                                             \
    {                                                                                               \
        free_TransactionParseResult_DisplayPczt((PtrT_TransactionParseResult_DisplayPczt)result);   \
        result = NULL;                                                                              \
    }

void GuiSetZcashUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

void *GuiGetZcashGUIData(void) {
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    char ufvk[ZCASH_UFVK_MAX_LEN] = {'\0'};
    uint8_t sfp[32];
    GetZcashUFVK(GetCurrentAccountIndex(), ufvk, sfp);

    PtrT_TransactionParseResult_DisplayPczt parseResult = NULL;
    do {
        parseResult = parse_zcash_tx(data, ufvk, sfp);
        CHECK_CHAIN_BREAK(parseResult);
        g_zcashData = parseResult->data;
        g_parseResult = (void *)parseResult;
        
    } while (0);
    return g_parseResult;
}

static lv_obj_t* GuiZcashOverviewTransparent(lv_obj_t *parent, lv_obj_t *last_view);
static lv_obj_t* GuiZcashOverviewOrchard(lv_obj_t *parent, lv_obj_t *last_view);
static lv_obj_t* GuiZcashOverviewFrom(lv_obj_t *parent, VecFFI_DisplayFrom *from, lv_obj_t *last_view);
static lv_obj_t* GuiZcashOverviewTo(lv_obj_t *parent, VecFFI_DisplayTo *to, lv_obj_t *last_view);

void GuiZcashOverview(lv_obj_t *parent, void *totalData) {
    lv_obj_set_size(parent, 408, 480);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* container = GuiCreateContainerWithParent(parent, 408, 480);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* last_view = CreateTransactionItemView(container, _("Amount"), g_zcashData->total_transfer_value, NULL);

    if(g_zcashData->transparent != NULL) {
        last_view = GuiZcashOverviewTransparent(container, last_view);
    }

    last_view = GuiZcashOverviewOrchard(container, last_view);
}

static lv_obj_t* GuiZcashOverviewTransparent(lv_obj_t *parent, lv_obj_t *last_view) {
    lv_obj_t* label = GuiCreateIllustrateLabel(parent, _("Transparent"));
    lv_obj_align_to(label, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    lv_obj_t* from_view = GuiZcashOverviewFrom(parent, g_zcashData->transparent->from, label);
    lv_obj_t* to_view = GuiZcashOverviewTo(parent, g_zcashData->transparent->to, from_view);

    return to_view;
}

static lv_obj_t* GuiZcashOverviewOrchard(lv_obj_t* parent, lv_obj_t *last_view) {
    lv_obj_t* label = GuiCreateIllustrateLabel(parent, _("Orchard"));
    lv_obj_align_to(label, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    lv_obj_t* from_view = GuiZcashOverviewFrom(parent, g_zcashData->orchard->from, label);
    lv_obj_t* to_view = GuiZcashOverviewTo(parent, g_zcashData->orchard->to, from_view);

    return to_view;
}

static lv_obj_t* GuiZcashOverviewFrom(lv_obj_t *parent, VecFFI_DisplayFrom *from, lv_obj_t *last_view) {
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

        valueLabel = GuiCreateIllustrateLabel(innerContainer, from->data[i].value);
        lv_obj_set_style_text_color(valueLabel, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(valueLabel, indexLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

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

static lv_obj_t* GuiZcashOverviewTo(lv_obj_t *parent, VecFFI_DisplayTo *to, lv_obj_t *last_view) {
    lv_obj_t* label, *container;
    uint16_t height = 0;

    container = CreateTransactionContentContainer(parent, 408, height);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
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

        if(to->data[i].visible) {
            valueLabel = GuiCreateIllustrateLabel(innerContainer, to->data[i].value);
            lv_obj_set_style_text_color(valueLabel, ORANGE_COLOR, LV_PART_MAIN);
            lv_obj_align_to(valueLabel, indexLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
            if (to->data[i].is_change) {
                lv_obj_t *tagContainer = GuiCreateContainerWithParent(innerContainer, 87, 30);
                lv_obj_set_style_radius(tagContainer, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(tagContainer, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(tagContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_t *tagLabel = lv_label_create(tagContainer);

                lv_label_set_text(tagLabel, "Change");
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

            if(to->data[i].memo != NULL) {
                char *memo = (char *)malloc(MAX_MEMO_LENGTH);
                snprintf_s(memo, MAX_MEMO_LENGTH, "Memo: %s", to->data[i].memo);
                lv_obj_t *memoLabel = GuiCreateIllustrateLabel(innerContainer, memo);
                lv_obj_align(memoLabel, LV_ALIGN_TOP_LEFT, 0, innerHeight);
                lv_obj_set_style_text_color(memoLabel, WHITE_COLOR, LV_PART_MAIN);
                lv_obj_set_style_text_opa(memoLabel, LV_OPA_56, LV_PART_MAIN);
                lv_obj_update_layout(memoLabel);

                innerHeight += lv_obj_get_height(memoLabel);
            }

        } else {
            innerHeight += 30;
            addressLabel = GuiCreateIllustrateLabel(innerContainer, "Unknown Output");
            lv_obj_set_width(addressLabel, 360);
            lv_obj_align(addressLabel, LV_ALIGN_TOP_LEFT, 0, innerHeight);
            lv_obj_set_style_text_color(addressLabel, RED_COLOR, LV_PART_MAIN);
            innerHeight += 30;
        }

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

PtrT_TransactionCheckResult GuiGetZcashCheckResult(void)
{
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    char ufvk[ZCASH_UFVK_MAX_LEN] = {'\0'};
    uint8_t sfp[32];
    GetZcashUFVK(GetCurrentAccountIndex(), ufvk, sfp);
    return check_zcash_tx(data, ufvk, sfp);
}

UREncodeResult *GuiGetZcashSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encodeResult = sign_zcash_tx(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}
