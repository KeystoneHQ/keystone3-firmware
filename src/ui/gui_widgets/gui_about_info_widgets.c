#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_msg.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_about_info_widgets.h"
#include "version.h"
#include "err_code.h"
#include "gui_page.h"

#ifdef COMPILE_MAC_SIMULATOR
#include "simulator_model.h"
#include "fingerprint_process.h"
#endif

static void GuiAboutNVSBarInit();
static void GuiAboutInfoEntranceWidget(lv_obj_t *parent);
static void UnHandler(lv_event_t *e);
static void LogExportHandler(lv_event_t *e);

static lv_obj_t *g_noticeHintBox = NULL;                                // notice hintbox
static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;

void GuiAboutInfoWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    GuiAboutInfoEntranceWidget(cont);
}

void GuiAboutInfoWidgetsDeInit()
{
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
    GuiAboutNVSBarInit();
}


void GuiAboutInfoWidgetsRestart()
{}


static void GuiAboutNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Device Info");
}


void GuiAboutInfoEntranceWidget(lv_obj_t *parent)
{
    char version[32] = {0};
    GetSoftWareVersion(version);
    char *versionPrefix = "Firmware ";
    char *startPointer = strstr(version, versionPrefix);
    char versionStr[32] = {0};
    uint8_t fpVersion[32] = "v";
    if (startPointer) {
        strncpy(versionStr, version + strlen(versionPrefix), strlen(version));
    } else {
        strncpy(versionStr, version, strlen(version));
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
    };

    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
                             UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 12);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 138);

    titleLabel = GuiCreateTextLabel(parent, _("about_info_serial_number"));
    contentLabel = GuiCreateNoticeLabel(parent, serialNumber);

    table[0].obj = titleLabel;
    table[1].obj = contentLabel;
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
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
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                             LogExportHandler, NULL);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -189);


    titleLabel = GuiCreateTextLabel(parent, _("about_info_device_uid"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);

    table[0].obj = titleLabel;
    table[1].obj = imgArrow;
    table[1].align = LV_ALIGN_RIGHT_MID;
    table[1].position.x = -24;
    table[1].position.y = 0;
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                             OpenViewHandler, &g_DevicePublicKeyView);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -290);

    titleLabel = GuiCreateTextLabel(parent, _("about_info_fingerprint_firnware_version"));
    contentLabel = GuiCreateNoticeLabel(parent, fpVersion);

    table[0].obj = titleLabel;
    table[0].align = LV_ALIGN_DEFAULT;
    table[0].position.x = 24;
    table[0].position.y = 24;

    table[1].obj = contentLabel;
    table[1].align = LV_ALIGN_DEFAULT;
    table[1].position.x = 24;
    table[1].position.y = 64;
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
                             UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 12, -63);
}

void GuiAboutWidgetsLogExport(bool en, int32_t errCode)
{
    char *title = "";
    char *desc = "";
    char *right = "";
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
        char logName[64] = {0};
        char sn[32] = {0};
        char buf[80] = "File name:\n";
        GetSerialNumber(sn);
        sprintf(logName, "0:Log_%s_%d.bin", sn, GetCurrentStampTime());
        LogSetLogName(logName);
        sprintf(logName, "Log_%s_%d.bin", sn, GetCurrentStampTime());
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

static void UnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    }
}