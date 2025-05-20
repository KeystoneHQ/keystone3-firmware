#include "gui_eth_batch_tx_widgets.h"
#include "keystore.h"
#include "gui.h"
#include "gui_page.h"
#include "gui_model.h"
#include "rust.h"
#include "gui_chain_components.h"
#include "gui_obj.h"
#include "gui_eth.h"
#include "gui_qr_hintbox.h"
#include "fingerprint_process.h"
#include "gui_button.h"
#include "gui_keyboard_hintbox.h"

#ifndef COMPILE_SIMULATOR
#include "keystore.h"

#else
#define FP_SUCCESS_CODE 0
#define RECOGNIZE_UNLOCK 0
#define RECOGNIZE_OPEN_SIGN 1
#define RECOGNIZE_SIGN 2

#define NO_ENCRYPTION 0
#define AES_KEY_ENCRYPTION 1
#define RESET_AES_KEY_ENCRYPTION 2
#define FINGERPRINT_EN_SING_ERR_TIMES (5)
#define FINGERPRINT_RESPONSE_MSG_LEN (23)
#define FINGERPRINT_RESPONSE_DEFAULT_TIMEOUT (0xFF)
#define FINGERPRINT_SING_ERR_TIMES (3)
#define FINGERPRINT_SING_DISABLE_ERR_TIMES (15)
#endif

#ifdef COMPILE_SIMULATOR
#include "simulator_model.h"
#endif

#include "abi_ethereum.h"

#define QRCODE_CONFIRM_SIGN_PROCESS 66
#define FINGER_SIGN_MAX_COUNT 5

static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static bool g_isMulti = false;

static uint32_t g_currentTxIndex = 0;
static uint32_t g_txCount = 0;

static TransactionParseResult_DisplayETHBatchTx *g_parseResult = NULL;
//don't need to free g_currentTransaction and g_displayEthBatchTx, since they are freed in g_parseResult
static DisplayETHBatchTx *g_displayEthBatchTx = NULL;
static DisplayETH *g_currentTransaction = NULL;

static Erc20Contract_t *g_currentErc20Contract = NULL;
static EvmNetwork_t g_currentNetwork;
static Response_EthParsedErc20Approval *g_parseErc20Approval = NULL;
static Response_DisplaySwapkitContractData *g_swapkitContractData = NULL;
static Response_DisplayContractData *g_contractData = NULL;

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_cont;
static lv_obj_t *g_txContainer = NULL;
static lv_obj_t *g_overviewContainer = NULL;
static lv_obj_t *g_detailContainer = NULL;
static lv_obj_t *g_parseErrorHintBox = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;
static lv_obj_t *g_bottomBtnContainer = NULL;
static lv_obj_t *g_signSlider = NULL;

static lv_obj_t *g_fingerSingContainer = NULL;
static lv_obj_t *g_fpErrorImg = NULL;
static lv_obj_t *g_fpErrorLabel = NULL;
static uint32_t g_fingerSignCount = FINGER_SIGN_MAX_COUNT;
static uint32_t g_fingerSignErrCount = 0;
static lv_timer_t *g_fpRecognizeTimer;

//GUI Render Functions
static void GuiCreatePageContent(lv_obj_t *parent);
static void *GuiParseEthBatchTxData(void);
static void CheckSliderProcessHandler(lv_event_t *e);
static void SignByPasswordCb(bool cancel);
static void GuiEthBatchTxNavBarInit();
static void GuiEthBatchTxNavBarRefresh();
static void GuiRenderCurrentTransaction(bool showSwapHint, bool showSignSlider);
static void GuiRenderTransactionFrame(lv_obj_t *parent);
static void GuiRenderBottomBtn(lv_obj_t *parent, bool showSignSlider);

static bool HandleCurrentTransaction(uint32_t index);
static void HandleCurrentTransactionParseFail(uint32_t errorCode, const char *errorMessage);

//GUI Event Handlers
static void HandleClickPreviousBtn(lv_event_t *e);
static void HandleClickNextBtn(lv_event_t *e);
static void HandleClickAddressChecker(lv_event_t *e);

static void ClearPageData() {
    g_currentTxIndex = 0;
    g_txCount = 0;
    g_currentErc20Contract = NULL;

    if(g_parseResult != NULL) {
        free_TransactionParseResult_DisplayETHBatchTx(g_parseResult);
        g_parseResult = NULL;
    }

    if(g_parseErc20Approval!=NULL) {
        free_Response_EthParsedErc20Approval(g_parseErc20Approval);
        g_parseErc20Approval = NULL;
    }

    if(g_swapkitContractData != NULL) {
        free_Response_DisplaySwapkitContractData(g_swapkitContractData);
        g_swapkitContractData = NULL;
    }

    if(g_contractData != NULL) {
        free_Response_DisplayContractData(g_contractData);
        g_contractData = NULL;
    }

    if(g_isMulti) {
        free_ur_parse_multi_result((PtrT_URParseMultiResult)g_urMultiResult);
        g_urMultiResult = NULL;
    }
    else {
        free_ur_parse_result((PtrT_URParseResult)g_urResult);
        g_urResult = NULL;
    }
}

