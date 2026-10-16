#include "gui_analyze.h"
#include "rust.h"
#include "account_public_info.h"
#include "gui_chain_components.h"
#include "keystore.h"
#include "gui_chain.h"
#include "version.h"
#include "secret_cache.h"
#include "screen_manager.h"
#include "account_manager.h"
#include "assert.h"
#include "gui_qr_hintbox.h"

#define SQUADS_V4_CREATE_MULTISIG_CONTRACT_ADDRESS "5DH2e3cJmFpyi6mk65EGFediunm4ui6BiKNUNrhWtD1b"
#define SOL_COMPONENT_WIDTH 376
#define SOL_COMPONENT_CONTENT_WIDTH (SOL_COMPONENT_WIDTH - 48)
#define SOL_CONFIG_REVIEW_MAX_HEIGHT 420
typedef struct SolanaLearnMoreData {
    PtrString title;
    PtrString content;
} SolanaLearnMoreData_t;
static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

static ViewType g_viewType = ViewTypeUnKnown;
static void SolanaAddressLearnMore(lv_event_t *e);
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
    if (g_parseResult == NULL) {
        return NULL;
    }
    if (g_viewType == SolanaTx) {
        PtrT_TransactionParseResult_DisplaySolanaTx result = g_parseResult;
        if (result->error_code != 0 || result->data == NULL) {
            return NULL;
        }
    } else if (g_viewType == SolanaMessage) {
        PtrT_TransactionParseResult_DisplaySolanaMessage result = g_parseResult;
        if (result->error_code != 0 || result->data == NULL) {
            return NULL;
        }
    } else {
        return NULL;
    }
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    return SignInternal(solana_sign_checked_tx, data);
}

void *GuiGetSolData(void)
{
    CHECK_FREE_PARSE_SOL_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        char *path = sol_get_path(data);
        ChainType pubkeyIndex = CheckSolPathSupport(path);
        PtrT_TransactionParseResult_DisplaySolanaTx parseResult =
            pubkeyIndex == XPUB_TYPE_NUM ? solana_parse_tx(data) :
            solana_parse_tx_with_pubkey(data, GetCurrentAccountPublicKey(pubkeyIndex));
        free_ptr_string(path);
        if (parseResult->error_code != 0) {
            CHECK_CHAIN_PRINT(parseResult);
            free_TransactionParseResult_DisplaySolanaTx(parseResult);
            break;
        }
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetSolCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    PtrT_TransactionCheckResult result = solana_check(data, mfp, sizeof(mfp));
    if (result == NULL || result->error_code != 0 || g_viewType != SolanaTx) {
        return result;
    }
    char *path = sol_get_path(data);
    ChainType pubkeyIndex = CheckSolPathSupport(path);
    free_ptr_string(path);
    if (pubkeyIndex == XPUB_TYPE_NUM) {
        return result;
    }
    free_TransactionCheckResult(result);
    return solana_check_tx_signer(data, GetCurrentAccountPublicKey(pubkeyIndex));
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
        char *pubKey = pubkeyIndex == XPUB_TYPE_NUM ? "" :
                       GetCurrentAccountPublicKey(pubkeyIndex);
        PtrT_TransactionParseResult_DisplaySolanaMessage parseResult = solana_parse_message(data, pubKey);
        free_ptr_string(path);
        if (parseResult->error_code != 0) {
            CHECK_CHAIN_PRINT(parseResult);
            free_TransactionParseResult_DisplaySolanaMessage(parseResult);
            break;
        }
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
    if (indata == NULL || maxLen == 0) {
        return;
    }
    if (message == NULL || message->from == NULL) {
        ((char *)indata)[0] = '\0';
        return;
    }
    if (strlen(message->from) >= maxLen) {
        if (maxLen <= 4) {
            snprintf_s((char *)indata, maxLen, "%.*s", (int)(maxLen - 1), "...");
        } else {
            snprintf_s((char *)indata, maxLen, "%.*s...", (int)(maxLen - 4), message->from);
        }
    } else {
        strcpy_s((char *)indata, maxLen, message->from);
    }
}

void GetSolMessageUtf8(void *indata, void *param, uint32_t maxLen)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    snprintf_s((char *)indata, maxLen, "%.*s", (int)(maxLen - 1), message->utf8_message);
}

void GetSolMessageRaw(void *indata, void *param, uint32_t maxLen)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    snprintf_s((char *)indata, maxLen, "%.*s", (int)(maxLen - 1), message->raw_message);
}

static lv_obj_t *GuiShowSolSigningPath(lv_obj_t *parent)
{
    void *data = g_isMulti ? (g_urMultiResult == NULL ? NULL : g_urMultiResult->data) :
                 (g_urResult == NULL ? NULL : g_urResult->data);
    char *path = data == NULL ? NULL : sol_get_path(data);
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_remove_style_all(container);
    lv_obj_update_layout(parent);
    lv_obj_set_size(container, lv_obj_get_width(parent), LV_SIZE_CONTENT);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_t *label = GuiCreateIllustrateLabel(container, "");
    lv_obj_set_width(label, lv_obj_get_width(parent));
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_height(label, LV_SIZE_CONTENT);
    if (path != NULL) {
        lv_label_set_text_fmt(label, "%s\nM/%s", _("Path"), path);
        free_ptr_string(path);
    } else {
        lv_label_set_text_fmt(label, "%s\n-", _("Path"));
    }
    lv_obj_update_layout(container);
    return container;
}

void GuiShowSolMessagePaged(lv_obj_t *parent, void *param, bool raw)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    GuiShowPagedMessageText(
        parent, raw ? message->raw_message : message->utf8_message, !raw,
        raw ? _("solana_blind_sign_title") : NULL,
        raw ? _("solana_unparsed_message_warning") : NULL);
}

static lv_obj_t *GuiSolMessageField(lv_obj_t *parent, const char *text, bool title)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, text);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_height(label, LV_SIZE_CONTENT);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    if (title) lv_obj_set_style_text_opa(label, 144, LV_PART_MAIN);
    return label;
}

