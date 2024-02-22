#ifndef BTC_ONLY
#include "gui_analyze.h"
#include "rust.h"
#include "account_public_info.h"
#include "keystore.h"
#include "gui_chain.h"
#include "version.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "account_manager.h"
#include "assert.h"
#include "cjson/cJSON.h"

static uint8_t GetSolPublickeyIndex(char* rootPath);

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static ViewType g_viewType = ViewTypeUnKnown;

#define CHECK_FREE_PARSE_SOL_RESULT(result)                                                                                       \
    if (result != NULL)                                                                                                           \
    {                                                                                                                             \
        switch (g_viewType)                                                                                                       \
        {                                                                                                                         \
        case SolanaTx:                                                                                                            \
            free_TransactionParseResult_DisplaySolanaTx((PtrT_TransactionParseResult_DisplaySolanaTx)result);                     \
            break;                                                                                                                \
        case SolanaMessage:                                                                                                       \
            free_TransactionParseResult_DisplaySolanaMessage((PtrT_TransactionParseResult_DisplaySolanaMessage)result);           \
            break;                                                                                                                \
        default:                                                                                                                  \
            break;                                                                                                                \
        }                                                                                                                         \
        result = NULL;                                                                                                            \
    }


void GuiSetSolUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_viewType = g_isMulti ? urMultiResult->t : g_urResult->t;
#endif
}

UREncodeResult *GuiGetSolSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
#ifndef COMPILE_SIMULATOR
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = solana_sign_tx(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
#else
    UREncodeResult *encodeResult = NULL;
    encodeResult->is_multi_part = 0;
    encodeResult->data = "xpub6CZZYZBJ857yVCZXzqMBwuFMogBoDkrWzhsFiUd1SF7RUGaGryBRtpqJU6AGuYGpyabpnKf5SSMeSw9E9DSA8ZLov53FDnofx9wZLCpLNft";
    encodeResult->encoder = NULL;
    encodeResult->error_code = 0;
    encodeResult->error_message = NULL;
    return encodeResult;
#endif
}

void *GuiGetSolData(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_SOL_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplaySolanaTx parseResult = solana_parse_tx(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
#else
    return NULL;
#endif
}

PtrT_TransactionCheckResult GuiGetSolCheckResult(void)
{
#ifndef COMPILE_SIMULATOR
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return solana_check(data,  mfp, sizeof(mfp));
#else
    return NULL;
#endif
}

void *GuiGetSolMessageData(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_PARSE_SOL_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        char *path = sol_get_path(data);
        char pubkeyIndex = GetSolPublickeyIndex(path);
        char *pubKey = GetCurrentAccountPublicKey(pubkeyIndex);
        PtrT_TransactionParseResult_DisplaySolanaMessage parseResult = solana_parse_message(data, pubKey);
        free_ptr_string(path);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
#else
    return NULL;
#endif
}

void FreeSolMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_SOL_RESULT(g_parseResult);
#endif
}


void GetSolMessageType(void *indata, void *param)
{

    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (message->utf8_message) {
        strcpy((char *)indata, "utf8_message");
    } else {
        strcpy((char *)indata, "raw_message");
    }

}

void GetSolMessageFrom(void *indata, void *param)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (strlen(message->from) >= LABEL_MAX_BUFF_LEN) {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN - 3, "%s", message->from);
        strcat((char *)indata, "...");
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN, "%.*s...", LABEL_MAX_BUFF_LEN - 4, message->from);
    } else {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN, "%s", message->from);
    }
}
void GetSolMessageUtf8(void *indata, void *param)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (strlen(message->utf8_message) >= LABEL_MAX_BUFF_LEN) {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN - 3, "%s", message->utf8_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN, "%s", message->utf8_message);
    }
}

void GetSolMessageRaw(void *indata, void *param)
{
    int len = strlen("\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (strlen(message->raw_message) >= LABEL_MAX_BUFF_LEN - len) {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN - 3 - len, "%s", message->raw_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, LABEL_MAX_BUFF_LEN, "%s%s", message->raw_message, "\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    }
}

