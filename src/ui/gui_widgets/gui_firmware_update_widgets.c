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
#include "gui_keyboard_hintbox.h"
#include "gui_page.h"
#include "account_manager.h"
#include "gui_about_info_widgets.h"
#include "secret_cache.h"
#include "version.h"
#include "user_memory.h"
#include "keystore.h"
#include "user_fatfs.h"
#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#include "simulator_model.h"
#else
#include "safe_str_lib.h"
#endif

typedef enum {
    FIRMWARE_UPDATE_SELECT = 0,
    FIRMWARE_UPDATE_USB_INSTRUCTION,
    FIRMWARE_UPDATE_SD_INSTRUCTION,
#ifndef BTC_ONLY
    FIRMWARE_UPDATE_MULTI_TO_BTC_WARNING,
#endif
    FIRMWARE_UPDATE_USB_UPDATING,

    FIRMWARE_UPDATE_BUTT,
} FIRMWARE_UPDATE_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    lv_obj_t *tileSelect;
    lv_obj_t *tileUsbInstruction;
    lv_obj_t *tileSdInstruction;
#ifndef BTC_ONLY
    lv_obj_t *tileMultiToBtcWarning;
#endif
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
#ifndef BTC_ONLY
static void GuiCreateMultiToBtcWarningTile(lv_obj_t *parent);
static void StartKnownWarningCountDownTimer(void);
static void KnownWarningCountDownTimerHandler(lv_timer_t *timer);
static void KnownWarningHandler(lv_event_t *e);
static void KnownWarningCancelHandler(lv_event_t *e);
#endif
static void ConfirmSdCardUpdate(void);
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

#ifndef BTC_ONLY
static lv_timer_t *g_knownWarningCountDownTimer = NULL;
static uint32_t g_knownWarningCountDown = 0;
static lv_obj_t *g_knownWarningBtn;
#endif

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

    strcpy_s(desc, sizeof(desc), _("firmware_update_sd_dialog_desc"));
    g_noticeHintBox = GuiCreateUpdateHintbox(&imgFirmwareUp, _("firmware_update_sd_dialog_title"),
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
        lv_label_set_text_fmt(g_calCheckSumLabel, "%s(v%s):\n%s", _("about_info_verify_checksum_text"), version, tempBuf);
    } else {
        lv_obj_t *btn = GuiCreateTextBtn(g_noticeHintBox, _(""));
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
    tileView = GuiCreateTileView(g_firmwareUpdateWidgets.cont);
    g_firmwareUpdateWidgets.tileView = tileView;

    g_firmwareUpdateWidgets.tileSelect = lv_tileview_add_tile(tileView, FIRMWARE_UPDATE_SELECT, 0, LV_DIR_HOR);
    GuiCreateSelectTile(g_firmwareUpdateWidgets.tileSelect);

    g_firmwareUpdateWidgets.tileUsbInstruction = lv_tileview_add_tile(tileView, FIRMWARE_UPDATE_USB_INSTRUCTION, 0, LV_DIR_HOR);
    GuiCreateUsbInstructionTile(g_firmwareUpdateWidgets.tileUsbInstruction);

    g_firmwareUpdateWidgets.tileSdInstruction = lv_tileview_add_tile(tileView, FIRMWARE_UPDATE_SD_INSTRUCTION, 0, LV_DIR_HOR);
    GuiCreateSdCardnstructionTile(g_firmwareUpdateWidgets.tileSdInstruction);
#ifndef BTC_ONLY
    g_firmwareUpdateWidgets.tileMultiToBtcWarning = lv_tileview_add_tile(tileView, FIRMWARE_UPDATE_MULTI_TO_BTC_WARNING, 0, LV_DIR_HOR);
    GuiCreateMultiToBtcWarningTile(g_firmwareUpdateWidgets.tileMultiToBtcWarning);
#endif

    g_firmwareUpdateWidgets.currentTile = FIRMWARE_UPDATE_SELECT;
}

