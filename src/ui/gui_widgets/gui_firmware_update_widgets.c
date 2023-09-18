/**************************************************************************************************
 * Copyright (c) Keystone 2020-2025. All rights reserved.
 * Description: Gui firmware update widgets.
 * Author: leon sun
 * Create: 2023-7-18
 ************************************************************************************************/

#include "gui_firmware_update_widgets.h"
#include "gui.h"
#include "gui_views.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_obj.h"
#include "gui_status_bar.h"
#include "gui_lock_widgets.h"
#include "gui_setup_widgets.h"
#include "background_task.h"
#include "firmware_update.h"
#include "keystore.h"
#include "gui_keyboard_hintbox.h"
#include "gui_page.h"


typedef enum {
    FIRMWARE_UPDATE_SELECT = 0,
    FIRMWARE_UPDATE_USB_INSTRUCTION,
    FIRMWARE_UPDATE_SD_INSTRUCTION,

    FIRMWARE_UPDATE_USB_UPDATING,

    FIRMWARE_UPDATE_BUTT,
} CREATE_WALLET_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    lv_obj_t *tileSelect;
    lv_obj_t *tileUsbInstruction;
    lv_obj_t *tileSdInstruction;
    lv_obj_t *tileUpdating;
    lv_obj_t *qrCodeCont;
} FirmwareUpdateWidgets_t;

static void GuiCreateSelectTile(lv_obj_t *parent);
static void GuiViaSdCardHandler(lv_event_t *e);
static void GuiViaUsbHandler(lv_event_t *e);
static void GuiCreateUsbInstructionTile(lv_obj_t *parent);
static void GuiQrcodeHandler(lv_event_t *e);
static void CloseQrcodeHandler(lv_event_t *e);
static int GetEntryEnum(void);
static void GuiCreateSdCardnstructionTile(lv_obj_t *parent);
static void FirmwareSdcardUpdateHandler(lv_event_t *e);

static FirmwareUpdateWidgets_t g_firmwareUpdateWidgets;
static const char g_firmwareUpdateUrl[] = "https://keyst.one/webusb";
static const char g_firmwareSdUpdateUrl[] = "https://keyst.one/firmware";
static lv_obj_t *g_waitAnimCont = NULL;
static void *g_param = NULL;
static lv_obj_t *g_noticeHintBox = NULL;

static KeyboardWidget_t *g_keyboardWidget = NULL;
static PageWidget_t *g_pageWidget;


void GuiCreateSdCardUpdateHintbox(char *version)
{
    GUI_DEL_OBJ(g_noticeHintBox)
    static uint32_t param = SIG_INIT_SD_CARD_OTA_COPY;
    char desc[150] = {0};
    sprintf(desc, "A new firmware version is available. Do you want to update your device's firmware to the latest version?");
    g_noticeHintBox = GuiCreateResultHintbox(lv_scr_act(), 416, &imgFirmwareUp, "Update Available",
                      desc, "Not Now", DARK_GRAY_COLOR, "Update", ORANGE_COLOR);
    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeHintBox);
    lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);

    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeHintBox);
    lv_obj_add_event_cb(rightBtn, FirmwareSdcardUpdateHandler, LV_EVENT_CLICKED, &param);
}

static int GetEntryEnum(void)
{
    if (g_param != NULL) {
        return *(int *)g_param;
    }
    return FIRMWARE_UPDATE_ENTRY_BUTT;
}

