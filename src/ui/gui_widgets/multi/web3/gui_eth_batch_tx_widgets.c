#include "gui_eth_batch_tx_widgets.h"
#include "keystore.h"
#include "gui.h"
#include "gui_page.h"
#include "gui_model.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_model.h"
#endif

#define QRCODE_CONFIRM_SIGN_PROCESS 66
#define FINGER_SIGN_MAX_COUNT 5

static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static void *g_parseResult = NULL;
static bool g_isMulti = false;

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_cont;
static KeyboardWidget_t *g_keyboardWidget = NULL;
static lv_obj_t *g_signSlider = NULL;
static uint32_t g_fingerSignCount = FINGER_SIGN_MAX_COUNT;

static void GuiCreatePageContent(lv_obj_t *parent);
static void *GuiParseEthBatchTxData(void);
static void CheckSliderProcessHandler(lv_event_t *e);
static void SignByPasswordCb(bool cancel);

static void OnReturnHandler(lv_event_t *e) {
    GuiCloseToTargetView(&g_homeView);
}

static void GuiEthBatchTxNavBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
}


static uint8_t GetEthPublickeyIndex(char* rootPath)
{
    if (strcmp(rootPath, "44'/60'/0'") == 0) return XPUB_TYPE_ETH_BIP44_STANDARD;
    if (strcmp(rootPath, "44'/60'/1'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_1;
    if (strcmp(rootPath, "44'/60'/2'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_2;
    if (strcmp(rootPath, "44'/60'/3'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_3;
    if (strcmp(rootPath, "44'/60'/4'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_4;
    if (strcmp(rootPath, "44'/60'/5'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_5;
    if (strcmp(rootPath, "44'/60'/6'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_6;
    if (strcmp(rootPath, "44'/60'/7'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_7;
    if (strcmp(rootPath, "44'/60'/8'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_8;
    if (strcmp(rootPath, "44'/60'/9'") == 0) return XPUB_TYPE_ETH_LEDGER_LIVE_9;

    return -1;
}

void GuiSetEthBatchTxData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi) {
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

void GuiEthBatchTxWidgetsInit() {
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    GuiEthBatchTxNavBarInit();
    GuiCreatePageContent(g_cont);
    GuiModelParseTransaction(GuiParseEthBatchTxData);
}

static void GuiCreatePageContent(lv_obj_t *container) {
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style(container, NULL, LV_PART_SCROLLBAR);

    g_signSlider = GuiCreateConfirmSlider(g_pageWidget->contentZone, CheckSliderProcessHandler);
}

static void CheckSliderProcessHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_RELEASED) {
        int32_t value = lv_slider_get_value(lv_event_get_target(e));
        if (value >= QRCODE_CONFIRM_SIGN_PROCESS) {
            if ((GetCurrentAccountIndex() < 3) && GetFingerSignFlag() && g_fingerSignCount < 3) {
                // SignByFinger();
            } else {
                SignByPasswordCb(false);
            }
            lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_OFF);
        } else {
            lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_ON);
        }
    }
}

void GuiEthBatchTxWidgetsSignVerifyPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

void GuiEthBatchTxWidgetsTransactionParseFail() {}

void GuiEthBatchTxWidgetsTransactionParseSuccess() {}

void GuiEthBatchTxWidgetsVerifyPasswordSuccess() {
    printf("GuiEthBatchTxWidgetsVerifyPasswordSuccess\n");
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    uint8_t viewType = EthBatchTx;
    GuiFrameOpenViewWithParam(&g_transactionSignatureView, &viewType, sizeof(viewType));
}

UREncodeResult *GuiGetEthBatchTxSignQrCodeData() {
    UREncodeResult *encodeResult;
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    encodeResult = eth_sign_batch_tx(data, seed, len);
    ClearSecretCache();
    return encodeResult;
}

static void SignByPasswordCb(bool cancel)
{
    // if (cancel) {
    //     FpCancelCurOperate();
    // }
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_SIGN_TRANSACTION_WITH_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void SignByPasswordCbHandler(lv_event_t *e)
{
    SignByPasswordCb(true);
}

static void CloseContHandler(lv_event_t *e)
{
}

void GuiEthBatchTxWidgetsDeInit() {
    GUI_PAGE_DEL(g_pageWidget);
}

void GuiEthBatchTxWidgetsRefresh() {

}

PtrT_TransactionCheckResult GuiGetEthBatchTxCheckResult(void) {
    // uint8_t mfp[4] = {0};
    // void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    // GetMasterFingerPrint(mfp);
    // return eth_check_batch_tx(data, mfp, sizeof(mfp));
}

static void *GuiParseEthBatchTxData(void) {
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    char *ethXpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    return eth_check_then_parse_batch_tx(data, mfp, sizeof(mfp), ethXpub);
}
