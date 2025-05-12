#include "gui_eth_batch_tx_widgets.h"
#include "keystore.h"
#include "gui.h"
#include "gui_page.h"
#include "gui_model.h"
#include "rust.h"
#include "gui_chain_components.h"
#include "gui_obj.h"
#include "gui_eth.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_model.h"
#endif

#include "abi_ethereum.h"

const ABIItem_t allowed_swap_abi_map[] = {
    {   "0xd37bbe5744d730a1d98d8dc97c42f0ca46ad7146_1fece7b4",
        "{\"name\":\"THORChain Router\",\"address\":\"0xd37bbe5744d730a1d98d8dc97c42f0ca46ad7146\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"vault\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"string\",\"name\":\"memo\",\"type\":\"string\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd37bbe5744d730a1d98d8dc97c42f0ca46ad7146_44bc937b",
        "{\"name\":\"THORChain Router\",\"address\":\"0xd37bbe5744d730a1d98d8dc97c42f0ca46ad7146\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"vault\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"string\",\"name\":\"memo\",\"type\":\"string\"},{\"internalType\":\"uint256\",\"name\":\"expiration\",\"type\":\"uint256\"}],\"name\":\"depositWithExpiry\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
};

#define QRCODE_CONFIRM_SIGN_PROCESS 66
#define FINGER_SIGN_MAX_COUNT 5

static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static TransactionParseResult_DisplayETHBatchTx *g_parseResult = NULL;
static bool g_isMulti = false;
static DisplayETHBatchTx *g_displayEthBatchTx = NULL;

static uint32_t g_currentTxIndex = 0;
static uint32_t g_txCount = 0;
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
static KeyboardWidget_t *g_keyboardWidget = NULL;
static lv_obj_t *g_bottomBtnContainer = NULL;
static lv_obj_t *g_signSlider = NULL;
static uint32_t g_fingerSignCount = FINGER_SIGN_MAX_COUNT;

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
    // if (cancel) {
    //     FpCancelCurOperate();
    // }
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

static void ParseSwapContractData(const char* address, const char* inputData) {
    char selectorId[9] = {0};
    strncpy(selectorId, inputData, 8);
    char* address_key = (char*)SRAM_MALLOC(strlen(address) + 9);
    strcpy_s(address_key, strlen(address) + 9, address);
    strcat_s(address_key, strlen(address) + 9, "_");
    strcat_s(address_key, strlen(address) + 9, selectorId);
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(allowed_swap_abi_map); i++) {
        struct ABIItem item = allowed_swap_abi_map[i];
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
    GuiEthBatchTxNavBarRefresh();
    if(!HandleCurrentTransaction(g_currentTxIndex)) {
        return;
    }

    bool showSwapHint = g_currentTxIndex == 0 && g_txCount > 1;
    bool showSignSlider = g_currentTxIndex == g_txCount - 1;

    GuiRenderCurrentTransaction(showSwapHint, showSignSlider);
}

static void OnReturnHandler(lv_event_t *e) {
    GuiCloseToTargetView(&g_homeView);
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

void GuiEthBatchTxWidgetsTransactionParseFail() {
    printf("GuiEthBatchTxWidgetsTransactionParseFail\n");
    printf("error: %s\n", g_parseResult->error_message);
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
        if( g_swapkitContractData->error_code != 0 || g_contractData->error_code != 0) {
            return false;
        }
    }
    else {
        ParseErc20ContractData(g_currentTransaction->detail->input, g_currentErc20Contract->decimals);
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
}

static void GuiEthBatchTxNavBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
}

static void GuiEthBatchTxNavBarRefresh() {
    char* text = malloc(BUFFER_SIZE_32);
    sprintf(text, "%s (%d/%d)", _("confirm_transaction"), g_currentTxIndex + 1, g_txCount);
    SetCoinWallet(g_pageWidget->navBarWidget, CHAIN_ETH, text);
    if(g_currentTxIndex == 0) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, OnReturnHandler, NULL);
    }
    else {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, HandleClickPreviousBtn, NULL);
    }
}

