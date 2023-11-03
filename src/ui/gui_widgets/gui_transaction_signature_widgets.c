#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_enter_passcode.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_transaction_detail_widgets.h"
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


static void GuiTransactionSignatureNVSBarInit();
static void GuiCreateSignatureQRCode(lv_obj_t *parent);
static void GuiShowSignatureQRCode(GetUR func, lv_obj_t *qr);
static lv_obj_t *GuiCreateQRCode(lv_obj_t *parent, uint16_t w, uint16_t h);
static lv_res_t UpdateQrCodeAndFullscreenVersion(lv_obj_t *qrcode, const void *data, uint32_t data_len);

static ViewType g_viewType;
static uint8_t g_chainType = CHAIN_BUTT;
static PageWidget_t *g_pageWidget;

void GuiTransactionSignatureInit(uint8_t viewType)
{
    g_viewType = viewType;
    g_chainType = ViewTypeToChainTypeSwitch(g_viewType);
    g_pageWidget = CreatePageWidget();
    GuiTransactionSignatureNVSBarInit();
    GuiCreateSignatureQRCode(g_pageWidget->contentZone);
}

void GuiTransactionSignatureDeInit(void)
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiTransactionSignatureRefresh(void)
{
}


static void GuiTransactionSignatureNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, GoToHomeViewHandler, NULL);
    if (IsMessageType(g_viewType)) {
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, _("transaction_parse_broadcast_message"));
    } else {
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, NULL);
    }

}


static void GuiCreateSignatureQRCode(lv_obj_t *parent)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(cont, LV_ALIGN_DEFAULT);

    lv_obj_t *qrCont = GuiCreateContainerWithParent(cont, 408, 408);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);
    lv_obj_t *qrcode = GuiCreateQRCode(qrCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(GuiCreateQRCode, 420, 420);

    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("transaction_parse_scan_by_software"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 576 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *btn = GuiCreateBtn(cont, _("Done"));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_add_event_cb(btn, GoToHomeViewHandler, LV_EVENT_CLICKED, NULL);

    char *data = NULL;
    switch (g_viewType)
    {
    case BtcNativeSegwitTx:
    case BtcSegwitTx:
    case BtcLegacyTx:
    case BtcTx:
    case LtcTx:
    case DashTx:
    case BchTx:
        GuiShowSignatureQRCode(GuiGetSignQrCodeData, qrcode);
        break;
    case EthTx:
    case EthPersonalMessage:
    case EthTypedData:
        GuiShowSignatureQRCode(GuiGetEthSignQrCodeData, qrcode);
        break;
    case TronTx:
        GuiShowSignatureQRCode(GuiGetTrxSignQrCodeData, qrcode);
        break;
    case CosmosTx:
    case CosmosEvmTx:
        GuiShowSignatureQRCode(GuiGetCosmosSignQrCodeData, qrcode);
        break;
    case SuiTx:
        GuiShowSignatureQRCode(GuiGetSuiSignQrCodeData, qrcode);
        break;
    case SolanaTx:
    case SolanaMessage:
        GuiShowSignatureQRCode(GuiGetSolSignQrCodeData, qrcode);
        break;
    case AptosTx:
        GuiShowSignatureQRCode(GuiGetAptosSignQrCodeData, qrcode);
        break;
    case CardanoTx:
        GuiShowSignatureQRCode(GuiGetAdaSignQrCodeData, qrcode);
        break;
    default:
        data = "";
        lv_qrcode_update(qrcode, data, strlen(data));
        lv_obj_t *fullScreenQrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
        if (fullScreenQrcode)
        {
            lv_qrcode_update(fullScreenQrcode, data, strlen(data));
        }
        break;
    }
}

static void GuiShowSignatureQRCode(GetUR func, lv_obj_t *qr)
{
    UpdateQrCode(func, qr, UpdateQrCodeAndFullscreenVersion);
}

static lv_obj_t *GuiCreateQRCode(lv_obj_t *parent, uint16_t w, uint16_t h)
{
    lv_obj_t *qrcode = lv_qrcode_create(parent, w, BLACK_COLOR, WHITE_COLOR);
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    return qrcode;
}

static lv_res_t UpdateQrCodeAndFullscreenVersion(lv_obj_t *qrcode, const void *data, uint32_t data_len)
{
    lv_qrcode_update(qrcode, data, data_len);
    lv_obj_t *fullScreenQrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
    if (fullScreenQrcode)
    {
        lv_qrcode_update(fullScreenQrcode, data, data_len);
    }
}
