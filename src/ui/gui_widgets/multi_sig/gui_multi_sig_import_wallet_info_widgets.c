#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "version.h"
#include "err_code.h"
#include "firmware_update.h"
#include "gui_page.h"
#include "gui_multi_sig_import_wallet_info_widgets.h"

static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;

static void GuiImportWalletInfoNVSBarInit();
static void GuiImportWalletInfoContent(lv_obj_t *parent);

void GuiImportWalletInfoWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    g_cont = cont;
    GuiImportWalletInfoContent(cont);
}

void GuiImportWalletInfoWidgetsDeInit()
{
    GUI_DEL_OBJ(g_cont)
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiImportWalletInfoWidgetsRefresh()
{
    GuiImportWalletInfoNVSBarInit();
}

void GuiImportWalletInfoWidgetsRestart()
{}

static void GuiImportWalletInfoNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Wallet Info"));
}

void GuiImportWalletInfoContent(lv_obj_t *parent)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 514);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_ON);

    lv_obj_t *section = GuiCreateContainerWithParent(cont, 408, 100);
    lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(section, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 24, LV_PART_MAIN);
    lv_obj_t *obj = GuiCreateNoticeLabel(section, _("Wallet Name"));
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 24, 16);
    obj = GuiCreateIllustrateLabel(section, "My Wallet Name");
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 24, 54);

    lv_obj_t *prev = section;
    section = GuiCreateContainerWithParent(cont, 408, 62);
    lv_obj_align_to(section, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_set_style_bg_color(section, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 24, LV_PART_MAIN);
    obj = GuiCreateNoticeLabel(section, _("Policy"));
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 24, 16);
    obj = GuiCreateIllustrateLabel(section, "2 of 4");
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 94, 20);

    prev = section;
    section = GuiCreateContainerWithParent(cont, 408, 62);
    lv_obj_align_to(section, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_set_style_bg_color(section, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 24, LV_PART_MAIN);
    obj = GuiCreateNoticeLabel(section, _("Format"));
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 24, 16);
    obj = GuiCreateIllustrateLabel(section, "P2WSH");
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 108, 20);

    prev = section;
    section = GuiCreateContainerWithParent(cont, 408, 400);
    lv_obj_align_to(section, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_set_style_bg_color(section, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 24, LV_PART_MAIN);
    prev = NULL;
    for (int i = 0; i < 4; i++) {
        obj = GuiCreateNoticeLabel(section, "#F5870A 1/3");
        lv_obj_align_to(obj, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 16);
        lv_label_set_recolor(obj, true);
        prev = obj;
        obj = GuiCreateIllustrateLabel(section, "EB16731F");
        lv_obj_align_to(obj, prev, LV_ALIGN_OUT_TOP_LEFT, 16, 0);
        obj = GuiCreateNoticeLabel(section, "vpub5Y28KoUWVNWQ2xrVHNv6P9CDn3dXTzerd8BwMAj2nt7Uej6MRP7wMF4d7GYNUuVEesDkYAtYy5DŸLT3fYMEJNaNW855TkkZLSX6MHmuXKZ");
        lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
        lv_obj_align_to(obj, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        lv_obj_set_style_width(obj, 360, LV_PART_MAIN);
        prev = obj;
        obj = GuiCreateNoticeLabel(section, "m/45’");
        lv_obj_align_to(obj, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 16);
    }

}
