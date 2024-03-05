#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_about_info_widgets.h"
#include "gui_page.h"
#include "user_msg.h"
#include "user_memory.h"
#include "presetting.h"
#include "version.h"
#include "err_code.h"
#include "secret_cache.h"
#include "fingerprint_process.h"
#include "log.h"
#ifndef COMPILE_SIMULATOR
#include "drv_battery.h"
#endif

#ifdef COMPILE_MAC_SIMULATOR
#include "simulator_model.h"
#include "simulator_mock_define.h"
#endif

static void GuiAboutNVSBarInit();
static void GuiAboutInfoEntranceWidget(lv_obj_t *parent);
static void LogExportHandler(lv_event_t *e);
static void StartFirmwareCheckSumHandler(lv_event_t *e);
static void CloseVerifyHintBoxHandler(lv_event_t *e);
static void OpenVerifyFirmwareHandler(lv_event_t *e);

uint32_t GetBatteryMilliVolt();
static lv_obj_t *g_firmwareVerifyCont = NULL;
static lv_obj_t *g_noticeHintBox = NULL;
static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;

void GuiUpdateCheckSumPercent(uint8_t percent)
{
    if (g_noticeHintBox == NULL) {
        return;
    }

    // g_noticeHintBox check sum anim
    lv_obj_t *label = lv_obj_get_child(g_noticeHintBox, lv_obj_get_child_cnt(g_noticeHintBox) - 1);
    lv_label_set_text_fmt(label, "%d%%", percent);
    if (percent == 100) {
        GUI_DEL_OBJ(g_noticeHintBox)
        lv_obj_clean(g_firmwareVerifyCont);
        lv_obj_t *label = GuiCreateTitleLabel(g_firmwareVerifyCont, _("about_info_verify_checksum_title"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 12);

        lv_obj_t *cont = GuiCreateContainerWithParent(g_firmwareVerifyCont, 408, 138);
        lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(cont, LV_OPA_20, LV_PART_MAIN);
        lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 108);
        lv_obj_set_style_radius(cont, 24, 0);

        char version[32] = {0};
        GetSoftWareVersion(version);
        label = GuiCreateNoticeLabel(cont, version);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);

        char hash[128] = {0};
        char tempBuf[128] = {0};
        SecretCacheGetChecksum(hash);
        ConvertToLowerCase(hash);
        snprintf(tempBuf, sizeof(tempBuf), "#F5870A %.8s#%.24s\n%.24s#F5870A %.8s#", hash, &hash[8], &hash[32], &hash[56]);
        label = GuiCreateIllustrateLabel(cont, tempBuf);
        lv_label_set_recolor(label, true);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 62);

        label = GuiCreateIllustrateLabel(g_firmwareVerifyCont, _("about_info_verify_checksum_desc"));

        lv_obj_t *btn = GuiCreateBtn(g_firmwareVerifyCont, _("Done"));
        lv_obj_set_size(btn, 408, 66);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 710 - GUI_MAIN_AREA_OFFSET);
        lv_obj_add_event_cb(btn, CloseVerifyHintBoxHandler, LV_EVENT_CLICKED, NULL);

        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 286);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseVerifyHintBoxHandler, NULL);
    }
}

void GuiAboutInfoWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    g_cont = cont;
    GuiAboutInfoEntranceWidget(cont);
}

void GuiAboutInfoWidgetsDeInit()
{
    GUI_DEL_OBJ(g_firmwareVerifyCont)
    GUI_DEL_OBJ(g_noticeHintBox)
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiAboutInfoWidgetsRefresh()
{
    if (g_firmwareVerifyCont == NULL) {
        GuiAboutNVSBarInit();
    }
}


void GuiAboutInfoWidgetsRestart()
{}


static void GuiAboutNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("device_info_title"));
    lv_obj_add_flag(g_pageWidget->contentZone, LV_OBJ_FLAG_SCROLLABLE);
}