void GuiSetEthBatchTxData(URParseResult *urResult, URParseMultiResult *urMultiResult, bool multi) {
    g_urResult = urResult;
    g_urMultiResult = urMultiResult;
    g_isMulti = multi;
}

void GuiEthBatchTxWidgetsVerifyPasswordSuccess() {
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
    if (cancel) {
        FpCancelCurOperate();
    }
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_SIGN_TRANSACTION_WITH_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void HandleClickPreviousBtn(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        if(g_currentTxIndex > 0) {
            g_currentTxIndex--;
            GuiEthBatchTxWidgetsRefresh();
        }
    }
}

static void HandleClickNextBtn(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        if(g_currentTxIndex < g_txCount - 1) {
            g_currentTxIndex++;
            GuiEthBatchTxWidgetsRefresh();
        }
    }
}

static void HandleClickAddressChecker(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        char *address = lv_event_get_user_data(e);
        char *text = malloc(BUFFER_SIZE_128);
        sprintf(text, "https://etherscan.io/address/%s", address);
        GuiQRCodeHintBoxOpen(text, _("Check the Address"), text);
    }
}
static void SignByPasswordCbHandler(lv_event_t *e)
{
    SignByPasswordCb(true);
}

static void CloseContHandler(lv_event_t *e)
{
}

void GuiEthBatchTxWidgetsDeInit() {
    GUI_DEL_OBJ(g_fingerSingContainer)
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GUI_PAGE_DEL(g_pageWidget);
    ClearPageData();
}

static void ParseSwapContractData(const char* address, const char* inputData) {
    char selectorId[9] = {0};
    strncpy(selectorId, inputData, 8);
    char* address_key = (char*)SRAM_MALLOC(strlen(address) + 10);
    strcpy_s(address_key, strlen(address) + 10, address);
    strcat_s(address_key, strlen(address) + 10, "_");
    strcat_s(address_key, strlen(address) + 10, selectorId);
    for (size_t i = 0; i < GetEthereumABIMapSize(); i++) {
        struct ABIItem item = ethereum_abi_map[i];
        if (strcasecmp(item.address, address_key) == 0) {
            g_swapkitContractData = eth_parse_swapkit_contract(inputData, (char *)item.json);
            g_contractData = eth_parse_contract_data(inputData, (char *)item.json);
            if (g_swapkitContractData->error_code == 0 && g_contractData->error_code == 0) {
                return;
            } else {
                if (g_swapkitContractData->error_code != 0) {
                    HandleCurrentTransactionParseFail(g_swapkitContractData->error_code, g_swapkitContractData->error_message);
                    free_Response_DisplaySwapkitContractData(g_swapkitContractData);
                }
                if (g_contractData->error_code != 0) {
                    HandleCurrentTransactionParseFail(g_contractData->error_code, g_contractData->error_message);
                    free_Response_DisplayContractData(g_contractData);
                }
                return;
            }
        }
    }
}

static void ParseErc20ContractData(const char* inputData, const uint8_t decimals) {
    g_parseErc20Approval = eth_parse_erc20_approval(inputData, decimals);
    g_contractData = eth_parse_contract_data(inputData, (char *)ethereum_erc20_json);
    if(g_parseErc20Approval->error_code != 0 || g_contractData->error_code != 0) {
        if (g_parseErc20Approval->error_code != 0) {
            HandleCurrentTransactionParseFail(g_parseErc20Approval->error_code, g_parseErc20Approval->error_message);
        }
        if (g_contractData->error_code != 0) {
            HandleCurrentTransactionParseFail(g_contractData->error_code, g_contractData->error_message);
        }
        return;
    }
}

void GuiEthBatchTxWidgetsRefresh() {
    if(g_parseResult -> error_code == 0) {
        GuiEthBatchTxNavBarRefresh();
        if(!HandleCurrentTransaction(g_currentTxIndex)) {
            return;
        }

        bool showSwapHint = g_currentTxIndex == 0 && g_txCount > 1;
        bool showSignSlider = g_currentTxIndex == g_txCount - 1;

        GuiRenderCurrentTransaction(showSwapHint, showSignSlider);
    }
}

static void GuiReturnHome() {
    GuiCloseToTargetView(&g_homeView);
}

static void OnReturnHandler(lv_event_t *e) {
    GuiReturnHome();
}

static void *GuiParseEthBatchTxData(void) {
    void *data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);

    char *ethXpub = GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD);
    PtrT_TransactionParseResult_DisplayETHBatchTx result = eth_check_then_parse_batch_tx(data, mfp, sizeof(mfp), ethXpub);
    g_parseResult = result;
    return g_parseResult;
}

