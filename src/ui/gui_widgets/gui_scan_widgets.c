#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_enter_passcode.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_scan_widgets.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_analyze.h"
#include "gui_button.h"
#include "gui_qr_code.h"
#include "secret_cache.h"
#include "qrdecode_task.h"
#include "gui_chain.h"
#include "assert.h"
#include "gui_web_auth_widgets.h"
#include "gui_qr_hintbox.h"
#include "motor_manager.h"
#include "gui_lock_widgets.h"
#include "screen_manager.h"
#include "fingerprint_process.h"
#include "gui_fullscreen_mode.h"
#include "gui_keyboard_hintbox.h"
#include "gui_page.h"
#include "account_manager.h"
#include "gui_btc.h"
#ifdef BTC_ONLY
#include "gui_multisig_read_sdcard_widgets.h"
#endif

static void GuiScanNavBarInit();
static void GuiSetScanCorner(void);
static void ThrowError(int32_t errorCode);
static void GuiScanStart();

#ifdef BTC_ONLY
static lv_obj_t *g_noticeWindow;
#endif

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_scanErrorHintBox = NULL;
static ViewType g_qrcodeViewType;
static uint8_t g_chainType = CHAIN_BUTT;
static ViewType g_viewTypeFilter[2];

void GuiScanInit(void *param, uint16_t len)
{
    if (param == NULL) {
        for (int i = 0; i < NUMBER_OF_ARRAYS(g_viewTypeFilter); i++) {
            g_viewTypeFilter[i] = 0xFF;
        }
    } else {
        memcpy_s(g_viewTypeFilter, sizeof(g_viewTypeFilter), param, len);
        for (int i = 0; i < NUMBER_OF_ARRAYS(g_viewTypeFilter); i++) {
            printf("g_viewTypeFilter %d = %d\n", i, g_viewTypeFilter[i]);
        }
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_pageWidget = CreatePageWidget();
    GuiScanNavBarInit();
}

void GuiScanDeInit()
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
#ifdef BTC_ONLY
    GUI_DEL_OBJ(g_noticeWindow);
#endif

    SetPageLockScreen(true);
}

void GuiScanRefresh()
{
    SetPageLockScreen(false);
    GuiScanStart();
}

static bool IsViewTypeSupported(ViewType viewType, ViewType *viewTypeFilter, size_t filterSize)
{
    for (size_t i = 0; i < filterSize; i++) {
        if (viewType == viewTypeFilter[i]) {
            return true;
        }
    }
    return false;
}

void GuiScanResult(bool result, void *param)
{
    if (result) {
        UrViewType_t urViewType = *(UrViewType_t *)param;
        g_qrcodeViewType = urViewType.viewType;
        if (g_viewTypeFilter[0] != 0xFF) {
            if (!IsViewTypeSupported(g_qrcodeViewType, g_viewTypeFilter, NUMBER_OF_ARRAYS(g_viewTypeFilter))) {
                g_scanErrorHintBox = GuiCreateErrorCodeWindow(ERR_MULTISIG_WALLET_CONFIG_INVALID, &g_scanErrorHintBox, GuiScanStart);
                return;
            }
        }
        g_chainType = ViewTypeToChainTypeSwitch(g_qrcodeViewType);
        // Not a chain based transaction, e.g. WebAuth
        if (GetMnemonicType() == MNEMONIC_TYPE_SLIP39) {
#ifndef BTC_ONLY
            //we don't support ADA in Slip39 Wallet;
            if (g_chainType == CHAIN_ADA || g_qrcodeViewType == KeyDerivationRequest) {
                ThrowError(ERR_INVALID_QRCODE);
                return;
            }
#endif
        }
        if (g_chainType == CHAIN_BUTT) {
            if (g_qrcodeViewType == WebAuthResult) {
                GuiCLoseCurrentWorkingView();
                GuiFrameOpenView(&g_webAuthResultView);
            }
#ifndef BTC_ONLY
            if (g_qrcodeViewType == KeyDerivationRequest) {
                GuiCLoseCurrentWorkingView();
                GuiFrameOpenView(&g_keyDerivationRequestView);
            }
#else
            if (g_qrcodeViewType == MultisigWalletImport) {
                GuiCLoseCurrentWorkingView();
                GuiFrameOpenView(&g_importMultisigWalletInfoView);
            }

            if (g_qrcodeViewType == MultisigCryptoImportXpub ||
                    g_qrcodeViewType ==  MultisigBytesImportXpub) {
                GuiCLoseCurrentWorkingView();
            }
#endif
            return;
        }
        uint8_t accountNum = 0;
        GetExistAccountNum(&accountNum);
        if (accountNum <= 0) {
            ThrowError(ERR_INVALID_QRCODE);
            return;
        }
        GuiModelCheckTransaction(g_qrcodeViewType);
    } else {
        ThrowError(ERR_INVALID_QRCODE);
    }
}