static uint8_t GetSolPublickeyIndex(char* rootPath)
{
    if (strcmp(rootPath, "44'/501'/0'") == 0) return XPUB_TYPE_SOL_BIP44_0;
    if (strcmp(rootPath, "44'/501'/1'") == 0) return XPUB_TYPE_SOL_BIP44_1;
    if (strcmp(rootPath, "44'/501'/2'") == 0) return XPUB_TYPE_SOL_BIP44_2;
    if (strcmp(rootPath, "44'/501'/3'") == 0) return XPUB_TYPE_SOL_BIP44_3;
    if (strcmp(rootPath, "44'/501'/4'") == 0) return XPUB_TYPE_SOL_BIP44_4;
    if (strcmp(rootPath, "44'/501'/5'") == 0) return XPUB_TYPE_SOL_BIP44_5;
    if (strcmp(rootPath, "44'/501'/6'") == 0) return XPUB_TYPE_SOL_BIP44_6;
    if (strcmp(rootPath, "44'/501'/7'") == 0) return XPUB_TYPE_SOL_BIP44_7;
    if (strcmp(rootPath, "44'/501'/8'") == 0) return XPUB_TYPE_SOL_BIP44_8;
    if (strcmp(rootPath, "44'/501'/9'") == 0) return XPUB_TYPE_SOL_BIP44_9;
    if (strcmp(rootPath, "44'/501'") == 0) return XPUB_TYPE_SOL_BIP44_ROOT;
    if (strcmp(rootPath, "44'/501'/0'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_0;
    if (strcmp(rootPath, "44'/501'/1'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_1;
    if (strcmp(rootPath, "44'/501'/2'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_2;
    if (strcmp(rootPath, "44'/501'/3'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_3;
    if (strcmp(rootPath, "44'/501'/4'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_4;
    if (strcmp(rootPath, "44'/501'/5'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_5;
    if (strcmp(rootPath, "44'/501'/6'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_6;
    if (strcmp(rootPath, "44'/501'/7'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_7;
    if (strcmp(rootPath, "44'/501'/8'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_8;
    if (strcmp(rootPath, "44'/501'/9'/0'") == 0) return XPUB_TYPE_SOL_BIP44_CHANGE_9;
    ASSERT(0);

    return -1;
}

static void SetContainerDefaultStyle(lv_obj_t *container)
{
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetTitleLabelStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, &openSans_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetContentLableStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, &openSans_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
}

static void SetVotesOnOrderLableStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetVotesOnContentLableStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xF5870A), LV_PART_MAIN);
}

static void GuiShowSolTxTransferOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{

    lv_obj_t *valueContainer = GuiCreateContainerWithParent(parent, 408, 106);
    lv_obj_align(valueContainer, LV_ALIGN_DEFAULT, 0, 0);
    SetContainerDefaultStyle(valueContainer);

    lv_obj_t *label = lv_label_create(valueContainer);
    lv_label_set_text(label, "Value");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(label);

    label = lv_label_create(valueContainer);
    lv_label_set_text(label, overviewData->transfer_value);
    lv_obj_set_style_text_font(label, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 50);

    lv_obj_t *mainActionContainer = GuiCreateContainerWithParent(parent, 408, 62);
    lv_obj_align_to(mainActionContainer, valueContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    SetContainerDefaultStyle(mainActionContainer);

    label = lv_label_create(mainActionContainer);
    lv_label_set_text(label, "Main Action");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(label);

    label = lv_label_create(mainActionContainer);
    lv_label_set_text(label, overviewData->main_action);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 151, 16);
    SetContentLableStyle(label);

    lv_obj_t *addressContainer = GuiCreateContainerWithParent(parent, 408, 224);
    lv_obj_align_to(addressContainer, mainActionContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    SetContainerDefaultStyle(addressContainer);

    label = lv_label_create(addressContainer);
    lv_label_set_text(label, "From");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(label);


    label = lv_label_create(addressContainer);
    lv_label_set_text(label, overviewData->transfer_from);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, 306);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);
    SetContentLableStyle(label);

    label = lv_label_create(addressContainer);
    lv_label_set_text(label, "To");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 130);
    SetTitleLabelStyle(label);

    label = lv_label_create(addressContainer);
    lv_label_set_text(label, overviewData->transfer_to);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, 306);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 168);
    SetContentLableStyle(label);
}

static void GuiShowSolTxVoteOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *votesOnContainer = GuiCreateContainerWithParent(parent, 408, 150);
    lv_obj_align(votesOnContainer, LV_ALIGN_DEFAULT, 0, 0);
    SetContainerDefaultStyle(votesOnContainer);

    lv_obj_t *label = lv_label_create(votesOnContainer);
    lv_label_set_text(label, "Votes on");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(label);

    int containerYOffset = 0;
    int votesOnContainerHeight = 16 + 30 + 16;

    lv_obj_t *prevLabel = label;

    PtrT_VecFFI_DisplaySolanaTxOverviewVotesOn votesOn = overviewData->votes_on;
    for (int i = 0; i < votesOn->size; i++) {
        label = lv_label_create(votesOnContainer);
        char order[12] = {0};
        sprintf(order, "%d.", i + 1);
        lv_label_set_text(label, order);
        SetVotesOnOrderLableStyle(label);
        lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        prevLabel = label;

        label = lv_label_create(votesOnContainer);
        lv_label_set_text(label, votesOn->data[i].slot);
        SetVotesOnContentLableStyle(label);
        lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

        votesOnContainerHeight = votesOnContainerHeight + 40 + 4;

    }
    lv_obj_set_height(votesOnContainer, votesOnContainerHeight);

    containerYOffset = containerYOffset + votesOnContainerHeight;

    lv_obj_t *mainActionContainer = GuiCreateContainerWithParent(parent, 408, 62);
    lv_obj_align_to(mainActionContainer, votesOnContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    SetContainerDefaultStyle(mainActionContainer);

    label = lv_label_create(mainActionContainer);
    lv_label_set_text(label, "Main Action");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(label);

    label = lv_label_create(mainActionContainer);
    lv_label_set_text(label, overviewData->main_action);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 151, 16);
    SetContentLableStyle(label);

    lv_obj_t *voteAccountContainer = GuiCreateContainerWithParent(parent, 408, 130);
    lv_obj_align_to(voteAccountContainer, mainActionContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    SetContainerDefaultStyle(voteAccountContainer);

    label = lv_label_create(voteAccountContainer);
    lv_label_set_text(label, "Vote Account");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    SetTitleLabelStyle(label);

    label = lv_label_create(voteAccountContainer);
    lv_label_set_text(label, overviewData->vote_account);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, 306);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);
    SetContentLableStyle(label);
}

static void GuiShowSolTxGeneralOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    PtrT_VecFFI_DisplaySolanaTxOverviewGeneral general = overviewData->general;

    int containerYOffset = 0;

    for (int i = 0; i < general->size; i++) {
        lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, 150);
        lv_obj_align(container, LV_ALIGN_DEFAULT, 0, containerYOffset);
        SetContainerDefaultStyle(container);

        char *program = general->data[i].program;
        lv_obj_t *orderLabel = lv_label_create(container);
        char order[10] = {0};
        sprintf(order, "#%d", i + 1);
        lv_label_set_text(orderLabel, order);
        lv_obj_set_style_text_font(orderLabel, &openSans_20, LV_PART_MAIN);
        lv_obj_set_style_text_color(orderLabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
        lv_obj_align(orderLabel, LV_ALIGN_TOP_LEFT, 24, 16);

        lv_obj_t *label = lv_label_create(container);
        lv_label_set_text(label, "Program");
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 62);
        SetTitleLabelStyle(label);

        label = lv_label_create(container);
        lv_label_set_text(label, program);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 121, 62);
        SetContentLableStyle(label);

        if (0 == strcmp(program, "Unknown")) {
            lv_obj_set_height(container, 108);
            containerYOffset = containerYOffset + 108 + 16;
            continue;
        } else {
            containerYOffset = containerYOffset + 150 + 16;
        }

        char *method = general->data[i].method;
        label = lv_label_create(container);
        lv_label_set_text(label, "Method");
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 100);
        SetTitleLabelStyle(label);

        label = lv_label_create(container);
        lv_label_set_text(label, method);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 113, 100);
        SetContentLableStyle(label);
    }
}

static void GuiShowSolTxUnknownOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, 302);
    lv_obj_align(container, LV_ALIGN_DEFAULT, 0, 0);
    SetContainerDefaultStyle(container);

    lv_obj_t *img = GuiCreateImg(container, &imgUnknown);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *label = lv_label_create(container);
    lv_label_set_text(label, "No Transaction Details");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 144);
    lv_obj_set_style_text_font(label, &openSans_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);

    label = lv_label_create(container);
    lv_label_set_text(label, "All data in this transaction are customized and can’t be decoded currently.");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 188);
    lv_obj_set_style_text_font(label, &openSans_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(label, 360);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
}

void GuiShowSolTxOverview(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    DisplaySolanaTx *txData = (DisplaySolanaTx*)totalData;
    PtrT_DisplaySolanaTxOverview overviewData = txData->overview;
    if (0 == strcmp(overviewData->display_type, "Transfer")) {
        GuiShowSolTxTransferOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "Vote")) {
        GuiShowSolTxVoteOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "General")) {
        GuiShowSolTxGeneralOverview(parent, overviewData);
    } else {
        GuiShowSolTxUnknownOverview(parent, overviewData);
    }
}

void GuiShowSolTxDetail(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, 444);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 408, 444);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(cont, 0, 0);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cont, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cont, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cont, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cont, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    DisplaySolanaTx *txData = (DisplaySolanaTx*)totalData;
    PtrString txDetail = txData->detail;

    lv_obj_t *label = lv_label_create(cont);
    cJSON *root = cJSON_Parse((const char *)txDetail);
    lv_label_set_text(label, cJSON_Print(root));
    cJSON_Delete(root);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, 360);
    SetTitleLabelStyle(label);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
}
#endif
