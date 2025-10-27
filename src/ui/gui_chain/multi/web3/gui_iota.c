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
static bool g_isSignMessageHash = false;

void GuiSetIotaUrData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
    g_isSignMessageHash = false;
}

#define CHECK_FREE_PARSE_RESULT(result)                                                                                   \
    if (result != NULL)                                                                                                   \
    {                                                                                                                     \
        if (g_isSignMessageHash)                                                                                          \
        {                                                                                                                 \
            free_TransactionParseResult_DisplayIotaSignMessageHash((PtrT_TransactionParseResult_DisplayIotaSignMessageHash)result);                                     \
        } else                                                                                                            \
        {                                                                                                                 \
            free_TransactionParseResult_DisplayIotaIntentData((PtrT_TransactionParseResult_DisplayIotaIntentData)result); \
        }                                                                                                                 \
        result = NULL;                                                                                                    \
    }                                                                                                                     \


void *GuiGetIotaData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayIotaIntentData parseResult = iota_parse_intent(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

void *GuiGetIotaSignMessageHashData(void)
{
    CHECK_FREE_PARSE_RESULT(g_parseResult);
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        PtrT_TransactionParseResult_DisplayIotaSignMessageHash parseResult = iota_parse_sign_message_hash(data);
        CHECK_CHAIN_BREAK(parseResult);
        g_isSignMessageHash = true;
        g_parseResult = (void *)parseResult;
    } while (0);
    return g_parseResult;
}

PtrT_TransactionCheckResult GuiGetIotaCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    PtrT_TransactionCheckResult result = sui_check_request(data, mfp, sizeof(mfp));
    return result;
}

PtrT_TransactionCheckResult GuiGetIotaSignHashCheckResult(void)
{
    uint8_t mfp[4];
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    GetMasterFingerPrint(mfp);
    return sui_check_sign_hash_request(data, mfp, sizeof(mfp));
}

void FreeIotaMemory(void)
{
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    CHECK_FREE_PARSE_RESULT(g_parseResult);
}

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
        encodeResult = iota_sign_intent(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}

bool GetIotaIsTransaction(void *indata, void *param)
{
    return !GetIotaIsMessage(indata, param);
}

bool GetIotaIsTransfer(void *indata, void *param)
{
    DisplayIotaIntentData *iota = (DisplayIotaIntentData *)param;
    if (iota->method != NULL) {
        return false;
    }
    return true;
}

bool GetIotaIsMessage(void *indata, void *param)
{
    DisplayIotaIntentData *iota = (DisplayIotaIntentData *)param;
    return !strcmp(iota->transaction_type, "Message");
}

void GuiIotaTxOverviewMessage(lv_obj_t *parent, void *totalData)
{
    DisplayIotaIntentData *txData = (DisplayIotaIntentData *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_t *container = CreateSingleInfoTwoLineView(parent, "address", txData->sender);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 0);

    container = CreateSingleInfoTwoLineView(parent, "Message", txData->message);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

void GuiIotaTxOverview(lv_obj_t *parent, void *totalData)
{
    if (GetIotaIsMessage(NULL, totalData)) {
        GuiIotaTxOverviewMessage(parent, totalData);
        return;
    }
    DisplayIotaIntentData *txData = (DisplayIotaIntentData *)totalData;
    lv_obj_set_size(parent, 408, 444);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *container = NULL;
    if (txData->amount != NULL) {
        if (strcmp(txData->amount, "max") == 0) {
            container = CreateNoticeView(parent, 408, 212, _("iota_max_amount_notice"));
            lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 0);
        } else {
            container = CreateValueOverviewValue(parent, "amount", txData->amount, NULL, NULL);
            lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 0);
        }
    }

    if (txData->network != NULL) {
        container = CreateSingleInfoView(parent, "network", txData->network);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }

    if (txData->method != NULL) {
        container = CreateSingleInfoView(parent, "Method", txData->method);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }

    container = CreateSingleInfoTwoLineView(parent, "sender", txData->sender);
    GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    if (txData->recipient != NULL) {
        container = CreateSingleInfoTwoLineView(parent, "recipient", txData->recipient);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }

    if (txData->to != NULL) {
        container = CreateSingleInfoTwoLineView(parent, "to", txData->to);
        GuiAlignToPrevObj(container, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    }
}

void GuiIotaTxRawData(lv_obj_t *parent, void *totalData)
{
    DisplayIotaIntentData *txData = (DisplayIotaIntentData *)totalData;
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_size(parent, 408, 444);

    lv_obj_t *container = CreateContentContainer(parent, 408, 444);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *rawDataLabel = GuiCreateIllustrateLabel(container, txData->details);
    lv_obj_set_width(rawDataLabel, 360);
    lv_obj_align(rawDataLabel, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_pad_bottom(rawDataLabel, 16, LV_PART_MAIN);
}

UREncodeResult *GuiGetIotaSignHashQrCodeData(void)
{
    bool enable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    do {
        uint8_t seed[64];
        GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encodeResult = iota_sign_hash(data, seed, len);
        ClearSecretCache();
        CHECK_CHAIN_BREAK(encodeResult);
    } while (0);
    SetLockScreen(enable);
    return encodeResult;
}