// GUI Impelementation Part
static lv_obj_t* GuiOvewviewSwapHint(lv_obj_t *parent) {
    lv_obj_t *swapHint = CreateTransactionContentContainer(parent, 408, 182);
    lv_obj_align(swapHint, LV_ALIGN_TOP_LEFT, 0, 4);
    lv_obj_t *img, *title, *text;
    img = GuiCreateImg(swapHint, &imgNotice);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 24, 24);
    title = GuiCreateIllustrateLabel(swapHint, _("Notice"));
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 68, 24);
    lv_obj_set_style_text_color(title, ORANGE_COLOR, LV_PART_MAIN);
    text = GuiCreateIllustrateLabel(swapHint, _("swap_token_approve_hint"));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 24, 68);
    return swapHint;
}

static lv_obj_t* GuiRenderSwapSummary(lv_obj_t *parent, const char* from_asset, const char* from_amount, const char* to_asset) {
    uint16_t height = 16;//top padding
    lv_obj_t *container = CreateTransactionContentContainer(parent, 408, 0);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 4);

    lv_obj_t *label;
    label = GuiCreateIllustrateLabel(container, _("Swap"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    height += 30 + 4;
    char* text = malloc(BUFFER_SIZE_128);
    sprintf(text, "%s %s", from_amount, from_asset);
    label = GuiCreateLittleTitleLabel(container, text);
    lv_obj_set_width(label, 360);
    lv_obj_update_layout(label);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);

    height += lv_obj_get_self_height(label) + 8;

    label = GuiCreateIllustrateLabel(container, _("To"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, height);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    lv_obj_t *titleLabel = label;
    lv_obj_update_layout(label);
    uint16_t titleWidth = lv_obj_get_self_width(label);

    label = GuiCreateIllustrateLabel(container, to_asset);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_update_layout(label);
    lv_obj_t *valueLabel = label;

    uint16_t valueWidth = lv_obj_get_width(valueLabel);
    uint16_t valueHeight = lv_obj_get_height(valueLabel);

    uint16_t totalWidth = 24 + titleWidth + 16 + valueWidth + 24;
    bool overflow = totalWidth > 408 || valueHeight > 30;

    height += 30; //title height;

    if (!overflow) {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    } else {
        lv_obj_align_to(valueLabel, titleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_obj_set_width(valueLabel, 360);
        lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(valueLabel);

        height += lv_obj_get_height(valueLabel);
    }

    height += 16;

    lv_obj_set_height(container, height);

    return container;
}

static lv_obj_t *GuiRenderUnknownSwapSummary(lv_obj_t *parent, const char* from_asset, const char* from_amount, const char* to_asset) {
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

static void GuiRenderOverview(lv_obj_t *parent, bool showSwapHint) {
    lv_obj_set_style_bg_color(parent, BLUE_COLOR, LV_PART_MAIN);
    lv_obj_t *last_view = NULL;
    if(showSwapHint) {
        last_view = GuiOvewviewSwapHint(parent);
    }
    if(g_parseErc20Approval != NULL) {
        bool showAmount = false;
        if(strcmp(g_parseErc20Approval->data->value, "0") != 0) {
            showAmount = true;

        }
        last_view = GuiRenderApprove(parent, showAmount, last_view);
    }
    else {
        if (g_swapkitContractData != NULL) {
            Erc20Contract_t *erc20Contract = FindErc20Contract(g_swapkitContractData->data->swap_in_asset);
            if(erc20Contract != NULL) {
                //is known erc20 token
                SimpleResponse_c_char *response = format_value_with_decimals(g_swapkitContractData->data->swap_in_amount, erc20Contract->decimals);
                if(response->error_code == 0) {
                    char *amount = response->data;
                    last_view = GuiRenderSwapSummary(parent, erc20Contract->symbol, amount, g_swapkitContractData->data->swap_out_asset);
                }
                else {
                    printf("format_value_with_decimals failed\n");
                }

            }
            else {
                //is unknown erc20 token
                last_view = GuiRenderUnknownSwapSummary(parent, g_swapkitContractData->data->swap_in_asset, g_swapkitContractData->data->swap_in_amount, g_swapkitContractData->data->swap_out_asset);
            }
            last_view = CreateTransactionItemView(parent, _("Network"), g_currentNetwork.name, last_view);
            last_view = CreateTransactionItemView(parent, _("From"), g_currentTransaction->overview->from, last_view);
            last_view = CreateTransactionItemView(parent, _("Destination"), g_swapkitContractData->data->receive_address, last_view);
        }
    }
}

static void GuiRenderDetail(lv_obj_t *parent) {

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
    g_detailContainer = lv_tabview_add_tab(tabView, _("Detail"));
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

    lv_obj_set_width(tab_btns, 200);
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
