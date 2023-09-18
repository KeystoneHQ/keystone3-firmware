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

static void GuiAboutNVSBarInit();
static void GuiAboutTermsEntranceWidget(lv_obj_t *parent);
static int GetLvObjHeight(lv_obj_t *obj);
static lv_obj_t* GuiGetTermsItemContainer(lv_obj_t* parent, char *tittle, char *content, int *height);
static void GuiQrcodeHandler(lv_event_t *e);
static void CloseQrcodeHandler(lv_event_t *e);

static lv_obj_t *g_cont;

static const char g_termsWebSiteUrl[] = "https://keyst.one/terms";
static lv_obj_t *g_qrCodeCont;
static PageWidget_t *g_pageWidget;


void GuiAboutTermsWidgetsInit()
{
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
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Terms of Use");
}


static lv_obj_t* GuiGetTermsItemContainer(lv_obj_t* parent, char *tittle, char *content, int *height)
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

    lv_obj_t *tittleLabel;
    tittleLabel = GuiCreateLittleTitleLabel(cont, tittle);
    lv_label_set_long_mode(tittleLabel, LV_LABEL_LONG_WRAP);
    lv_obj_align(tittleLabel, LV_ALIGN_DEFAULT, 0, 0);
    int16_t tittleHight = GetLvObjHeight(tittleLabel);


    lv_obj_t *contentLabel = lv_label_create(cont);
    lv_label_set_text(contentLabel, content);
    lv_obj_set_style_text_font(contentLabel, g_defTextFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(contentLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(contentLabel, LV_OPA_56, LV_PART_MAIN);
    lv_label_set_long_mode(contentLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(contentLabel, 410);
    lv_obj_align(contentLabel, LV_ALIGN_DEFAULT, 0, 4 + tittleHight);
    int16_t contentHeight = GetLvObjHeight(contentLabel);

    *height = tittleHight + contentHeight + 4;

    lv_obj_set_height(cont, *height);

    return cont;
}

void GuiAboutTermsEntranceWidget(lv_obj_t *parent)
{


    lv_obj_t *label, *img;
    label = GuiCreateLittleTitleLabel(parent, "Keystone Terms of Use");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 22, 13);

    label = GuiCreateLabel(parent, "To access the full version of the TERMS OF USE, please visit the following link:");
    lv_obj_set_style_text_opa(label, LV_OPA_90, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 22, 77);


    label = GuiCreateIllustrateLabel(parent, g_termsWebSiteUrl);
    lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 22, 141);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(parent, &imgQrcodeTurquoise);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 254, 144);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);


    int dy = 195;
    int height = 0;

    char *tittle = "Eligibility";
    char *text = "You must be 18 years old or above to access and use our Products or Services.";
    lv_obj_t *itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Keystone Product & Services";
    text = "Our hardware wallet securely manages cryptocurrencies.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "No Retrieval of Sensitive Information";
    text = "We do not store your sensitive information like passwords or seed phrases. Keep your credentials safe.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Risks";
    text = "Be aware of the risks associated with cryptocurrencies and technology vulnerabilities.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Limitation of Liability & Disclaimer of Warranties";
    text = "We provide our Services \"as is\" without warranties. We are not liable for any losses incurred while using our Products or Services.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Prohibited Conduct";
    text = "Our Products and Services are protected by intellectual property laws.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Ownership & Proprietary Rights";
    text = "You are responsible for your actions while using the Products and Services.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Indemnity";
    text = "You must be 18 years old or above to access and use our Products or Services.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Discontinuance of Services";
    text = "We may modify or discontinue our services. Remember to back up your seed phrase to access your cryptocurrencies.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "DISCLAIMERS";
    text = "The information provided is not financial advice. Seek professional advice before making any decisions.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Governing Law and Dispute Resolution";
    text = "The Terms are governed by Hong Kong SAR laws, and any dispute must be filed within one year.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Contact Us";
    text = "If you have any questions or concerns, please email us at support@keyst.one.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
    lv_obj_align(itemObj, LV_ALIGN_DEFAULT, 22, dy);
    dy += (height + 24);

    tittle = "Modification of these Terms";
    text = "We reserve the right to change these Terms at our discretion.";
    itemObj = GuiGetTermsItemContainer(g_cont, tittle, text, &height);
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
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *parent, *button, *qrCodeCont, *qrCode, *label;

    if (code == LV_EVENT_CLICKED) {
        if (g_qrCodeCont == NULL) {
            g_qrCodeCont = GuiCreateHintBox(g_cont, 480, 654, true);
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
            lv_qrcode_update(qrCode, g_termsWebSiteUrl, (uint32_t)strlen(g_termsWebSiteUrl));

            label = GuiCreateLittleTitleLabel(parent, "Terms of Use");
            lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -156);
            label = GuiCreateIllustrateLabel(parent, g_termsWebSiteUrl);
            lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -114);

            button = GuiCreateBtn(parent, "OK");
            lv_obj_set_size(button, 94, 66);
            lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
            lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
            lv_obj_add_event_cb(button, CloseQrcodeHandler, LV_EVENT_CLICKED, NULL);
        } else {
            lv_obj_clear_flag(g_qrCodeCont, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void CloseQrcodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_add_flag(g_qrCodeCont, LV_OBJ_FLAG_HIDDEN);
    }
}