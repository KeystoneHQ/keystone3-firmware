#include "gui_multisig_transaction_signature_widgets.h"
#include <gui_page.h>
#include "user_memory.h"
#include "gui_views.h"
#include "gui_btc.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#else
#include "safe_str_lib.h"
#endif

static PageWidget_t *g_pageWidget = NULL;
static lv_obj_t *g_cont = NULL;
static lv_obj_t *g_qrCont = NULL;
static lv_obj_t *g_signStatusLabel = NULL;

#define MAX_SIGN_STATUS_LEN 24

static char *g_signStatus = NULL;
static bool g_signCompleted = false;

static void GuiMultisigTransactionSignatureContent(lv_obj_t *parent);
static void GuiMultisigTransactionSignatureSetupUR(lv_obj_t *parent);

void GuiMultisigTransactionSignatureSetSignStatus(char *signStatus, bool signCompleted)
{
    if (g_signStatus)
        SRAM_FREE(g_signStatus);
    g_signStatus = SRAM_MALLOC(MAX_SIGN_STATUS_LEN);
    strncpy_s(g_signStatus, MAX_SIGN_STATUS_LEN, signStatus, strlen(signStatus));
    g_signCompleted = signCompleted;
    GuiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
}

void GuiMultisigTransactionSignatureNVSBarInit();

void GuiMultisigTransactionSignaureWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    GuiMultisigTransactionSignatureNVSBarInit();
    g_cont = g_pageWidget->contentZone;
    GuiMultisigTransactionSignatureSetupUR(g_cont);
}

static void GuiMultisigTransactionSignatureSetupUR(lv_obj_t *parent)
{
    g_qrCont = GuiCreateContainerWithParent(parent, 408, 450);
    lv_obj_set_style_bg_color(g_qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_qrCont, 24, LV_PART_MAIN);
    lv_obj_align(g_qrCont, LV_ALIGN_TOP_LEFT, 36, 0);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(g_qrCont, 336, 336);
    lv_obj_align(qrcode, LV_ALIGN_TOP_LEFT, 36, 36);
    GuiAnimatingQRCodeInitWithCustomSize(qrcode, GuiGetSignQrCodeData, false, 336, 336, NULL);

    g_signStatusLabel = GuiCreateIllustrateLabel(g_qrCont, "Signature Status: ");
    lv_obj_align_to(g_signStatusLabel, qrcode, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

static void GuiMultisigTransactionSignatureContent(lv_obj_t *parent)
{
    lv_obj_t *text = GuiCreateIllustrateLabel(g_qrCont, g_signStatus);
    lv_obj_align_to(text, g_signStatusLabel, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    char *hint = _("multisig_signature_hint_1");
    if (g_signCompleted) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Broadcast Transaction"));
        hint = _("multisig_signature_hint_2");
    }

    text = GuiCreateIllustrateLabel(g_cont, hint);
    lv_obj_align_to(text, g_qrCont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_text_align(text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *btn = GuiCreateBtn(g_cont, _("Done"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_add_event_cb(btn, GoToHomeViewHandler, LV_EVENT_CLICKED, NULL);
}

void GuiMultisigTransactionSignatureNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, GoToHomeViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Multi-Signature"));
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SDCARD, NULL, NULL);
}

void GuiMultisigTransactionSignaureWidgetsDeInit()
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiMultisigTransactionSignaureWidgetsRefresh()
{
    GuiMultisigTransactionSignatureContent(g_cont);
}