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
#ifdef BTC_ONLY
#include "gui_multisig_read_sdcard_widgets.h"
#endif

static void GuiScanNavBarInit();
static void GuiSetScanCorner(void);
static void ThrowError();
static void GuiDealScanErrorResult(int errorType);
static void CloseScanErrorDataHandler(lv_event_t *e);
static void GuiScanStart();

#ifdef BTC_ONLY
static void SelectMicroCardFileHandler(lv_event_t *e);
static lv_obj_t *g_noticeWindow;
#endif

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_scanErrorHintBox = NULL;
static ViewType g_qrcodeViewType;
static uint8_t g_chainType = CHAIN_BUTT;

void GuiScanInit()
{
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

void GuiScanResult(bool result, void *param)
{
    if (result) {
        UrViewType_t urViewType = *(UrViewType_t *)param;
        g_qrcodeViewType = urViewType.viewType;
        g_chainType = ViewTypeToChainTypeSwitch(g_qrcodeViewType);
        // Not a chain based transaction, e.g. WebAuth
        if (GetMnemonicType() == MNEMONIC_TYPE_SLIP39) {
#ifndef BTC_ONLY
            //we don't support ADA in Slip39 Wallet;
            if (g_chainType == CHAIN_ADA || g_qrcodeViewType == KeyDerivationRequest) {
                ThrowError();
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
            ThrowError();
            return;
        }
        GuiModelCheckTransaction(g_qrcodeViewType);
    } else {
        ThrowError();
    }
}

void GuiTransactionCheckPass(void)
{
#ifndef COMPILE_SIMULATOR
    GuiModelTransactionCheckResultClear();
    SetPageLockScreen(true);
    GuiFrameOpenViewWithParam(&g_transactionDetailView, &g_qrcodeViewType, sizeof(g_qrcodeViewType));
#else
#ifndef BTC_ONLY
    g_qrcodeViewType =  EthTx;
#else
    g_qrcodeViewType =  BtcNativeSegwitTx;
#endif
    GuiFrameOpenViewWithParam(&g_transactionDetailView, &g_qrcodeViewType, sizeof(g_qrcodeViewType));
#endif
}

//Here return the error code and error message so that we can distinguish the error type later.
void GuiTransactionCheckFailed(PtrT_TransactionCheckResult result)
{
    switch (result->error_code) {
    case BitcoinNoMyInputs:
    case BitcoinWalletTypeError:
        GuiCreateRustErrorWindow(result->error_code, result->error_message, NULL);
        break;
    default:
        ThrowError();
        break;
    }
    GuiModelTransactionCheckResultClear();
}

static void GuiScanNavBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseTimerCurrentViewHandler, NULL);
#ifdef BTC_ONLY
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SDCARD, SelectMicroCardFileHandler, NULL);
#endif
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
}

static void ThrowError()
{
    GuiSetScanCorner();
    GuiDealScanErrorResult(0);
}

static void GuiDealScanErrorResult(int errorType)
{
    g_scanErrorHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_scanErrorHintBox, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_scanErrorHintBox, _("scan_qr_code_error_invalid_qrcode"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_scanErrorHintBox, _("scan_qr_code_error_invalid_qrcode_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_scanErrorHintBox, _("OK"), g_defTextFont);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, CloseScanErrorDataHandler, LV_EVENT_CLICKED, NULL);
}

static void CloseScanErrorDataHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_scanErrorHintBox)
        GuiScanStart();
    }
}

static void GuiScanStart()
{
    GuiSetScanCorner();
    GuiModeControlQrDecode(true);
}

#ifdef BTC_ONLY
void SelectMicroCardFile(void)
{
    if (SdCardInsert()) {
        static uint8_t fileFilterType = ONLY_PSBT;
        GuiFrameOpenViewWithParam(&g_multisigReadSdcardView, &fileFilterType, sizeof(fileFilterType));
    } else {
        g_noticeWindow = GuiCreateErrorCodeWindow(ERR_UPDATE_SDCARD_NOT_DETECTED, &g_noticeWindow);
    }
}

static void SelectMicroCardFileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        SelectMicroCardFile();
    }
}
#endif