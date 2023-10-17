#include "gui_connect_eternl_widgets.h"
#include "gui_connect_wallet_widgets.h"
#include "gui.h"
#include "gui_page.h"

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_cont;

static void GuiCreatePageContent(lv_obj_t *parent);
static void GotoScanQRCodeHandler(lv_event_t *e);
static void CleanHandler(lv_event_t *e);

void GuiCreateConnectEternlWidget()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style(cont, NULL, LV_PART_SCROLLBAR);
    g_cont = cont;
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CleanHandler, NULL);
    SetWallet(g_pageWidget->navBarWidget, WALLET_LIST_ETERNL, NULL);
    GuiCreatePageContent(g_cont);
}

static void GuiCreatePageContent(lv_obj_t *parent)
{
    lv_obj_t *button, *label;
    button = GuiCreateBtn(parent, _("connect_wallet_title"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, 24);
    lv_obj_add_event_cb(button, GotoScanQRCodeHandler, LV_EVENT_CLICKED, NULL);
}

static void CleanHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (g_cont != NULL)
        {
            lv_obj_del(g_cont);
            g_cont = NULL;
        }
        if (g_pageWidget != NULL)
        {
            DestroyPageWidget(g_pageWidget);
            g_pageWidget = NULL;
        }
    }
}

static void GotoScanQRCodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        GuiFrameOpenView(&g_qrCodeView);
    }
}