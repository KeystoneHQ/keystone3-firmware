#ifndef BTC_ONLY

#include "gui_stellar.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

static void CreateStellarNoticeCOntainer(lv_obj_t *parent, char *title, char *context, lv_coord_t w, lv_coord_t h);

#define CHECK_FREE_PARSE_RESULT(result)                                                                                   \
    if (result != NULL)                                                                                                   \
    {                                                                                                                     \
        free_TransactionParseResult_DisplayStellarTx((PtrT_TransactionParseResult_DisplayStellarTx)result);               \
        result = NULL;                                                                                                    \
    }


PtrT_TransactionCheckResult GuiGetStellarCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return stellar_check_tx(data, mfp, sizeof(mfp));
}

void GetStellarRawMessage(void *indata, void *param, uint32_t maxLen)
{
    DisplayStellarTx *tx = (DisplayStellarTx *)param;
    if (tx->raw_message == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, tx->raw_message);
}

int GetStellarRawMessageLength(void *param)
{
    DisplayStellarTx *data = (DisplayStellarTx *)param;
    if (data->raw_message == NULL) {
        return 0;
    }
    return strlen(data->raw_message) + 1;
}

void *GuiGetStellarData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayStellarTx parseResult = stellar_parse(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void FreeStellarMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

void GuiSetStellarUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

UREncodeResult *GuiGetStellarSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = stellar_sign(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

static void CreateStellarNoticeCOntainer(lv_obj_t *parent, char *title, char *context, lv_coord_t w, lv_coord_t h)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(parent, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(parent, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(parent, 31, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *rightImg = GuiCreateImg(parent, &imgNotice);
    lv_obj_align(rightImg, LV_ALIGN_TOP_LEFT, 24, 22);
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(16090890), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 68, 24);
    lv_obj_t *contextContainer = lv_label_create(parent);
    lv_label_set_text(contextContainer, context);
    lv_obj_set_style_text_font(contextContainer, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(contextContainer, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align(contextContainer, LV_ALIGN_TOP_LEFT, 24, 68);
    lv_obj_set_size(contextContainer, w, h);
}

void GuiStellarTxNotice(lv_obj_t *parent, void *totalData)
{
    CreateStellarNoticeCOntainer(
        parent,
        "Notice",
        "XDR (External Data Representation) standardizes data encoding for consistent serialization. You can compare this with the software wallet.",
        360,
        120
    );
}

void GuiStellarHashNotice(lv_obj_t *parent, void *totalData)
{
    CreateStellarNoticeCOntainer(
        parent,
        _("scan_qr_code_signing_notice"),
        _("scan_qr_code_signing_hash_signing_desc"),
        360,
        120
    );
}

#endif
