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

static void GuiScanNavBarInit();
static void GuiSetScanCorner(void);
static GuiChainCoinType ViewTypeToChainTypeSwitch(uint8_t ViewType);
static void ThrowError();
static void GuiDealScanErrorResult(int errorType);
static void CloseScanErrorDataHandler(lv_event_t *e);

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_scanErrorHintBox = NULL;
static ViewType g_qrcodeViewType;
static uint8_t g_chainType = CHAIN_BUTT;

void GuiScanInit()
{
    if (g_pageWidget != NULL)
    {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_pageWidget = CreatePageWidget();
    SetPageLockScreen(false);
    GuiScanNavBarInit();
}

void GuiScanDeInit()
{
    if (g_pageWidget != NULL)
    {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }

     // for learn more hintbox in eth contract data block;
    if (GuiQRHintBoxIsActive())
    {
        GuiQRHintBoxRemove();
    }
    SetPageLockScreen(true);
}

void GuiScanRefresh()
{
    GuiSetScanCorner();
    GuiModeControlQrDecode(true);
}

void GuiScanResult(bool result, void *param)
{
    if (result)
    {
        UrViewType_t urViewType = *(UrViewType_t *)param;
        g_qrcodeViewType = urViewType.viewType;
        g_chainType = ViewTypeToChainTypeSwitch(g_qrcodeViewType);
        // Not a chain based transaction, e.g. WebAuth
        if(GetMnemonicType() == MNEMONIC_TYPE_SLIP39)
        {
            //we don't support ADA in Slip39 Wallet;
            if(g_chainType == CHAIN_ADA || g_qrcodeViewType== KeyDerivationRequest)
            {
                ThrowError();
                return;
            }
        }
        if (g_chainType == CHAIN_BUTT)
        {
            if (g_qrcodeViewType == WebAuthResult)
            {
                GuiCLoseCurrentWorkingView();
                GuiFrameOpenView(&g_webAuthResultView);
            }
            if (g_qrcodeViewType == KeyDerivationRequest)
            {
                GuiCLoseCurrentWorkingView();
                GuiFrameOpenView(&g_keyDerivationRequestView);
            }
            return;
        }
        uint8_t accountNum = 0;
        GetExistAccountNum(&accountNum);
        if (accountNum <= 0)
        {
            ThrowError();
            return;
        }
        // g_qrCodeWidgetView.analysis = GuiTemplateReload(g_qrCodeWidgetView.cont, g_qrcodeViewType);
        // if (g_qrCodeWidgetView.analysis != NULL)
        // {
        //     g_fingerSignCount = 0;
        //     if (IsMessageType(g_qrcodeViewType))
        //     {
        //         SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, _("transaction_parse_confirm_message"));
        //     }
        //     else
        //     {
        //         SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, NULL);
        //     }
        //     GuiCreateConfirmSlider(g_qrCodeWidgetView.cont, CheckSliderProcessHandler);
        //     g_pagePhase = PAGE_PHASE_TRANSACTION_DETAIL;
        //     SetPageLockScreen(true);
        // }
        // else
        // {
            ThrowError();
        // }
    }
    else
    {
        ThrowError();
    }

}

static void GuiScanNavBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseTimerCurrentViewHandler, NULL);
}

static void GuiSetScanCorner(void)
{
    UpdatePageContentZone(g_pageWidget);

    lv_obj_t *cont = g_pageWidget->contentZone;
    printf("qrcode refresh...\n");
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


static GuiChainCoinType ViewTypeToChainTypeSwitch(uint8_t ViewType)
{
    switch (ViewType)
    {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
        return CHAIN_BTC;
    case LtcTx:
        return CHAIN_LTC;
    case DashTx:
        return CHAIN_DASH;
    case BchTx:
        return CHAIN_BCH;
    case EthPersonalMessage:
    case EthTx:
    case EthTypedData:
        return CHAIN_ETH;
    case TronTx:
        return CHAIN_TRX;
    case CosmosTx:
    case CosmosEvmTx:
        return GuiGetCosmosTxChain();
    case SuiTx:
        return CHAIN_SUI;
    case SolanaTx:
    case SolanaMessage:
        return CHAIN_SOL;
    case AptosTx:
        return CHAIN_APT;
    case CardanoTx:
        return CHAIN_ADA;
    default:
        return CHAIN_BUTT;
    }
    return CHAIN_BUTT;
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

    lv_obj_t *btn = GuiCreateBtnWithFont(g_scanErrorHintBox, _("OK"), &openSansEnText);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, CloseScanErrorDataHandler, LV_EVENT_CLICKED, NULL);
}

static void CloseScanErrorDataHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        GUI_DEL_OBJ(g_scanErrorHintBox)
        GuiScanRefresh();
    }
}