#include "gui_views.h"
#include "gui_boot_update_widgets.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "presetting.h"
#include "gui_model.h"
#include "version.h"

static lv_obj_t *g_bootUpdateCont = NULL;
static lv_obj_t *g_noticeWindow = NULL;

void GuiCreateBootUpdateHandler(lv_event_t * e)
{
    if (GetCurrentDisplayPercent() <= 60 ||
        GetUsbDetectState() == false) {
        g_noticeWindow = GuiCreateConfirmHintBox(&imgFailed, _("boot_update_limit_title"), _("boot_update_limit_desc"), NULL, _("OK"), WHITE_COLOR_OPA20);
        lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
        return;
    }

    printf("GuiCreateBootUpdateHandler\n");
    GuiModelUpdateBoot();
}

void GuiCreateBootUpdateSkipHandler(lv_event_t * e)
{
    printf("GuiCreateBootUpdateSkipHandler\n");
    GuiCloseCurrentWorkingView();
}

void GuiBootUpdateDeInit(void)
{
    GUI_DEL_OBJ(g_bootUpdateCont)
}

void GuiBootUpdateSuccess(void)
{
    printf("GuiBootUpdateSuccess\n");
}

void GuiBootUpdateFail(void)
{
    printf("GuiBootUpdateFail\n");
    lv_obj_clean(g_bootUpdateCont);

    lv_obj_t *img = GuiCreateImg(g_bootUpdateCont, &imgWarn);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 180);

    lv_obj_t *label = GuiCreateNoticeLabel(g_bootUpdateCont, "Bootloader update failed. Please restart the device and try again.\nEnsure the device has at least 80% battery and is connected to a power source.\nIf the issue persists after restart, please contact support Team.");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 336);

    label = GuiCreateNoticeLabel(g_bootUpdateCont, "support@keyst.one");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 570);

    char serialNumber[SERIAL_NUMBER_MAX_LEN];
    char buff[BUFFER_SIZE_128];
    GetSerialNumber(serialNumber);
    snprintf_s(buff, sizeof(buff), "sn:%s", serialNumber);
    label = GuiCreateNoticeLabel(g_bootUpdateCont, buff);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 620);
}

void GuiBootUpdateInit(void)
{
    lv_obj_t *tempObj;
    printf("GuiBootUpdateInit\n");
    if (g_bootUpdateCont == NULL) {
        g_bootUpdateCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
        tempObj = GuiCreateImg(g_bootUpdateCont, &imgFirmwareUp);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 150);
        tempObj = GuiCreateLittleTitleLabel(g_bootUpdateCont, _("boot_update_title"));
        GuiAlignToPrevObj(tempObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, _("boot_update_desc1"));
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        GuiAlignToPrevObj(tempObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, _("boot_update_desc2"));
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        GuiAlignToPrevObj(tempObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
        lv_label_set_recolor(tempObj, true);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, _("boot_update_desc3"));
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        GuiAlignToPrevObj(tempObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, _("boot_update_desc4"));
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        GuiAlignToPrevObj(tempObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
        lv_label_set_recolor(tempObj, true);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, _("boot_update_desc5"));
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_recolor(tempObj, true);
        GuiAlignToPrevObj(tempObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

        tempObj = GuiCreateLabelWithFontAndTextColor(g_bootUpdateCont, "support@keyst.one", &openSansEnIllustrate, 0x1BE0C6);
        GuiAlignToPrevObj(tempObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

        char serialNumber[SERIAL_NUMBER_MAX_LEN];
        char buff[BUFFER_SIZE_128];
        GetSerialNumber(serialNumber);
        uint32_t major, minor, build;
        GetBootSoftwareVersion(&major, &minor, &build);
        // snprintf_s(buff, sizeof(buff), "SN:%s ", serialNumber);
        snprintf_s(buff, sizeof(buff), "SN:%s boot:%d.%d.%d", serialNumber, major, minor, build);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, buff);
        GuiAlignToPrevObj(tempObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

        tempObj = GuiCreateBtn(g_bootUpdateCont, "Update");
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_MID, 0, -36);
        lv_obj_set_width(tempObj, 408);
        lv_obj_add_event_cb(tempObj, GuiCreateBootUpdateHandler, LV_EVENT_CLICKED, NULL);

        tempObj = GuiCreateBtn(g_bootUpdateCont, "Skip");
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, 0, -100);
        lv_obj_add_event_cb(tempObj, GuiCreateBootUpdateSkipHandler, LV_EVENT_CLICKED, NULL);

        tempObj = GuiCreateBtn(g_bootUpdateCont, "Restart");
        lv_obj_align(tempObj, LV_ALIGN_BOTTOM_LEFT, 0, -100);
        lv_obj_add_event_cb(tempObj, GuiCreateBootUpdateRestartToAppHandler, LV_EVENT_CLICKED, NULL);
    }
}