void GuiTransactionCheckPass(void)
{
    GuiModelTransactionCheckResultClear();
    SetPageLockScreen(true);
    GuiCLoseCurrentWorkingView();
#ifndef BTC_ONLY
    if (g_chainType == CHAIN_ARWEAVE) {
        if (GetIsTempAccount()) {
            ThrowError(ERR_INVALID_QRCODE);
            return;
        }
        bool hasArXpub = IsArweaveSetupComplete();
        if (!hasArXpub) {
            GuiPendingHintBoxRemove();
            GoToHomeViewHandler(NULL);
            GuiCreateAttentionHintbox(SIG_SETUP_RSA_PRIVATE_KEY_PARSER_CONFIRM);
            return;
        }
    }
#endif
    GuiFrameOpenViewWithParam(&g_transactionDetailView, &g_qrcodeViewType, sizeof(g_qrcodeViewType));
}

//Here return the error code and error message so that we can distinguish the error type later.
void GuiTransactionCheckFailed(PtrT_TransactionCheckResult result)
{
    switch (result->error_code) {
    case BitcoinNoMyInputs:
    case BitcoinWalletTypeError:
    case MasterFingerprintMismatch:
    case UnsupportedTransaction:
        GuiCreateRustErrorWindow(result->error_code, result->error_message, NULL, GuiScanStart);
        break;
    default:
        ThrowError(ERR_INVALID_QRCODE);
        break;
    }
    GuiModelTransactionCheckResultClear();
#if BTC_ONLY
    FreePsbtUxtoMemory();
#endif
}

static void GuiScanNavBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseTimerCurrentViewHandler, NULL);
}

static void GuiSetScanCorner(void)
{
    UpdatePageContentZone(g_pageWidget);

    lv_obj_t *cont = g_pageWidget->contentZone;
    static lv_point_t topLinePoints[2] = {{0, 0}, {322, 0}};
    static lv_point_t bottomLinePoints[2] = {{0, 0}, {322, 0}};
    static lv_point_t leftLinePoints[2] = {{0, 0}, {0, 322}};
    static lv_point_t rightLinePoints[2] = {{0, 0}, {0, 322}};
    lv_obj_t *line = GuiCreateLine(cont, topLinePoints, 2);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 80, 224 - GUI_MAIN_AREA_OFFSET);

    line = GuiCreateLine(cont, bottomLinePoints, 2);
    lv_obj_align(line, LV_ALIGN_BOTTOM_LEFT, 80, -254);
    line = GuiCreateLine(cont, leftLinePoints, 2);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 80, 224 - GUI_MAIN_AREA_OFFSET);

    line = GuiCreateLine(cont, rightLinePoints, 2);
    lv_obj_align(line, LV_ALIGN_BOTTOM_RIGHT, -78, -254);

    lv_obj_t *img = GuiCreateImg(cont, &imgLTCorner);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 80, 223 - GUI_MAIN_AREA_OFFSET);

    img = GuiCreateImg(cont, &imgLTCorner);
    lv_obj_align(img, LV_ALIGN_TOP_RIGHT, -77 + 28, 223 - GUI_MAIN_AREA_OFFSET - 1);
    lv_img_set_angle(img, 900);
    lv_img_set_pivot(img, 0, 0);

    img = GuiCreateImg(cont, &imgLTCorner);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 79, -254 + 28 + 1);
    lv_img_set_angle(img, -900);
    lv_img_set_pivot(img, 0, 0);

    img = GuiCreateImg(cont, &imgLTCorner);
    lv_obj_align(img, LV_ALIGN_BOTTOM_RIGHT, -77 + 28, -254 + 28 + 1);
    lv_img_set_angle(img, 1800);
    lv_img_set_pivot(img, 0, 0);

#ifdef BTC_ONLY
    if (IsViewTypeSupported(MultisigWalletImport, g_viewTypeFilter, NUMBER_OF_ARRAYS(g_viewTypeFilter))) {
        lv_obj_t *label = GuiCreateNoticeLabel(cont, _("multisig_scan_multisig_notice"));
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 466);
    }
#endif
}

static void ThrowError(int32_t errorCode)
{
    GuiSetScanCorner();
    g_scanErrorHintBox = GuiCreateErrorCodeWindow(errorCode, &g_scanErrorHintBox, GuiScanStart);
}

static void GuiScanStart()
{
    GuiSetScanCorner();
    GuiModeControlQrDecode(true);
}