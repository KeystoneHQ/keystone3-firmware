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

static void SetFlexContainerStyle(lv_obj_t *container, lv_flex_flow_t flow, lv_coord_t padding_y)
{
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(container, flow);
    lv_obj_set_style_pad_top(container, padding_y, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(container, padding_y, LV_PART_MAIN);
    lv_obj_set_style_pad_left(container, 24, LV_PART_MAIN);
}

static void SetTitleLabelStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

lv_obj_t *GuiCreateSuiAutoHeightContainer(lv_obj_t *parent, uint16_t width, uint16_t padding_x)
{
    lv_obj_t * container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    SetContainerDefaultStyle(container);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(container, padding_x, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(container, padding_x, LV_PART_MAIN);
    return container;
}
lv_obj_t* GuiCreateSuiNoticeCard(lv_obj_t* parent)
{
    lv_obj_t* card = GuiCreateSuiAutoHeightContainer(parent, 408, 24);
    SetContainerDefaultStyle(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xF55831), LV_PART_MAIN);
    lv_obj_set_style_radius(card, 8, LV_PART_MAIN);

    lv_obj_t* warningIcon = GuiCreateImg(card, &imgInfoOrange);
    lv_obj_align(warningIcon, LV_ALIGN_TOP_LEFT, 24, 0);

    lv_obj_t* title_label = GuiCreateTextLabel(card, "Notice");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xF55831), LV_PART_MAIN);
    lv_obj_align_to(title_label, warningIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t* content_label = GuiCreateIllustrateLabel(card, _("sign_message_hash_notice_content"));
    lv_obj_set_style_text_color(content_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_width(content_label, lv_pct(90));
    lv_obj_align_to(content_label, warningIcon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    return card;
}

lv_obj_t *GuiCreateLabelCard(lv_obj_t *parent, const char *title, const char *content)
{
    lv_obj_t *card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    SetFlexContainerStyle(card, LV_FLEX_FLOW_ROW, 16);
    lv_obj_t *title_label = GuiCreateTextLabel(card, title);
    lv_obj_align_to(title_label, card, LV_ALIGN_OUT_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(title_label);
    lv_obj_t *content_label = GuiCreateIllustrateLabel(card, content);
    lv_obj_align_to(content_label, title_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    return card;
}

lv_obj_t *GuiCreateSuiFromAddressCard(lv_obj_t *parent, const char *content)
{
    lv_obj_t *card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    SetFlexContainerStyle(card, LV_FLEX_FLOW_COLUMN, 16);
    lv_obj_t *title_label = GuiCreateTextLabel(card, "From Address");
    lv_obj_align_to(title_label, card, LV_ALIGN_OUT_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(title_label);
    lv_obj_t *content_label = GuiCreateIllustrateLabel(card, content);
    lv_obj_align_to(content_label, title_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_set_width(content_label, lv_pct(90));
    lv_label_set_long_mode(content_label, LV_LABEL_LONG_WRAP);

    return card;
}


void GuiShowSuiSignMessageHashOverview(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    DisplaySuiSignMessageHash *hashData = (DisplaySuiSignMessageHash *)totalData;
    lv_obj_t * noticeCard = GuiCreateSuiNoticeCard(parent);
    lv_obj_align(noticeCard, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_update_layout(noticeCard);
    int containerYOffset = lv_obj_get_height(noticeCard) + 16;
    // network path container
    lv_obj_t *network_card = GuiCreateLabelCard(parent, "Network", hashData->network);
    lv_obj_align(network_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
    lv_obj_update_layout(network_card);
    containerYOffset += lv_obj_get_height(network_card) + 16;
    // message hash container
    lv_obj_t *message_hash_card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align(message_hash_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
    SetFlexContainerStyle(message_hash_card, LV_FLEX_FLOW_COLUMN, 16);
    lv_obj_t *message_hash_label = GuiCreateTextLabel(message_hash_card, "Hash");
    lv_obj_set_style_text_opa(message_hash_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(message_hash_label, message_hash_card, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 16);

    // color message hash
    char hash[128] = {0};
    strncpy(hash, hashData->message, sizeof(hash) - 1);
    char tempBuf[128] = {0};
    snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);

    lv_obj_t *message_hash_value = GuiCreateIllustrateLabel(message_hash_card, tempBuf);
    lv_label_set_recolor(message_hash_value, true);
    lv_obj_align_to(message_hash_value, message_hash_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
}



void GuiShowSuiSignMessageHashDetails(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    DisplaySuiSignMessageHash *hashData = (DisplaySuiSignMessageHash *)totalData;
    // network card
    lv_obj_t *network_card = GuiCreateLabelCard(parent, "Network", hashData->network);
    lv_obj_align(network_card, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_update_layout(network_card);
    int containerYOffset = lv_obj_get_height(network_card) + 16;
    // path card
    // path = m/ + hashData->path
    char path[128] = {0};
    snprintf(path, sizeof(path), "m/%s", hashData->path);
    lv_obj_t *path_card = GuiCreateLabelCard(parent, "Path", path);
    lv_obj_align(path_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
    lv_obj_update_layout(path_card);
    containerYOffset += lv_obj_get_height(path_card) + 16;

    // if from_address is empty, don't show from address card
    if (strlen(hashData->from_address) > 0) {
        // from address card
        lv_obj_t *from_address_card = GuiCreateSuiFromAddressCard(parent, hashData->from_address);
        lv_obj_align(from_address_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
        lv_obj_update_layout(from_address_card);
        containerYOffset += lv_obj_get_height(from_address_card) + 16;
    }

    // message hash container
    lv_obj_t *message_hash_card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    lv_obj_align(message_hash_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
    SetFlexContainerStyle(message_hash_card, LV_FLEX_FLOW_COLUMN, 16);
    lv_obj_t *message_hash_label = GuiCreateTextLabel(message_hash_card, "Hash");
    lv_obj_set_style_text_opa(message_hash_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(message_hash_label, message_hash_card, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 1);

    // color message hash
    char hash[128] = {0};
    strncpy(hash, hashData->message, sizeof(hash) - 1);
    char tempBuf[128] = {0};
    snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);

    lv_obj_t *message_hash_value = GuiCreateIllustrateLabel(message_hash_card, tempBuf);
    lv_label_set_recolor(message_hash_value, true);
    lv_obj_align_to(message_hash_value, message_hash_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_label_set_long_mode(message_hash_value, LV_LABEL_LONG_WRAP);

    // message hash notice content
    lv_obj_t *message_hash_notice_content = GuiCreateIllustrateLabel(message_hash_card, _("sign_message_hash_notice_content"));
    lv_obj_set_width(message_hash_notice_content, lv_pct(90));
    lv_obj_set_style_text_color(message_hash_notice_content, lv_color_hex(0xF5C131), LV_PART_MAIN);
    lv_obj_align_to(message_hash_notice_content, message_hash_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_label_set_long_mode(message_hash_notice_content, LV_LABEL_LONG_WRAP);
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