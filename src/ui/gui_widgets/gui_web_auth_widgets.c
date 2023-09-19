#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_qrcode_widgets.h"
#include "gui_web_auth_widgets.h"
#include "gui_web_auth_result_widgets.h"
#include "gui_firmware_update_widgets.h"
#include "gui_setup_widgets.h"
#include "gui_qr_hintbox.h"
#include "gui_page.h"

static void GuiWebAuthQrcodeHandler(lv_event_t *e);
void GuiWebAuthHidePending();
void GuiWebAuthRenderAuthCode(lv_obj_t *parent);
void GuiWebAuthInitNVSBar();
void GuiWebAuthDeInitNVSBar();
static void GuiGoToFirmwareUpdateViewHandler(lv_event_t *e);
typedef struct WebAuthWidget {
    lv_obj_t *cont;
} WebAuthWidget_t;
static WebAuthWidget_t g_webAuthWidget;

static uint8_t g_webAuthEntry = WEB_AUTH_ENTRY_BUTT;
static uint8_t g_firmwareWareEntry = FIRMWARE_UPDATE_ENTRY_SETUP;
static PageWidget_t *g_pageWidget;

void GuiWebAuthSetEntry(uint8_t entry)
{
    g_webAuthEntry = entry;
}

void GuiWebAuthSuccessCbSetup()
{
    GuiFrameOpenViewWithParam(&g_firmwareUpdateView, &g_firmwareWareEntry, sizeof(g_firmwareWareEntry));
}

void GuiWebAuthSuccessCbSetting()
{
    GuiFrameCLoseView(&g_webAuthView); // web_auth view
}

void GuiWebAuthScanQRCodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenView(&g_qrCodeView);
        if (g_webAuthEntry == WEB_AUTH_ENTRY_SETUP) {
            GuiWebAuthResultSetSuccessCb(GuiWebAuthSuccessCbSetup);
        } else {
            GuiWebAuthResultSetSuccessCb(GuiWebAuthSuccessCbSetting);
        }
    }
}

void GuiWebAuthIntroWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("verify_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("verify_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 1");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 346 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("verify_cont1"));
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 60, 346 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 244, 30);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 60, 296);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, GuiWebAuthQrcodeHandler, LV_EVENT_CLICKED, NULL);

    label = GuiCreateIllustrateLabel(cont, _("verify_cont1_link"));
    lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 0, 0);

    lv_obj_t *img;
    img = GuiCreateImg(cont, &imgQrcodeTurquoise);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 220, 0);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 2");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 482 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("verify_cont2"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 60, 482 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 3");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 554 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("verify_cont3"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 60, 554 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *btn = GuiCreateBtn(parent, _("Scan QR Code"));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_add_event_cb(btn, GuiWebAuthScanQRCodeHandler, LV_EVENT_CLICKED, NULL);
}

void GuiWebAuthAreaInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(cont, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    GuiWebAuthIntroWidget(cont);
    g_webAuthWidget.cont = cont;
}

void GuiWebAuthAreaDeInit()
{
    GUI_DEL_OBJ(g_webAuthWidget.cont)
    g_webAuthEntry = WEB_AUTH_ENTRY_BUTT;
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

// Unlock, Return from QRCode, Return from Purpose(in setup)
void GuiWebAuthAreaRefresh()
{
    GuiWebAuthInitNVSBar();
    if (g_webAuthEntry == WEB_AUTH_ENTRY_SETUP) {
        GuiSetSetupPhase(SETUP_PAHSE_WEB_AUTH);
        if (g_reboot) {
            if (!GuiJudgeCurrentPahse(SETUP_PAHSE_WEB_AUTH)) {
                GuiFrameOpenViewWithParam(&g_firmwareUpdateView, &g_firmwareWareEntry, sizeof(g_firmwareWareEntry));
            } else {
                g_reboot = false;
            }
        }
    }
}

void GuiWebAuthAreaRestart()
{
    GuiWebAuthAreaDeInit();
    GuiWebAuthAreaInit();
}

void GuiWebAuthInitNVSBar()
{
    if (g_webAuthEntry == WEB_AUTH_ENTRY_SETUP) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_NEW_SKIP, GuiGoToFirmwareUpdateViewHandler, NULL);
    } else {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    }
}

static void GuiGoToFirmwareUpdateViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenViewWithParam(&g_firmwareUpdateView, &g_firmwareWareEntry, sizeof(g_firmwareWareEntry));
    }
}

static void GuiOpenQRCodeHintBox()
{
    GuiQRCodeHintBoxOpen(_("verify_qr_link"), _("verify_qr_title"), _("verify_qr_link"));
}

static void GuiWebAuthQrcodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiOpenQRCodeHintBox();
    }
}