void GuiFirmwareUpdateInit(void *param)
{
    g_param = param;
    lv_obj_t *tileView;
    CLEAR_OBJECT(g_firmwareUpdateWidgets);
    g_pageWidget = CreatePageWidget();
    g_firmwareUpdateWidgets.cont = g_pageWidget->contentZone;
    tileView = lv_tileview_create(g_firmwareUpdateWidgets.cont);
    g_firmwareUpdateWidgets.tileView = tileView;
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(tileView, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(tileView, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_clear_flag(tileView, LV_OBJ_FLAG_SCROLLABLE);

    g_firmwareUpdateWidgets.tileSelect = lv_tileview_add_tile(tileView, FIRMWARE_UPDATE_SELECT, 0, LV_DIR_HOR);
    GuiCreateSelectTile(g_firmwareUpdateWidgets.tileSelect);

    g_firmwareUpdateWidgets.tileUsbInstruction = lv_tileview_add_tile(tileView, FIRMWARE_UPDATE_USB_INSTRUCTION, 0, LV_DIR_HOR);
    GuiCreateUsbInstructionTile(g_firmwareUpdateWidgets.tileUsbInstruction);

    g_firmwareUpdateWidgets.tileSdInstruction = lv_tileview_add_tile(tileView, FIRMWARE_UPDATE_SD_INSTRUCTION, 0, LV_DIR_HOR);
    GuiCreateSdCardnstructionTile(g_firmwareUpdateWidgets.tileSdInstruction);

    g_firmwareUpdateWidgets.currentTile = FIRMWARE_UPDATE_SELECT;
}

void GuiFirmwareSdCardCopy(void)
{
    if (GuiCheckIfViewOpened(&g_lockView)) {
        GuiLockScreenTurnOff();
    }
    GUI_DEL_OBJ(g_waitAnimCont)
    GUI_DEL_OBJ(g_noticeHintBox)

    g_waitAnimCont = GuiCreateAnimHintBox(lv_scr_act(), 480, 386, 82);
    lv_obj_t *title = GuiCreateTextLabel(g_waitAnimCont, _("Copying"));
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -194);
    lv_obj_t *desc = GuiCreateNoticeLabel(g_waitAnimCont, _("The copying process of the latest firmware from the MicroSD card may take 15 to 45 seconds."));
    lv_obj_align(desc, LV_ALIGN_BOTTOM_MID, 0, -86);
    lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);
}

void GuiFirmwareSdCardCopyResult(bool en)
{
    GUI_DEL_OBJ(g_waitAnimCont)
    if (en) {
        printf("copy success\n");
    } else {
        printf("copy failed\n");
        g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_UPDATE_FIRMWARE_NOT_DETECTED, &g_noticeHintBox);
    }
}

void GuiFirmwareUpdateDeInit(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    g_param = NULL;
    printf("GuiFirmwareUpdateDeInit\n");
    GUI_DEL_OBJ(g_noticeHintBox)
    GUI_DEL_OBJ(g_waitAnimCont)
    lv_obj_del(g_firmwareUpdateWidgets.cont);
    CLEAR_OBJECT(g_firmwareUpdateWidgets);
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}


void GuiFirmwareUpdateRefresh(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "");
    if (GetEntryEnum() == FIRMWARE_UPDATE_ENTRY_SETUP && g_firmwareUpdateWidgets.currentTile == FIRMWARE_UPDATE_SELECT) {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_NEW_SKIP, OpenViewHandler, &g_purposeView);
    } else {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    }

    if (GetEntryEnum() == FIRMWARE_UPDATE_ENTRY_SETUP) {
        GuiSetSetupPhase(SETUP_PAHSE_FIRMWARE_UPDATE);
        if (g_reboot) {
            if (!GuiJudgeCurrentPahse(SETUP_PAHSE_FIRMWARE_UPDATE)) {
                GuiFrameOpenView(&g_purposeView);
            } else {
                g_reboot = false;
            }
        }
    }
    GuiCreateSelectTile(g_firmwareUpdateWidgets.tileSelect);
}