static void SignByFinger(void)
{
    GUI_DEL_OBJ(g_fingerSingContainer)

    g_fingerSingContainer = GuiCreateHintBox(428);
    lv_obj_t *cont = g_fingerSingContainer;
    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("scan_qr_code_sign_fingerprint_verify_fingerprint"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 402);

    lv_obj_t *button = GuiCreateImgButton(cont, &imgClose, 64, CloseContHandler, cont);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 384, 394);

    g_fpErrorImg = GuiCreateImg(cont, &imgYellowFinger);
    lv_obj_align(g_fpErrorImg, LV_ALIGN_BOTTOM_MID, 0, -178);

    lv_obj_t *arc = GuiCreateArc(cont);
    lv_obj_set_style_arc_opa(arc, LV_OPA_10, LV_PART_MAIN);
    lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, -154);

    g_fpErrorLabel = GuiCreateIllustrateLabel(cont, _("scan_qr_code_sign_unsigned_content_fingerprint_failed_desc"));
    lv_obj_set_style_text_color(g_fpErrorLabel, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(g_fpErrorLabel, LV_ALIGN_BOTTOM_MID, 0, -100);
    lv_obj_add_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);

    button = GuiCreateImgLabelAdaptButton(cont, _("enter_passcode"), &imgLockedLock, SignByPasswordCbHandler, cont);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -27);
    FpRecognize(RECOGNIZE_SIGN);
}

static void CheckSliderProcessHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_RELEASED) {
        int32_t value = lv_slider_get_value(lv_event_get_target(e));
        if (value >= QRCODE_CONFIRM_SIGN_PROCESS) {
            if ((GetCurrentAccountIndex() < 3) && GetFingerSignFlag() && g_fingerSignCount < 3) {
                SignByFinger();
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

static void RecognizeFailHandler(lv_timer_t *timer)
{
    if (g_fingerSingContainer != NULL) {
        lv_img_set_src(g_fpErrorImg, &imgYellowFinger);
        lv_obj_add_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);
    }
    lv_timer_del(timer);
    g_fpRecognizeTimer = NULL;
}

void GuiEthBatchTxWidgetsSignDealFingerRecognize(void *param)
{
    uint8_t errCode = *(uint8_t *)param;
    static uint16_t passCodeType = SIG_SIGN_TRANSACTION_WITH_PASSWORD;
    if (g_fingerSingContainer == NULL) {
        return;
    }
    if (errCode == FP_SUCCESS_CODE) {
        lv_img_set_src(g_fpErrorImg, &imgYellowFinger);
        GuiModelVerifyAccountPassWord(&passCodeType);
        g_fingerSignErrCount = 0;
    } else {
        g_fingerSignErrCount++;
        g_fingerSignCount++;
        if (g_fpErrorLabel != NULL && lv_obj_has_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(g_fpErrorLabel, LV_OBJ_FLAG_HIDDEN);
        }
        lv_img_set_src(g_fpErrorImg, &imgRedFinger);
        printf("g_fingerSingCount is %d\n", g_fingerSignCount);
        if (g_fingerSignCount < FINGERPRINT_SING_ERR_TIMES) {
            FpRecognize(RECOGNIZE_SIGN);
            g_fpRecognizeTimer = lv_timer_create(RecognizeFailHandler, 1000, NULL);
        } else {
            SignByPasswordCb(false);
        }
        printf("g_fingerSignErrCount.... = %d\n", g_fingerSignErrCount);
        if (g_fingerSignErrCount >= FINGERPRINT_SING_DISABLE_ERR_TIMES) {
            for (int i = 0; i < 3; i++) {
                UpdateFingerSignFlag(i, false);
            }
        }
    }
}

void GuiEthBatchTxWidgetsTransactionParseFail(void* params) {
    printf("GuiEthBatchTxWidgetsTransactionParseFail\n");
    printf("error: %s\n", g_parseResult->error_message);
    g_parseErrorHintBox = GuiCreateErrorCodeWindow(ERR_INVALID_QRCODE, &g_parseErrorHintBox, GuiReturnHome);
}

static void HandleCurrentTransactionParseFail(uint32_t errorCode, const char *errorMessage) {
    printf("error: %s\n", errorMessage);
}

static bool HandleCurrentTransaction(uint32_t index) {
    //clear previous data
    if(g_contractData != NULL) {
        free_Response_DisplayContractData(g_contractData);
        g_contractData = NULL;
    }
    if(g_swapkitContractData != NULL) {
        free_Response_DisplaySwapkitContractData(g_swapkitContractData);
        g_swapkitContractData = NULL;
    }
    if(g_parseErc20Approval != NULL) {
        free_Response_EthParsedErc20Approval(g_parseErc20Approval);
        g_parseErc20Approval = NULL;
    }

    g_currentTransaction = &g_displayEthBatchTx->txs->data[g_currentTxIndex];
    g_currentNetwork = FindEvmNetwork(g_currentTransaction->chain_id);
    g_currentErc20Contract = FindErc20Contract(g_currentTransaction->overview->to);
    if(g_currentErc20Contract == NULL) {
        ParseSwapContractData(g_currentTransaction->overview->to, g_currentTransaction->detail->input);
        // even if parse failed, we should render the general transaction info
        // if( g_swapkitContractData->error_code != 0 || g_contractData->error_code != 0) {
        //     return false;
        // }
        return true;
    }
    else {
        ParseErc20ContractData(g_currentTransaction->detail->input, g_currentErc20Contract->decimals);
        // this case should not happen
        if(g_parseErc20Approval->error_code != 0 || g_contractData->error_code != 0) {
            return false;
        }
    }
    return true;
}

void GuiEthBatchTxWidgetsTransactionParseSuccess() {
    g_displayEthBatchTx = g_parseResult->data;
    g_txCount = g_displayEthBatchTx->txs->size;

    g_currentTxIndex = 0;
    GuiEthBatchTxWidgetsRefresh();
}

void GuiEthBatchTxWidgetsInit() {
    g_pageWidget = NULL;
    g_cont = NULL;
    g_txContainer = NULL;
    g_overviewContainer = NULL;
    g_detailContainer = NULL;
    g_parseErrorHintBox = NULL;
    g_keyboardWidget = NULL;
    g_bottomBtnContainer = NULL;
    g_signSlider = NULL;

    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    GuiEthBatchTxNavBarInit();
    GuiCreatePageContent(g_cont);
    GuiEmitSignal(SIG_SHOW_TRANSACTION_LOADING, NULL, 0);
    GuiModelParseTransaction(GuiParseEthBatchTxData);
}

static void GuiCreatePageContent(lv_obj_t *container) {
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style(container, NULL, LV_PART_SCROLLBAR);
}

static void GuiEthBatchTxNavBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
}

static void GuiEthBatchTxNavBarRefresh() {
    char* text = malloc(BUFFER_SIZE_128);
    if(g_txCount > 1) {
        sprintf(text, "%s (%d/%d)", _("confirm_transaction"), g_currentTxIndex + 1, g_txCount);
    }
    else {
        sprintf(text, "%s", _("confirm_transaction"));
    }
    SetCoinWallet(g_pageWidget->navBarWidget, CHAIN_ETH, text);
    if(g_currentTxIndex == 0) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
    }
    else {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, HandleClickPreviousBtn, NULL);
    }
}

