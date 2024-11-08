#ifndef BTC_ONLY
#include "define.h"
#include "rust.h"
#include "keystore.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "user_memory.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

void GuiSetSuiUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                                   \
    if (result != NULL)                                                                                                   \
    {                                                                                                                     \
        free_TransactionParseResult_DisplaySuiIntentMessage((PtrT_TransactionParseResult_DisplaySuiIntentMessage)result); \
        result = NULL;                                                                                                    \
    }

void *GuiGetSuiData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplaySuiIntentMessage parseResult = sui_parse_intent(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void *GuiGetSuiSignMessageHashData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplaySuiSignMessageHash parseResult = sui_parse_sign_message_hash(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetSuiCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return sui_check_request(data, mfp, sizeof(mfp));
}

PtrT_TransactionCheckResult GuiGetSuiSignHashCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return sui_check_sign_hash_request(data, mfp, sizeof(mfp));
}

static void SetContainerDefaultStyle(lv_obj_t *container)
{
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetTitleLabelStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetContentLableStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
}

void GuiShowSuiSignMessageHashOverview(lv_obj_t *parent, void *totalData)
{
    DisplaySuiSignMessageHash *hashData = (DisplaySuiSignMessageHash *)totalData;
    // notice container
    lv_obj_t *noticeContainer = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align(noticeContainer, LV_ALIGN_DEFAULT, 0, 0);
    SetContainerDefaultStyle(noticeContainer);
    // Create a flex container for icon and notice label
    lv_obj_t *iconNoticeContainer = lv_obj_create(noticeContainer);
    lv_obj_set_size(iconNoticeContainer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(iconNoticeContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(iconNoticeContainer, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(iconNoticeContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(iconNoticeContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(iconNoticeContainer, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_left(iconNoticeContainer, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(iconNoticeContainer, LV_OPA_TRANSP, LV_PART_MAIN); // Set transparent background
    // Create image inside the flex container
    lv_obj_t *img = GuiCreateImg(iconNoticeContainer, &imgInfoOrange);
    lv_obj_set_style_pad_right(img, 2, LV_PART_MAIN); // Add right padding to the image
    lv_obj_t *noticeLabel = lv_label_create(iconNoticeContainer);
    lv_obj_set_style_text_font(noticeLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(noticeLabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_label_set_text(noticeLabel, "Notice");

    lv_obj_t *noticeContent = lv_label_create(noticeContainer);
    lv_obj_set_width(noticeContent, lv_pct(90));
    lv_label_set_long_mode(noticeContent, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(noticeContent, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(noticeContent, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // multi language support
    lv_label_set_text(noticeContent, _("solana_parse_tx_notice"));
    lv_obj_set_style_text_opa(noticeContent, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(noticeContent, iconNoticeContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 10);

    // get the height of the notice container
    lv_obj_update_layout(noticeContainer);
    int containerYOffset = lv_obj_get_height(noticeContainer) + 16; // set padding between containers

    // container will auto adjust height
    lv_obj_t *network_path_container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align(network_path_container, LV_ALIGN_DEFAULT, 0, containerYOffset);
    SetContainerDefaultStyle(network_path_container);

    // network label
    lv_obj_t * network_label = GuiCreateTextLabel(network_path_container, "Network");
    lv_obj_align_to(network_label, network_path_container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(network_label);
    lv_obj_set_style_pad_right(network_label, 2, LV_PART_MAIN); // Add right padding to the image
    lv_obj_t * network_value = GuiCreateIllustrateLabel(network_path_container, hashData->network);
    lv_label_set_long_mode(network_value, LV_LABEL_LONG_WRAP);
    lv_obj_t *path_label = GuiCreateTextLabel(network_path_container, "Path");
    lv_obj_align_to(path_label, network_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(path_label);

    // path value
    lv_obj_t *path_content = GuiCreateIllustrateLabel(network_path_container, hashData->path);
    lv_obj_align_to(path_content, path_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_label_set_long_mode(path_content, LV_LABEL_LONG_WRAP);

    // update the height of the network_path_container
    lv_obj_update_layout(network_path_container);
    containerYOffset += lv_obj_get_height(network_path_container); // set padding between containers
    lv_obj_t *message_hash_container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align(message_hash_container, LV_ALIGN_DEFAULT, 0, containerYOffset + 16);
    SetContainerDefaultStyle(message_hash_container);

    lv_obj_t *message_hash_label = GuiCreateTextLabel(message_hash_container, "Message Hash");
    lv_obj_align_to(message_hash_label, message_hash_container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(message_hash_label);

    lv_obj_t *message_hash_value = GuiCreateIllustrateLabel(message_hash_container, hashData->message);
    lv_obj_align_to(message_hash_value, message_hash_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_label_set_long_mode(message_hash_value, LV_LABEL_LONG_WRAP);
}


void FreeSuiMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

int GetSuiDetailLen(void *param)
{
    DisplaySuiIntentMessage *tx = (DisplaySuiIntentMessage *)param;
    return strlen(tx->detail) + 1;
}

void GetSuiDetail(void *indata, void *param, uint32_t maxLen)
{
    DisplaySuiIntentMessage *tx = (DisplaySuiIntentMessage *)param;
    // strcpy_s will exceed the stack size and the copy will fail
    strcpy((char *)indata, tx->detail);
}

UREncodeResult *GuiGetSuiSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encodeResult = sui_sign_intent(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

UREncodeResult *GuiGetSuiSignHashQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encodeResult = sui_sign_hash(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}
#endif