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
#include "gui_animating_qrcode.h"

static void GuiTransactionSignatureNVSBarInit();
static void GuiCreateSignatureQRCode(lv_obj_t *parent);

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
    GuiAnimatingQRCodeDestroyTimer();
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiTransactionSignatureRefresh(void)
{
    GuiAnimatingQRCodeControl(false);
}

void GuiTransactionSignatureHandleURGenerate(char *data, uint16_t len)
{
    GuiAnimantingQRCodeFirstUpdate(data, len);
}

void GuiTransactionSignatureHandleURUpdate(char *data, uint16_t len)
{
    GuiAnimatingQRCodeUpdate(data, len);
}

static void GuiTransactionSignatureNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, GoToHomeViewHandler, NULL);
#ifndef BTC_ONLY
    if (IsMessageType(g_viewType)) {
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, _("transaction_parse_broadcast_message"));
    } else if (isTonSignProof(g_viewType)) {
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, _("ton_sign_proof_title"));
    } else {
#endif
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainType, NULL);
#ifndef BTC_ONLY
    }
#endif
}

static void GuiCreateSignatureQRCode(lv_obj_t *parent)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(cont, LV_ALIGN_DEFAULT);

    lv_obj_t *qrBgCont = GuiCreateContainerWithParent(cont, 408, 408);
    lv_obj_align(qrBgCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(qrBgCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrBgCont, 24, LV_PART_MAIN);
    lv_obj_t *qrCont = GuiCreateContainerWithParent(qrBgCont, 336, 336);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 36);

    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("transaction_parse_scan_by_software"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 576 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *btn = GuiCreateTextBtn(cont, _("Done"));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_add_event_cb(btn, GoToHomeViewHandler, LV_EVENT_CLICKED, NULL);

    GenerateUR func = GetUrGenerator(g_viewType);

    if (func) {
        bool showPending = true;
#if BTC_ONLY
        showPending = false;
#endif
        GuiAnimatingQRCodeInitWithCustomSize(qrCont, func, showPending, 336, 336, (char *)_("sign_transaction"));
    }
}