// GUI Impelementation Part
static lv_obj_t* GuiRenderSwapSummary(lv_obj_t *parent, const char* from_asset, const char* from_amount, const char* to_asset) {
    char* text = malloc(BUFFER_SIZE_128);
    sprintf(text, "%s %s", from_amount, from_asset);
    return CreateTransactionOvewviewCard(parent, _("Swap"), text, _("To"), to_asset);
}

static lv_obj_t *GuiRenderUnknownErc20SwapSummary(lv_obj_t *parent, const char* from_asset, const char* from_amount, const char* to_asset) {
    lv_obj_t *last_view = NULL;
    last_view = CreateTransactionItemView(parent, _("Operation"), "Swap", last_view);
    last_view = CreateTransactionItemView(parent, _("From"), from_asset, last_view);
    last_view = CreateTransactionItemView(parent, _("Value"), from_amount, last_view);
    last_view = CreateTransactionItemView(parent, _("To"), to_asset, last_view);
    return last_view;
}

static lv_obj_t *GuiRenderApprove(lv_obj_t *parent, const bool showAmount, lv_obj_t *last_view) {
    char* text = malloc(BUFFER_SIZE_32);
    if(showAmount) {
        text = "Approve";
    }
    else {
        text = "Revoke";
    }
    last_view = CreateTransactionItemView(parent, _("Operation"), text, last_view);

    lv_obj_t *container = CreateTransactionContentContainer(parent, 408, 290);

    lv_obj_align_to(container, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t *label;

    label = GuiCreateIllustrateLabel(container, _("Network"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, g_currentNetwork.name);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);

    if(showAmount) {
        label = GuiCreateIllustrateLabel(container, _("Amount"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 100);
        lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

        char* amount = malloc(BUFFER_SIZE_64);
        sprintf(amount, "%s %s", g_parseErc20Approval->data->value, g_currentErc20Contract->symbol);

        label = GuiCreateIllustrateLabel(container, amount);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 138);
    }
    else {
        label = GuiCreateIllustrateLabel(container, _("Token"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 100);
        lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(container, g_currentErc20Contract->symbol);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 138);
    }

    label = GuiCreateIllustrateLabel(container, _("Spender"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 184);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, g_parseErc20Approval->data->spender);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 222);
    lv_obj_set_width(label, 360);

    return container;
}

