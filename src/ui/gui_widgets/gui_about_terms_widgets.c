#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_about_terms_widgets.h"
#include "version.h"
#include "gui_page.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

static void GuiAboutNVSBarInit();
static void GuiAboutTermsEntranceWidget(lv_obj_t *parent);
static int GetLvObjHeight(lv_obj_t *obj);
static lv_obj_t* GuiGetTermsItemContainer(lv_obj_t* parent, const char *title, const char *content, int *height);
static void GuiQrcodeHandler(lv_event_t *e);
static void CloseQrcodeHandler(lv_event_t *e);

static lv_obj_t *g_cont;

static const char *g_termsWebSiteUrl = NULL;
static lv_obj_t *g_qrCodeCont;
static PageWidget_t *g_pageWidget;

void TermWebSiteInit()
{
    g_termsWebSiteUrl = _("about_terms_website_url");
}

void GuiAboutTermsWidgetsInit()
{
    TermWebSiteInit();
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style(cont,  NULL, LV_PART_SCROLLBAR);

    g_cont = cont;
    GuiAboutTermsEntranceWidget(cont);
}

void GuiAboutTermsWidgetsDeInit()
{
    if (g_cont != NULL) {
        GUI_DEL_OBJ(g_qrCodeCont)
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiAboutTermsWidgetsRefresh()
{
    GuiAboutNVSBarInit();
}

void GuiAboutTermsWidgetsRestart()
{

}

static void GuiAboutNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("about_terms_title"));
}

static lv_obj_t* GuiGetTermsItemContainer(lv_obj_t* parent, const char *title, const char *content, int *height)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_width(cont, 408);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(cont, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    }

    lv_obj_t *titleLabel;
    titleLabel = GuiCreateLittleTitleLabel(cont, title);
    lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align(titleLabel, LV_ALIGN_DEFAULT, 0, 0);
    int16_t titleHight = GetLvObjHeight(titleLabel);

    lv_obj_t *contentLabel = GuiCreateIllustrateLabel(cont, content);
    lv_obj_set_style_text_opa(contentLabel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(contentLabel, LV_ALIGN_DEFAULT, 0, 4 + titleHight);
    int16_t contentHeight = GetLvObjHeight(contentLabel);

    *height = titleHight + contentHeight + 4;

    lv_obj_set_height(cont, *height);

    return cont;
}

void GuiAboutTermsEntranceWidget(lv_obj_t *parent)
{
    GuiAddObjFlag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *label, *img;
    label = GuiCreateLittleTitleLabel(parent, _("about_terms_subtitle"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 22, 13);

    label = GuiCreateIllustrateLabel(parent, _("about_terms_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_90, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    label = GuiCreateIllustrateLabel(parent, g_termsWebSiteUrl);
    lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);

    img = GuiCreateImg(parent, &imgQrcodeTurquoise);
    lv_obj_align_to(img, label, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);

    int dy = lv_obj_get_y2(label) + 24;
    int height = 0;

    const char *title = _("about_terms_eligibility");
    const char *text = _("about_terms_eligibility_desc");
    lv_obj_t *itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    title = _("about_terms_product_and_services");
    text = _("about_terms_product_and_services_desc");
    itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    title = _("about_terms_risks");
    text = _("about_terms_risks_desc");
    itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    title = _("about_terms_prohibited_conduct");
    text = _("about_terms_prohibited_product_desc");
    itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    title = _("about_terms_ownership");
    text = _("about_terms_ownership_desc");
    itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    title = _("about_terms_discontinuance_service");
    text = _("about_terms_discontinuance_service_desc");
    itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    title = _("about_terms_disclaimers");
    text = _("about_terms_disclaimers_desc");
    itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    title = _("about_terms_contact_us");
    text = _("about_terms_contact_us_desc");
    itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    title = _("about_terms_modification");
    text = _("about_terms_modification_desc");
    itemObj = GuiGetTermsItemContainer(g_cont, title, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    lv_obj_t *bottom = lv_obj_create(g_cont);
    lv_obj_set_size(bottom, 480, 38);
    lv_obj_set_style_bg_opa(bottom, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(bottom, LV_ALIGN_DEFAULT, 0, dy);
}

static int GetLvObjHeight(lv_obj_t *obj)
{
    lv_obj_update_layout(obj);
    return lv_obj_get_height(obj);
}

static void GuiQrcodeHandler(lv_event_t *e)
{
    lv_obj_t *parent, *button, *qrCodeCont, *qrCode, *label;

        if (g_qrCodeCont == NULL) {
            g_qrCodeCont = GuiCreateHintBox(654);
            parent = g_qrCodeCont;

            qrCodeCont = lv_obj_create(parent);
            lv_obj_set_size(qrCodeCont, 408, 408);
            lv_obj_set_style_border_width(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_clip_corner(qrCodeCont, 0, 0);
            lv_obj_set_style_pad_all(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(qrCodeCont, 16, LV_PART_MAIN);
            lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_color(qrCodeCont, WHITE_COLOR, LV_PART_MAIN);
            lv_obj_align(qrCodeCont, LV_ALIGN_BOTTOM_MID, 0, -210);

            qrCode = lv_qrcode_create(qrCodeCont, 360, BLACK_COLOR, WHITE_COLOR);
            lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 0);
            lv_qrcode_update(qrCode, g_termsWebSiteUrl, (uint32_t)strnlen_s(g_termsWebSiteUrl, BUFFER_SIZE_128));

            label = GuiCreateLittleTitleLabel(parent, _("about_terms_title"));
            lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -156);
            label = GuiCreateIllustrateLabel(parent, g_termsWebSiteUrl);
            lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -114);

            button = GuiCreateTextBtn(parent, _("OK"));
            lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
            lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
            lv_obj_add_event_cb(button, CloseQrcodeHandler, LV_EVENT_CLICKED, NULL);
            lv_obj_clear_flag(g_qrCodeCont, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void CloseQrcodeHandler(lv_event_t *e)
{
        lv_obj_add_flag(g_qrCodeCont, LV_OBJ_FLAG_HIDDEN);
}