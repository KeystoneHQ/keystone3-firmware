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
#include "account_manager.h"
#include "gui_about_info_widgets.h"
#include "secret_cache.h"
#ifndef COMPILE_SIMULATOR
#include "user_fatfs.h"
#endif

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
static void GuiFirmwareUpdateViewSha256(char *version, uint8_t percent);
static void CloseQrcodeHandler(lv_event_t *e);
static int GetEntryEnum(void);
static void GuiCreateSdCardnstructionTile(lv_obj_t *parent);
static void FirmwareSdcardUpdateHandler(lv_event_t *e);
static void FirmwareSdcardCheckSha256Handler(lv_event_t *e);
static void FirmwareSdcardCheckSha256HintBoxHandler(lv_event_t *e);
static void GuiFirmwareUpdateCancelUpdate(lv_event_t *e);

static FirmwareUpdateWidgets_t g_firmwareUpdateWidgets;
static const char *g_firmwareUpdateUrl = NULL;
static const char *g_firmwareSdUpdateUrl = NULL;
static lv_obj_t *g_waitAnimCont = NULL;
static void *g_param = NULL;
static lv_obj_t *g_noticeHintBox = NULL;
static lv_obj_t *g_calCheckSumLabel = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;
static PageWidget_t *g_pageWidget;

static void UrlInit()
{
    if (g_firmwareUpdateUrl == NULL) {
        g_firmwareUpdateUrl = _("firmware_update_usb_qr_link");
    }
    if (g_firmwareSdUpdateUrl == NULL) {
        g_firmwareSdUpdateUrl = _("firmware_update_sd_desc2_link");
    }
}

void GuiCreateSdCardUpdateHintbox(char *version, bool checkSumDone)
{
    GUI_DEL_OBJ(g_noticeHintBox)
    static uint32_t param = SIG_INIT_SD_CARD_OTA_COPY;
    char desc[150] = {0};

    sprintf(desc, _("firmware_update_sd_dialog_desc"));
    uint16_t height = checkSumDone ? 518 : 458;
    g_noticeHintBox = GuiCreateUpdateHintbox(lv_scr_act(), height, &imgFirmwareUp, _("firmware_update_sd_dialog_title"),
                      desc, _("not_now"), DARK_GRAY_COLOR, _("Update"), ORANGE_COLOR, checkSumDone);

    g_calCheckSumLabel = lv_obj_get_child(g_noticeHintBox, 3);
    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeHintBox);
    lv_obj_add_event_cb(leftBtn, GuiFirmwareUpdateCancelUpdate, LV_EVENT_CLICKED, &g_noticeHintBox);

    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeHintBox);
    lv_obj_add_event_cb(rightBtn, FirmwareSdcardUpdateHandler, LV_EVENT_CLICKED, &param);
    if (checkSumDone) {
        char hash[128] = {0};
        char tempBuf[128] = {0};
        SecretCacheGetChecksum(hash);
        ConvertToLowerCase(hash);
        snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);
        lv_label_set_text_fmt(g_calCheckSumLabel, "Checksum(v%s):\n%s", version, tempBuf);
    } else {
        lv_obj_t *btn = GuiCreateBtn(g_noticeHintBox, _(""));
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
        lv_obj_set_size(btn, 250, 50);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -120);
        lv_obj_add_event_cb(btn, FirmwareSdcardCheckSha256HintBoxHandler, LV_EVENT_CLICKED, NULL);
        lv_label_set_text_fmt(g_calCheckSumLabel, _("firmware_update_sd_checksum_fmt_version"), version);
    }
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
    UrlInit();
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
    // if (GuiCheckIfViewOpened(&g_lockView)) {
    //     GuiLockScreenTurnOff();
    // }
    GUI_DEL_OBJ(g_waitAnimCont)
    GUI_DEL_OBJ(g_noticeHintBox)

    g_waitAnimCont = GuiCreateAnimHintBox(lv_scr_act(), 480, 386, 82);
    lv_obj_t *title = GuiCreateTextLabel(g_waitAnimCont, _("firmware_update_sd_copying_title"));
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -194);
    lv_obj_t *desc = GuiCreateNoticeLabel(g_waitAnimCont, _("firmware_update_sd_copying_desc"));
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
    PassWordPinHintRefresh(g_keyboardWidget);
}