static void GuiRenderApprovalOverview(lv_obj_t *parent, bool showSwapHint) {
    lv_obj_t *last_view = NULL;
    if(showSwapHint) {
        last_view = CreateNoticeCard(parent, _("swap_token_approve_hint"));
    }
    bool showAmount = false;
    if(strcmp(g_parseErc20Approval->data->value, "0") != 0) {
        showAmount = true;
    }
    char* text = malloc(BUFFER_SIZE_32);
    if(showAmount) {
        text = "Approve";
    }
    else {
        text = "Revoke";
    }
    last_view = CreateTransactionItemView(parent, _("Operation"), text, last_view);

    lv_obj_t *container = CreateRelativeTransactionContentContainer(parent, 408, 290, last_view);

    lv_obj_t *label;

    label = GuiCreateIllustrateLabel(container, _("Network"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, g_currentNetwork.name);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 54);

    if(showAmount) {
        label = GuiCreateIllustrateLabel(container, _("Amount"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 100);
        lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

        char* amount = malloc(BUFFER_SIZE_64);
        sprintf(amount, "%s %s", g_parseErc20Approval->data->value, g_currentErc20Contract->symbol);

        label = GuiCreateIllustrateLabel(container, amount);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 138);
    }
    else {
        label = GuiCreateIllustrateLabel(container, _("Token"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 100);
        lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(container, g_currentErc20Contract->symbol);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 138);
    }

    label = GuiCreateIllustrateLabel(container, _("Spender"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 184);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, g_parseErc20Approval->data->spender);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 222);
    lv_obj_set_width(label, 360);
}

static lv_obj_t *GuiRenderFromToCard(lv_obj_t *parent, const char* title, const char* from, const char* title2, const char* to, const char* to_badge, lv_obj_t *last_view) {
    uint16_t height = 16; //top padding
    lv_obj_t *container = CreateRelativeTransactionContentContainer(parent, 408, 0, last_view);

    lv_obj_t *label;
    label = GuiCreateIllustrateLabel(container, title);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    height += 30 + 8;

    label = GuiCreateIllustrateLabel(container, from);
    lv_obj_set_width(label, 360);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);

    height += 60 + 8;

    label = GuiCreateIllustrateLabel(container, title2);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    height += 30 + 8;

    label = GuiCreateIllustrateLabel(container, to);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_width(label, 360);

    height += 60 + 8;

    if (to_badge != NULL) {
        lv_obj_t *img;

        img = GuiCreateImg(container, &imgContract);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 24, height);

        label = GuiCreateIllustrateLabel(container, to_badge);

        lv_obj_set_style_text_color(label, PURPLE_COLOR, LV_PART_MAIN);
        lv_obj_align_to(label, img, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
        height += 30 + 8;
    }

    height += 8;

    lv_obj_set_height(container, height);
    lv_obj_update_layout(container);

    return container;
}

static void GuiRenderSwapOverview(lv_obj_t *parent) {
    lv_obj_t *last_view = NULL;
    Erc20Contract_t *erc20Contract = FindErc20Contract(g_swapkitContractData->data->swap_in_asset);
    bool is_eth = strcmp(g_swapkitContractData->data->swap_in_asset, "0x0000000000000000000000000000000000000000") == 0;
    if (is_eth) {
        erc20Contract = malloc(sizeof(Erc20Contract_t));
        erc20Contract->symbol = "ETH";
        erc20Contract->decimals = 18;
    }
    if(erc20Contract != NULL) {
        //is known erc20 token
        SimpleResponse_c_char *response = format_value_with_decimals(g_swapkitContractData->data->swap_in_amount, erc20Contract->decimals);
        if(response->error_code == 0) {
            char *amount = response->data;
            last_view = GuiRenderSwapSummary(parent, erc20Contract->symbol, amount, g_swapkitContractData->data->swap_out_asset);
        }
        else {
            //this should not happen
            printf("format_value_with_decimals failed\n");
        }
    }
    else {
        last_view = GuiRenderUnknownErc20SwapSummary(parent, g_swapkitContractData->data->swap_in_asset, g_swapkitContractData->data->swap_in_amount, g_swapkitContractData->data->swap_out_asset);
    }
    if(erc20Contract != NULL) {
        free(erc20Contract);
        erc20Contract = NULL;
    }
    last_view = CreateTransactionItemView(parent, _("Network"), g_currentNetwork.name, last_view);

    char* to_badge = NULL;
    if(g_swapkitContractData->data->swap_out_asset_contract_address != NULL) {
        erc20Contract = FindErc20Contract(g_swapkitContractData->data->swap_out_asset_contract_address);
        if(erc20Contract != NULL) {
            to_badge = erc20Contract->symbol;
        }
    }

    last_view = GuiRenderFromToCard(parent, _("From"), g_currentTransaction->overview->from, _("Contract Address"), g_swapkitContractData->data->swap_out_asset_contract_address, to_badge, last_view);

    last_view = CreateTransactionItemView(parent, _("Destination"), g_swapkitContractData->data->receive_address, last_view);

    if(erc20Contract != NULL) {
        free(erc20Contract);
        erc20Contract = NULL;
    }
}

