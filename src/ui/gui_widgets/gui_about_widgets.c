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
#include "version.h"
#include "err_code.h"
#include "firmware_update.h"

static lv_obj_t *g_cont;

static void GuiAboutNVSBarInit();
static void GuiAboutEntranceWidget(lv_obj_t *parent);
// static void UnHandler(lv_event_t *e);

void GuiAboutWidgetsInit()
{
    GuiAboutNVSBarInit();

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    g_cont = cont;
    lv_obj_t *img = GuiCreateImg(cont, &imgAboutIcon);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 45);
    GuiAboutEntranceWidget(cont);
}

void GuiAboutWidgetsDeInit()
{
    GUI_DEL_OBJ(g_cont)
}

void GuiAboutWidgetsRefresh()
{
    GuiAboutNVSBarInit();
}


void GuiAboutWidgetsRestart()
{}

static void GuiAboutNVSBarInit()
{
    GuiNvsBarSetLeftCb(NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, _("device_setting_about_title"));
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
}


void GuiAboutEntranceWidget(lv_obj_t *parent)
{
    //about ketystone
    lv_obj_t *label = GuiCreateTextLabel(parent, "About Keystone");
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);
    uint8_t memberCnt = 2;

    GuiButton_t table[3] = {
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 24},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_DEFAULT,
            .position = {396, 24},
        },
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, 2,
                                       OpenViewHandler, &g_aboutKeystoneView);

    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 205);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 297);

    //terms of use
    label = GuiCreateTextLabel(parent, "Terms of Use");
    imgArrow = GuiCreateImg(parent, &imgArrowRight);

    table[0].obj = label;
    table[1].obj = imgArrow;

    button = GuiCreateButton(parent, 456, 84, table, 2,
                             OpenViewHandler, &g_aboutTermsView);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 306);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 398);

    //device info
    label = GuiCreateTextLabel(parent, "Device Info");
    imgArrow = GuiCreateImg(parent, &imgArrowRight);

    table[0].obj = label;
    table[1].obj = imgArrow;

    button = GuiCreateButton(parent, 456, 84, table, 2,
                             OpenViewHandler, &g_aboutInfoView);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 407);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 499);

    //firmware
    char version[32] = {0};
    char fileVersion[16] = {0};
    GetSoftWareVersion(version);

    label = GuiCreateTextLabel(parent, version);
    imgArrow = GuiCreateImg(parent, &imgArrowRight);

    table[0].obj = label;
    table[1].obj = imgArrow;
    // if (CheckOtaBinVersion(fileVersion)) {
    //     lv_obj_t *versionLabel = GuiCreateIllustrateLabel(parent, fileVersion);
    //     lv_obj_set_style_text_color(versionLabel, ORANGE_COLOR, LV_PART_MAIN);
    //     lv_label_set_text_fmt(versionLabel, "v%s Available", fileVersion);
    //     table[2].align = LV_ALIGN_BOTTOM_LEFT;
    //     table[2].position.x = 24;
    //     table[2].position.y = -24;
    //     table[2].obj = versionLabel;
    //     memberCnt = 3;
    // }

    button = GuiCreateButton(parent, 456, 84 + (memberCnt - 2) * 34, table, memberCnt,
                             OpenViewHandler, &g_firmwareUpdateView);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 508);
}

// static void UnHandler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     if (code == LV_EVENT_CLICKED) {
//     }
// }