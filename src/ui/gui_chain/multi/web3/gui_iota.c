#include "define.h"
#include "rust.h"
#include "keystore.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"
#include "user_memory.h"
#include "gui_chain_components.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

void GuiSetIotaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

// #define CHECK_FREE_PARSE_RESULT(result)                                                                                   \
//     if (result != NULL)                                                                                                   \
//     {                                                                                                                     \
//         free_TransactionParseResult_DisplayIotaIntentMessage((PtrT_TransactionParseResult_DisplayIotaIntentMessage)result); \
//         result = NULL;                                                                                                    \
//     }

void *GuiGetIotaData(void)
{
    // CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        printf("%s %d..\n", __func__, __LINE__);
        PtrT_TransactionParseResult_DisplayIotaIntentData parseResult = iota_parse_intent(data);
        printf("kind: %s\n", parseResult->data->kind);
        printf("sender = %s\n", parseResult->data->sender);
        printf("owner = %s\n", parseResult->data->owner);
        printf("price = %s\n", parseResult->data->price);
        printf("gas budget = %s\n", parseResult->data->gas_budget);
        printf("%s %d..\n", __func__, __LINE__);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

// void *GuiGetIotaSignMessageHashData(void)
// {
//     CHECK_FREE_PARSE_RESULT(g_parseResult);
//     void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
//     do {
//         PtrT_TransactionParseResult_DisplayIotaSignMessageHash parseResult = iota_parse_sign_message_hash(data);
//         CHECK_CHAIN_BREAK(parseResult);
//         g_parseResult = (void *)parseResult;
//     } while (0);
//     return g_parseResult;
// }

PtrT_TransactionCheckResult GuiGetIotaCheckResult(void)
{
    printf("%s %d..\n", __func__, __LINE__);
    uint8_t mfp[4];
    printf("%s %d..\n", __func__, __LINE__);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    printf("%s %d..\n", __func__, __LINE__);
    GetMasterFingerPrint(mfp);
    printf("%s %d..\n", __func__, __LINE__);
    PtrT_TransactionCheckResult result = sui_check_request(data, mfp, sizeof(mfp));
    printf("%s %d..\n", __func__, __LINE__);
    return result;
}

// PtrT_TransactionCheckResult GuiGetIotaSignHashCheckResult(void)
// {
//     uint8_t mfp[4];
//     void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
//     GetMasterFingerPrint(mfp);
//     return iota_check_sign_hash_request(data, mfp, sizeof(mfp));
// }

// static void SetContainerDefaultStyle(lv_obj_t *container)
// {
//     lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
// }

// static void SetFlexContainerStyle(lv_obj_t *container, lv_flex_flow_t flow, lv_coord_t padding_y)
// {
//     lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_set_flex_flow(container, flow);
//     lv_obj_set_style_pad_top(container, padding_y, LV_PART_MAIN);
//     lv_obj_set_style_pad_bottom(container, padding_y, LV_PART_MAIN);
//     lv_obj_set_style_pad_left(container, 24, LV_PART_MAIN);
// }

// static void SetTitleLabelStyle(lv_obj_t *label)
// {
//     lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
//     lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
//     lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
// }

// lv_obj_t *GuiCreateIotaAutoHeightContainer(lv_obj_t *parent, uint16_t width, uint16_t padding_x)
// {
//     lv_obj_t * container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
//     SetContainerDefaultStyle(container);
//     lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
//     lv_obj_set_style_pad_top(container, padding_x, LV_PART_MAIN);
//     lv_obj_set_style_pad_bottom(container, padding_x, LV_PART_MAIN);
//     return container;
// }

// void GuiShowIotaSignMessageHashOverview(lv_obj_t *parent, void *totalData)
// {
//     lv_obj_set_size(parent, 408, 444);
//     lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
//     DisplayIotaSignMessageHash *hashData = (DisplayIotaSignMessageHash *)totalData;
//     lv_obj_t * noticeCard = GuiCreateIotaNoticeCard(parent);
//     lv_obj_align(noticeCard, LV_ALIGN_DEFAULT, 0, 0);
//     lv_obj_update_layout(noticeCard);
//     int containerYOffset = lv_obj_get_height(noticeCard) + 16;
//     // network path container
//     lv_obj_t *network_card = GuiCreateLabelCard(parent, "Network", hashData->network);
//     lv_obj_align(network_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
//     lv_obj_update_layout(network_card);
//     containerYOffset += lv_obj_get_height(network_card) + 16;
//     // message hash container
//     lv_obj_t *message_hash_card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
//     lv_obj_align(message_hash_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
//     SetFlexContainerStyle(message_hash_card, LV_FLEX_FLOW_COLUMN, 16);
//     lv_obj_t *message_hash_label = GuiCreateTextLabel(message_hash_card, "Hash");
//     lv_obj_set_style_text_opa(message_hash_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_align_to(message_hash_label, message_hash_card, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 16);

//     // color message hash
//     char hash[128] = {0};
//     strncpy(hash, hashData->message, sizeof(hash) - 1);
//     char tempBuf[128] = {0};
//     snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);

//     lv_obj_t *message_hash_value = GuiCreateIllustrateLabel(message_hash_card, tempBuf);
//     lv_label_set_recolor(message_hash_value, true);
//     lv_obj_align_to(message_hash_value, message_hash_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
// }

// void GuiShowIotaSignMessageHashDetails(lv_obj_t *parent, void *totalData)
// {
//     lv_obj_set_size(parent, 408, 444);
//     lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
//     DisplayIotaSignMessageHash *hashData = (DisplayIotaSignMessageHash *)totalData;
//     // network card
//     lv_obj_t *network_card = GuiCreateLabelCard(parent, "Network", hashData->network);
//     lv_obj_align(network_card, LV_ALIGN_DEFAULT, 0, 0);
//     lv_obj_update_layout(network_card);
//     int containerYOffset = lv_obj_get_height(network_card) + 16;
//     // path card
//     // path = m/ + hashData->path
//     char path[128] = {0};
//     snprintf(path, sizeof(path), "m/%s", hashData->path);
//     lv_obj_t *path_card = GuiCreateLabelCard(parent, "Path", path);
//     lv_obj_align(path_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
//     lv_obj_update_layout(path_card);
//     containerYOffset += lv_obj_get_height(path_card) + 16;

//     // from address card
//     lv_obj_t *from_address_card = GuiCreateIotaFromAddressCard(parent, hashData->from_address);
//     lv_obj_align(from_address_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
//     lv_obj_update_layout(from_address_card);
//     containerYOffset += lv_obj_get_height(from_address_card) + 16;

//     // message hash container
//     lv_obj_t *message_hash_card = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
//     lv_obj_align(message_hash_card, LV_ALIGN_DEFAULT, 0, containerYOffset);
//     SetFlexContainerStyle(message_hash_card, LV_FLEX_FLOW_COLUMN, 16);
//     lv_obj_t *message_hash_label = GuiCreateTextLabel(message_hash_card, "Hash");
//     lv_obj_set_style_text_opa(message_hash_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
//     lv_obj_align_to(message_hash_label, message_hash_card, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 1);

//     // color message hash
//     char hash[128] = {0};
//     strncpy(hash, hashData->message, sizeof(hash) - 1);
//     char tempBuf[128] = {0};
//     snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);

//     lv_obj_t *message_hash_value = GuiCreateIllustrateLabel(message_hash_card, tempBuf);
//     lv_label_set_recolor(message_hash_value, true);
//     lv_obj_align_to(message_hash_value, message_hash_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
//     lv_label_set_long_mode(message_hash_value, LV_LABEL_LONG_WRAP);

//     // message hash notice content
//     lv_obj_t *message_hash_notice_content = GuiCreateIllustrateLabel(message_hash_card, _("sign_message_hash_notice_content"));
//     lv_obj_set_width(message_hash_notice_content, lv_pct(90));
//     lv_obj_set_style_text_color(message_hash_notice_content, lv_color_hex(0xF5C131), LV_PART_MAIN);
//     lv_obj_align_to(message_hash_notice_content, message_hash_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
//     lv_label_set_long_mode(message_hash_notice_content, LV_LABEL_LONG_WRAP);
// }

void FreeIotaMemory(void)
{
    // CHECK_FREE_UR_RESULT(g_urResult, false);
    // CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    // CHECK_FREE_PARSE_RESULT(g_parseResult);
}

// int GetIotaDetailLen(void *param)
// {
//     DisplayIotaIntentMessage *tx = (DisplayIotaIntentMessage *)param;
//     return strlen(tx->detail) + 1;
// }

// void GetSuiDetail(void *indata, void *param, uint32_t maxLen)
// {
//     DisplayIotaIntentMessage *tx = (DisplayIotaIntentMessage *)param;
//     // strcpy_s will exceed the stack size and the copy will fail
//     strcpy((char *)indata, tx->detail);
// }

UREncodeResult *GuiGetIotaSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        // encodeResult = iota_sign_intent(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

void GuiIotaTxOverview(lv_obj_t *parent, void *totalData)
{
    DisplayIotaIntentData *txData = (DisplayIotaIntentData *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *container = CreateSingleInfoView(parent, "kind", txData->kind);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 0);

    container = CreateSingleInfoTwoLineView(parent, "sender", txData->sender);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    container = CreateSingleInfoTwoLineView(parent, "owner", txData->owner);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    container = CreateSingleInfoView(parent, "price", txData->price);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    container = CreateSingleInfoView(parent, "gas budget", txData->gas_budget);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

void GuiIotaTxRawData(lv_obj_t *parent, void *totalData)
{
}

// UREncodeResult *GuiGetIotaSignHashQrCodeData(void)
// {
//     bool enable = IsPreviousLockScreenEnable();
//     SetLockScreen(false);
//     UREncodeResult *encodeResult;
//     void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
//     do {
//         uint8_t seed[64];
//         GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
//         int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
//         encodeResult = sui_sign_hash(data, seed, len);
//         ClearSecretCache();
//         CHECK_CHAIN_BREAK(encodeResult);
//     } while (0);
//     SetLockScreen(enable);
//     return encodeResult;
// }