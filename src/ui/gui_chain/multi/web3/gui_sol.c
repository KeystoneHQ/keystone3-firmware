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
#include "user_memory.h"
#include "gui_qr_hintbox.h"

typedef struct SolanaLearnMoreData {
    PtrString title;
    PtrString content;
} SolanaLearnMoreData_t;
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
        }                                                                                                                       \
        result = NULL;                                                                                                            \
    }

void GuiSetSolUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_viewType = g_isMulti ? urMultiResult->t : g_urResult->t;
}

UREncodeResult *GuiGetSolSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
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
}

void *GuiGetSolData(void)
{
    CHECK_FREE_PARSE_SOL_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplaySolanaTx parseResult = solana_parse_tx(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetSolCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return solana_check(data,  mfp, sizeof(mfp));
}

bool GetSolMessageFromExist(void *indata, void *param)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (message->from == NULL) {
        return false;
    }
    return true;
}

bool GetSolMessageFromNotExist(void *indata, void *param)
{
    return !GetSolMessageFromExist(indata, param);
}

void *GuiGetSolMessageData(void)
{
    CHECK_FREE_PARSE_SOL_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        char *path = sol_get_path(data);
        ChainType pubkeyIndex = CheckSolPathSupport(path);
        char *pubKey = "";
        if (pubkeyIndex != XPUB_TYPE_NUM) {
            pubKey = GetCurrentAccountPublicKey(pubkeyIndex);
        }
        PtrT_TransactionParseResult_DisplaySolanaMessage parseResult = solana_parse_message(data, pubKey);
        free_ptr_string(path);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void FreeSolMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_SOL_RESULT(g_parseResult);
}

void GetSolMessagePos(uint16_t *x, uint16_t *y, void *param)
{
    if (GetSolMessageFromExist(NULL, param)) {
        *x = 0;
    } else {
        *x = 24;
    }
    *y = 11;
}

static void learn_more_click_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    static SolanaLearnMoreData_t solanaLearnMoredata;
    SolanaLearnMoreData_t *item = (SolanaLearnMoreData_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        GuiNormalHitBoxOpen(item->title, item->content);
    }
}

void GetSolMessageType(void *indata, void *param, uint32_t maxLen)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (message->utf8_message) {
        strcpy_s((char *)indata, maxLen, "utf8_message");
    } else {
        strcpy_s((char *)indata, maxLen, "raw_message");
    }

}

void GetSolMessageFrom(void *indata, void *param, uint32_t maxLen)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (strlen(message->from) >= maxLen) {
        snprintf((char *)indata, maxLen - 3, "%s", message->from);
        strcat((char *)indata, "...");
        snprintf((char *)indata, maxLen, "%.*s...", maxLen - 4, message->from);
    } else {
        strcpy_s((char *)indata, maxLen, message->from);
    }
}

void GetSolMessageUtf8(void *indata, void *param, uint32_t maxLen)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (strlen(message->utf8_message) >= maxLen) {
        snprintf((char *)indata, maxLen - 3, "%s", message->utf8_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, maxLen, "%s", message->utf8_message);
    }
}

void GetSolMessageRaw(void *indata, void *param, uint32_t maxLen)
{
    int len = strlen("\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    if (strlen(message->raw_message) >= maxLen - len) {
        snprintf((char *)indata, maxLen - 3 - len, "%s", message->raw_message);
        strcat((char *)indata, "...");
    } else {
        snprintf((char *)indata, maxLen, "%s%s", message->raw_message, "\n#F5C131 The data is not parseable. Please#\n#F5C131 refer to the software wallet interface#\n#F5C131 for viewing.#");
    }
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

static void SetVotesOnOrderLableStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defLittleTitleFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void SetVotesOnContentLableStyle(lv_obj_t *label)
{
    lv_obj_set_style_text_font(label, g_defLittleTitleFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xF5870A), LV_PART_MAIN);
}