static void GuiRenderGeneralOverview(lv_obj_t *parent) {
    lv_obj_t *last_view = NULL;

    last_view = CreateTransactionOvewviewCard(parent, _("Value"), g_currentTransaction->overview->value, _("Max Txn Fee"), g_currentTransaction->overview->max_txn_fee);

    last_view = CreateTransactionItemView(parent, _("Network"), g_currentNetwork.name, last_view);

    char* to_badge = NULL;
    if(g_contractData != NULL && g_contractData->error_code == 0) {
        last_view = CreateTransactionItemView(parent, _("Method"), g_contractData->data->method_name, last_view);
        to_badge = g_contractData->data->contract_name;
    }

    last_view = GuiRenderFromToCard(parent, _("From"), g_currentTransaction->overview->from, _("To"), g_currentTransaction->overview->to, to_badge, last_view);
}

static void GuiRenderOverview(lv_obj_t *parent, bool showSwapHint) {
    lv_obj_t *last_view = NULL;
    bool isApprove = g_parseErc20Approval != NULL;
    bool isSwap = g_swapkitContractData != NULL && g_swapkitContractData->error_code == 0;
    if(isApprove) {
        GuiRenderApprovalOverview(parent, showSwapHint);
    }
    else if (isSwap) {
        GuiRenderSwapOverview(parent);
    }
    else {
        //is general transaction
        GuiRenderGeneralOverview(parent);
    }
}

static lv_obj_t *GuiRenderDetailTransactionInfoCard(lv_obj_t *parent, lv_obj_t *last_view) {
    uint16_t height = 16 ; //top padding
    lv_obj_t *container = CreateRelativeTransactionContentContainer(parent, 408, 0, last_view);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 4);

    lv_obj_t *titleLabel, *valueLabel;

    titleLabel = GuiCreateIllustrateLabel(container, _("Value"));
    lv_obj_set_style_text_opa(titleLabel, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

    char* value = malloc(BUFFER_SIZE_32);
    sprintf(value, "%s %s", g_currentTransaction->detail->value, g_currentNetwork.symbol);

    valueLabel = GuiCreateIllustrateLabel(container, value);
    lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    lv_obj_set_style_text_color(valueLabel, ORANGE_COLOR, LV_PART_MAIN);

    height += 30 + 8;

    titleLabel = GuiCreateIllustrateLabel(container, _("Max Fee"));
    lv_obj_set_style_text_opa(titleLabel, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

    valueLabel = GuiCreateIllustrateLabel(container, g_currentTransaction->detail->max_txn_fee);
    lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    height += 30 + 8;

    titleLabel = GuiCreateIllustrateLabel(container, "  \xE2\x80\xA2  Max Fee Price * Gas Limit");
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

    height += 30 + 8;

    if(g_currentTransaction->detail->max_priority != NULL) {
        titleLabel = GuiCreateIllustrateLabel(container, _("Max Priority"));
        lv_obj_set_style_text_opa(titleLabel, LV_OPA_64, LV_PART_MAIN);
        lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

        valueLabel = GuiCreateIllustrateLabel(container, g_currentTransaction->detail->max_priority);
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

        height += 30 + 8;

        titleLabel = GuiCreateIllustrateLabel(container, "  \xE2\x80\xA2  Max Priority Fee Price * Gas Limit");
        lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

        height += 30 + 8;
    }

    if(g_currentTransaction->detail->max_fee_price != NULL) {
        titleLabel = GuiCreateIllustrateLabel(container, _("Max Fee Price"));
        lv_obj_set_style_text_opa(titleLabel, LV_OPA_64, LV_PART_MAIN);
        lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

        valueLabel = GuiCreateIllustrateLabel(container, g_currentTransaction->detail->max_fee_price);
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

        height += 30 + 8;
    }

    if(g_currentTransaction->detail->max_priority_price != NULL) {
        titleLabel = GuiCreateIllustrateLabel(container, _("Max Priority Fee Price"));
        lv_obj_set_style_text_opa(titleLabel, LV_OPA_64, LV_PART_MAIN);
        lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

        valueLabel = GuiCreateIllustrateLabel(container, g_currentTransaction->detail->max_priority_price);
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

        height += 30 + 8;
    }

    if(g_currentTransaction->detail->gas_price != NULL) {
        titleLabel = GuiCreateIllustrateLabel(container, _("Gas Price"));
        lv_obj_set_style_text_opa(titleLabel, LV_OPA_64, LV_PART_MAIN);
        lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

        valueLabel = GuiCreateIllustrateLabel(container, g_currentTransaction->detail->gas_price);
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

        height += 30 + 8;
    }

    titleLabel = GuiCreateIllustrateLabel(container, _("Gas Limit"));
    lv_obj_set_style_text_opa(titleLabel, LV_OPA_64, LV_PART_MAIN);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 24, height);

    valueLabel = GuiCreateIllustrateLabel(container, g_currentTransaction->detail->gas_limit);
    lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    height += 30;

    height += 16; //bottom padding

    lv_obj_set_height(container, height);
    lv_obj_update_layout(container);

    return container;
}