void GuiFirmwareUpdatePrevTile(void)
{
    switch (g_firmwareUpdateWidgets.currentTile) {
    case FIRMWARE_UPDATE_SELECT:
        GuiCLoseCurrentWorkingView();
        return;
    case FIRMWARE_UPDATE_SD_INSTRUCTION:
        g_firmwareUpdateWidgets.currentTile--;
    case FIRMWARE_UPDATE_USB_INSTRUCTION:
        g_firmwareUpdateWidgets.currentTile--;
        break;
    default:
        return;
    }
    lv_obj_set_tile_id(g_firmwareUpdateWidgets.tileView, g_firmwareUpdateWidgets.currentTile, 0, LV_ANIM_OFF);
    if (g_firmwareUpdateWidgets.tileView == FIRMWARE_UPDATE_SELECT) {
        GuiCreateSelectTile(g_firmwareUpdateWidgets.tileSelect);
    }
    GuiFirmwareUpdateRefresh();
}


static void GuiCreateSelectTile(lv_obj_t *parent)
{
    lv_obj_clean(parent);
    uint8_t memberCnt = 3;
    lv_obj_t *label, *img, *button, *imgArrow, *line;
    label = GuiCreateTitleLabel(parent, "Firmware update");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);
    label = GuiCreateNoticeLabel(parent, "Update your firmware to the latest version to leverage the newly added features in Keystone.");
    lv_obj_set_width(label, 408);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 72);

    img = GuiCreateImg(parent, &imgMicroSd);
    label = GuiCreateLittleTitleLabel(parent, "Via MicroSD Card");
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    GuiButton_t table1[4] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {24, 40},},
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {76, 40},},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {372, 40},},
    };

    // char fileVersion[16] = {0};
    // if (CheckOtaBinVersion(fileVersion)) {
    //     lv_obj_t *versionLabel = GuiCreateIllustrateLabel(parent, fileVersion);
    //     lv_obj_set_style_text_color(versionLabel, ORANGE_COLOR, LV_PART_MAIN);
    //     lv_label_set_text_fmt(versionLabel, "v%s Available", fileVersion);
    //     table1[3].align = LV_ALIGN_DEFAULT;
    //     table1[3].position.x = 76;
    //     table1[3].position.y = 81;
    //     table1[3].obj = versionLabel;
    //     memberCnt = 4;
    // }

    button = GuiCreateButton(parent, 408, 120, table1, memberCnt, GuiViaSdCardHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 210);

    img = GuiCreateImg(parent, &imgUsbConnection);
    label = GuiCreateLittleTitleLabel(parent, "Via USB");
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    GuiButton_t table2[] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {24, 40},},
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {76, 40},},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {372, 40},},
    };
    button = GuiCreateButton(parent, 408, 120, table2, NUMBER_OF_ARRAYS(table2), GuiViaUsbHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 330);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 210);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 330);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 450);
}


static void GuiViaSdCardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_firmwareUpdateWidgets.currentTile = FIRMWARE_UPDATE_SD_INSTRUCTION;
        lv_obj_set_tile_id(g_firmwareUpdateWidgets.tileView, g_firmwareUpdateWidgets.currentTile, 0, LV_ANIM_OFF);
        GuiFirmwareUpdateRefresh();
    }
}

static void GuiViaUsbHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_firmwareUpdateWidgets.currentTile = FIRMWARE_UPDATE_USB_INSTRUCTION;
        lv_obj_set_tile_id(g_firmwareUpdateWidgets.tileView, g_firmwareUpdateWidgets.currentTile, 0, LV_ANIM_OFF);
        GuiFirmwareUpdateRefresh();
    }
}


static void GuiCreateUsbInstructionTile(lv_obj_t *parent)
{
    lv_obj_t *label, *img;

    label = GuiCreateTitleLabel(parent, _("firmware_update_usb_title"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 1#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 100);
    label = GuiCreateIllustrateLabel(parent, "#F5870A 2#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 172);
    label = GuiCreateIllustrateLabel(parent, "#F5870A 3#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 278);
    label = GuiCreateIllustrateLabel(parent, "#F5870A 4#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 350);

    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc1"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 100);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc2"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 172);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc2_link"));
    lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 236);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(parent, &imgQrcodeTurquoise);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 322, 239);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc3"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 278);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc4"));
    lv_obj_set_width(label, 390);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 350);

    label = GuiCreateBoldIllustrateLabel(parent, _("firmware_update_usb_title2"));
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 480);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc5"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 510);
}