lv_obj_t *GuiCreateAutoHeightContainer(lv_obj_t *parent, uint16_t width, uint16_t padding_x)
{
    lv_obj_t * container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
    SetContainerDefaultStyle(container);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(container, padding_x, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(container, padding_x, LV_PART_MAIN);
    return container;
}

lv_obj_t* GuiCreateWarningCard(lv_obj_t* parent)
{
    lv_obj_t* card = GuiCreateAutoHeightContainer(parent, 408, 24);
    SetContainerDefaultStyle(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xF55831), LV_PART_MAIN);
    lv_obj_set_style_radius(card, 8, LV_PART_MAIN);

    lv_obj_t* warningIcon = GuiCreateImg(card, &imgWarningRed);
    lv_obj_align(warningIcon, LV_ALIGN_TOP_LEFT, 24, 0);

    lv_obj_t* title_label = GuiCreateTextLabel(card, "WARNING");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xF55831), LV_PART_MAIN);
    lv_obj_align_to(title_label, warningIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t* content_label = GuiCreateIllustrateLabel(card, _("solana_warning"));
    lv_obj_set_style_text_color(content_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_width(content_label, lv_pct(90));
    lv_obj_align_to(content_label, warningIcon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    return card;
}


lv_obj_t * CreateSolanaSquadsProposalOverviewCard(lv_obj_t *parent, PtrString program, PtrString method, PtrString memo, PtrString data)
{
    lv_obj_t *container = GuiCreateAutoHeightContainer(parent, 408, 16);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
    SetContainerDefaultStyle(container);
    lv_obj_t *programLabel = GuiCreateTextLabel(container, "Program");
    lv_obj_align(programLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(programLabel);

    lv_obj_t *squadsIcon = GuiCreateImg(container, &imgSquads);
    lv_obj_align_to(squadsIcon, programLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t *squadsLabel = GuiCreateIllustrateLabel(container, program);
    lv_obj_set_style_text_color(squadsLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
    lv_obj_align_to(squadsLabel, squadsIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t *methodLabel = GuiCreateTextLabel(container, "Method");
    SetTitleLabelStyle(methodLabel);
    lv_obj_align_to(methodLabel, squadsIcon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t *methodValueLabel = GuiCreateIllustrateLabel(container, method);
    lv_obj_set_style_text_color(methodValueLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(methodValueLabel, methodLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    if (strlen(memo) > 0) {
        lv_obj_t *memoLabel = lv_label_create(container);
        lv_label_set_text(memoLabel, "Memo");
        SetTitleLabelStyle(memoLabel);
        lv_obj_align_to(memoLabel, methodValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

        lv_obj_t *memoValueLabel = GuiCreateIllustrateLabel(container, memo);
        // long text unwrap
        lv_label_set_long_mode(memoValueLabel, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(memoValueLabel, lv_pct(90));
        lv_obj_set_style_text_color(memoValueLabel, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(memoValueLabel, memoLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    }

    if (strlen(data) > 0) {
        lv_obj_t *dataLabel = GuiCreateTextLabel(container, "Data");
        SetTitleLabelStyle(dataLabel);
        if (strlen(memo) != 0) {
            lv_obj_align_to(dataLabel, methodValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 100);
        } else {
            lv_obj_align_to(dataLabel, methodValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        }
        lv_obj_t *dataValueLabel = GuiCreateIllustrateLabel(container, data);
        lv_obj_set_style_text_color(dataValueLabel, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(dataValueLabel, dataLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    }
    return container;
}

lv_obj_t *CreateSquadsSolanaTransferOverviewCard(lv_obj_t *parent, PtrString from, PtrString to, PtrString amount, PtrString note)
{
    lv_obj_t *container = GuiCreateAutoHeightContainer(parent, 408, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(container);
    if (strcmp(amount, "0.05 SOL") == 0) {
        lv_label_set_text(label, "Platform Fee");
    } else if (strcmp(amount, "0.001 SOL") == 0) {
        lv_label_set_text(label, "Account Deposit");
    } else {
        lv_label_set_text(label, "Amount");
    }
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(label);

    lv_obj_t * amountlabel = lv_label_create(container);
    lv_label_set_text(amountlabel, amount);
    lv_obj_set_style_text_font(amountlabel, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(amountlabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(amountlabel, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);


    lv_obj_t * fromlabel = GuiCreateTextLabel(container, "From");
    lv_obj_align_to(fromlabel, amountlabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(fromlabel);

    lv_obj_t * fromValuelabel = GuiCreateIllustrateLabel(container, from);
    lv_label_set_long_mode(fromValuelabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(fromValuelabel, 306);
    SetContentLableStyle(fromValuelabel);
    lv_obj_align_to(fromValuelabel, fromlabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * tolabel = lv_label_create(container);
    lv_label_set_text(tolabel, "To");
    SetTitleLabelStyle(tolabel);
    lv_obj_align_to(tolabel, fromValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_t * toValuelabel = lv_label_create(container);
    lv_label_set_text(toValuelabel, to);
    lv_label_set_long_mode(toValuelabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(toValuelabel, 306);
    lv_obj_align_to(toValuelabel, tolabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetContentLableStyle(toValuelabel);

    // add label fot to label
    if (strcmp(to, "5DH2e3cJmFpyi6mk65EGFediunm4ui6BiKNUNrhWtD1b") == 0 && strcmp(amount, "0.05 SOL") == 0) {
        lv_obj_t *squadsIcon = GuiCreateImg(container, &imgSquads);
        lv_obj_align_to(squadsIcon, toValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
        lv_obj_t *squadsLabel = lv_label_create(container);
        lv_label_set_text(squadsLabel, "Squads");
        lv_obj_set_style_text_font(squadsLabel, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(squadsLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
        lv_obj_align_to(squadsLabel, squadsIcon, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    } else if (strcmp(to, "5DH2e3cJmFpyi6mk65EGFediunm4ui6BiKNUNrhWtD1b") == 0 && strcmp(amount, "0.05 SOL") != 0) {
        lv_obj_t * unkonwnAddressLabel = lv_label_create(container);
        lv_label_set_text(unkonwnAddressLabel, "Unknown Address");
        lv_obj_set_style_text_color(unkonwnAddressLabel, lv_color_hex(0xF55831), LV_PART_MAIN);
        lv_obj_set_style_bg_color(unkonwnAddressLabel, lv_color_hex(0xF5583133), LV_PART_MAIN);
        lv_obj_set_style_radius(unkonwnAddressLabel, 12, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(unkonwnAddressLabel, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_pad_left(unkonwnAddressLabel, 12, LV_PART_MAIN);
        lv_obj_set_style_pad_top(unkonwnAddressLabel, 8, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(unkonwnAddressLabel, 8, LV_PART_MAIN);
        lv_obj_set_size(unkonwnAddressLabel, lv_pct(90), LV_SIZE_CONTENT);
        lv_obj_align_to(unkonwnAddressLabel, toValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    }



    if (strlen(note) > 0) {
        lv_obj_t * notelabel = GuiCreateTextLabel(container, "Note");
        SetTitleLabelStyle(notelabel);
        lv_obj_align_to(notelabel, toValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        lv_obj_t * noteValuelabel = GuiCreateIllustrateLabel(container, note);
        lv_label_set_long_mode(noteValuelabel, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(noteValuelabel, 306);
        lv_obj_align_to(noteValuelabel, notelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
        SetContentLableStyle(noteValuelabel);
    }
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


lv_obj_t* GuiCreateSolNoticeCard(lv_obj_t* parent)
{
    lv_obj_t* card = GuiCreateAutoHeightContainer(parent, 408, 24);
    SetContainerDefaultStyle(card);
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN);

    lv_obj_t* noticeIcon = GuiCreateImg(card, &imgNotice);
    lv_obj_align(noticeIcon, LV_ALIGN_TOP_LEFT, 24, 0);

    lv_obj_t* title_label = GuiCreateTextLabel(card, "Notice");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(title_label, noticeIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t* content_label = GuiCreateIllustrateLabel(card, _("spl_notice"));
    lv_obj_set_style_text_color(content_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_width(content_label, lv_pct(90));
    lv_obj_align_to(content_label, noticeIcon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    return card;
}


void SolanaSplTokenAddressLearnMore(lv_event_t *e)
{
    const char* address = (const char*)lv_event_get_user_data(e);
    if (address != NULL) {
        char url[512];
        snprintf(url, sizeof(url), "https://solscan.io/token/%s", address);
        GuiQRCodeHintBoxOpenBig(url, "Scan to double-check the Token account", "", url);
    }
}

static lv_obj_t * GuiShowSplTokenInfoOverviewCard(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *container = GuiCreateAutoHeightContainer(parent, 408, 16);
    SetContainerDefaultStyle(container);
    PtrT_DisplaySolanaTxSplTokenTransferOverview splTokenInfo = overviewData->spl_token_transfer;

    lv_obj_t *tokenNameLabel = lv_label_create(container);
    lv_label_set_text(tokenNameLabel, "Token Name");
    lv_obj_align(tokenNameLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(tokenNameLabel);

    lv_obj_t *tokenNameContainer = lv_obj_create(container);
    lv_obj_set_size(tokenNameContainer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(tokenNameContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tokenNameContainer, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(tokenNameContainer, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(tokenNameContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tokenNameContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Easter egg for dogwifcat EKpQGSJtjMFqKZ9KQanSqYXRcF8fBopzLHYxdM65zcjm
    if (strcmp(splTokenInfo->token_mint_account, "EKpQGSJtjMFqKZ9KQanSqYXRcF8fBopzLHYxdM65zcjm") == 0) {
        lv_obj_t *dogwifcatIcon = GuiCreateImg(tokenNameContainer, &imgWIF);
        lv_obj_set_style_pad_right(dogwifcatIcon, 8, LV_PART_MAIN);
    }

    lv_obj_t *tokenNameValueLabel = lv_label_create(tokenNameContainer);
    lv_label_set_text(tokenNameValueLabel, splTokenInfo->token_name);

    if (strcmp(splTokenInfo->token_name, "Unknown") == 0) {
        SetContentLableStyle(tokenNameValueLabel);
    } else {
        lv_obj_set_style_text_color(tokenNameValueLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
    }

    lv_obj_set_style_text_font(tokenNameValueLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_align_to(tokenNameContainer, tokenNameLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t *tokenMintLabel = lv_label_create(container);
    lv_label_set_text(tokenMintLabel, "Token Account");
    SetTitleLabelStyle(tokenMintLabel);

    lv_obj_align_to(tokenMintLabel, tokenNameContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t *tokenMintValueLabel = lv_label_create(container);
    lv_obj_set_width(tokenMintValueLabel, 360);
    lv_label_set_text(tokenMintValueLabel, splTokenInfo->token_mint_account);
    SetContentLableStyle(tokenMintValueLabel);
    lv_label_set_long_mode(tokenMintValueLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(tokenMintValueLabel, tokenMintLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);


    lv_obj_t * checkTokenAccountLabel = lv_label_create(container);
    lv_label_set_text(checkTokenAccountLabel, "Check Token Account");
    lv_obj_set_style_text_color(checkTokenAccountLabel, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_t * checkTokenAccountIcon = GuiCreateImg(checkTokenAccountLabel, &imgQrcodeTurquoise);
    lv_obj_align_to(checkTokenAccountIcon, checkTokenAccountLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    lv_obj_add_flag(checkTokenAccountIcon, LV_OBJ_FLAG_CLICKABLE);

    const char* tokenAddress = splTokenInfo->token_mint_account;
    lv_obj_add_event_cb(checkTokenAccountIcon, SolanaSplTokenAddressLearnMore, LV_EVENT_CLICKED, (void*)tokenAddress);

    if (strcmp(splTokenInfo->token_name, "Unknown") == 0) {
        lv_obj_t * noticeBar = GuiCreateAutoHeightContainer(container, 408, 8);
        lv_obj_set_width(noticeBar, 360);
        lv_obj_set_style_bg_color(noticeBar, lv_color_hex(0xF5583133), LV_PART_MAIN);
        lv_obj_set_style_radius(noticeBar, 12, LV_PART_MAIN);

        lv_obj_t * noticeLabel = lv_label_create(noticeBar);
        lv_label_set_text(noticeLabel, "Unknown Token Account");
        lv_obj_set_style_text_color(noticeLabel, lv_color_hex(0xF55831), LV_PART_MAIN);
        lv_obj_align(noticeLabel, LV_ALIGN_TOP_LEFT, 12, 8);

        lv_obj_align_to(noticeBar, tokenMintValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

        lv_obj_align_to(checkTokenAccountLabel, noticeBar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    } else {
        lv_obj_align_to(checkTokenAccountLabel, tokenMintValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    }
    return container;
}
static void GuiShowJupiterV6SwapOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *swapOverviewContainer = GuiCreateAutoHeightContainer(parent, 408, 16);
    SetContainerDefaultStyle(swapOverviewContainer);
    PtrT_DisplaySolanaTxOverviewJupiterV6Swap jupiterV6SwapOverview = overviewData->jupiter_v6_swap;
    lv_obj_t * swapLabel = lv_label_create(swapOverviewContainer);
    lv_label_set_text(swapLabel, "Swap");
    lv_obj_align(swapLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(swapLabel);
    lv_obj_t * amountValueLabel = lv_label_create(swapOverviewContainer);
    lv_label_set_text(amountValueLabel, jupiterV6SwapOverview->token_a_overview->token_amount);
    lv_obj_set_style_text_color(amountValueLabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_set_style_text_font(amountValueLabel, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_align_to(amountValueLabel, swapLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    // if tokenA exist in address lookup table, then show info icon
    if (jupiterV6SwapOverview->token_a_overview->exist_in_address_lookup_table) {
        lv_obj_t *info_icon = GuiCreateImg(swapOverviewContainer, &imgInfoSmall);
        lv_obj_set_style_pad_right(info_icon, 0, LV_PART_MAIN);
        lv_obj_add_flag(info_icon, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align_to(info_icon, amountValueLabel, LV_ALIGN_OUT_RIGHT_MID, 4, 0);
        static SolanaLearnMoreData_t tokenAmountLearnMoredata;
        tokenAmountLearnMoredata.title = "Token Amount";
        // if current instruction name is "JupiterV6Route" we show a different desc
        if (strcmp(jupiterV6SwapOverview->instruction_name, "JupiterV6Route") == 0) {
            tokenAmountLearnMoredata.content = _("instruction_does_not_contain_token_info");
        } else {
            tokenAmountLearnMoredata.content = _("token_in_alt_desc");
        }
        lv_obj_add_event_cb(info_icon, learn_more_click_event_handler, LV_EVENT_CLICKED, &tokenAmountLearnMoredata);
    }

    // TokenB
    lv_obj_t * toLabel = lv_label_create(swapOverviewContainer);
    lv_label_set_text(toLabel, "To");
    SetTitleLabelStyle(toLabel);
    // if current instruction name is "JupiterV6Route" we dont need to show the mint account info
    if (strcmp(jupiterV6SwapOverview->instruction_name, "JupiterV6Route") == 0) {
        lv_obj_align_to(toLabel, amountValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    } else {
        // mint account label
        lv_obj_t * mintAccountLabel = lv_label_create(swapOverviewContainer);
        lv_label_set_text(mintAccountLabel, "Mint Account");
        SetTitleLabelStyle(mintAccountLabel);
        // mint account value
        lv_obj_t * mintAccountValueLabel = lv_label_create(swapOverviewContainer);
        lv_label_set_text(mintAccountValueLabel, jupiterV6SwapOverview->token_a_overview->token_address);
        lv_label_set_long_mode(mintAccountValueLabel, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(mintAccountValueLabel, lv_pct(90));
        SetContentLableStyle(mintAccountValueLabel);

        lv_obj_t * checkTokenAMintAccountLabel = lv_label_create(swapOverviewContainer);
        lv_label_set_text(checkTokenAMintAccountLabel, "Check Token Account");
        lv_obj_set_style_text_color(checkTokenAMintAccountLabel, lv_color_hex(0x1BE0C6), LV_PART_MAIN);

        lv_obj_t * checkTokenAMintAccountIcon = GuiCreateImg(swapOverviewContainer, &imgQrcodeTurquoise);

        lv_obj_add_flag(checkTokenAMintAccountIcon, LV_OBJ_FLAG_CLICKABLE);
        const char* tokenAAddress = jupiterV6SwapOverview->token_a_overview->token_address;
        lv_obj_add_event_cb(checkTokenAMintAccountIcon, SolanaSplTokenAddressLearnMore, LV_EVENT_CLICKED, (void*)tokenAAddress);
        lv_obj_align_to(mintAccountLabel, amountValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
        lv_obj_align_to(mintAccountValueLabel, mintAccountLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_obj_align_to(checkTokenAMintAccountLabel, mintAccountValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
        lv_obj_align_to(checkTokenAMintAccountIcon, checkTokenAMintAccountLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
        lv_obj_align_to(toLabel, checkTokenAMintAccountLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    }
    // amount label value
    lv_obj_t * tokenBAmountValueLabel = lv_label_create(swapOverviewContainer);
    lv_label_set_text(tokenBAmountValueLabel, jupiterV6SwapOverview->token_b_overview->token_amount);
    lv_obj_align_to(tokenBAmountValueLabel, toLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_set_style_text_color(tokenBAmountValueLabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_set_style_text_font(tokenBAmountValueLabel, &openSansEnLittleTitle, LV_PART_MAIN);
    // if tokenB exist in address lookup table, then show info icon
    if (jupiterV6SwapOverview->token_b_overview->exist_in_address_lookup_table) {
        lv_obj_t *info_icon = GuiCreateImg(swapOverviewContainer, &imgInfoSmall);
        lv_obj_set_style_pad_right(info_icon, 0, LV_PART_MAIN);
        lv_obj_add_flag(info_icon, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align_to(info_icon, tokenBAmountValueLabel, LV_ALIGN_OUT_RIGHT_MID, 4, 0);
        static SolanaLearnMoreData_t tokenAmountLearnMoredata;
        tokenAmountLearnMoredata.title = _("token_details");
        tokenAmountLearnMoredata.content = _("token_in_alt_desc");
        lv_obj_add_event_cb(info_icon, learn_more_click_event_handler, LV_EVENT_CLICKED, &tokenAmountLearnMoredata);
    }
    // mint account label
    lv_obj_t * tokenBmintAccountLabel = lv_label_create(swapOverviewContainer);
    lv_label_set_text(tokenBmintAccountLabel, "Mint Account");
    lv_obj_align_to(tokenBmintAccountLabel, tokenBAmountValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetTitleLabelStyle(tokenBmintAccountLabel);

    lv_obj_t * tokenBmintAccountValueLabel = lv_label_create(swapOverviewContainer);
    lv_label_set_text(tokenBmintAccountValueLabel, jupiterV6SwapOverview->token_b_overview->token_address);
    lv_obj_align_to(tokenBmintAccountValueLabel, tokenBmintAccountLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_label_set_long_mode(tokenBmintAccountValueLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(tokenBmintAccountValueLabel, lv_pct(90));
    SetContentLableStyle(tokenBmintAccountValueLabel);

    lv_obj_t * checkTokenBMintAccountLabel = lv_label_create(swapOverviewContainer);
    lv_label_set_text(checkTokenBMintAccountLabel, "Check Token Account");
    lv_obj_set_style_text_color(checkTokenBMintAccountLabel, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align_to(checkTokenBMintAccountLabel, tokenBmintAccountValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * checkTokenBMintAccountIcon = GuiCreateImg(swapOverviewContainer, &imgQrcodeTurquoise);
    lv_obj_align_to(checkTokenBMintAccountIcon, checkTokenBMintAccountLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    lv_obj_add_flag(checkTokenBMintAccountIcon, LV_OBJ_FLAG_CLICKABLE);
    const char* tokenBAddress = jupiterV6SwapOverview->token_b_overview->token_address;
    lv_obj_add_event_cb(checkTokenBMintAccountIcon, SolanaSplTokenAddressLearnMore, LV_EVENT_CLICKED, (void*)tokenBAddress);
    // jupiterv6 platform overview container
    lv_obj_t * platformOverviewContainer = GuiCreateAutoHeightContainer(parent, 408, 16);
    SetContainerDefaultStyle(platformOverviewContainer);
    // swap platform label
    lv_obj_t * swapPlatformLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(swapPlatformLabel, "Swap Platform");
    lv_obj_align(swapPlatformLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(swapPlatformLabel);
    // platform icon label
    lv_obj_t * jupiterIcon = GuiCreateImg(platformOverviewContainer, &imgJupiter);
    lv_obj_align_to(jupiterIcon, swapPlatformLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t *jupiterLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(jupiterLabel, "Jupiter Aggregator v6");
    lv_obj_set_style_text_font(jupiterLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(jupiterLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
    lv_obj_align_to(jupiterLabel, jupiterIcon, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    // platform address label
    lv_obj_t * platformAddressLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(platformAddressLabel, jupiterV6SwapOverview->program_address);
    lv_obj_align_to(platformAddressLabel, jupiterIcon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_label_set_long_mode(platformAddressLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(platformAddressLabel, lv_pct(90));
    SetContentLableStyle(platformAddressLabel);

    // Slippage Tolerance label
    lv_obj_t * slippageToleranceLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(slippageToleranceLabel, "Slippage Tolerance");
    lv_obj_align_to(slippageToleranceLabel, platformAddressLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(slippageToleranceLabel);
    // slippage value
    lv_obj_t * slippageValueLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(slippageValueLabel, jupiterV6SwapOverview->slippage_bps);
    lv_obj_align_to(slippageValueLabel, slippageToleranceLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetContentLableStyle(slippageValueLabel);
    // Partner Referral Fee
    lv_obj_t * partnerReferralFeeLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(partnerReferralFeeLabel, "Partner Referral Fee");
    lv_obj_align_to(partnerReferralFeeLabel, slippageValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(partnerReferralFeeLabel);
    // partner referral fee value
    lv_obj_t * partnerReferralFeeValueLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(partnerReferralFeeValueLabel, jupiterV6SwapOverview->platform_fee_bps);
    lv_obj_align_to(partnerReferralFeeValueLabel, partnerReferralFeeLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetContentLableStyle(partnerReferralFeeValueLabel);
    // platform container align to swap container
    lv_obj_align_to(platformOverviewContainer, swapOverviewContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
}
static void GuiShowSplTokenTransferOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *tokenInfoCard = GuiShowSplTokenInfoOverviewCard(parent, overviewData);
    lv_obj_t *container = GuiCreateAutoHeightContainer(parent, 408, 16);
    SetContainerDefaultStyle(container);
    PtrT_DisplaySolanaTxSplTokenTransferOverview splTokenTransfer = overviewData->spl_token_transfer;
    if (strcmp(splTokenTransfer->token_name, "Unknown") == 0) {
        lv_obj_t *noticeCard = GuiCreateSolNoticeCard(parent);
        lv_obj_align_to(tokenInfoCard, noticeCard, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }
    lv_obj_align_to(container, tokenInfoCard, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);


    lv_obj_t *label = lv_label_create(container);
    lv_label_set_text(label, "Amount");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(label);

    lv_obj_t * amountlabel = lv_label_create(container);
    lv_label_set_text(amountlabel, splTokenTransfer->amount);
    lv_obj_set_style_text_color(amountlabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_set_style_text_font(amountlabel, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_align_to(amountlabel, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * authoritylabel = lv_label_create(container);
    lv_label_set_text(authoritylabel, "Authority Account");
    lv_obj_align_to(authoritylabel, amountlabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(authoritylabel);
    lv_obj_t *authInfoSIcon = GuiCreateImg(container, &imgInfoS);
    lv_obj_align_to(authInfoSIcon, authoritylabel, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    lv_obj_add_flag(authInfoSIcon, LV_OBJ_FLAG_CLICKABLE);
    static SolanaLearnMoreData_t authLearnMoredata;
    authLearnMoredata.title = "Authority Account";
    authLearnMoredata.content = _("solana_auth_account");
    lv_obj_add_event_cb(authInfoSIcon, learn_more_click_event_handler, LV_EVENT_CLICKED, &authLearnMoredata);

    lv_obj_t * authorityValueLabel = lv_label_create(container);
    lv_label_set_text(authorityValueLabel, splTokenTransfer->authority);
    lv_label_set_long_mode(authorityValueLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(authorityValueLabel, 306);
    SetContentLableStyle(authorityValueLabel);
    lv_obj_align_to(authorityValueLabel, authoritylabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * sourcelabel = lv_label_create(container);
    lv_label_set_text(sourcelabel, "Source Account");
    lv_obj_align_to(sourcelabel, authorityValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    SetTitleLabelStyle(sourcelabel);
    lv_obj_t *sourceInfoSIcon = GuiCreateImg(container, &imgInfoS);
    lv_obj_align_to(sourceInfoSIcon, sourcelabel, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    lv_obj_add_flag(sourceInfoSIcon, LV_OBJ_FLAG_CLICKABLE);
    static SolanaLearnMoreData_t sourceLearnMoredata;
    sourceLearnMoredata.title = "Source Account";
    sourceLearnMoredata.content = _("solana_source_ata");
    lv_obj_add_event_cb(sourceInfoSIcon, learn_more_click_event_handler, LV_EVENT_CLICKED, &sourceLearnMoredata);

    lv_obj_t * sourceValuelabel = lv_label_create(container);
    lv_label_set_text(sourceValuelabel, splTokenTransfer->source);
    lv_label_set_long_mode(sourceValuelabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(sourceValuelabel, 306);
    SetContentLableStyle(sourceValuelabel);
    lv_obj_align_to(sourceValuelabel, sourcelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * destLabel = lv_label_create(container);
    lv_label_set_text(destLabel, "Destination Account");
    SetTitleLabelStyle(destLabel);
    lv_obj_align_to(destLabel, sourceValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_t * destValuelabel = lv_label_create(container);
    lv_label_set_text(destValuelabel, splTokenTransfer->destination);
    lv_label_set_long_mode(destValuelabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(destValuelabel, 306);
    lv_obj_align_to(destValuelabel, destLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetContentLableStyle(destValuelabel);

    lv_obj_t *destInfoSIcon = GuiCreateImg(container, &imgInfoS);
    lv_obj_align_to(destInfoSIcon, destLabel, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    lv_obj_add_flag(destInfoSIcon, LV_OBJ_FLAG_CLICKABLE);
    static SolanaLearnMoreData_t destLearnMoredata;
    destLearnMoredata.title = "Destination Account";
    destLearnMoredata.content = _("solana_ata_desc");
    lv_obj_add_event_cb(destInfoSIcon, learn_more_click_event_handler, LV_EVENT_CLICKED, &destLearnMoredata);
}

static void GuiShowSolTxSquadsProposalOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    int containerYOffset = 0;
    PtrT_VecFFI_DisplaySolanaTxProposalOverview squadsProposal = overviewData->squads_proposal;
    for (int i = 0; i < squadsProposal->size; i++) {
        char *program = squadsProposal->data[i].program;
        char *method = squadsProposal->data[i].method;
        char *memo = squadsProposal->data[i].memo;
        if (strcmp(method, "Transfer") == 0) {
            continue;
        }
        lv_obj_t * proposalCard = CreateSolanaSquadsProposalOverviewCard(parent, program, method, memo, "");
        lv_obj_align(proposalCard, LV_ALIGN_TOP_LEFT, 0, containerYOffset);
        // force update layout to calculate the card height
        lv_obj_update_layout(proposalCard);
        int height = lv_obj_get_height(proposalCard) + 16;
        containerYOffset += height;
    }
    for (int i = 0; i < squadsProposal->size; i++) {
        char *method = squadsProposal->data[i].method;
        char *data = squadsProposal->data[i].data;
        if (strcmp(method, "Transfer") != 0) {
            continue;
        }
        lv_obj_t *feeContainer =  GuiCreateAutoHeightContainer(parent, 408, 16);
        lv_obj_t *feeLabel = lv_label_create(feeContainer);
        lv_label_set_text(feeLabel, "Fee");
        lv_obj_set_style_text_color(feeLabel, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_align(feeLabel, LV_ALIGN_TOP_LEFT, 24, 0);
        SetTitleLabelStyle(feeLabel);

        lv_obj_t *feeValue = lv_label_create(feeContainer);
        lv_label_set_text(feeValue, data);
        lv_obj_set_style_text_color(feeValue, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(feeValue, feeLabel, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
        lv_obj_align(feeContainer, LV_ALIGN_TOP_LEFT, 0, containerYOffset);
    }

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
        char order[BUFFER_SIZE_16] = {0};
        snprintf_s(order, BUFFER_SIZE_16, "%d.", i + 1);
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
        char order[BUFFER_SIZE_16] = {0};
        snprintf_s(order, BUFFER_SIZE_16, "#%d", i + 1);
        lv_label_set_text(orderLabel, order);
        lv_obj_set_style_text_font(orderLabel, g_defIllustrateFont, LV_PART_MAIN);
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
static void GuiShowSolTxUnknownOverview(lv_obj_t *parent)
{
    uint16_t height = 177;
    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, 302);
    lv_obj_align(container, LV_ALIGN_DEFAULT, 0, 0);
    SetContainerDefaultStyle(container);

    lv_obj_t *img = GuiCreateImg(container, &imgUnknown);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *label = GuiCreateTextLabel(container, _("unknown_transaction_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 144);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_refr_size(label);
    height += lv_obj_get_self_height(label);

    label = GuiCreateNoticeLabel(container, _("unknown_transaction_desc"));
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -36);
    lv_obj_set_width(label, 360);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_refr_size(label);
    height += lv_obj_get_self_height(label);
    lv_obj_set_height(container, height);
}



void SolanaAddressLearnMore(lv_event_t *e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    const char* address = (const char*)lv_obj_get_user_data(obj);
    if (address != NULL) {
        char url[512];
        snprintf(url, sizeof(url), "https://solscan.io/account/ %s#tableEntries", address);
        GuiQRCodeHintBoxOpenBig(url, "Address Lookup Table URL", _("solana_alt_notice"), url);
    }
}

lv_obj_t* GuiCreateSolanaTextLabel(lv_obj_t *parent, PtrString content)
{
    lv_obj_t *textLabelContainer = GuiCreateContainerWithParent(parent, 87, 30);
    lv_obj_set_style_radius(textLabelContainer, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(textLabelContainer, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(textLabelContainer, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *textLabel = lv_label_create(textLabelContainer);
    lv_label_set_text(textLabel, content);
    lv_obj_set_style_text_font(textLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(textLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(textLabel, 163, LV_PART_MAIN);
    lv_obj_align(textLabel, LV_ALIGN_CENTER, 0, 0);
    return textLabelContainer;
}



static void GuiShowSolTxMultiSigCreateDetail(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    PtrT_DisplaySolanaTxOverviewSquadsV4MultisigCreate squadsMultisigCreate = overviewData->squads_multisig_create;
    PtrString wallet_desc = squadsMultisigCreate->wallet_desc;
    PtrString wallet_name = squadsMultisigCreate->wallet_name;

    uint16_t threshold = squadsMultisigCreate->threshold;
    uintptr_t member_count = squadsMultisigCreate->member_count;

    PtrT_VecFFI_PtrString members = squadsMultisigCreate->members;
    PtrString total_value = squadsMultisigCreate->total_value;
    PtrT_VecFFI_ProgramOverviewTransfer transfers = squadsMultisigCreate->transfers;
    lv_obj_t *walletNameContainer = GuiCreateAutoHeightContainer(parent, 408, 16);
    lv_obj_t *walletNameLabel = GuiCreateTextLabel(walletNameContainer, "Wallet Name");
    SetTitleLabelStyle(walletNameLabel);
    lv_obj_align(walletNameLabel, LV_ALIGN_TOP_LEFT, 24, 0);

    lv_obj_t *walletNameValue = GuiCreateIllustrateLabel(walletNameContainer, wallet_name);
    lv_obj_set_style_text_color(walletNameValue, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align_to(walletNameValue, walletNameLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t *totalValueContainer =  GuiCreateAutoHeightContainer(parent, 408, 16);
    lv_obj_t *totalValueLabel = lv_label_create(totalValueContainer);
    lv_label_set_text(totalValueLabel, "Transaction Total");
    lv_obj_align(totalValueLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(totalValueLabel);

    lv_obj_t *totalValueValue = lv_label_create(totalValueContainer);
    lv_label_set_text(totalValueValue, total_value);
    lv_obj_set_style_text_font(totalValueValue, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(totalValueValue, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(totalValueValue, totalValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    lv_obj_t *descriptionLabel = GuiCreateIllustrateLabel(totalValueContainer, _("solana_squads_amount_lm"));
    lv_obj_set_style_text_color(descriptionLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_width(descriptionLabel, 360);
    lv_label_set_long_mode(descriptionLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(descriptionLabel, totalValueValue, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t *learnMoreLabel = GuiCreateTextLabel(totalValueContainer, "Learn More");
    lv_obj_add_flag(learnMoreLabel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_color(learnMoreLabel, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align_to(learnMoreLabel, descriptionLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    static SolanaLearnMoreData_t solanaLearnMoredata;
    solanaLearnMoredata.title = "Amount Details";
    solanaLearnMoredata.content =  _("solana_squads_amount_desc");

    lv_obj_add_event_cb(learnMoreLabel, learn_more_click_event_handler, LV_EVENT_CLICKED, &solanaLearnMoredata);

    lv_obj_t *memberContainer = GuiCreateAutoHeightContainer(parent, 408, 16);
    lv_obj_set_style_border_width(memberContainer, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(memberContainer, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < members->size; i++) {
        lv_obj_t *memberItem = lv_obj_create(memberContainer);
        lv_obj_set_style_border_width(memberItem, 0, LV_PART_MAIN);
        lv_obj_set_width(memberItem, lv_pct(90));
        lv_obj_set_height(memberItem, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(memberItem, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_bg_opa(memberItem, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_pad_all(memberItem, 0, LV_PART_MAIN); // remove all default padding
        lv_obj_set_style_pad_left(memberItem, 24, LV_PART_MAIN);
        lv_obj_set_style_pad_top(memberItem, 16, LV_PART_MAIN);
        lv_obj_add_flag(memberItem, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *memberItemLabel = lv_label_create(memberItem);
        char memberText[64];
        snprintf(memberText, sizeof(memberText), "Member %d", i + 1);
        if (i == 0) {
            lv_obj_set_width(memberItemLabel, lv_pct(100));
            lv_obj_t *ownerLabel = GuiCreateSolanaTextLabel(memberItemLabel, "Owner");
            lv_obj_align(ownerLabel, LV_ALIGN_LEFT_MID, 110, 0);
        }
        lv_label_set_text(memberItemLabel, memberText);
        lv_obj_set_style_text_color(memberItemLabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
        lv_obj_t *memberItemValue = lv_label_create(memberItem);
        lv_label_set_text(memberItemValue, members->data[i]);
        lv_obj_set_width(memberItemValue, lv_pct(100));
        lv_label_set_long_mode(memberItemValue, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_color(memberItemValue, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_font(memberItemValue, g_defIllustrateFont, LV_PART_MAIN);
    }
    lv_obj_t *thresholdContainer =  GuiCreateAutoHeightContainer(parent, 408, 16);
    lv_obj_t *thresholdLabel = lv_label_create(thresholdContainer);
    lv_label_set_text(thresholdLabel, "Threshold / Member");
    lv_obj_set_style_text_color(thresholdLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align(thresholdLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(thresholdLabel);

    lv_obj_t *thresholdLabelValue = lv_label_create(thresholdContainer);
    char thresholdText[64];
    snprintf(thresholdText, sizeof(thresholdText), "%d/%d", threshold, member_count);
    lv_label_set_text(thresholdLabelValue, thresholdText);
    lv_obj_set_style_text_color(thresholdLabelValue, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(thresholdLabelValue, thresholdLabel, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_t *firstTransferCardContainer = GuiCreateAutoHeightContainer(parent, 408, 16);
    CreateSquadsSolanaTransferOverviewCard(firstTransferCardContainer,  transfers->data[1].from, transfers->data[1].to, transfers->data[1].value, "");

    lv_obj_t *secondTransferCardContainer = GuiCreateAutoHeightContainer(parent, 408, 16);
    CreateSquadsSolanaTransferOverviewCard(secondTransferCardContainer,  transfers->data[0].from, transfers->data[0].to, transfers->data[0].value, "");
    lv_obj_align(walletNameContainer, LV_ALIGN_TOP_LEFT, 0, 0);
    if (strcmp(wallet_desc, "") != 0) {
        lv_obj_t *walletDescContainer =  GuiCreateAutoHeightContainer(parent, 408, 16);
        lv_obj_t *walletDescLabel = lv_label_create(walletDescContainer);
        lv_label_set_text(walletDescLabel, "Description");
        lv_obj_align(walletDescLabel, LV_ALIGN_TOP_LEFT, 24, 0);
        SetTitleLabelStyle(walletDescLabel);

        lv_obj_t *walletDescValue = lv_label_create(walletDescContainer);
        lv_label_set_text(walletDescValue, wallet_desc);
        lv_obj_set_style_text_font(walletDescValue, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(walletDescValue, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_width(walletDescValue, lv_pct(100));
        lv_label_set_long_mode(walletDescValue, LV_LABEL_LONG_WRAP);
        lv_obj_align_to(walletDescValue, walletDescLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

        lv_obj_align_to(walletDescContainer, walletNameContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
        lv_obj_align_to(memberContainer, walletDescContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    } else {
        lv_obj_align_to(memberContainer, walletNameContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    }

    lv_obj_align_to(thresholdContainer, memberContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_align_to(totalValueContainer, thresholdContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_align_to(firstTransferCardContainer, totalValueContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_align_to(secondTransferCardContainer, firstTransferCardContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
}

static void GuiShowSolTxMultiSigCreateOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    PtrT_DisplaySolanaTxOverviewSquadsV4MultisigCreate squadsMultisigCreate = overviewData->squads_multisig_create;

    lv_obj_t *thresholdContainer =  GuiCreateAutoHeightContainer(parent, 408, 16);
    lv_obj_t *thresholdLabel = GuiCreateTextLabel(thresholdContainer, "Threshold / Member");
    lv_obj_set_style_text_color(thresholdLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align(thresholdLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(thresholdLabel);

    uint16_t threshold = squadsMultisigCreate->threshold;
    uintptr_t member_count = squadsMultisigCreate->member_count;
    lv_obj_t *thresholdLabelValue = lv_label_create(thresholdContainer);
    char thresholdText[64];
    snprintf(thresholdText, sizeof(thresholdText), "%d/%d", threshold, member_count);
    lv_label_set_text(thresholdLabelValue, thresholdText);
    lv_obj_set_style_text_color(thresholdLabelValue, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(thresholdLabelValue, thresholdLabel, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    PtrString total_value = squadsMultisigCreate->total_value;
    lv_obj_t *totalValueContainer =  GuiCreateAutoHeightContainer(parent, 408, 16);
    lv_obj_t *totalValueLabel = GuiCreateTextLabel(totalValueContainer, "Value");
    lv_obj_align(totalValueLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    SetTitleLabelStyle(totalValueLabel);

    lv_obj_t *totalValueValue = lv_label_create(totalValueContainer);
    lv_label_set_text(totalValueValue, total_value);
    lv_obj_set_style_text_font(totalValueValue, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(totalValueValue, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(totalValueValue, totalValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    lv_obj_t *descriptionLabel = GuiCreateIllustrateLabel(totalValueContainer, _("solana_squads_amount_brief"));
    lv_obj_set_style_text_color(descriptionLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_width(descriptionLabel, 360);
    lv_label_set_long_mode(descriptionLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(descriptionLabel, totalValueValue, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t *firstTransferCardContainer = GuiCreateAutoHeightContainer(parent, 408, 16);
    PtrT_VecFFI_ProgramOverviewTransfer transfers = squadsMultisigCreate->transfers;

    lv_obj_t * tolabel = lv_label_create(firstTransferCardContainer);
    lv_label_set_text(tolabel, "To");
    SetTitleLabelStyle(tolabel);
    lv_obj_align(tolabel, LV_ALIGN_TOP_LEFT, 24, 0);
    lv_obj_t * toValuelabel = lv_label_create(firstTransferCardContainer);
    lv_label_set_text(toValuelabel, transfers->data[1].to);
    lv_label_set_long_mode(toValuelabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(toValuelabel, 306);
    lv_obj_align_to(toValuelabel, tolabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    SetContentLableStyle(toValuelabel);

    if (strcmp(transfers->data[1].to, "5DH2e3cJmFpyi6mk65EGFediunm4ui6BiKNUNrhWtD1b") == 0) {
        lv_obj_t *squadsIcon = GuiCreateImg(firstTransferCardContainer, &imgSquads);
        lv_obj_align_to(squadsIcon, toValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
        lv_obj_t *squadsLabel = lv_label_create(firstTransferCardContainer);
        lv_label_set_text(squadsLabel, "Squads");
        lv_obj_set_style_text_font(squadsLabel, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(squadsLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
        lv_obj_align_to(squadsLabel, squadsIcon, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

        lv_obj_align(thresholdContainer, LV_ALIGN_TOP_LEFT, 0, 0);
    } else {
        lv_obj_t * waringCard = GuiCreateWarningCard(parent);
        lv_obj_align(waringCard, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_align_to(thresholdContainer, waringCard, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

        lv_obj_t * unkonwnAddressLabel = lv_label_create(firstTransferCardContainer);
        lv_label_set_text(unkonwnAddressLabel, "Unknown Address");
        lv_obj_set_style_text_color(unkonwnAddressLabel, lv_color_hex(0xF55831), LV_PART_MAIN);
        lv_obj_set_style_bg_color(unkonwnAddressLabel, lv_color_hex(0xF5583133), LV_PART_MAIN);
        lv_obj_set_style_radius(unkonwnAddressLabel, 12, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(unkonwnAddressLabel, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_pad_left(unkonwnAddressLabel, 12, LV_PART_MAIN);
        lv_obj_set_style_pad_top(unkonwnAddressLabel, 8, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(unkonwnAddressLabel, 8, LV_PART_MAIN);
        lv_obj_set_size(unkonwnAddressLabel, lv_pct(90), LV_SIZE_CONTENT);
        lv_obj_align_to(unkonwnAddressLabel, toValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    }


    lv_obj_align_to(totalValueContainer, thresholdContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_align_to(firstTransferCardContainer, totalValueContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
}

static bool CheckIsAddressLookupTableAccount(const char* account)
{
    // if address is containe "#" then it is a AddressLookupTable account
    return strchr(account, '#') != NULL;
}

static void GuiShowSolTxInstructionsOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    PtrT_DisplaySolanaTxOverviewUnknownInstructions unknown_instructions = overviewData->unknown_instructions;
    PtrT_VecFFI_Instruction overview_instructions = unknown_instructions->overview_instructions;
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
    for (int i = 0; i < overview_instructions->size; i++) {
        // container will auto adjust height
        lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, LV_SIZE_CONTENT);
        lv_obj_align(container, LV_ALIGN_DEFAULT, 0, containerYOffset);
        SetContainerDefaultStyle(container);

        lv_obj_t *orderLabel = lv_label_create(container);
        char order[BUFFER_SIZE_16] = {0};
        snprintf_s(order, BUFFER_SIZE_16, "Instruction#%d", i + 1);
        lv_label_set_text(orderLabel, order);
        lv_obj_set_style_text_font(orderLabel, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(orderLabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
        lv_obj_align(orderLabel, LV_ALIGN_TOP_LEFT, 24, 16);

        PtrT_VecFFI_PtrString accounts = overview_instructions->data[i].accounts;
        // accounts label
        lv_obj_t *accounts_label = NULL;
        if (accounts->size != 0) {
            accounts_label = lv_label_create(container);
            lv_label_set_text(accounts_label, "accounts");
            lv_obj_set_style_text_color(accounts_label, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_set_style_text_opa(accounts_label, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_align(accounts_label, LV_ALIGN_TOP_LEFT, 24, 44);
        }

        lv_obj_t *accounts_cont = NULL;
        for (int j = 0; j < accounts->size; j++) {
            if (accounts_cont == NULL) {
                accounts_cont = lv_obj_create(container);
                lv_obj_set_width(accounts_cont, lv_pct(88));
                lv_obj_set_flex_flow(accounts_cont, LV_FLEX_FLOW_COLUMN);
                lv_obj_set_flex_align(accounts_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
                lv_obj_align_to(accounts_cont, accounts_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
                lv_obj_set_style_bg_opa(accounts_cont, LV_OPA_TRANSP, LV_PART_MAIN);
                lv_obj_set_style_pad_left(accounts_cont, 0, LV_PART_MAIN);  // remove left padding
                lv_obj_set_style_pad_right(accounts_cont, 0, LV_PART_MAIN);  // remove right padding
                lv_obj_add_flag(accounts_cont, LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_set_style_border_width(accounts_cont, 0, LV_PART_MAIN);
                lv_obj_set_height(accounts_cont, accounts->size == 2 ? 150 : 200);
            }
            char order[BUFFER_SIZE_16] = {0};
            snprintf_s(order, BUFFER_SIZE_16, "%d", j + 1);
            lv_obj_t *orderLabel = GuiCreateNoticeLabel(accounts_cont, order);
            lv_obj_t *account_label = GuiCreateIllustrateLabel(accounts_cont, accounts->data[j]);
            if (CheckIsAddressLookupTableAccount(accounts->data[j])) {
                lv_label_set_text(account_label, accounts->data[j] + 6);
                lv_obj_t *info_icon = GuiCreateImg(accounts_cont, &imgInfoSmall);
                lv_obj_set_style_pad_right(info_icon, 0, LV_PART_MAIN);
                lv_obj_add_flag(info_icon, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_set_user_data(info_icon, accounts->data[j]);
                lv_obj_add_event_cb(info_icon, SolanaAddressLearnMore, LV_EVENT_CLICKED, NULL);
            } else {
                lv_label_set_text(account_label, accounts->data[j]);
            }
            lv_obj_set_style_text_color(account_label, WHITE_COLOR, LV_PART_MAIN);
            lv_label_set_long_mode(account_label, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_border_width(account_label, 0, LV_PART_MAIN);
        }
        char buff[BUFFER_SIZE_128] = {0};
        snprintf_s(buff, BUFFER_SIZE_128, "#4b4b4b data# %s", overview_instructions->data[i].data);
        lv_obj_t *data_label = GuiCreateIllustrateLabel(container, buff);
        lv_label_set_recolor(data_label, true);
        // lv_obj_align_to(data_label, orderLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
        lv_obj_align_to(data_label, lv_obj_get_child(lv_obj_get_parent(data_label),
                        lv_obj_get_child_cnt(lv_obj_get_parent(data_label)) - 2), LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

        // program address label
        snprintf_s(buff, BUFFER_SIZE_128, "#4b4b4b programAddress# %s", overview_instructions->data[i].program_address);
        lv_obj_t *programAddress_label = GuiCreateIllustrateLabel(container, buff);
        lv_label_set_recolor(programAddress_label, true);
        GuiAlignToPrevObj(programAddress_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

        // calculate the height of the container
        lv_obj_update_layout(container);
        containerYOffset += lv_obj_get_height(container) + 16; // set padding between containers
    }
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
    } else if (0 == strcmp(overviewData->display_type, "squads_multisig_create")) {
        GuiShowSolTxMultiSigCreateOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "TokenTransfer")) {
        GuiShowSplTokenTransferOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "squads_proposal")) {
        GuiShowSolTxSquadsProposalOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "jupiterv6_swap")) {
        // todo add jupiterv6 swap overview
        GuiShowJupiterV6SwapOverview(parent, overviewData);
    } else {
        GuiShowSolTxInstructionsOverview(parent, overviewData);
    }
}

void GuiShowSolTxDetail(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, LV_SIZE_CONTENT);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    DisplaySolanaTx *txData = (DisplaySolanaTx*)totalData;
    PtrT_DisplaySolanaTxOverview overviewData = txData->overview;
    if (0 == strcmp(overviewData->display_type, "squads_multisig_create")) {
        GuiShowSolTxMultiSigCreateDetail(parent, overviewData);
        return ;
    }
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

    PtrString txDetail = txData->detail;

    lv_obj_t *label = lv_label_create(cont);
    cJSON *root = cJSON_Parse((const char *)txDetail);
    char *retStr = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    lv_label_set_text(label, retStr);
    EXT_FREE(retStr);
    cJSON_Delete(root);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, 360);
    SetTitleLabelStyle(label);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
}