void GuiAboutInfoEntranceWidget(lv_obj_t *parent)
{
    char version[BUFFER_SIZE_32] = {0};
    GetSoftWareVersion(version);
    const char *versionPrefix = "Firmware ";
    char *startPointer = strstr(version, versionPrefix);
    char versionStr[BUFFER_SIZE_32] = {0};
    char fpVersion[BUFFER_SIZE_32] = "v";
    if (startPointer) {
        strncpy(versionStr, version + strlen(versionPrefix), strnlen_s(version, BUFFER_SIZE_32));
    } else {
        strncpy(versionStr, version, strnlen_s(version, BUFFER_SIZE_32));
    }

    char serialNumber[64] = {0};
    GetSerialNumber(serialNumber);

    lv_obj_t *titleLabel, *contentLabel, *line, *button;

    titleLabel = GuiCreateTextLabel(parent, _("about_info_firmware_version"));
    contentLabel = GuiCreateNoticeLabel(parent, versionStr);
    GuiGetFpVersion(&fpVersion[1]);

    GuiButton_t table[] = {
        {
            .obj = titleLabel,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 24},
        },
        {
            .obj = contentLabel,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 64},
        },
        {
            .obj = GuiCreateImg(parent, &imgArrowRight),
            .align = LV_ALIGN_TOP_RIGHT,
            .position = {-24, 24},
        }
    };

    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
                             OpenVerifyFirmwareHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 12);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 138);

    titleLabel = GuiCreateTextLabel(parent, _("about_info_serial_number"));
    contentLabel = GuiCreateNoticeLabel(parent, serialNumber);

    table[0].obj = titleLabel;
    table[1].obj = contentLabel;
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table) - 1,
                             UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 147);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 273);

    titleLabel = GuiCreateTextLabel(parent, _("about_info_export_log"));
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);

    table[0].obj = titleLabel;
    table[1].obj = imgArrow;
    table[1].align = LV_ALIGN_RIGHT_MID;
    table[1].position.x = -24;
    table[1].position.y = 0;
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table) - 1,
                             LogExportHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 383);

    titleLabel = GuiCreateTextLabel(parent, _("about_info_device_uid"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);

    table[0].obj = titleLabel;
    table[1].obj = imgArrow;
    table[1].align = LV_ALIGN_RIGHT_MID;
    table[1].position.x = -24;
    table[1].position.y = 0;
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table) - 1,
                             OpenViewHandler, &g_DevicePublicKeyView);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 282);

    titleLabel = GuiCreateTextLabel(parent, _("about_info_fingerprint_firmware_version"));
    contentLabel = GuiCreateNoticeLabel(parent, fpVersion);
    if (!FpModuleIsChipState()) {
        lv_obj_set_style_text_color(contentLabel, RED_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_opa(contentLabel, LV_OPA_100, LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(contentLabel, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_text_opa(contentLabel, LV_OPA_80, LV_PART_MAIN);
    }

    table[0].obj = titleLabel;
    table[0].align = LV_ALIGN_DEFAULT;
    table[0].position.x = 24;
    table[0].position.y = 24;

    table[1].obj = contentLabel;
    table[1].align = LV_ALIGN_DEFAULT;
    table[1].position.x = 24;
    table[1].position.y = 64;
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table) - 1,
                             UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 484);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 610);

    titleLabel = GuiCreateTextLabel(parent, _("about_info_battery_voltage"));
    contentLabel = GuiCreateNoticeLabel(parent, "");
    lv_label_set_text_fmt(contentLabel, "%umV", GetBatteryMilliVolt());
    table[0].obj = titleLabel;
    table[0].align = LV_ALIGN_DEFAULT;
    table[0].position.x = 24;
    table[0].position.y = 24;

    table[1].obj = contentLabel;
    table[1].align = LV_ALIGN_DEFAULT;
    table[1].position.x = 24;
    table[1].position.y = 64;
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table) - 1,
                             UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 619);
}

void GuiAboutWidgetsLogExport(bool en, int32_t errCode)
{
    const char *title = "";
    const char *desc = "";
    const char *right = "";
    const void *src;
    lv_color_t rightColor;
    if (en) {
        title = _("about_info_result_export_successful");
        desc = _("about_info_result_export_successful_desc");
        src = &imgSuccess;
        rightColor = ORANGE_COLOR;
        right = _("Done");
    } else {
        title = _("about_info_result_export_failed");
        src = &imgFailed;
        rightColor = DARK_GRAY_COLOR;
        right = _("OK");
        if (errCode == ERROR_LOG_HAVE_NO_SD_CARD) {
            desc = _("about_info_result_export_failed_desc_no_sdcard");
        } else if (errCode == ERROR_LOG_NOT_ENOUGH_SPACE) {
            desc = _("about_info_result_export_failed_desc_no_space");
        }
    }
    printf("errcode = %d\n", errCode);
    g_noticeHintBox = GuiCreateResultHintbox(lv_scr_act(), 386, src,
                      title, desc, NULL, DARK_GRAY_COLOR, right, rightColor);
    lv_obj_t *descLabel = lv_obj_get_child(g_noticeHintBox, 0);
    lv_obj_set_style_text_opa(descLabel, LV_OPA_100, LV_PART_MAIN);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeHintBox);
    lv_obj_add_event_cb(rightBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
}