void GuiFirmwareSdCardCopy(void)
{
    // if (GuiCheckIfViewOpened(&g_lockView)) {
    //     GuiLockScreenTurnOff();
    // }
    GUI_DEL_OBJ(g_waitAnimCont)
    GUI_DEL_OBJ(g_noticeHintBox)

    g_waitAnimCont = GuiCreateAnimHintBox(480, 386, 82);
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
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        g_noticeHintBox = GuiCreateErrorCodeWindow(ERR_UPDATE_FIRMWARE_NOT_DETECTED, &g_noticeHintBox, NULL);
    }
}

void GuiFirmwareUpdateDeInit(void)
{
#ifndef BTC_ONLY
    if (g_knownWarningCountDownTimer != NULL) {
        lv_timer_del(g_knownWarningCountDownTimer);
        g_knownWarningCountDownTimer = NULL;
    }
#endif
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
#ifndef BTC_ONLY
    if (g_firmwareUpdateWidgets.currentTile != FIRMWARE_UPDATE_MULTI_TO_BTC_WARNING) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    } else {
        if (g_pageWidget->navBarWidget->leftBtn != NULL && lv_obj_is_valid(g_pageWidget->navBarWidget->leftBtn)) {
            lv_obj_del(g_pageWidget->navBarWidget->leftBtn);
            g_pageWidget->navBarWidget->leftBtn = NULL;
        }
    }
#else
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
#endif
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
#ifndef BTC_ONLY
    case FIRMWARE_UPDATE_MULTI_TO_BTC_WARNING:
        g_firmwareUpdateWidgets.currentTile--;
        break;
#endif
    default:
        return;
    }
    printf("g_firmwareUpdateWidgets.currentTile=%d\n", g_firmwareUpdateWidgets.currentTile);
    lv_obj_set_tile_id(g_firmwareUpdateWidgets.tileView, g_firmwareUpdateWidgets.currentTile, 0, LV_ANIM_OFF);
    if (g_firmwareUpdateWidgets.tileView == FIRMWARE_UPDATE_SELECT) {
        GuiCreateSelectTile(g_firmwareUpdateWidgets.tileSelect);
    }
    GuiFirmwareUpdateRefresh();
}