static lv_obj_t *GuiRenderDetailFromTo(lv_obj_t *parent, lv_obj_t *last_view) {
    char* to_badge = NULL;

    if(g_currentErc20Contract != NULL) {
        to_badge = g_currentErc20Contract->symbol;
    }
    else if (g_contractData != NULL && g_contractData->error_code == 0) {
        to_badge = g_contractData->data->contract_name;
    }

    return GuiRenderFromToCard(parent, _("From"), g_currentTransaction->detail->from, _("To"), g_currentTransaction->detail->to, to_badge, last_view);
}

static lv_obj_t *GuiRenderDetailContractData(lv_obj_t *parent, lv_obj_t *last_view) {
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("Input Data"));
    lv_obj_align_to(label, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    lv_obj_t *container = CreateRelativeTransactionContentContainer(parent, 408, 0, label);

    uint16_t height = 16;

    for(int i = 0; i < g_contractData->data->params->size ; i++) {
        DisplayContractParam param = g_contractData->data->params->data[i];
        lv_obj_t *label = GuiCreateIllustrateLabel(container, param.name);
        lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
        height += 30 + 8;

        bool showAddressChecker = false;

        //handle address case
        if(strcmp(param.param_type, "address") == 0) {
            bool asset_is_eth = strcmp(param.value, "0x0000000000000000000000000000000000000000") == 0;
            Erc20Contract_t *erc20Contract = FindErc20Contract(param.value);
            char* text = malloc(BUFFER_SIZE_64);
            if(erc20Contract != NULL) {
                sprintf(text, "%s (#1BE0C6 %s#)", param.value, erc20Contract->symbol);
                showAddressChecker = true;
            }
            else if (asset_is_eth) {
                sprintf(text, "%s (#F5870A %s#)", param.value, g_currentNetwork.symbol);
                showAddressChecker = false;
            }
            else {
                sprintf(text, "%s", param.value);
                showAddressChecker = true;
            }
            label = GuiCreateIllustrateLabel(container, text);
            lv_label_set_recolor(label, true);
        }
        else {
            label = GuiCreateIllustrateLabel(container, param.value);
        }

        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
        lv_obj_set_width(label, 360);
        lv_obj_update_layout(label);

        uint16_t labelHeight = lv_obj_get_height(label);
        height += labelHeight + 8;

        if(showAddressChecker) {
            lv_obj_t *cont = GuiCreateContainerWithParent(container, 360, 30);
            lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_MAIN);
            lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 24, height);

            label = GuiCreateIllustrateLabel(cont, _("Check the Address"));
            lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
            lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);

            lv_obj_t *img;
            img = GuiCreateImg(cont, &imgQrcodeTurquoise);
            lv_obj_align_to(img, label, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

            lv_obj_add_event_cb(cont, HandleClickAddressChecker, LV_EVENT_CLICKED, param.value);

            height += 30 + 8;
        }
    }
    height += 8;

    lv_obj_set_height(container, height);
    lv_obj_update_layout(container);

    return container;
}

static lv_obj_t *GuiRenderDetailInputData(lv_obj_t *parent, lv_obj_t *last_view) {
    lv_obj_t *container = CreateRelativeTransactionContentContainer(parent, 408, 0, last_view);

    uint16_t height = 16;

    char* input_data = malloc(BUFFER_SIZE_128);

    if (strlen(g_currentTransaction->detail->input) > 51) {
        char data[49];
        strncpy(data, g_currentTransaction->detail->input, 48);
        data[48] = '\0';
        snprintf_s(input_data, BUFFER_SIZE_128, "0x%s...", data);
    } else {
        snprintf_s(input_data, BUFFER_SIZE_128, "0x%s", g_currentTransaction->detail->input);
    }

    lv_obj_t *label = GuiCreateIllustrateLabel(container, input_data);
    lv_obj_align_to(label, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, height);
    lv_obj_update_layout(label);
    height += lv_obj_get_height(label) + 8;

    label = GuiCreateIllustrateLabel(container, _("Unknown Data"));
    lv_obj_align_to(label, last_view, LV_ALIGN_OUT_BOTTOM_LEFT, 0, height);
    lv_obj_set_style_text_color(label, YELLOW_COLOR, LV_PART_MAIN);
    lv_obj_update_layout(label);
    height += lv_obj_get_height(label);

    height += 16;
    lv_obj_set_height(container, height);
    lv_obj_update_layout(container);

    return container;
}