void GuiFirmwareUpdateWidgetRefresh(void)
{
    PassWordPinHintRefresh(g_keyboardWidget);
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

void GuiFirmwareUpdateSha256Percent(uint8_t percent)
{
    printf("percent = %d\n", percent);
    if (g_noticeHintBox == NULL) {
        return;
    }
    char version[16] = {0};
    if (percent == 100) {
        CheckOtaBinVersion(version);
    }

    if (lv_obj_is_valid(g_calCheckSumLabel)) {
        lv_label_set_text_fmt(g_calCheckSumLabel, _("firmware_update_sd_checksum_fmt"), percent);
        if (percent == 100) {
            GuiCreateSdCardUpdateHintbox(version, true);
        }
    } else {
        GuiFirmwareUpdateViewSha256(version, percent);
    }
}

static void GuiCreateSelectTile(lv_obj_t *parent)
{
    lv_obj_clean(parent);
    uint8_t memberCnt = 3;
    lv_obj_t *label, *img, *button, *imgArrow, *line;
    label = GuiCreateTitleLabel(parent, _("firmware_update_title"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);
    label = GuiCreateNoticeLabel(parent, _("firmware_update_desc"));
    lv_obj_set_width(label, 408);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 72);

    img = GuiCreateImg(parent, &imgMicroSd);
    label = GuiCreateLittleTitleLabel(parent, _("firmware_update_via_sd"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    GuiButton_t table1[4] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {24, 40},},
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {76, 40},},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {372, 40},},
    };

    char fileVersion[16] = {0};
    if (CheckOtaBinVersion(fileVersion)) {
        lv_obj_t *versionLabel = GuiCreateIllustrateLabel(parent, fileVersion);
        lv_obj_set_style_text_color(versionLabel, ORANGE_COLOR, LV_PART_MAIN);
        lv_label_set_text_fmt(versionLabel, "v%s Available", fileVersion);
        table1[3].align = LV_ALIGN_DEFAULT;
        table1[3].position.x = 76;
        table1[3].position.y = 81;
        table1[3].obj = versionLabel;
        memberCnt = 4;
    }

    button = GuiCreateButton(parent, 408, 120, table1, memberCnt, GuiViaSdCardHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 210);

    img = GuiCreateImg(parent, &imgUsbConnection);
    label = GuiCreateLittleTitleLabel(parent, _("firmware_update_via_usb"));
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
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc1"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 100);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 2#");
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc2"));
    lv_obj_set_width(label, 384);
    GuiAlignToPrevObj(label, LV_ALIGN_DEFAULT, 30, 0);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc2_link"));
    lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 236);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    img = GuiCreateImg(parent, &imgQrcodeTurquoise);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 322, 239);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);
    GuiAlignToPrevObj(img, LV_ALIGN_RIGHT_MID, 30, 0);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 3#");
    lv_label_set_recolor(label, true);
    lv_obj_align_to(label, lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) - 3), LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc3"));
    lv_obj_set_width(label, 384);
    GuiAlignToPrevObj(label, LV_ALIGN_DEFAULT, 30, 0);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 4#");
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);

    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc4"));
    lv_obj_set_width(label, 390);
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_DEFAULT, 30, 0);

    label = GuiCreateBoldIllustrateLabel(parent, _("firmware_update_usb_title2"));
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc5"));
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

static void FirmwareSdcardUpdateHandler(lv_event_t *e)
{
    char fileVersion[16] = {0};
    GUI_DEL_OBJ(g_noticeHintBox)
    lv_event_code_t code = lv_event_get_code(e);
    uint16_t *walletSetIndex = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        GuiModelStopCalculateCheckSum();
        if (CHECK_BATTERY_LOW_POWER()) {
            g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_KEYSTORE_SAVE_LOW_POWER, &g_noticeHintBox);
        } else if (!SdCardInsert()) {
            //firmware_update_sd_failed_access_title
            g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_UPDATE_SDCARD_NOT_DETECTED, &g_noticeHintBox);
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
                SetKeyboardWidgetSig(g_keyboardWidget, walletSetIndex);
            }
        } else {
            if (strlen(fileVersion) == 0) {
                //no file
                g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_UPDATE_FIRMWARE_NOT_DETECTED, &g_noticeHintBox);
            } else {
                g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_UPDATE_NO_UPGRADABLE_FIRMWARE, &g_noticeHintBox);
            }

        }
    }
}

static void FirmwareSdcardCheckSha256Handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (!SdCardInsert()) {
            return;
        }
        g_noticeHintBox = GuiCreateAnimHintBox(lv_scr_act(), 480, 400, 76);
        lv_obj_t *title = GuiCreateTextLabel(g_noticeHintBox, _("Calculating"));
        lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -194);
        lv_obj_t *btn = GuiCreateBtn(g_noticeHintBox, _("Cancel"));
        lv_obj_set_size(btn, 408, 66);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
        lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_add_event_cb(btn, GuiStopFirmwareCheckSumHandler, LV_EVENT_CLICKED, &g_noticeHintBox);

        lv_obj_t *desc = GuiCreateNoticeLabel(g_noticeHintBox, "0%");
        lv_obj_align(desc, LV_ALIGN_BOTTOM_MID, 0, -140);
        lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);
        GuiModelCalculateBinSha256();