static void FirmwareSdcardUpdateHandler(lv_event_t *e)
{
    char fileVersion[16] = {0};
    GUI_DEL_OBJ(g_noticeHintBox)
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (CHECK_BATTERY_LOW_POWER()) {
            g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_KEYSTORE_SAVE_LOW_POWER, &g_noticeHintBox);
        } else if (!SdCardInsert()) {
            g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_UPDATE_FIRMWARE_NOT_DETECTED, &g_noticeHintBox);
        } else if (CheckOtaBinVersion(fileVersion)) {
            uint8_t accountCnt = 0;
            GetExistAccountNum(&accountCnt);
            if (accountCnt == 0) {
                GuiFirmwareSdCardCopy();
                GuiModelCopySdCardOta();
            } else {
                GuiDeleteKeyboardWidget(g_keyboardWidget);
                g_keyboardWidget = GuiCreateKeyboardWidget(g_firmwareUpdateWidgets.cont);
                SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
            }
        } else {
            g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_UPDATE_FIRMWARE_NOT_DETECTED, &g_noticeHintBox);
        }
    }
}

static void GuiCreateSdCardnstructionTile(lv_obj_t *parent)
{
    static uint32_t param = SIG_INIT_SD_CARD_OTA_COPY;
    lv_obj_t *label, *img;

    label = GuiCreateTitleLabel(parent, "Update via MicroSD");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 1#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 100);
    label = GuiCreateIllustrateLabel(parent, "#F5870A 2#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 172);
    label = GuiCreateIllustrateLabel(parent, "#F5870A 3#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 248);
    label = GuiCreateIllustrateLabel(parent, "#F5870A 4#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 350);

    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc1"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 100);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc2"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 172);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc2_link"));
    lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 206);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(parent, &imgQrcodeTurquoise);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 322, 206);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc3"));
    lv_obj_set_width(label, 390);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 248);

    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc4"));
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 350);

    lv_obj_t *btn = GuiCreateBtn(parent, _("Update"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 710 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_event_cb(btn, FirmwareSdcardUpdateHandler, LV_EVENT_CLICKED, &param);
}

static void GuiQrcodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *parent, *button, *qrCodeCont, *qrCode, *label;

    if (code == LV_EVENT_CLICKED) {
        if (g_firmwareUpdateWidgets.qrCodeCont == NULL) {
            g_firmwareUpdateWidgets.qrCodeCont = GuiCreateHintBox(g_firmwareUpdateWidgets.tileUsbInstruction, 480, 654, true);
            parent = g_firmwareUpdateWidgets.qrCodeCont;

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
            if (g_firmwareUpdateWidgets.currentTile == FIRMWARE_UPDATE_USB_INSTRUCTION) {
                lv_qrcode_update(qrCode, _("firmware_update_usb_qr_link"), (uint32_t)strlen(_("firmware_update_usb_qr_link")));
            } else {
                lv_qrcode_update(qrCode, g_firmwareSdUpdateUrl, (uint32_t)strlen(g_firmwareSdUpdateUrl));
            }

            label = GuiCreateLittleTitleLabel(parent, _("firmware_update_usb_qr_title"));
            lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -156);
            if (g_firmwareUpdateWidgets.currentTile == FIRMWARE_UPDATE_USB_INSTRUCTION) {
                label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_qr_link"));
            } else {
                label = GuiCreateIllustrateLabel(parent, g_firmwareSdUpdateUrl);
            }
            lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -114);

            button = GuiCreateBtn(parent, "OK");
            lv_obj_set_size(button, 94, 66);
            lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
            lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
            lv_obj_add_event_cb(button, CloseQrcodeHandler, LV_EVENT_CLICKED, NULL);
        }
    }
}


static void CloseQrcodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_firmwareUpdateWidgets.qrCodeCont)
    }
}

void GuiFirmwareUpdateVerifyPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}