static void ConfirmLogExportHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeHintBox)
        if (!SdCardInsert()) {
            GuiAboutWidgetsLogExport(false, ERROR_LOG_HAVE_NO_SD_CARD);
        } else {
            PubValueMsg(LOG_MSG_EXPORT, 0);
        }
    }
}

static void LogExportHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        char logName[BUFFER_SIZE_64] = {0};
        char sn[BUFFER_SIZE_32] = {0};
        char buf[BUFFER_SIZE_128] = "File name:\n";
        GetSerialNumber(sn);
        snprintf_s(logName, BUFFER_SIZE_64, "0:Log_%s_%d.bin", sn, GetCurrentStampTime());
        LogSetLogName(logName);
        snprintf_s(logName, BUFFER_SIZE_64, "Log_%s_%d.bin", sn, GetCurrentStampTime());
        strcat(buf, logName);
        g_noticeHintBox = GuiCreateResultHintbox(lv_scr_act(), 386, &imgSdCardL,
                          _("about_info_export_to_sdcard"), buf, _("Cancel"), DARK_GRAY_COLOR, _("Export"), ORANGE_COLOR);
        lv_obj_t *descLabel = lv_obj_get_child(g_noticeHintBox, 1);
        lv_obj_set_style_text_opa(descLabel, LV_OPA_100, LV_PART_MAIN);
        lv_obj_set_style_text_color(descLabel, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeHintBox);
        lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeHintBox);
        lv_obj_add_event_cb(rightBtn, ConfirmLogExportHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
    }
}

void GuiCreateVerifyFirmwareInstructionTile(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("about_info_verify_source_code_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 12);

    label = GuiCreateNoticeLabel(parent, _("about_info_verify_firmware_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 72);

    label = GuiCreateIllustrateLabel(parent, "1");
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 232);

    label = GuiCreateIllustrateLabel(parent, _("about_info_verify_firmware_step1"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 60, 232);
    lv_label_set_recolor(label, true);
    label = GuiCreateIllustrateLabel(parent, _("firmware_update_verify_firmware_qr_link"));
    lv_obj_set_style_text_color(label, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 326);

    label = GuiCreateIllustrateLabel(parent, "2");
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 398);

    label = GuiCreateIllustrateLabel(parent, _("about_info_verify_firmware_step3"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 60, 398);
    lv_label_set_recolor(label, true);

    lv_obj_t *btn = GuiCreateBtn(parent, _("Show Checksum"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 710 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_event_cb(btn, StartFirmwareCheckSumHandler, LV_EVENT_CLICKED, NULL);

    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseVerifyHintBoxHandler, parent);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "");
}

static void StartFirmwareCheckSumHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
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
        GuiModelCalculateCheckSum();
    }
}

void GuiStopFirmwareCheckSumHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiModelStopCalculateCheckSum();
        void **param = lv_event_get_user_data(e);
        if (param != NULL) {
            lv_obj_t *obj = *param;
            lv_obj_del(obj);
            *param = NULL;
        }
    }
}

static void CloseVerifyHintBoxHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_firmwareVerifyCont)
        GuiAboutNVSBarInit();
    }
}

static void OpenVerifyFirmwareHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_scroll_to_y(g_cont, 0, LV_ANIM_OFF);
        g_firmwareVerifyCont = GuiCreateContainerWithParent(g_pageWidget->contentZone, 480, 800 - GUI_MAIN_AREA_OFFSET);
        lv_obj_clear_flag(g_pageWidget->contentZone, LV_OBJ_FLAG_SCROLLABLE);
        GuiCreateVerifyFirmwareInstructionTile(g_firmwareVerifyCont);
    }
}