#ifdef COMPILE_SIMULATOR
        GuiFirmwareUpdateSha256Percent(100);
#endif
    }
}

static void FirmwareSdcardCheckSha256HintBoxHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (SdCardInsert()) {
            lv_obj_del(lv_event_get_target(e));
            GuiModelCalculateBinSha256();
        }
    }
}

static void GuiCreateSdCardnstructionTile(lv_obj_t *parent)
{
    static uint32_t param = SIG_INIT_SD_CARD_OTA_COPY;
    lv_obj_t *label, *img;

    label = GuiCreateTitleLabel(parent, _("firmware_update_sd_title"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 1#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 100);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc1"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 100);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 2#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 172);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc2"));
    lv_obj_set_width(label, 384);
    GuiAlignToPrevObj(label, LV_ALIGN_DEFAULT, 30, 0);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc2_link"));
    lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(parent, &imgQrcodeTurquoise);
    GuiAlignToPrevObj(img, LV_ALIGN_RIGHT_MID, 30, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GuiQrcodeHandler, LV_EVENT_CLICKED, NULL);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 3#");
    lv_label_set_recolor(label, true);
    lv_obj_align_to(label, lv_obj_get_child(parent, lv_obj_get_child_cnt(parent) - 3), LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);

    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc3"));
    lv_obj_set_width(label, 390);
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_DEFAULT, 30, 0);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 4#");
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);

    label = GuiCreateIllustrateLabel(parent, _("firmware_update_sd_desc4"));
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_DEFAULT, 30, 0);

    lv_obj_t *btn = NULL;
    if (FatfsFileExist(OTA_FILE_PATH)) {
        btn = GuiCreateBtn(parent, _("about_info_verify_firmware_title"));
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
        label = lv_obj_get_child(btn, 0);
        lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_set_size(btn, 400, 30);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 605 - GUI_MAIN_AREA_OFFSET);
        lv_obj_add_event_cb(btn, FirmwareSdcardCheckSha256Handler, LV_EVENT_CLICKED, &param);
    }

    btn = GuiCreateBtn(parent, _("Update"));
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

            button = GuiCreateBtn(parent, _("OK"));
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
    if (g_keyboardWidget != NULL) {
        GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
    }
}

static void GuiFirmwareUpdateViewSha256(char *version, uint8_t percent)
{
    if (percent == 0xFF) {
        GuiDeleteAnimHintBox();
        g_noticeHintBox = NULL;
        return;
    }
    lv_obj_t *label = lv_obj_get_child(g_noticeHintBox, lv_obj_get_child_cnt(g_noticeHintBox) - 1);
    lv_label_set_text_fmt(label, "%d%%", percent);
    if (percent == 100) {
        GuiDeleteAnimHintBox();
        g_noticeHintBox = NULL;
        uint32_t hintHeight = 220 + 48;
        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 482, 300);
        lv_obj_t *btn = GuiCreateBtn(g_noticeHintBox, _("OK"));
        lv_obj_set_size(btn, 94, 66);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);

        label = GuiCreateIllustrateLabel(g_noticeHintBox, _("Checksum\n\n"));
        lv_label_set_recolor(label, true);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -130);

        lv_obj_t *desc = GuiCreateIllustrateLabel(g_noticeHintBox, _("firmware_update_sd_checksum_notice"));
        lv_obj_align_to(desc, label, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
        hintHeight = hintHeight + lv_obj_get_height(desc) + 12;

        lv_obj_t *title = GuiCreateTextLabel(g_noticeHintBox, _("Verify Firmware"));
        lv_obj_align_to(title, desc, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
        hintHeight = hintHeight + lv_obj_get_height(title) + 12;

        char hash[128] = {0};
        char tempBuf[128] = {0};
        SecretCacheGetChecksum(hash);
        ConvertToLowerCase(hash);
        snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);
        lv_obj_t *label = lv_obj_get_child(g_noticeHintBox, lv_obj_get_child_cnt(g_noticeHintBox) - 3);
        lv_label_set_text_fmt(label, "Checksum(v%s):\n%s", version, tempBuf);
    }
}

static void GuiFirmwareUpdateCancelUpdate(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiModelStopCalculateCheckSum();
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        void **param = lv_event_get_user_data(e);
        if (param != NULL) {
            *param = NULL;
        }
    }
}