void GuiFirmwareUpdateSha256Percent(uint8_t percent)
{
    if (g_noticeHintBox == NULL) {
        return;
    }
    char version[SOFTWARE_VERSION_MAX_LEN] = {0};
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

    char fileVersion[SOFTWARE_VERSION_MAX_LEN] = {0};
    if (CheckOtaBinVersion(fileVersion)) {
        lv_obj_t *versionLabel = GuiCreateIllustrateLabel(parent, fileVersion);
        lv_obj_set_style_text_color(versionLabel, ORANGE_COLOR, LV_PART_MAIN);
        lv_label_set_text_fmt(versionLabel, "v%s %s", fileVersion, _("firmware_update_sd_dialog_head"));
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
    g_firmwareUpdateWidgets.currentTile = FIRMWARE_UPDATE_SD_INSTRUCTION;
    lv_obj_set_tile_id(g_firmwareUpdateWidgets.tileView, g_firmwareUpdateWidgets.currentTile, 0, LV_ANIM_OFF);
    GuiFirmwareUpdateRefresh();
}

static void GuiViaUsbHandler(lv_event_t *e)
{
    g_firmwareUpdateWidgets.currentTile = FIRMWARE_UPDATE_USB_INSTRUCTION;
    lv_obj_set_tile_id(g_firmwareUpdateWidgets.tileView, g_firmwareUpdateWidgets.currentTile, 0, LV_ANIM_OFF);
    GuiFirmwareUpdateRefresh();
}

static void GuiCreateUsbInstructionTile(lv_obj_t *parent)
{
    lv_obj_t *label, *img;

    label = GuiCreateTitleLabel(parent, _("firmware_update_usb_title"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 1#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 100);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_desc1"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 100);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 2#");
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_desc2"));
    lv_obj_set_width(label, 384);
    GuiAlignToPrevObj(label, LV_ALIGN_DEFAULT, 30, 0);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_qr_link"));
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

    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_title2"));
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_usb_desc5"));
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

static void ConfirmSdCardUpdate(void)
{
    static uint16_t walletSetIndex = SIG_INIT_SD_CARD_OTA_COPY;
    uint8_t accountCnt = 0;
    GetExistAccountNum(&accountCnt);
    if (accountCnt == 0) {
        GuiFirmwareSdCardCopy();
        GuiModelCopySdCardOta();
    } else {
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        g_keyboardWidget = GuiCreateKeyboardWidget(g_firmwareUpdateWidgets.cont);
        SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
        SetKeyboardWidgetSig(g_keyboardWidget, &walletSetIndex);
    }
}

static void FirmwareSdcardUpdateHandler(lv_event_t *e)
{
    char fileVersion[SOFTWARE_VERSION_MAX_LEN] = {0};
    GUI_DEL_OBJ(g_noticeHintBox)
    GuiModelStopCalculateCheckSum();
    if (CHECK_BATTERY_LOW_POWER()) {
        g_noticeHintBox = GuiCreateErrorCodeWindow(ERR_KEYSTORE_SAVE_LOW_POWER, &g_noticeHintBox, NULL);
    } else if (!SdCardInsert()) {
        //firmware_update_sd_failed_access_title
        g_noticeHintBox = GuiCreateErrorCodeWindow(ERR_UPDATE_SDCARD_NOT_DETECTED, &g_noticeHintBox, NULL);
    } else if (CheckOtaBinVersion(fileVersion)) {
#ifndef BTC_ONLY
        printf("fileVersion=%s\n", fileVersion);
        if (strstr(fileVersion, "BTC") == NULL) {
            ConfirmSdCardUpdate();
        } else {
            printf("firmware from MultiCoin to BTC\n");
            if (g_firmwareUpdateWidgets.tileView == NULL) {
                g_noticeHintBox = GuiCreateContainerWithParent(lv_scr_act(), lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET_NEW);
                lv_obj_align(g_noticeHintBox, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET_NEW);
                lv_obj_add_flag(g_noticeHintBox, LV_OBJ_FLAG_CLICKABLE);
                GuiCreateMultiToBtcWarningTile(g_noticeHintBox);
            } else {
                g_firmwareUpdateWidgets.currentTile = FIRMWARE_UPDATE_MULTI_TO_BTC_WARNING;
                lv_obj_set_tile_id(g_firmwareUpdateWidgets.tileView, g_firmwareUpdateWidgets.currentTile, 0, LV_ANIM_OFF);
                GuiFirmwareUpdateRefresh();
            }
            StartKnownWarningCountDownTimer();
        }
#else
        ConfirmSdCardUpdate();
#endif
    } else {
        if (strnlen_s(fileVersion, 16) == 0) {
            g_noticeHintBox = GuiCreateErrorCodeWindow(ERR_UPDATE_FIRMWARE_NOT_DETECTED, &g_noticeHintBox, NULL);
        } else {
            g_noticeHintBox = GuiCreateErrorCodeWindow(ERR_UPDATE_NO_UPGRADABLE_FIRMWARE, &g_noticeHintBox, NULL);
        }

    }
}

static void FirmwareSdcardCheckSha256Handler(lv_event_t *e)
{
    if (!SdCardInsert()) {
        return;
    }
    g_noticeHintBox = GuiCreateAnimHintBox(480, 400, 76);
    lv_obj_t *title = GuiCreateTextLabel(g_noticeHintBox, _("calculat_modal_title"));
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -194);
    lv_obj_t *btn = GuiCreateTextBtn(g_noticeHintBox, _("Cancel"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, GuiStopFirmwareCheckSumHandler, LV_EVENT_CLICKED, &g_noticeHintBox);

    lv_obj_t *desc = GuiCreateNoticeLabel(g_noticeHintBox, "0%");
    lv_obj_align(desc, LV_ALIGN_BOTTOM_MID, 0, -140);
    lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);
    GuiModelCalculateBinSha256();
}

static void FirmwareSdcardCheckSha256HintBoxHandler(lv_event_t *e)
{
    if (SdCardInsert()) {
        lv_obj_del(lv_event_get_target(e));
        GuiModelCalculateBinSha256();
    }
}

static void GuiCreateSdCardnstructionTile(lv_obj_t *parent)
{
#define UPDATE_BTN_X_OFFSET (-20)
    GuiAddObjFlag(parent, LV_OBJ_FLAG_SCROLLABLE);
    static uint32_t param = SIG_INIT_SD_CARD_OTA_COPY;
    lv_obj_t *label, *img, *btn = NULL;
    int16_t btnOffset = UPDATE_BTN_X_OFFSET;

    label = GuiCreateTitleLabel(parent, _("firmware_update_sd_title"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 1#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 100);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_desc1"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 100);

    label = GuiCreateIllustrateLabel(parent, "#F5870A 2#");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 172);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, -30, 12);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_desc2"));
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

    if (FatfsFileExist(OTA_FILE_PATH)) {
        btn = GuiCreateTextBtn(parent, _("verify_firmware"));
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
        label = lv_obj_get_child(btn, 0);
        lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_set_size(btn, 400, 30);
        GuiAlignToPrevObj(btn, LV_ALIGN_OUT_BOTTOM_MID, btnOffset, 20);
        lv_obj_add_event_cb(btn, FirmwareSdcardCheckSha256Handler, LV_EVENT_CLICKED, &param);
        btnOffset = 0;
    }

    btn = GuiCreateTextBtn(parent, _("Update"));
    lv_obj_set_size(btn, 408, 66);
    GuiAlignToPrevObj(btn, LV_ALIGN_OUT_BOTTOM_LEFT, btnOffset, 20);
    lv_obj_add_event_cb(btn, FirmwareSdcardUpdateHandler, LV_EVENT_CLICKED, &param);

    lv_obj_t *spacer = GuiCreateSpacer(parent, 24);
    GuiAlignToPrevObj(spacer, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
}

#ifndef BTC_ONLY
static void GuiCreateMultiToBtcWarningTile(lv_obj_t *parent)
{
    lv_obj_t *label, *img, *btn;

    img = GuiCreateImg(parent, &imgMultiCoin);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 134, -548);
    img = GuiCreateImg(parent, &imgArrowNextRed);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 222, -566);
    img = GuiCreateImg(parent, &imgBtcOnly);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 274, -548);

    label = GuiCreateLittleTitleLabel(parent, _("Warning"));
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -476);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_btc_only_warning_desc"));
    lv_obj_set_style_text_color(label, WHITE_COLOR_OPA64, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, 408);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -224);

    btn = GuiCreateTextBtn(parent, _("Cancel"));
    lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_set_size(btn, 192, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_add_event_cb(btn, KnownWarningCancelHandler, LV_EVENT_CLICKED, NULL);

    btn = GuiCreateTextBtn(parent, _("firmware_update_btc_only_button_i_know"));
    lv_obj_set_size(btn, 192, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, KnownWarningHandler, LV_EVENT_CLICKED, NULL);
    g_knownWarningBtn = btn;
}