void GuiShowSolMessageReview(lv_obj_t *parent, void *param)
{
    DisplaySolanaMessage *message = (DisplaySolanaMessage *)param;
    bool raw = message->utf8_message == NULL;
    lv_obj_set_size(parent, 408, 500);
    lv_obj_set_style_pad_left(parent, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_right(parent, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_top(parent, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(parent, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    if (message->from != NULL) {
        GuiSolMessageField(parent, _("From"), true);
        lv_obj_t *address = GuiSolMessageField(parent, message->from, false);
        lv_obj_set_style_text_font(address, &openSansEnIllustrate, LV_PART_MAIN);
    } else {
        lv_obj_t *notice = GuiCreateContainerWithParent(parent, 360, LV_SIZE_CONTENT);
        CreateNoticeCard(notice, _("custom_path_parse_notice"));
    }
    GuiSolMessageField(parent, _("Path"), true);
    void *data = g_isMulti ? (g_urMultiResult == NULL ? NULL : g_urMultiResult->data) :
                 (g_urResult == NULL ? NULL : g_urResult->data);
    char *path = data == NULL ? NULL : sol_get_path(data);
    lv_obj_t *pathValue = GuiSolMessageField(parent, "", false);
    lv_obj_set_style_text_font(pathValue, &openSansEnIllustrate, LV_PART_MAIN);
    lv_label_set_text_fmt(pathValue, "M/%s", path == NULL ? "-" : path);
    if (path != NULL) free_ptr_string(path);
    GuiSolMessageField(parent, raw ? _("Raw Message") : _("Message"), true);
    lv_obj_t *pager = GuiCreateContainerWithParent(parent, 360, 0);
    lv_obj_set_style_bg_opa(pager, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_grow(pager, 1);
    lv_obj_update_layout(parent);
    GuiShowSolMessagePaged(pager, param, raw);
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
    lv_obj_t * container = GuiCreateContainerWithParent(parent, width, LV_SIZE_CONTENT);
    GuiSetTransactionContainerStyle(container);
    lv_obj_set_style_pad_all(container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(container, padding_x, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(container, padding_x, LV_PART_MAIN);
    return container;
}

lv_obj_t* GuiCreateWarningCard(lv_obj_t* parent)
{
    lv_obj_t* card = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 24);
    GuiSetTransactionContainerStyle(card);
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

static lv_obj_t *GuiCreateAltWarningCard(lv_obj_t *parent, const char *reference)
{
    lv_obj_t *card = GuiCreateWarningCard(parent);
    lv_obj_t *title = lv_obj_get_child(card, 1);
    lv_obj_t *content = lv_obj_get_child(card, 2);
    if (title != NULL) {
        lv_label_set_text(title, _("solana_alt_address_title"));
    }
    if (content != NULL) {
        lv_label_set_text(content, _("solana_alt_unresolved_notice"));
    }

    lv_obj_t *checkLabel = lv_label_create(card);
    lv_label_set_text(checkLabel, _("solana_alt_check"));
    lv_obj_set_style_text_font(checkLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(checkLabel, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align_to(checkLabel, content != NULL ? content : title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    lv_obj_t *checkIcon = GuiCreateImg(card, &imgQrcodeTurquoise);
    lv_obj_align_to(checkIcon, checkLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    lv_obj_add_flag(checkIcon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(checkIcon, (void *)reference);
    lv_obj_add_event_cb(checkIcon, SolanaAddressLearnMore, LV_EVENT_CLICKED, NULL);
    return card;
}


lv_obj_t * CreateSolanaSquadsProposalOverviewCard(lv_obj_t *parent, PtrString program, PtrString method, PtrString memo, PtrString data)
{
    lv_obj_t *container = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, LV_PART_MAIN);
    GuiSetTransactionContainerStyle(container);
    lv_obj_t *programLabel = GuiCreateTextLabel(container, "Program");
    lv_obj_align(programLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    GuiSetTransactionTitleStyle(programLabel);

    lv_obj_t *squadsIcon = GuiCreateImg(container, &imgSquads);
    lv_obj_align_to(squadsIcon, programLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t *squadsLabel = GuiCreateIllustrateLabel(container, program);
    lv_obj_set_style_text_color(squadsLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
    lv_obj_align_to(squadsLabel, squadsIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t *methodLabel = GuiCreateTextLabel(container, "Method");
    GuiSetTransactionTitleStyle(methodLabel);
    lv_obj_align_to(methodLabel, squadsIcon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t *methodValueLabel = GuiCreateIllustrateLabel(container, method);
    lv_obj_set_style_text_color(methodValueLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(methodValueLabel, methodLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    if (strlen(memo) > 0) {
        lv_obj_t *memoLabel = lv_label_create(container);
        lv_label_set_text(memoLabel, "Memo");
        GuiSetTransactionTitleStyle(memoLabel);
        lv_obj_align_to(memoLabel, methodValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

        lv_obj_t *memoValueLabel = GuiCreateIllustrateLabel(container, memo);
        // long text unwrap
        lv_label_set_long_mode(memoValueLabel, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(memoValueLabel, lv_pct(90));
        lv_obj_set_style_text_color(memoValueLabel, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(memoValueLabel, memoLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    }

    if (strlen(data) > 0) {
        lv_obj_t *dataLabel = GuiCreateTextLabel(container, _("solana_advanced_data"));
        GuiSetTransactionTitleStyle(dataLabel);
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

    const char *toTitle = strcmp(to, SQUADS_V4_CREATE_MULTISIG_CONTRACT_ADDRESS) == 0 ? "Platform Fee" : "Account Deposit";
    lv_obj_t *label = GuiCreateNoticeLabel(container, toTitle);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 0);

    lv_obj_t * amountlabel = lv_label_create(container);
    lv_label_set_text(amountlabel, amount);
    lv_obj_set_style_text_font(amountlabel, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(amountlabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(amountlabel, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * fromlabel = GuiCreateTextLabel(container, "From");
    lv_obj_align_to(fromlabel, amountlabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    GuiSetTransactionTitleStyle(fromlabel);

    lv_obj_t * fromValuelabel = GuiCreateIllustrateLabel(container, from);
    lv_obj_set_width(fromValuelabel, 306);
    lv_obj_align_to(fromValuelabel, fromlabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * tolabel = lv_label_create(container);
    lv_label_set_text(tolabel, "To");
    GuiSetTransactionTitleStyle(tolabel);
    lv_obj_align_to(tolabel, fromValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_t * toValuelabel = GuiCreateIllustrateLabel(container, to);
    lv_obj_set_width(toValuelabel, 306);
    lv_obj_align_to(toValuelabel, tolabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    // add label fot to label
    if (strcmp(to, SQUADS_V4_CREATE_MULTISIG_CONTRACT_ADDRESS) == 0) {
        lv_obj_t *squadsIcon = GuiCreateImg(container, &imgSquads);
        lv_obj_align_to(squadsIcon, toValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
        lv_obj_t *squadsLabel = GuiCreateIllustrateLabel(container, "Squads");
        lv_obj_set_style_text_color(squadsLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
        lv_obj_align_to(squadsLabel, squadsIcon, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    }

    if (strlen(note) > 0) {
        lv_obj_t * notelabel = GuiCreateTextLabel(container, "Note");
        GuiSetTransactionTitleStyle(notelabel);
        lv_obj_align_to(notelabel, toValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        lv_obj_t * noteValuelabel = GuiCreateIllustrateLabel(container, note);
        lv_obj_set_width(noteValuelabel, 306);
        lv_obj_align_to(noteValuelabel, notelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    }
}
static void GuiShowSolTxTransferOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *lastView = NULL;
    if (overviewData->transfer_to_in_lookup_table) {
        lastView = GuiCreateAltWarningCard(parent, overviewData->transfer_to_lookup_table_reference);
    }
    lv_obj_t *overviewCard = CreateTransactionOverviewCardWithWidth(
        parent,
        _("Value"),
        overviewData->transfer_value,
        _("Main Action"),
        overviewData->main_action,
        SOL_COMPONENT_WIDTH);
    if (lastView != NULL) {
        lv_obj_align_to(overviewCard, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }
    lastView = overviewCard;
    lastView = CreateTransactionItemViewWithWidth(
        parent,
        _("From"),
        overviewData->transfer_from,
        lastView,
        SOL_COMPONENT_WIDTH);
    CreateTransactionItemViewWithWidth(
        parent,
        overviewData->transfer_to_in_lookup_table ? _("solana_unresolved_destination") : _("To"),
        overviewData->transfer_to,
        lastView,
        SOL_COMPONENT_WIDTH);
}


lv_obj_t* GuiCreateSolNoticeCard(lv_obj_t* parent)
{
    lv_obj_t* card = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 24);
    GuiSetTransactionContainerStyle(card);
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

static lv_obj_t *GuiCreateUnusualDecimalsWarningCard(lv_obj_t *parent, uint8_t decimals)
{
    lv_obj_t *card = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 24);
    GuiSetTransactionContainerStyle(card);
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN);

    lv_obj_t *title = GuiCreateTextLabel(card, "Unusual Token Precision");
    lv_obj_set_style_text_color(title, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 24, 0);

    char description[160] = {0};
    snprintf_s(
        description,
        sizeof(description),
        "This unknown token declares %u decimals. Verify the mint address and raw amount before signing.",
        decimals);
    lv_obj_t *content = GuiCreateIllustrateLabel(card, description);
    lv_obj_set_width(content, SOL_COMPONENT_CONTENT_WIDTH);
    lv_obj_align_to(content, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    return card;
}


void SolanaSplTokenAddressLearnMore(lv_event_t *e)
{
    const char* address = (const char*)lv_event_get_user_data(e);
    if (address != NULL) {
        char url[512];
        snprintf_s(url, sizeof(url), "https://solscan.io/token/%s", address);
        GuiQRCodeHintBoxOpenBig(url, "Scan to double-check the Token account", "", url);
    }
}

static lv_obj_t * GuiShowSplTokenInfoOverviewCard(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *container = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    GuiSetTransactionContainerStyle(container);
    PtrT_DisplaySolanaTxSplTokenTransferOverview splTokenInfo = overviewData->spl_token_transfer;

    lv_obj_t *tokenNameLabel = lv_label_create(container);
    lv_label_set_text(tokenNameLabel, "Token Name");
    lv_obj_align(tokenNameLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    GuiSetTransactionTitleStyle(tokenNameLabel);

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
        lv_obj_set_style_text_font(tokenNameValueLabel, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(tokenNameValueLabel, WHITE_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(tokenNameValueLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
    }

    lv_obj_set_style_text_font(tokenNameValueLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_align_to(tokenNameContainer, tokenNameLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t *tokenMintLabel = lv_label_create(container);
    lv_label_set_text(tokenMintLabel, "Token Account");
    GuiSetTransactionTitleStyle(tokenMintLabel);

    lv_obj_align_to(tokenMintLabel, tokenNameContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t *tokenMintValueLabel = GuiCreateIllustrateLabel(container, splTokenInfo->token_mint_account);
    lv_obj_set_width(tokenMintValueLabel, SOL_COMPONENT_CONTENT_WIDTH);
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
        lv_obj_t * noticeBar = GuiCreateAutoHeightContainer(container, SOL_COMPONENT_CONTENT_WIDTH, 8);
        lv_obj_set_width(noticeBar, SOL_COMPONENT_CONTENT_WIDTH);
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
static lv_obj_t *GuiShowJupiterV6SwapOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *swapOverviewContainer = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    GuiSetTransactionContainerStyle(swapOverviewContainer);
    PtrT_DisplaySolanaTxOverviewJupiterV6Swap jupiterV6SwapOverview = overviewData->jupiter_v6_swap;
    lv_obj_t * swapLabel = lv_label_create(swapOverviewContainer);
    lv_label_set_text(swapLabel, "Swap");
    lv_obj_align(swapLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    GuiSetTransactionTitleStyle(swapLabel);
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
    GuiSetTransactionTitleStyle(toLabel);
    // if current instruction name is "JupiterV6Route" we dont need to show the mint account info
    if (strcmp(jupiterV6SwapOverview->instruction_name, "JupiterV6Route") == 0) {
        lv_obj_align_to(toLabel, amountValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    } else {
        // mint account label
        lv_obj_t * mintAccountLabel = lv_label_create(swapOverviewContainer);
        lv_label_set_text(mintAccountLabel, "Mint Account");
        GuiSetTransactionTitleStyle(mintAccountLabel);
        // mint account value
        lv_obj_t * mintAccountValueLabel = GuiCreateIllustrateLabel(swapOverviewContainer, jupiterV6SwapOverview->token_a_overview->token_address);
        lv_obj_set_width(mintAccountValueLabel, lv_pct(90));

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
    GuiSetTransactionTitleStyle(tokenBmintAccountLabel);

    lv_obj_t * tokenBmintAccountValueLabel = GuiCreateIllustrateLabel(swapOverviewContainer, jupiterV6SwapOverview->token_b_overview->token_address);
    lv_obj_align_to(tokenBmintAccountValueLabel, tokenBmintAccountLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_set_width(tokenBmintAccountValueLabel, lv_pct(90));

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
    lv_obj_t * platformOverviewContainer = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    GuiSetTransactionContainerStyle(platformOverviewContainer);
    // swap platform label
    lv_obj_t * swapPlatformLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(swapPlatformLabel, "Swap Platform");
    lv_obj_align(swapPlatformLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    GuiSetTransactionTitleStyle(swapPlatformLabel);
    // platform icon label
    lv_obj_t * jupiterIcon = GuiCreateImg(platformOverviewContainer, &imgJupiter);
    lv_obj_align_to(jupiterIcon, swapPlatformLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t *jupiterLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(jupiterLabel, "Jupiter Aggregator v6");
    lv_obj_set_style_text_font(jupiterLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(jupiterLabel, lv_color_hex(0xA485FF), LV_PART_MAIN);
    lv_obj_align_to(jupiterLabel, jupiterIcon, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    // platform address label
    lv_obj_t * platformAddressLabel = GuiCreateIllustrateLabel(platformOverviewContainer, jupiterV6SwapOverview->program_address);
    lv_obj_align_to(platformAddressLabel, jupiterIcon, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_set_width(platformAddressLabel, lv_pct(90));

    // Slippage Tolerance label
    lv_obj_t * slippageToleranceLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(slippageToleranceLabel, "Slippage Tolerance");
    lv_obj_align_to(slippageToleranceLabel, platformAddressLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    GuiSetTransactionTitleStyle(slippageToleranceLabel);
    // slippage value
    lv_obj_t * slippageValueLabel = GuiCreateIllustrateLabel(platformOverviewContainer, jupiterV6SwapOverview->slippage_bps);
    lv_obj_align_to(slippageValueLabel, slippageToleranceLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    // Partner Referral Fee
    lv_obj_t * partnerReferralFeeLabel = lv_label_create(platformOverviewContainer);
    lv_label_set_text(partnerReferralFeeLabel, "Partner Referral Fee");
    lv_obj_align_to(partnerReferralFeeLabel, slippageValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    GuiSetTransactionTitleStyle(partnerReferralFeeLabel);
    // partner referral fee value
    lv_obj_t * partnerReferralFeeValueLabel = GuiCreateIllustrateLabel(platformOverviewContainer, jupiterV6SwapOverview->platform_fee_bps);
    lv_obj_align_to(partnerReferralFeeValueLabel, partnerReferralFeeLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    // platform container align to swap container
    lv_obj_align_to(platformOverviewContainer, swapOverviewContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    return platformOverviewContainer;
}
static void GuiShowSplTokenTransferOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    PtrT_DisplaySolanaTxSplTokenTransferOverview splTokenTransfer = overviewData->spl_token_transfer;
    lv_obj_t *lastInfoCard = NULL;
    // Plain SPL Token `Transfer` instructions do not contain mint metadata.
    // In that case keep the original Amount/Account review card and its three
    // account explanations, but do not render an empty token-info card.
    if (strlen(splTokenTransfer->token_mint_account) > 0) {
        lv_obj_t *tokenInfoCard = GuiShowSplTokenInfoOverviewCard(parent, overviewData);
        lastInfoCard = tokenInfoCard;
        if (strcmp(splTokenTransfer->token_name, "Unknown") == 0) {
            lv_obj_t *noticeCard = GuiCreateSolNoticeCard(parent);
            lv_obj_align_to(tokenInfoCard, noticeCard, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
            lastInfoCard = tokenInfoCard;
        }
        if (splTokenTransfer->unusual_decimals) {
            lv_obj_t *warningCard =
                GuiCreateUnusualDecimalsWarningCard(parent, splTokenTransfer->decimals);
            lv_obj_align_to(warningCard, lastInfoCard, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
            lastInfoCard = warningCard;
        }
    }
    lv_obj_t *container = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    GuiSetTransactionContainerStyle(container);
    if (lastInfoCard != NULL) {
        lv_obj_align_to(container, lastInfoCard, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    }


    lv_obj_t *label = lv_label_create(container);
    lv_label_set_text(label, "Amount");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 0);
    GuiSetTransactionTitleStyle(label);

    lv_obj_t * amountlabel = lv_label_create(container);
    lv_label_set_text(amountlabel, splTokenTransfer->amount);
    lv_obj_set_style_text_color(amountlabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_set_style_text_font(amountlabel, GetOverviewAmountFont(splTokenTransfer->amount), LV_PART_MAIN);
    lv_obj_set_width(amountlabel, SOL_COMPONENT_CONTENT_WIDTH);
    lv_label_set_long_mode(amountlabel, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(amountlabel, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * authoritylabel = lv_label_create(container);
    lv_label_set_text(authoritylabel, "Authority Account");
    lv_obj_align_to(authoritylabel, amountlabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    GuiSetTransactionTitleStyle(authoritylabel);
    lv_obj_t *authInfoSIcon = GuiCreateImg(container, &imgInfoS);
    lv_obj_align_to(authInfoSIcon, authoritylabel, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    lv_obj_add_flag(authInfoSIcon, LV_OBJ_FLAG_CLICKABLE);
    static SolanaLearnMoreData_t authLearnMoredata;
    authLearnMoredata.title = "Authority Account";
    authLearnMoredata.content = _("solana_auth_account");
    lv_obj_add_event_cb(authInfoSIcon, learn_more_click_event_handler, LV_EVENT_CLICKED, &authLearnMoredata);

    lv_obj_t * authorityValueLabel = GuiCreateIllustrateLabel(container, splTokenTransfer->authority);
    lv_obj_set_width(authorityValueLabel, 306);
    lv_obj_align_to(authorityValueLabel, authoritylabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * sourcelabel = lv_label_create(container);
    lv_label_set_text(sourcelabel, "Source Account");
    lv_obj_align_to(sourcelabel, authorityValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    GuiSetTransactionTitleStyle(sourcelabel);
    lv_obj_t *sourceInfoSIcon = GuiCreateImg(container, &imgInfoS);
    lv_obj_align_to(sourceInfoSIcon, sourcelabel, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    lv_obj_add_flag(sourceInfoSIcon, LV_OBJ_FLAG_CLICKABLE);
    static SolanaLearnMoreData_t sourceLearnMoredata;
    sourceLearnMoredata.title = "Source Account";
    sourceLearnMoredata.content = _("solana_source_ata");
    lv_obj_add_event_cb(sourceInfoSIcon, learn_more_click_event_handler, LV_EVENT_CLICKED, &sourceLearnMoredata);

    lv_obj_t * sourceValuelabel = GuiCreateIllustrateLabel(container, splTokenTransfer->source);
    lv_obj_set_width(sourceValuelabel, 306);
    lv_obj_align_to(sourceValuelabel, sourcelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t * destLabel = lv_label_create(container);
    lv_label_set_text(destLabel, "Destination Account");
    GuiSetTransactionTitleStyle(destLabel);
    lv_obj_align_to(destLabel, sourceValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_t * destValuelabel = GuiCreateIllustrateLabel(container, splTokenTransfer->destination);
    lv_obj_set_width(destValuelabel, 306);
    lv_obj_align_to(destValuelabel, destLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

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
        lv_obj_t *feeContainer = GuiCreateAutoHeightContainer(
            parent, SOL_COMPONENT_WIDTH, 16);
        lv_obj_t *feeLabel = lv_label_create(feeContainer);
        lv_label_set_text(feeLabel, "Fee");
        lv_obj_set_style_text_color(feeLabel, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_align(feeLabel, LV_ALIGN_TOP_LEFT, 24, 0);
        GuiSetTransactionTitleStyle(feeLabel);

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
    GuiSetTransactionContainerStyle(votesOnContainer);

    lv_obj_t *label = lv_label_create(votesOnContainer);
    lv_label_set_text(label, "Votes on");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    GuiSetTransactionTitleStyle(label);

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
    GuiSetTransactionContainerStyle(mainActionContainer);

    label = lv_label_create(mainActionContainer);
    lv_label_set_text(label, "Main Action");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    GuiSetTransactionTitleStyle(label);

    label = GuiCreateIllustrateLabel(mainActionContainer, overviewData->main_action);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 151, 16);

    lv_obj_t *voteAccountContainer = GuiCreateContainerWithParent(parent, 408, 130);
    lv_obj_align_to(voteAccountContainer, mainActionContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    GuiSetTransactionContainerStyle(voteAccountContainer);

    label = lv_label_create(voteAccountContainer);
    lv_label_set_text(label, "Vote Account");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    GuiSetTransactionTitleStyle(label);

    label = GuiCreateIllustrateLabel(voteAccountContainer, overviewData->vote_account);
    lv_obj_set_width(label, 306);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);
}

static char GuiSolProtocolTextByteAt(void *context, size_t offset);

static lv_obj_t *GuiCreateSolConfigReviewCard(lv_obj_t *parent, const char *text, lv_obj_t *lastView)
{
    lv_obj_t *card = CreateTransactionContentContainer(
        parent, SOL_COMPONENT_WIDTH, SOL_CONFIG_REVIEW_MAX_HEIGHT);
    lv_obj_set_style_max_height(card, SOL_CONFIG_REVIEW_MAX_HEIGHT, LV_PART_MAIN);
    lv_obj_set_style_pad_left(card, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_right(card, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_top(card, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(card, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_row(card, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    if (lastView != NULL) lv_obj_align_to(card, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_t *title = GuiCreateIllustrateLabel(card, _("Data"));
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_height(title, LV_SIZE_CONTENT);
    lv_obj_set_style_text_opa(title, 144, LV_PART_MAIN);
    lv_obj_t *pager = GuiCreateContainerWithParent(card, SOL_COMPONENT_CONTENT_WIDTH, 0);
    lv_obj_set_style_bg_opa(pager, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_grow(pager, 1);
    lv_obj_update_layout(card);
    const lv_font_t *font = &openSansEnIllustrate;
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        if (*p >= 0x80) {
            font = g_defIllustrateFont;
            break;
        }
    }
    GuiPagedMessageSource_t source = {
        .context = (void *)text,
        .length = strlen(text),
        .byte_at = GuiSolProtocolTextByteAt,
        .utf8 = true,
        .font = font,
    };
    GuiShowPagedMessage(pager, &source);
    return card;
}

static lv_obj_t *GuiShowSolTxGeneralOverview(
    lv_obj_t *parent,
    PtrT_DisplaySolanaTxOverview overviewData,
    lv_obj_t *lastView)
{
    PtrT_VecFFI_DisplaySolanaTxOverviewGeneral general = overviewData->general;

    if (general == NULL) {
        return lastView;
    }

    for (int i = 0; i < general->size; i++) {
        char *program = general->data[i].program;
        if (strcmp(program, "TransactionConfig") == 0) {
            lastView = CreateTransactionItemViewWithWidth(
                parent, "Transaction Config (V1)", general->data[i].memo,
                lastView, SOL_COMPONENT_WIDTH);
            continue;
        }
        char order[BUFFER_SIZE_16] = {0};
        snprintf_s(order, BUFFER_SIZE_16, "#%u", (unsigned int)general->data[i].instruction_index);
        const char *method = strlen(general->data[i].method) > 0
            ? general->data[i].method
            : "Unknown";
        lv_obj_t *actionCard;
        if (strcmp(method, "VaultTransactionCreate") == 0 &&
            strlen(general->data[i].memo) > 0) {
            // Preserve the established Squads review card even when a mixed
            // transaction reaches the General/addon path.  The memo belongs
            // to VaultTransactionCreate, not ProposalCreate.
            actionCard = CreateSolanaSquadsProposalOverviewCard(
                parent, "Squads", method, general->data[i].memo, "");
        } else {
            actionCard = CreateTransactionOverviewCardWithWidth(
                parent,
                order,
                program,
                _("Method"),
                method,
                SOL_COMPONENT_WIDTH);
        }
        if (lastView != NULL) {
            lv_obj_align_to(actionCard, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        }
        lastView = actionCard;

        if (strcmp(method, "ConfigTransactionCreate") == 0 && strlen(general->data[i].memo) > 0) {
            lastView = GuiCreateSolConfigReviewCard(parent, general->data[i].memo, lastView);
        }
        if (strlen(general->data[i].value) > 0) {
            lastView = CreateTransactionItemViewWithWidth(
                parent, _("Value"), general->data[i].value, lastView, SOL_COMPONENT_WIDTH);
            lastView = CreateTransactionItemViewWithWidth(
                parent, _("From"), general->data[i].from, lastView, SOL_COMPONENT_WIDTH);
            if (general->data[i].to_in_lookup_table) {
                lv_obj_t *warningCard = GuiCreateAltWarningCard(
                    parent, general->data[i].to_lookup_table_reference);
                lv_obj_align_to(warningCard, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
                lastView = warningCard;
            }
            lastView = CreateTransactionItemViewWithWidth(
                parent,
                general->data[i].to_in_lookup_table ? _("solana_unresolved_destination") : _("To"),
                general->data[i].to,
                lastView,
                SOL_COMPONENT_WIDTH);
        }
        if (strlen(general->data[i].amount) > 0) {
            if (general->data[i].unusual_decimals) {
                lv_obj_t *warningCard = GuiCreateUnusualDecimalsWarningCard(
                    parent, general->data[i].decimals);
                if (lastView != NULL) {
                    lv_obj_align_to(warningCard, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
                }
                lastView = warningCard;
            }
            lastView = CreateTransactionItemViewWithWidth(
                parent, _("Amount"), general->data[i].amount, lastView, SOL_COMPONENT_WIDTH);
            if (strlen(general->data[i].token) > 0) {
                lastView = CreateTransactionItemViewWithWidth(
                    parent, _("Token"), general->data[i].token, lastView, SOL_COMPONENT_WIDTH);
            }
            if (strlen(general->data[i].mint) > 0) {
                lastView = CreateTransactionItemViewWithWidth(
                    parent, _("Mint Address"), general->data[i].mint, lastView, SOL_COMPONENT_WIDTH);
            }
            lastView = CreateTransactionItemViewWithWidth(
                parent, _("Authority"), general->data[i].authority, lastView, SOL_COMPONENT_WIDTH);
            lastView = CreateTransactionItemViewWithWidth(
                parent, _("Source"), general->data[i].source, lastView, SOL_COMPONENT_WIDTH);
            lastView = CreateTransactionItemViewWithWidth(
                parent, _("Destination"), general->data[i].destination, lastView, SOL_COMPONENT_WIDTH);
        }
    }
    return lastView;
}

/*
 * Specialized overviews own their layout, so they do not all expose the
 * final card they created.  Find the bottom-most direct child and use it as
 * the anchor for generic sibling instructions.  This keeps the specialized
 * overview intact and appends addons after it instead of replacing it.
 */
static lv_obj_t *GuiGetSolTxBottomView(lv_obj_t *parent)
{
    lv_obj_update_layout(parent);
    lv_obj_t *bottomView = NULL;
    int32_t bottom = 0;
    uint32_t childCount = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < childCount; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        int32_t childBottom = lv_obj_get_y(child) + lv_obj_get_height(child);
        if (bottomView == NULL || childBottom > bottom) {
            bottomView = child;
            bottom = childBottom;
        }
    }
    return bottomView;
}

static bool GuiHasSolTxAdditionalUnknownPrograms(
    PtrT_DisplaySolanaTxOverview overviewData)
{
    PtrT_VecFFI_PtrString programs = overviewData->additional_unknown_programs;
    return programs != NULL && programs->size > 0;
}

static lv_obj_t *GuiAppendSolTxAdditionalUnknownProgramCards(
    lv_obj_t *parent,
    PtrT_DisplaySolanaTxOverview overviewData,
    lv_obj_t *lastView)
{
    PtrT_VecFFI_PtrString programs = overviewData->additional_unknown_programs;
    if (programs == NULL) {
        return lastView;
    }

    for (int i = 0; i < programs->size; i++) {
        char order[BUFFER_SIZE_16] = {0};
        snprintf_s(order, BUFFER_SIZE_16, "#%d", i + 1);
        lv_obj_t *programCard = CreateTransactionOverviewCardWithWidth(
            parent,
            _("solana_unknown_program"),
            order,
            _("solana_program_address"),
            programs->data[i],
            SOL_COMPONENT_WIDTH);
        if (lastView != NULL) {
            lv_obj_align_to(programCard, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        }
        lastView = programCard;
    }
    return lastView;
}

static void GuiShowSolTxAdditionalUnknownPrograms(
    lv_obj_t *parent,
    PtrT_DisplaySolanaTxOverview overviewData)
{
    PtrT_VecFFI_PtrString programs = overviewData->additional_unknown_programs;
    if (programs == NULL || programs->size == 0) {
        return;
    }

    lv_obj_t *warningCard = GuiCreateWarningCard(parent);
    lv_obj_align(warningCard, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_update_layout(warningCard);
    int32_t contentOffset = lv_obj_get_height(warningCard) + 16;

    lv_obj_t *lastView = warningCard;
    int32_t lastBottom = lv_obj_get_height(warningCard);
    int32_t childCount = lv_obj_get_child_cnt(parent);
    for (int32_t i = 0; i < childCount; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        // LVGL's child order is an implementation detail.  Identify the new
        // warning by pointer so every pre-existing overview card is shifted
        // exactly once, regardless of where the warning appears in the list.
        if (child == warningCard) {
            continue;
        }
        lv_obj_set_y(child, lv_obj_get_y(child) + contentOffset);
        int32_t childBottom = lv_obj_get_y(child) + lv_obj_get_height(child);
        if (childBottom > lastBottom) {
            lastBottom = childBottom;
            lastView = child;
        }
    }

    GuiAppendSolTxAdditionalUnknownProgramCards(parent, overviewData, lastView);
}
static void GuiShowSolTxUnknownOverview(lv_obj_t *parent)
{
    uint16_t height = 177;
    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, 302);
    lv_obj_align(container, LV_ALIGN_DEFAULT, 0, 0);
    GuiSetTransactionContainerStyle(container);

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

void SquadsLearnMoreHandler(lv_event_t *e)
{
    char *url = "https://app.squads.so/create-squad";
    GuiQRCodeHintBoxOpenBig(url, _("solana_squads_amount_title"), _("solana_squads_amount_desc"), url);
}

static void SolanaAddressLearnMore(lv_event_t *e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    const char *reference = lv_event_get_user_data(e);
    if (reference == NULL) {
        reference = (const char *)lv_obj_get_user_data(obj);
    }
    if (reference != NULL) {
        const char *table = reference;
        if (strncmp(table, "Table ", 6) == 0) {
            table += 6;
        }
        const char *separator = strchr(table, '#');
        size_t tableLen = separator == NULL ? strlen(table) : (size_t)(separator - table);
        if (tableLen == 0 || tableLen >= 45) {
            return;
        }
        char tableAddress[45] = {0};
        memcpy_s(tableAddress, sizeof(tableAddress), table, tableLen);
        char url[128] = {0};
        snprintf_s(
            url,
            sizeof(url),
            "https://solscan.io/account/%s#tableEntries",
            tableAddress);
        GuiQRCodeHintBoxOpenBig(
            url, "Address Lookup Table", _("solana_alt_notice"), url);
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
    lv_obj_t *walletNameContainer = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    lv_obj_t *walletNameLabel = GuiCreateTextLabel(walletNameContainer, "Wallet Name");
    GuiSetTransactionTitleStyle(walletNameLabel);
    lv_obj_align(walletNameLabel, LV_ALIGN_TOP_LEFT, 24, 0);

    lv_obj_t *walletNameValue = GuiCreateIllustrateLabel(walletNameContainer, wallet_name);
    lv_obj_set_style_text_color(walletNameValue, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align_to(walletNameValue, walletNameLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t *totalValueContainer =  GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    lv_obj_t *totalValueLabel = lv_label_create(totalValueContainer);
    lv_label_set_text(totalValueLabel, "Transaction Total");
    lv_obj_align(totalValueLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    GuiSetTransactionTitleStyle(totalValueLabel);

    lv_obj_t *totalValueValue = lv_label_create(totalValueContainer);
    lv_label_set_text(totalValueValue, total_value);
    lv_obj_set_style_text_font(totalValueValue, &openSansEnLittleTitle, LV_PART_MAIN);
    lv_obj_set_style_text_color(totalValueValue, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(totalValueValue, totalValueLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    lv_obj_t *descriptionLabel = GuiCreateIllustrateLabel(totalValueContainer, _("solana_squads_amount_lm"));
    lv_obj_set_style_text_color(descriptionLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_width(descriptionLabel, SOL_COMPONENT_CONTENT_WIDTH);
    lv_label_set_long_mode(descriptionLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(descriptionLabel, totalValueValue, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    lv_obj_t *learnMoreLabel = GuiCreateTextLabel(totalValueContainer, "Learn More");
    lv_obj_add_flag(learnMoreLabel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_text_color(learnMoreLabel, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align_to(learnMoreLabel, descriptionLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_add_event_cb(learnMoreLabel, SquadsLearnMoreHandler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *memberContainer = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
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
        lv_obj_clear_flag(memberItem, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *memberItemLabel = lv_label_create(memberItem);
        char memberText[64];
        snprintf_s(memberText, sizeof(memberText), "Member %u", (unsigned)(i + 1));
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
    lv_obj_t *thresholdContainer =  GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    lv_obj_t *thresholdLabel = lv_label_create(thresholdContainer);
    lv_label_set_text(thresholdLabel, "Threshold / Member");
    lv_obj_set_style_text_color(thresholdLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align(thresholdLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    GuiSetTransactionTitleStyle(thresholdLabel);

    lv_obj_t *thresholdLabelValue = lv_label_create(thresholdContainer);
    char thresholdText[64];
    snprintf_s(thresholdText, sizeof(thresholdText), "%u/%u", (unsigned)threshold, (unsigned)member_count);
    lv_label_set_text(thresholdLabelValue, thresholdText);
    lv_obj_set_style_text_color(thresholdLabelValue, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(thresholdLabelValue, thresholdLabel, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_t *firstTransferCardContainer = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    CreateSquadsSolanaTransferOverviewCard(firstTransferCardContainer,  transfers->data[1].from, transfers->data[1].to, transfers->data[1].value, "");

    lv_obj_t *secondTransferCardContainer = GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
    CreateSquadsSolanaTransferOverviewCard(secondTransferCardContainer,  transfers->data[0].from, transfers->data[0].to, transfers->data[0].value, "");
    lv_obj_align(walletNameContainer, LV_ALIGN_TOP_LEFT, 0, 0);
    if (strcmp(wallet_desc, "") != 0) {
        lv_obj_t *walletDescContainer =  GuiCreateAutoHeightContainer(parent, SOL_COMPONENT_WIDTH, 16);
        lv_obj_t *walletDescLabel = lv_label_create(walletDescContainer);
        lv_label_set_text(walletDescLabel, "Description");
        lv_obj_align(walletDescLabel, LV_ALIGN_TOP_LEFT, 24, 0);
        GuiSetTransactionTitleStyle(walletDescLabel);

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
    GuiSetTransactionTitleStyle(thresholdLabel);

    uint16_t threshold = squadsMultisigCreate->threshold;
    uintptr_t member_count = squadsMultisigCreate->member_count;
    lv_obj_t *thresholdLabelValue = lv_label_create(thresholdContainer);
    char thresholdText[64];
    snprintf_s(thresholdText, sizeof(thresholdText), "%u/%u", (unsigned)threshold, (unsigned)member_count);
    lv_label_set_text(thresholdLabelValue, thresholdText);
    lv_obj_set_style_text_color(thresholdLabelValue, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(thresholdLabelValue, thresholdLabel, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    PtrString total_value = squadsMultisigCreate->total_value;
    lv_obj_t *totalValueContainer =  GuiCreateAutoHeightContainer(parent, 408, 16);
    lv_obj_t *totalValueLabel = GuiCreateTextLabel(totalValueContainer, "Value");
    lv_obj_align(totalValueLabel, LV_ALIGN_TOP_LEFT, 24, 0);
    GuiSetTransactionTitleStyle(totalValueLabel);

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
    GuiSetTransactionTitleStyle(tolabel);
    lv_obj_align(tolabel, LV_ALIGN_TOP_LEFT, 24, 0);
    lv_obj_t * toValuelabel = GuiCreateIllustrateLabel(firstTransferCardContainer, transfers->data[1].to);
    lv_obj_set_width(toValuelabel, 306);
    lv_obj_align_to(toValuelabel, tolabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    if (strcmp(transfers->data[1].to, SQUADS_V4_CREATE_MULTISIG_CONTRACT_ADDRESS) == 0) {
        lv_obj_t *squadsIcon = GuiCreateImg(firstTransferCardContainer, &imgSquads);
        lv_obj_align_to(squadsIcon, toValuelabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
        lv_obj_t *squadsLabel = GuiCreateIllustrateLabel(firstTransferCardContainer, "Squads");
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

static bool GuiSolInstructionsUseAddressLookupTable(PtrT_VecFFI_Instruction instructions)
{
    if (instructions == NULL) {
        return false;
    }
    for (size_t i = 0; i < instructions->size; i++) {
        if (instructions->data[i].program_address != NULL &&
            CheckIsAddressLookupTableAccount(instructions->data[i].program_address)) {
            return true;
        }
        PtrT_VecFFI_PtrString accounts = instructions->data[i].accounts;
        if (accounts == NULL) {
            continue;
        }
        for (size_t j = 0; j < accounts->size; j++) {
            if (accounts->data[j] != NULL && CheckIsAddressLookupTableAccount(accounts->data[j])) {
                return true;
            }
        }
    }
    return false;
}

#define SOL_ACCOUNTS_PER_PAGE 3
#define SOL_ACCOUNTS_PAGER_BOTTOM_PADDING 8

typedef struct {
    PtrT_VecFFI_PtrString accounts;
    size_t page;
    lv_obj_t *content;
    lv_obj_t *counter;
    lv_obj_t *previous;
    lv_obj_t *next;
} SolAccountsPager;

static void GuiSolAccountsRefresh(SolAccountsPager *state)
{
    lv_obj_clean(state->content);
    size_t start = state->page * SOL_ACCOUNTS_PER_PAGE;
    size_t end = start + SOL_ACCOUNTS_PER_PAGE;
    if (end > state->accounts->size) end = state->accounts->size;
    for (size_t j = start; j < end; j++) {
        lv_obj_t *account_label = GuiCreateIllustrateLabel(state->content, state->accounts->data[j]);
        lv_obj_set_width(account_label, lv_pct(100));
        if (CheckIsAddressLookupTableAccount(state->accounts->data[j])) {
            const char *separator = strchr(state->accounts->data[j], '#');
            char altReference[96] = {0};
            if (separator != NULL) {
                snprintf_s(
                    altReference,
                    sizeof(altReference),
                    "Table: %.*s\nIndex: %s",
                    (int)(separator - state->accounts->data[j]),
                    state->accounts->data[j],
                    separator + 1);
                lv_label_set_text(account_label, altReference);
            }
            lv_obj_t *infoButton = lv_btn_create(state->content);
            lv_obj_remove_style_all(infoButton);
            lv_obj_set_size(infoButton, 48, 48);
            lv_obj_clear_flag(infoButton, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_add_flag(infoButton, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_t *infoIcon = GuiCreateImg(infoButton, &imgInfoSmall);
            lv_obj_align(infoIcon, LV_ALIGN_LEFT_MID, 0, 0);
            lv_obj_clear_flag(infoIcon, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(infoButton, SolanaAddressLearnMore, LV_EVENT_CLICKED,
                                state->accounts->data[j]);
        } else {
            lv_label_set_text(account_label, state->accounts->data[j]);
        }
        lv_obj_set_style_text_color(account_label, WHITE_COLOR, LV_PART_MAIN);
        lv_label_set_long_mode(account_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_border_width(account_label, 0, LV_PART_MAIN);
    }
    lv_obj_update_layout(state->content);
    lv_obj_scroll_to_y(state->content, 0, LV_ANIM_OFF);
    size_t pages = (state->accounts->size + SOL_ACCOUNTS_PER_PAGE - 1) / SOL_ACCOUNTS_PER_PAGE;
    if (state->counter != NULL) {
        lv_label_set_text_fmt(state->counter, "%u / %u", (unsigned)(state->page + 1), (unsigned)pages);
        if (state->page == 0) lv_obj_add_state(state->previous, LV_STATE_DISABLED);
        else lv_obj_clear_state(state->previous, LV_STATE_DISABLED);
        if (state->page + 1 >= pages) lv_obj_add_state(state->next, LV_STATE_DISABLED);
        else lv_obj_clear_state(state->next, LV_STATE_DISABLED);
    }
}

static void GuiSolAccountsEvent(lv_event_t *event)
{
    SolAccountsPager *state = lv_event_get_user_data(event);
    if (lv_event_get_code(event) == LV_EVENT_DELETE) {
        lv_mem_free(state);
        return;
    }
    if (lv_event_get_target(event) == state->previous && state->page > 0) state->page--;
    else if (lv_event_get_target(event) == state->next &&
             (state->page + 1) * SOL_ACCOUNTS_PER_PAGE < state->accounts->size) state->page++;
    else return;
    GuiSolAccountsRefresh(state);
}

static lv_obj_t *GuiSolAccountsButton(lv_obj_t *parent, const char *text)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_size(button, 72, 48);
    lv_obj_t *label = GuiCreateIllustrateLabel(button, text);
    lv_obj_center(label);
    return button;
}

static lv_obj_t *GuiSolInstructionSection(lv_obj_t *parent, const char *title)
{
    lv_obj_t *section = GuiCreateContainerWithParent(parent, SOL_COMPONENT_CONTENT_WIDTH, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(section, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(section, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 12, LV_PART_MAIN);
    lv_obj_set_style_border_width(section, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(section, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(section, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_pad_all(section, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_row(section, 12, LV_PART_MAIN);
    lv_obj_set_flex_flow(section, LV_FLEX_FLOW_COLUMN);
    lv_obj_t *label = GuiCreateIllustrateLabel(section, title);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    return section;
}

static lv_obj_t *GuiShowSolAccounts(lv_obj_t *parent, PtrT_VecFFI_PtrString accounts)
{
    lv_obj_update_layout(parent);
    lv_coord_t width = lv_obj_get_content_width(parent);
    lv_obj_t *pager = GuiCreateContainerWithParent(
        parent, width, 384 + SOL_ACCOUNTS_PAGER_BOTTOM_PADDING);
    lv_obj_set_style_pad_bottom(pager, SOL_ACCOUNTS_PAGER_BOTTOM_PADDING, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pager, LV_OPA_TRANSP, LV_PART_MAIN);
    SolAccountsPager *state = lv_mem_alloc(sizeof(*state));
    ASSERT(state != NULL);
    memset_s(state, sizeof(*state), 0, sizeof(*state));
    state->accounts = accounts;
    state->content = GuiCreateContainerWithParent(pager, width, 320);
    lv_obj_set_style_bg_opa(state->content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_flow(state->content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(state->content, 8, LV_PART_MAIN);
    lv_obj_add_flag(state->content, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(state->content, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(state->content, LV_DIR_VER);
    const bool paginated = accounts->size > SOL_ACCOUNTS_PER_PAGE;
    if (paginated) {
        state->previous = GuiSolAccountsButton(pager, "<");
        lv_obj_align(state->previous, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        state->next = GuiSolAccountsButton(pager, ">");
        lv_obj_align(state->next, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        state->counter = GuiCreateIllustrateLabel(pager, "");
        lv_obj_align(state->counter, LV_ALIGN_BOTTOM_MID, 0, -9);
        lv_obj_add_event_cb(state->previous, GuiSolAccountsEvent, LV_EVENT_CLICKED, state);
        lv_obj_add_event_cb(state->next, GuiSolAccountsEvent, LV_EVENT_CLICKED, state);
    }
    lv_obj_add_event_cb(pager, GuiSolAccountsEvent, LV_EVENT_DELETE, state);
    GuiSolAccountsRefresh(state);
    if (!paginated) {
        lv_obj_set_height(state->content, LV_SIZE_CONTENT);
        lv_obj_update_layout(state->content);
        lv_coord_t contentHeight = lv_obj_get_height(state->content);
        if (contentHeight > 320) {
            contentHeight = 320;
        } else {
            lv_obj_clear_flag(state->content, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        }
        lv_obj_set_height(state->content, contentHeight);
        lv_obj_set_style_pad_bottom(pager, 0, LV_PART_MAIN);
        lv_obj_set_height(pager, contentHeight);
    }
    return pager;
}

static void GuiShowSolTxInstructionsOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    PtrT_DisplaySolanaTxOverviewUnknownInstructions unknown_instructions = overviewData->unknown_instructions;
    PtrT_VecFFI_Instruction overview_instructions = unknown_instructions->overview_instructions;
    const bool usesAlt = GuiSolInstructionsUseAddressLookupTable(overview_instructions);
    // notice container
    lv_obj_t *noticeContainer = GuiCreateContainerWithParent(parent, SOL_COMPONENT_WIDTH, LV_SIZE_CONTENT);
    lv_obj_align(noticeContainer, LV_ALIGN_DEFAULT, 0, 0);
    GuiSetTransactionContainerStyle(noticeContainer);
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
    lv_label_set_text(noticeLabel, usesAlt ? _("solana_warning_title") : _("scan_qr_code_signing_notice"));

    lv_obj_t *noticeContent = lv_label_create(noticeContainer);
    lv_obj_set_width(noticeContent, lv_pct(90));
    lv_label_set_long_mode(noticeContent, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(noticeContent, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(noticeContent, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_label_set_text(
        noticeContent,
        usesAlt
            ? _("solana_parse_tx_alt_warning_body")
            : _("solana_parse_tx_notice"));
    lv_obj_set_style_text_opa(noticeContent, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(noticeContent, iconNoticeContainer, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 10);

    lv_obj_t *lastView = GuiShowSolTxGeneralOverview(parent, overviewData, noticeContainer);
    lv_obj_update_layout(parent);
    int containerYOffset = lv_obj_get_y(lastView) + lv_obj_get_height(lastView) + 16;
    for (int i = 0; i < overview_instructions->size; i++) {
        // container will auto adjust height
        lv_obj_t *container = GuiCreateContainerWithParent(parent, SOL_COMPONENT_WIDTH, LV_SIZE_CONTENT);
        lv_obj_align(container, LV_ALIGN_DEFAULT, 0, containerYOffset);
        GuiSetTransactionContainerStyle(container);
        lv_obj_set_style_pad_bottom(container, 16, LV_PART_MAIN);

        lv_obj_t *orderLabel = lv_label_create(container);
        char order[BUFFER_SIZE_16] = {0};
        snprintf_s(order, BUFFER_SIZE_16, "Instruction#%d", i + 1);
        lv_label_set_text(orderLabel, order);
        lv_obj_set_style_text_font(orderLabel, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(orderLabel, lv_color_hex(0xF5870A), LV_PART_MAIN);
        lv_obj_align(orderLabel, LV_ALIGN_TOP_LEFT, 24, 16);

        lv_obj_t *programSection = GuiSolInstructionSection(container, _("solana_program_address"));
        lv_obj_align_to(programSection, orderLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        lv_obj_update_layout(programSection);
        lv_obj_t *programAddress = GuiCreateIllustrateLabel(
            programSection, overview_instructions->data[i].program_address);
        lv_obj_set_width(programAddress, lv_obj_get_content_width(programSection));
        lv_label_set_long_mode(programAddress, LV_LABEL_LONG_WRAP);
        lv_obj_set_height(programAddress, LV_SIZE_CONTENT);
        lv_obj_update_layout(container);

        PtrT_VecFFI_PtrString accounts = overview_instructions->data[i].accounts;
        lv_obj_t *lastSection = programSection;
        if (accounts->size != 0) {
            lv_obj_t *accountsSection = GuiSolInstructionSection(container, "accounts");
            lv_obj_align_to(accountsSection, programSection, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
            GuiShowSolAccounts(accountsSection, accounts);
            lv_obj_update_layout(container);
            lastSection = accountsSection;
        }
        lv_obj_t *dataSection = GuiSolInstructionSection(container, _("solana_advanced_data"));
        lv_obj_align_to(dataSection, lastSection, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        lv_obj_update_layout(dataSection);
        lv_coord_t dataWidth = lv_obj_get_content_width(dataSection);
        const char *instructionData = overview_instructions->data[i].data;
        lv_obj_t *data_label;
        if (instructionData == NULL || strlen(instructionData) <= 512) {
            data_label = GuiCreateIllustrateLabel(dataSection, instructionData == NULL ? "" : instructionData);
            lv_obj_set_width(data_label, dataWidth);
            lv_label_set_long_mode(data_label, LV_LABEL_LONG_WRAP);
            lv_obj_set_height(data_label, LV_SIZE_CONTENT);
        } else {
            data_label = GuiCreateContainerWithParent(dataSection, dataWidth, 220);
            lv_obj_set_style_bg_opa(data_label, LV_OPA_TRANSP, LV_PART_MAIN);
            GuiShowPagedMessageText(data_label, instructionData, true, NULL, NULL);
        }
        lv_obj_update_layout(container);

        // calculate the height of the container
        lv_obj_update_layout(container);
        containerYOffset += lv_obj_get_height(container) + 16; // set padding between containers
    }
}

static void GuiShowSolTxOrderedOverview(lv_obj_t *parent, PtrT_DisplaySolanaTxOverview overviewData)
{
    lv_obj_t *lastView = NULL;
    if (GuiHasSolTxAdditionalUnknownPrograms(overviewData)) {
        lastView = GuiCreateWarningCard(parent);
        lv_obj_align(lastView, LV_ALIGN_TOP_LEFT, 0, 0);
    }
    for (size_t i = 0; i < overviewData->overview_order->size; i++) {
        DisplaySolanaOverviewAction *action = &overviewData->overview_order->data[i];
        lv_obj_t *group = lv_obj_create(parent);
        lv_obj_remove_style_all(group);
        lv_obj_set_size(group, SOL_COMPONENT_WIDTH, LV_SIZE_CONTENT);
        lv_obj_clear_flag(group, LV_OBJ_FLAG_SCROLLABLE);
        if (action->kind == 0) {
            lv_obj_t *order = GuiCreateNoticeLabel(group, "");
            lv_label_set_text_fmt(order, "%s #%u", _("solana_advanced_instruction"), (unsigned)action->instruction_index);
            lv_obj_set_width(order, SOL_COMPONENT_WIDTH);
            lv_label_set_long_mode(order, LV_LABEL_LONG_WRAP);
            lv_obj_update_layout(order);
            lv_obj_t *content = lv_obj_create(group);
            lv_obj_remove_style_all(content);
            lv_obj_set_size(content, SOL_COMPONENT_WIDTH, LV_SIZE_CONTENT);
            lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_y(content, lv_obj_get_height(order) + 12);
            if (strcmp(overviewData->display_type, "Transfer") == 0) {
                GuiShowSolTxTransferOverview(content, overviewData);
            } else if (strcmp(overviewData->display_type, "TokenTransfer") == 0) {
                GuiShowSplTokenTransferOverview(content, overviewData);
            } else if (strcmp(overviewData->display_type, "Vote") == 0) {
                GuiShowSolTxVoteOverview(content, overviewData);
            } else if (strcmp(overviewData->display_type, "jupiterv6_swap") == 0) {
                GuiShowJupiterV6SwapOverview(content, overviewData);
            } else if (strcmp(overviewData->display_type, "squads_multisig_create") == 0) {
                GuiShowSolTxMultiSigCreateOverview(content, overviewData);
            } else if (strcmp(overviewData->display_type, "squads_proposal") == 0) {
                GuiShowSolTxSquadsProposalOverview(content, overviewData);
            }
        } else if (action->kind == 1) {
            VecFFI_DisplaySolanaTxOverviewGeneral item = {
                .data = &overviewData->general->data[action->index], .size = 1, .cap = 1,
            };
            DisplaySolanaTxOverview single = *overviewData;
            single.general = &item;
            GuiShowSolTxGeneralOverview(group, &single, NULL);
        } else if (action->kind == 2) {
            char order[BUFFER_SIZE_16] = {0};
            snprintf_s(order, sizeof(order), "#%u", (unsigned)action->instruction_index);
            CreateTransactionOverviewCardWithWidth(
                group, _("solana_unknown_program"), order, _("solana_program_address"),
                overviewData->additional_unknown_programs->data[action->index], SOL_COMPONENT_WIDTH);
        }
        lv_obj_update_layout(group);
        if (lastView != NULL) {
            lv_obj_align_to(group, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        }
        lastView = group;
    }
}

void GuiShowSolTxOverview(lv_obj_t *parent, void *totalData)
{
    lv_obj_set_size(parent, 408, 444);
    const lv_coord_t sidePadding = (408 - SOL_COMPONENT_WIDTH) / 2;
    lv_obj_set_style_pad_left(parent, sidePadding, LV_PART_MAIN);
    lv_obj_set_style_pad_right(parent, sidePadding, LV_PART_MAIN);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    DisplaySolanaTx *txData = (DisplaySolanaTx*)totalData;
    PtrT_DisplaySolanaTxOverview overviewData = txData->overview;
    if (overviewData->overview_order != NULL) {
        GuiShowSolTxOrderedOverview(parent, overviewData);
        lv_obj_update_layout(parent);
        lv_obj_scroll_to_y(parent, 0, LV_ANIM_OFF);
        return;
    }
    bool unknownProgramsRendered = false;
    if (0 == strcmp(overviewData->display_type, "Transfer")) {
        GuiShowSolTxTransferOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "Vote")) {
        GuiShowSolTxVoteOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "General")) {
        lv_obj_t *lastView = NULL;
        if (GuiHasSolTxAdditionalUnknownPrograms(overviewData)) {
            lastView = GuiCreateWarningCard(parent);
            lv_obj_align(lastView, LV_ALIGN_TOP_LEFT, 0, 0);
        }
        lastView = GuiShowSolTxGeneralOverview(parent, overviewData, lastView);
        GuiAppendSolTxAdditionalUnknownProgramCards(parent, overviewData, lastView);
        unknownProgramsRendered = true;
    } else if (0 == strcmp(overviewData->display_type, "squads_multisig_create")) {
        GuiShowSolTxMultiSigCreateOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "TokenTransfer")) {
        GuiShowSplTokenTransferOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "squads_proposal")) {
        GuiShowSolTxSquadsProposalOverview(parent, overviewData);
    } else if (0 == strcmp(overviewData->display_type, "jupiterv6_swap")) {
        lv_obj_t *lastView = GuiShowJupiterV6SwapOverview(parent, overviewData);
        GuiShowSolTxGeneralOverview(parent, overviewData, lastView);
    } else {
        GuiShowSolTxInstructionsOverview(parent, overviewData);
        return;
    }

    /*
     * Transfer, token-transfer, vote and Squads pages retain their original
     * purpose-built cards.  Any sibling instructions prepared by the parser
     * are appended below those cards as generic addons.
     */
    if (0 != strcmp(overviewData->display_type, "General") &&
        0 != strcmp(overviewData->display_type, "jupiterv6_swap")) {
        GuiShowSolTxGeneralOverview(parent, overviewData, GuiGetSolTxBottomView(parent));
    }
    if (!unknownProgramsRendered) {
        GuiShowSolTxAdditionalUnknownPrograms(parent, overviewData);
    }
    lv_obj_update_layout(parent);
    lv_obj_scroll_to_y(parent, 0, LV_ANIM_OFF);
}

static char GuiSolProtocolTextByteAt(void *context, size_t offset)
{
    return ((const char *)context)[offset];
}

static void GuiShowSolTxRawDetailCard(
    lv_obj_t *parent,
    PtrString txDetail,
    lv_obj_t *lastView)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    // Reuse the release pager: long pages remain scrollable within the viewport.
    lv_obj_set_size(cont, SOL_COMPONENT_WIDTH, 420);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    const char *rawDetail = txDetail == NULL ? "" : txDetail;
    GuiPagedMessageSource_t source = {
        .context = (void *)rawDetail,
        .length = strlen(rawDetail),
        .byte_at = GuiSolProtocolTextByteAt,
        .utf8 = true,
        .font = &openSansEnIllustrate,
    };
    GuiShowPagedMessage(cont, &source);
    if (lastView == NULL) {
        lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 0, 0);
    } else {
        lv_obj_align_to(cont, lastView, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
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
        lv_obj_update_layout(parent);
        lv_obj_t *last = GuiGetSolTxBottomView(parent);
        lv_obj_t *path = GuiShowSolSigningPath(parent);
        if (last != NULL) {
            lv_obj_align_to(path, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        }
        GuiShowSolTxRawDetailCard(parent, txData->detail, path);
        lv_obj_update_layout(parent);
        return;
    }
    lv_obj_t *path = GuiShowSolSigningPath(parent);
    GuiShowSolTxRawDetailCard(parent, txData->detail, path);
}

#define SOL_ADVANCED_ITEMS_PER_PAGE 4

static size_t GuiSolAdvancedTextPass(const char *text, char *output, size_t outputSize)
{
    size_t length = 0;
    while (*text != '\0') {
        const char *token = strstr(text, "{{solana_advanced_");
        const char *end = token == NULL ? NULL : strstr(token + 2, "}}");
        char key[64];
        if (end == NULL || (size_t)(end - token - 2) >= sizeof(key)) {
            size_t remaining = strlen(text);
            if (output != NULL) {
                ASSERT(length < outputSize && remaining < outputSize - length);
                memcpy_s(output + length, outputSize - length, text, remaining);
            }
            length += remaining;
            break;
        }
        size_t prefix = token - text;
        if (output != NULL) {
            ASSERT(length < outputSize && prefix < outputSize - length);
            memcpy_s(output + length, outputSize - length, text, prefix);
        }
        length += prefix;
        size_t keyLength = end - token - 2;
        memcpy_s(key, sizeof(key), token + 2, keyLength);
        key[keyLength] = '\0';
        const char *translated = _(key);
        size_t translatedLength = strlen(translated);
        if (output != NULL) {
            ASSERT(length < outputSize && translatedLength < outputSize - length);
            memcpy_s(output + length, outputSize - length, translated, translatedLength);
        }
        length += translatedLength;
        text = end + 2;
    }
    if (output != NULL) {
        ASSERT(length < outputSize);
        output[length] = '\0';
    }
    return length;
}

static char *GuiSolAdvancedText(const char *text)
{
    size_t size = GuiSolAdvancedTextPass(text, NULL, 0) + 1;
    char *translated = lv_mem_alloc(size);
    ASSERT(translated != NULL);
    GuiSolAdvancedTextPass(text, translated, size);
    return translated;
}

typedef struct {
    const VecFFI_DisplaySolanaAdvancedItem *items;
    size_t page;
    lv_obj_t *content;
    lv_obj_t *previous;
    lv_obj_t *next;
    lv_obj_t *counter;
} SolAdvancedPager;

static void GuiSolAdvancedRefresh(SolAdvancedPager *state)
{
    lv_obj_clean(state->content);
    lv_obj_scroll_to_y(state->content, 0, LV_ANIM_OFF);
    size_t start = state->page * SOL_ADVANCED_ITEMS_PER_PAGE;
    size_t end = start + SOL_ADVANCED_ITEMS_PER_PAGE;
    if (end > state->items->size) {
        end = state->items->size;
    }
    lv_obj_t *last = NULL;
    for (size_t i = start; i < end; i++) {
        const DisplaySolanaAdvancedItem *item = &state->items->data[i];
        char *title = GuiSolAdvancedText(item->title);
        char *value = GuiSolAdvancedText(item->value);
        last = CreateTransactionItemViewWithWidth(
            state->content, title, value, last, SOL_COMPONENT_WIDTH);
        lv_obj_t *titleLabel = lv_obj_get_child(last, 0);
        lv_obj_t *valueLabel = lv_obj_get_child(last, 1);
        if (lv_obj_get_width(titleLabel) > SOL_COMPONENT_CONTENT_WIDTH) {
            lv_obj_set_width(titleLabel, SOL_COMPONENT_CONTENT_WIDTH);
            lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_WRAP);
            lv_obj_update_layout(titleLabel);
            lv_obj_set_width(valueLabel, SOL_COMPONENT_CONTENT_WIDTH);
            lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_WRAP);
            lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
            lv_obj_update_layout(valueLabel);
            lv_obj_set_height(last, 32 + lv_obj_get_height(titleLabel) + lv_obj_get_height(valueLabel));
        }
        lv_mem_free(title);
        lv_mem_free(value);
    }
    size_t pages = (state->items->size + SOL_ADVANCED_ITEMS_PER_PAGE - 1) / SOL_ADVANCED_ITEMS_PER_PAGE;
    if (pages == 0) {
        pages = 1;
    }
    lv_label_set_text_fmt(state->counter, "%u / %u", (unsigned)(state->page + 1), (unsigned)pages);
    if (state->page == 0) {
        lv_obj_add_state(state->previous, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(state->previous, LV_STATE_DISABLED);
    }
    if (state->page + 1 >= pages) {
        lv_obj_add_state(state->next, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(state->next, LV_STATE_DISABLED);
    }
}

static void GuiSolAdvancedEvent(lv_event_t *event)
{
    SolAdvancedPager *state = lv_event_get_user_data(event);
    if (lv_event_get_code(event) == LV_EVENT_DELETE) {
        lv_mem_free(state);
        return;
    }
    if (lv_event_get_target(event) == state->previous && state->page > 0) {
        state->page--;
    } else if (lv_event_get_target(event) == state->next &&
               (state->page + 1) * SOL_ADVANCED_ITEMS_PER_PAGE < state->items->size) {
        state->page++;
    } else {
        return;
    }
    GuiSolAdvancedRefresh(state);
}

static lv_obj_t *GuiSolAdvancedButton(lv_obj_t *parent, const char *text)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_size(button, 72, 48);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, 30, LV_PART_MAIN);
    lv_obj_t *label = lv_label_create(button);
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    return button;
}

void GuiShowSolTxAdvanced(lv_obj_t *parent, void *totalData)
{
    DisplaySolanaTx *txData = (DisplaySolanaTx*)totalData;
    if (txData->advanced == NULL) {
        return;
    }
    lv_obj_set_size(parent, SOL_COMPONENT_WIDTH, 420);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    SolAdvancedPager *state = lv_mem_alloc(sizeof(SolAdvancedPager));
    if (state == NULL) {
        return;
    }
    memset_s(state, sizeof(*state), 0, sizeof(*state));
    state->items = txData->advanced;
    state->content = lv_obj_create(parent);
    lv_obj_remove_style_all(state->content);
    lv_obj_set_size(state->content, SOL_COMPONENT_WIDTH, 356);
    lv_obj_set_scroll_dir(state->content, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(state->content, LV_SCROLLBAR_MODE_OFF);

    state->previous = GuiSolAdvancedButton(parent, "<");
    lv_obj_align(state->previous, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    state->next = GuiSolAdvancedButton(parent, ">");
    lv_obj_align(state->next, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    state->counter = lv_label_create(parent);
    lv_obj_set_style_text_font(state->counter, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(state->counter, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align(state->counter, LV_ALIGN_BOTTOM_MID, 0, -9);
    lv_obj_add_event_cb(state->previous, GuiSolAdvancedEvent, LV_EVENT_CLICKED, state);
    lv_obj_add_event_cb(state->next, GuiSolAdvancedEvent, LV_EVENT_CLICKED, state);
    lv_obj_add_event_cb(parent, GuiSolAdvancedEvent, LV_EVENT_DELETE, state);
    GuiSolAdvancedRefresh(state);
}