static void GuiRenderDetail(lv_obj_t *parent) {
    lv_obj_t *last_view = NULL;
    last_view = GuiRenderDetailTransactionInfoCard(parent, last_view);

    last_view = CreateTransactionItemView(parent, _("Network"), g_currentNetwork.name, last_view);

    if(g_contractData != NULL && g_contractData->error_code == 0) {
        last_view = CreateTransactionItemView(parent, _("Method"), g_contractData->data->method_name, last_view);
    }

    last_view = GuiRenderDetailFromTo(parent, last_view);

    if(g_contractData != NULL && g_contractData->error_code == 0) {
        last_view = GuiRenderDetailContractData(parent, last_view);
    }
    else {
        last_view = GuiRenderDetailInputData(parent, last_view);
    }
}

static void GuiRenderTransactionFrame(lv_obj_t *parent) {
    lv_obj_t *tabView = lv_tabview_create(parent, LV_DIR_TOP, 64);
    lv_obj_set_style_bg_color(tabView, lv_color_hex(0x0), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tabView, lv_color_hex(0x0), LV_PART_ITEMS);
    lv_obj_set_style_border_color(tabView, lv_color_hex(0), LV_PART_ITEMS);

    lv_obj_clear_flag(tabView, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(tabView, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    line = (lv_obj_t *)GuiCreateLine(parent, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, 64);

    g_overviewContainer = lv_tabview_add_tab(tabView, _("Overview"));
    lv_obj_set_scrollbar_mode(g_overviewContainer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_overviewContainer, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_style_pad_all(g_overviewContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_overviewContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_overviewContainer, 0, LV_PART_MAIN);

    g_detailContainer = lv_tabview_add_tab(tabView, _("Details"));
    lv_obj_set_scrollbar_mode(g_detailContainer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_detailContainer, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_style_pad_all(g_detailContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_detailContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_detailContainer, 0, LV_PART_MAIN);

    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabView);
    lv_obj_set_style_bg_color(tab_btns, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(tab_btns, g_defIllustrateFont, LV_PART_ITEMS);
    lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(tab_btns, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);

    uint16_t width = 0;
    lv_obj_t *temp = GuiCreateIllustrateLabel(tabView, _("Overview"));
    width = lv_obj_get_self_width(temp) > 100 ? 300 : 200;
    lv_obj_del(temp);

    lv_obj_set_width(tab_btns, width);
}

static void GuiRenderBottomBtn(lv_obj_t *parent, bool showSignSlider) {
    //clear bottom container if need
    if (showSignSlider) {
        if(g_bottomBtnContainer != NULL) {
            lv_obj_del(g_bottomBtnContainer);
            g_bottomBtnContainer = NULL;
        }
        if(g_signSlider != NULL) {
            return;
        }
        g_signSlider = GuiCreateConfirmSlider(parent, CheckSliderProcessHandler);
        g_fingerSignCount = 0;
    }
    else {
        if (g_signSlider != NULL) {
            lv_obj_del(g_signSlider);
            g_signSlider = NULL;
        }
        if (g_bottomBtnContainer != NULL) {
            return;
        }
        g_bottomBtnContainer = GuiCreateContainerWithParent(parent, 480, 114);
        lv_obj_align(g_bottomBtnContainer, LV_ALIGN_BOTTOM_MID, 0, 0);

        lv_obj_t *leftBtn, *rightBtn;
        leftBtn = GuiCreateTextBtn(g_bottomBtnContainer, _("Previous"));
        lv_obj_align(leftBtn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        lv_obj_set_size(leftBtn, 192, 66);
        lv_obj_set_style_bg_color(leftBtn, DARK_GRAY_COLOR, LV_PART_MAIN);
        lv_obj_add_event_cb(leftBtn, HandleClickPreviousBtn, LV_EVENT_CLICKED, NULL);
        rightBtn = GuiCreateTextBtn(g_bottomBtnContainer, _("Next"));
        lv_obj_align(rightBtn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_set_size(rightBtn, 192, 66);
        lv_obj_set_style_bg_color(rightBtn, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_add_event_cb(rightBtn, HandleClickNextBtn, LV_EVENT_CLICKED, NULL);
    }
}

static void GuiRenderCurrentTransaction(bool showSwapHint, bool showSignSlider) {
    if(g_txContainer != NULL) {
        lv_obj_del(g_txContainer);
        g_txContainer = NULL;
        g_overviewContainer = NULL;
        g_detailContainer = NULL;
    }
    g_txContainer = GuiCreateContainerWithParent(g_cont, 408, 542);
    lv_obj_align(g_txContainer, LV_ALIGN_TOP_MID, 0, 0);

    GuiRenderTransactionFrame(g_txContainer);

    GuiRenderOverview(g_overviewContainer, showSwapHint);

    GuiRenderDetail(g_detailContainer);

    GuiRenderBottomBtn(g_cont, showSignSlider);
}
