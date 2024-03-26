#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_about_widgets.h"
#include "gui_setting_widgets.h"
#include "gui_page.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

#define CONTACT_ITEM_COUNT 4
typedef struct {
    const char *url;
    const char *title;
    const lv_img_dsc_t *icon;
    const lv_img_dsc_t *qrIcon;
} ContactItem_t;

static ContactItem_t *g_contactItems = NULL;

static lv_obj_t *g_cont;
static lv_obj_t *g_qrCodeCont;
static PageWidget_t *g_pageWidget;

static void GuiAboutNVSBarInit();
static void GuiAboutKeystoneEntranceWidget(lv_obj_t *parent);
static void ShowQRDialogHandler(lv_event_t *e);
static void GuiCloseQrcodeHandler(lv_event_t *e);

void ContactItemsInit()
{
    if (g_contactItems == NULL) {
        g_contactItems = SRAM_MALLOC(CONTACT_ITEM_COUNT * sizeof(ContactItem_t));
        g_contactItems[0].url = _("about_keystone_website_url");
        g_contactItems[0].title = _("about_keystone_website");
        g_contactItems[0].icon = &imgNetwork;
        g_contactItems[0].qrIcon = &imgQrcodeTurquoise;

        g_contactItems[1].url = _("about_keystone_twitter_url");
        g_contactItems[1].title = _("about_keystone_twitter");
        g_contactItems[1].icon = &imgTwitter;
        g_contactItems[1].qrIcon = &imgQrcodeTurquoise;

        g_contactItems[2].url = _("about_keystone_discord_url");
        g_contactItems[2].title = _("about_keystone_discord");
        g_contactItems[2].icon = &imgDiscord;
        g_contactItems[2].qrIcon = &imgQrcodeTurquoise;

        g_contactItems[3].url = _("about_keystone_telegram_url");
        g_contactItems[3].title = _("about_keystone_telegram");
        g_contactItems[3].icon = &imgTelegram;
        g_contactItems[3].qrIcon = &imgQrcodeTurquoise;
    }
}

void GuiAboutKeystoneWidgetsInit()
{
    ContactItemsInit();
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    g_cont = cont;
    GuiAboutKeystoneEntranceWidget(cont);
}

void GuiAboutKeystoneWidgetsDeInit()
{
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiAboutKeystoneWidgetsRefresh()
{
    GuiAboutNVSBarInit();
}


void GuiAboutKeystoneWidgetsRestart()
{}


static void GuiAboutNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("about_keystone_title"));
}


void GuiAboutKeystoneEntranceWidget(lv_obj_t *parent)
{

    lv_obj_t *imgIcon, *label, *imgQr;
    for (int i = 0; i < CONTACT_ITEM_COUNT; i++) {
        imgIcon = GuiCreateImg(parent, g_contactItems[i].icon);
        label = GuiCreateIllustrateLabel(parent, g_contactItems[i].url);
        imgQr = GuiCreateImg(parent, g_contactItems[i].qrIcon);

        GuiButton_t table[] = {
            {
                .obj = imgIcon,
                .align = LV_ALIGN_LEFT_MID,
                .position = {27, 0},
            },
            {
                .obj = label,
                .align = LV_ALIGN_LEFT_MID,
                .position = {76, 0},
            },
            {
                .obj = imgQr,
                .align = LV_ALIGN_LEFT_MID,
                .position = {400, 0},
            }
        };
        lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                           ShowQRDialogHandler, &g_contactItems[i]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 10, 12 + 84 * i);
    }
}



static void ShowQRDialogHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *parent, *button, *qrCodeCont, *qrCode, *label;

    if (code == LV_EVENT_CLICKED) {
        ContactItem_t *contactItem = (ContactItem_t*)lv_event_get_user_data(e);

        g_qrCodeCont = GuiCreateHintBox(g_cont, 480, 656, true);
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
        lv_obj_align(qrCodeCont, LV_ALIGN_BOTTOM_MID, 0, -220);

        qrCode = lv_qrcode_create(qrCodeCont, 360, BLACK_COLOR, WHITE_COLOR);
        lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 0);
        lv_qrcode_update(qrCode, contactItem->url, (uint32_t)strnlen_s(contactItem->url, 128));

        label = GuiCreateLittleTitleLabel(parent, contactItem->title);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -156);
        label = GuiCreateIllustrateLabel(parent, contactItem->url);
        lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -114);

        button = GuiCreateAdaptButton(parent, _("OK"));
        lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(button, GuiCloseQrcodeHandler, LV_EVENT_CLICKED, NULL);
    }
}

static void GuiCloseQrcodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_qrCodeCont != NULL) {
            lv_obj_del(g_qrCodeCont);
            g_qrCodeCont = NULL;
        }
    }
}