#include "gui.h"
#include "gui_button.h"
#include "gui_page.h"
#include "rust.h"
#include "gui_qr_hintbox.h"
#include "gui_status_bar.h"
#include "gui_views.h"
#include "gui_zcash_verify_widgets.h"

#define ZCASH_VERIFY_CONTENT_WIDTH 408
#define ZCASH_VERIFY_BTN_HEIGHT 66
#define ZCASH_VERIFY_BOTTOM_MARGIN 24

static PageWidget_t *g_pageWidget = NULL;
static lv_obj_t *g_cont = NULL;

static ViewType g_zcashVerifyScanFilter[2] = {ZcashAddressCheck, 0xFF};

static void OpenZcashVerifyAboutHandler(lv_event_t *e)
{
    GuiQRCodeHintBoxOpen(_("zcash_verify_about_link"), _("zcash_verify_about_title"),
                         _("zcash_verify_about_link"));
}

static void StartZcashAddressScanHandler(lv_event_t *e)
{
    GuiFrameOpenViewWithParam(&g_scanView, g_zcashVerifyScanFilter,
                              sizeof(g_zcashVerifyScanFilter));
}

static void GuiCreateLearnMoreWidget(lv_obj_t *parent, lv_obj_t *alignTo)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(cont, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(cont, OpenZcashVerifyAboutHandler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = GuiCreateIllustrateLabel(cont, _("learn_more"));
    lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);
    lv_obj_t *img = GuiCreateImg(cont, &imgQrcodeTurquoise);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_update_layout(cont);

    lv_obj_align_to(cont, alignTo, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

static void GuiCreateScanButtonWidget(lv_obj_t *parent)
{
    lv_obj_t *btn = GuiCreateTextBtn(parent, _("zcash_verify_scan_btn"));
    lv_obj_set_size(btn, ZCASH_VERIFY_CONTENT_WIDTH, ZCASH_VERIFY_BTN_HEIGHT);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -ZCASH_VERIFY_BOTTOM_MARGIN);
    lv_obj_add_event_cb(btn, StartZcashAddressScanHandler, LV_EVENT_CLICKED, NULL);
}

void GuiZcashVerifyWidgetsInit(void)
{
    g_pageWidget = CreatePageWidget();
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    g_cont = g_pageWidget->contentZone;
    lv_obj_add_flag(g_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = GuiCreateLittleTitleLabel(g_cont, _("zcash_verify_intro_title"));
    lv_obj_set_width(title, ZCASH_VERIFY_CONTENT_WIDTH);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 36, 12);

    lv_obj_t *desc = GuiCreateIllustrateLabel(g_cont, _("zcash_verify_intro_desc"));
    lv_obj_set_width(desc, ZCASH_VERIFY_CONTENT_WIDTH);
    lv_label_set_long_mode(desc, LV_LABEL_LONG_WRAP);
    lv_obj_align_to(desc, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    GuiCreateLearnMoreWidget(g_cont, desc);
    GuiCreateScanButtonWidget(g_cont);
}

void GuiZcashVerifyWidgetsDeInit(void)
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_cont = NULL;
}

void GuiZcashVerifyWidgetsRefresh(void)
{
}
