#ifndef BTC_ONLY

#include "gui_icp.h"

static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;

static void CreateIcpNoticeContainer(lv_obj_t *parent, char *title, char *context, lv_coord_t w, lv_coord_t h);

#define CHECK_FREE_PARSE_RESULT(result)                                                                                   \
    if (result != NULL)                                                                                                   \
    {                                                                                                                     \
        free_TransactionParseResult_DisplayIcpTx((PtrT_TransactionParseResult_DisplayIcpTx)result);               \
        result = NULL;                                                                                                    \
    }


PtrT_TransactionCheckResult GuiGetIcpCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return icp_check_tx(data, mfp, sizeof(mfp));
}

void GetIcpRawMessage(void *indata, void *param, uint32_t maxLen)
{
    DisplayIcpTx *tx = (DisplayIcpTx *)param;
    if (tx->raw_message == NULL) {
        return;
    }
    strcpy_s((char *)indata, maxLen, tx->raw_message);
}

int GetIcpRawMessageLength(void *param)
{
    DisplayIcpTx *data = (DisplayIcpTx *)param;
    if (data->raw_message == NULL) {
        return 0;
    }
    return strlen(data->raw_message) + 1;
}

void *GuiGetIcpParseData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    do {
        PtrT_TransactionParseResult_DisplayIcpTx parseResult = icp_parse(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void FreeIcpMemory(void)
{
#ifndef COMPILE_SIMULATOR
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
#endif
}

void GuiSetIcpUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

UREncodeResult *GuiGetIcpSignQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        encodeResult = icp_sign_tx(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

static void CreateIcpNoticeCOntainer(lv_obj_t *parent, char *title, char *context, lv_coord_t w, lv_coord_t h)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(parent, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(parent, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(parent, 31, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_t *rightImg = GuiCreateImg(parent, &imgNotice);
    // lv_obj_align(rightImg, LV_ALIGN_TOP_LEFT, 24, 22);
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


void GuiIcpHashNotice(lv_obj_t *parent, void *totalData)
{
    CreateIcpNoticeCOntainer(
        parent,
        "Notice",
        "Hash signing uses a private key to sign a transaction's hash for integrity and authenticity. You can compare this with the software wallet.",
        360,
        120
    );
}

#endif