static void StartKnownWarningCountDownTimer(void)
{
    char text[32];
    g_knownWarningCountDown = 5;
    if (g_knownWarningCountDownTimer != NULL) {
        lv_timer_del(g_knownWarningCountDownTimer);
    }
    lv_obj_clear_flag(g_knownWarningBtn, LV_OBJ_FLAG_CLICKABLE);
    snprintf_s(text, sizeof(text), "%s(5)", _("firmware_update_btc_only_button_i_know"));
    lv_label_set_text(lv_obj_get_child(g_knownWarningBtn, 0), text);
    lv_obj_set_style_bg_opa(g_knownWarningBtn, LV_OPA_60, LV_PART_MAIN);
    g_knownWarningCountDownTimer = lv_timer_create(KnownWarningCountDownTimerHandler, 1000, NULL);
}

static void KnownWarningCountDownTimerHandler(lv_timer_t *timer)
{
    lv_obj_t *btn = g_knownWarningBtn;
    char text[32];
    const char *preText = _("firmware_update_btc_only_button_i_know");
    if (btn == NULL) {
        g_knownWarningCountDown = 0;
        lv_timer_del(timer);
        g_knownWarningCountDownTimer = NULL;
        return;
    }
    g_knownWarningCountDown--;
    if (g_knownWarningCountDown > 0) {
        snprintf_s(text, sizeof(text), "%s(%d)", preText, g_knownWarningCountDown);
    } else {
        strcpy_s(text, sizeof(text), preText);
    }
    lv_label_set_text(lv_obj_get_child(btn, 0), text);
    if (g_knownWarningCountDown <= 0) {
        lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_timer_del(timer);
        g_knownWarningCountDownTimer = NULL;
    }
}

