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


typedef struct {
    const char *url;
    const char *tittle;
    const lv_img_dsc_t *icon;
    const lv_img_dsc_t *qrIcon;
} ContactItem_t;

static ContactItem_t g_contactItems[] = {
    {"keyst.one", "WebSite", &imgNetwork, &imgQrcodeTurquoise},
    {"twitter.com/KeystoneWallet", "Twitter", &imgTwitter, &imgQrcodeTurquoise},
    {"keyst.one/discord", "Discord", &imgDiscord, &imgQrcodeTurquoise},
    {"t.me/keystonewallet", "Telegram", &imgTelegram, &imgQrcodeTurquoise},
};

static lv_obj_t *g_cont;
static lv_obj_t *g_qrCodeCont;

static void GuiAboutNVSBarInit();
static void GuiAboutKeystoneEntranceWidget(lv_obj_t *parent);
static void ShowQRDialogHandler(lv_event_t *e);
static void GuiCloseQrcodeHandler(lv_event_t *e);

void GuiAboutKeystoneWidgetsInit()
{
    GuiAboutNVSBarInit();

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    g_cont = cont;
    GuiAboutKeystoneEntranceWidget(cont);
}

void GuiAboutKeystoneWidgetsDeInit()
{
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
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
    GuiNvsBarSetLeftCb(NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "About Keystone");
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
}


void GuiAboutKeystoneEntranceWidget(lv_obj_t *parent)
{

    lv_obj_t *imgIcon, *label, *imgQr;
    for (int i = 0; i < NUMBER_OF_ARRAYS(g_contactItems); i++) {
        imgIcon = GuiCreateImg(parent, g_contactItems[i].icon);
        label = GuiCreateLabel(parent, g_contactItems[i].url);
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
        lv_qrcode_update(qrCode, contactItem->url, (uint32_t)strlen(contactItem->url));

        label = GuiCreateLittleTitleLabel(parent, contactItem->tittle);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -156);
        label = GuiCreateIllustrateLabel(parent, contactItem->url);
        lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -114);

        button = GuiCreateBtn(parent, "OK");
        lv_obj_set_size(button, 94, 66);
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