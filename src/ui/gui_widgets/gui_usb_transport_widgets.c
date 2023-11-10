#include "gui.h"
#include "gui_views.h"
#include "gui_page.h"
#include "gui_usb_transport_widgets.h"
#include "eapdu_services/service_export_address.h"

static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;

static void ApproveButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ExportAddressApprove();
    }
}

static void RejectButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ExportAddressReject();
    }
}

void GuiUSBTransportWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    g_cont = cont;
    lv_obj_t *img = GuiCreateImg(cont, &imgConnectWithRabby);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *label;
    char *str = "Connection Request";
    label = GuiCreateLittleTitleLabel(cont, str);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 184);

    str = "Rabby Wallet want's to connect your Keystone via USB";
    label = GuiCreateLabel(cont, str);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_90, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 236);

    lv_obj_t *button = GuiCreateBtnWithFont(cont, &"Reject", g_defTextFont);
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_set_style_bg_color(button, DARK_GRAY_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(button, RejectButtonHandler, LV_EVENT_CLICKED, NULL);

    button = GuiCreateBtnWithFont(cont, &"Approve", g_defTextFont);
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_set_style_bg_color(button, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(button, ApproveButtonHandler, LV_EVENT_CLICKED, NULL);
}

void GuiUSBTransportWidgetsDeInit()
{
    GUI_DEL_OBJ(g_cont)
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiUSBTransportWidgetsRefresh()
{
    return;
}