static void KnownWarningHandler(lv_event_t *e)
{
    if (g_noticeHintBox != NULL) {
        GUI_DEL_OBJ(g_noticeHintBox);
        g_knownWarningBtn = NULL;
    }
    ConfirmSdCardUpdate();
}

static void KnownWarningCancelHandler(lv_event_t *e)
{
    if (g_noticeHintBox == NULL) {
        ReturnHandler(e);
    } else {
        GUI_DEL_OBJ(g_noticeHintBox);
        g_knownWarningBtn = NULL;
    }
}

#endif

static void GuiQrcodeHandler(lv_event_t *e)
{
    lv_obj_t *parent, *button, *qrCodeCont, *qrCode, *label;

    if (g_firmwareUpdateWidgets.qrCodeCont == NULL) {
        g_firmwareUpdateWidgets.qrCodeCont = GuiCreateHintBox(654);
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
            lv_qrcode_update(qrCode, _("firmware_update_usb_qr_link"), (uint32_t)strnlen_s(_("firmware_update_usb_qr_link"), BUFFER_SIZE_128));
        } else {
            lv_qrcode_update(qrCode, g_firmwareSdUpdateUrl, (uint32_t)strnlen_s(g_firmwareSdUpdateUrl, BUFFER_SIZE_128));
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

        button = GuiCreateTextBtn(parent, _("OK"));
        lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(button, CloseQrcodeHandler, LV_EVENT_CLICKED, NULL);
    }
}

static void CloseQrcodeHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_firmwareUpdateWidgets.qrCodeCont)
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
        char hash[128] = {0};
        char tempBuf[128] = {0};
        SecretCacheGetChecksum(hash);
        ConvertToLowerCase(hash);
        snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);

        g_noticeHintBox = GuiCreateConfirmHintBox(NULL, _("verify_firmware"), _("firmware_update_sd_checksum_notice"), "\n\n", _("OK"), WHITE_COLOR_OPA20);
        lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeHintBox);
        lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
        lv_obj_t *label = lv_obj_get_child(g_noticeHintBox, lv_obj_get_child_cnt(g_noticeHintBox) - 4);
        lv_label_set_recolor(label, true);
        lv_label_set_text_fmt(label, "%s(v%s):\n%s", _("about_info_verify_checksum_text"), version, tempBuf);
    }
}

static void GuiFirmwareUpdateCancelUpdate(lv_event_t *e)
{
    GuiModelStopCalculateCheckSum();
    lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    void **param = lv_event_get_user_data(e);
    if (param != NULL) {
        *param = NULL;
    }
}