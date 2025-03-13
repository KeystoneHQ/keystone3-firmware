#include "gui.h"
#include "gui_views.h"
#include "gui_boot_update_widgets.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "presetting.h"
#include "gui_model.h"
#include "version.h"

static lv_obj_t *g_bootUpdateCont = NULL;
static lv_obj_t *g_noticeWindow = NULL;
static lv_obj_t *g_startBtn = NULL;

void GuiCreateBootUpdateHandler(lv_event_t * e)
{
    if (GetCurrentDisplayPercent() <= 40 ||
            GetUsbDetectState() == false) {
        g_noticeWindow = GuiCreateConfirmHintBox(&imgFailed, _("error_box_low_power"), _("boot_update_limit_desc"), NULL, _("OK"), WHITE_COLOR_OPA20);
        lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
        return;
    }

    lv_obj_set_style_bg_color(g_startBtn, DARK_GRAY_COLOR, LV_PART_MAIN);
    lv_obj_clear_flag(g_startBtn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_clean(g_bootUpdateCont);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_bootUpdateCont, _("boot_update_process_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 312);

    label = GuiCreateNoticeLabel(g_bootUpdateCont, _("boot_update_process_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 362);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    printf("GuiCreateBootUpdateHandler\n");
    GuiModelUpdateBoot();
}

void GuiCreateBootUpdateSkipHandler(lv_event_t * e)
{
    printf("GuiCreateBootUpdateSkipHandler\n");
    GuiCLoseCurrentWorkingView();
}

void GuiBootUpdateDeInit(void)
{
    GUI_DEL_OBJ(g_bootUpdateCont)
}

void GuiBootUpdateSuccess(void)
{
    printf("GuiBootUpdateSuccess\n");
}

void GuiBootUpdateInit(void)
{
    lv_obj_t *tempObj, *dotImg;
    uint32_t height = 336 - GUI_STATUS_BAR_HEIGHT;
    printf("GuiBootUpdateInit\n");
    if (g_bootUpdateCont == NULL) {
        g_bootUpdateCont = GuiCreateContainer(480, 800 - GUI_STATUS_BAR_HEIGHT);
        lv_obj_align(g_bootUpdateCont, LV_ALIGN_TOP_MID, 0, GUI_STATUS_BAR_HEIGHT);
        tempObj = GuiCreateImg(g_bootUpdateCont, &imgFirmwareUp);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 64);
        tempObj = GuiCreateLittleTitleLabel(g_bootUpdateCont, _("boot_update_title"));
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 155);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, _("boot_update_desc1"));
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_DEFAULT, 36, 207);
        dotImg = GuiCreateImg(g_bootUpdateCont, &imgCircular);
        lv_obj_align(dotImg, LV_ALIGN_DEFAULT, 36, 307);
        tempObj = GuiCreateIllustrateLabel(g_bootUpdateCont, _("boot_update_desc2"));
        lv_obj_align_to(tempObj, dotImg, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
        dotImg = GuiCreateImg(g_bootUpdateCont, &imgCircular);
        lv_obj_align(dotImg, LV_ALIGN_DEFAULT, 36, 349);
        tempObj = GuiCreateIllustrateLabel(g_bootUpdateCont, _("boot_update_desc3"));
        lv_obj_align_to(tempObj, dotImg, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
        dotImg = GuiCreateImg(g_bootUpdateCont, &imgCircular);
        lv_obj_align(dotImg, LV_ALIGN_DEFAULT, 36, 391);
        tempObj = GuiCreateIllustrateLabel(g_bootUpdateCont, _("boot_update_desc4"));
        lv_obj_align_to(tempObj, dotImg, LV_ALIGN_DEFAULT, 30, -12);

        tempObj = GuiCreateIllustrateLabel(g_bootUpdateCont, _("boot_update_desc5"));
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 508);

        tempObj = GuiCreateLabelWithFontAndTextColor(g_bootUpdateCont, "support@keyst.one", &openSansEnIllustrate, 0x1BE0C6);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 550);

        char serialNumber[SERIAL_NUMBER_MAX_LEN];
        char buff[BUFFER_SIZE_128];
        GetSerialNumber(serialNumber);
        snprintf_s(buff, sizeof(buff), "SN:%s ", serialNumber);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, buff);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 600);

        g_startBtn = GuiCreateTextBtn(g_bootUpdateCont, _("Start"));
        lv_obj_align(g_startBtn, LV_ALIGN_BOTTOM_MID, 0, -36);
        lv_obj_set_width(g_startBtn, 408);
        lv_obj_add_event_cb(g_startBtn, GuiCreateBootUpdateHandler, LV_EVENT_CLICKED, NULL);

        // tempObj = GuiCreateBtn(g_bootUpdateCont, "Skip");
        // lv_obj_align(tempObj, LV_ALIGN_BOTTOM_RIGHT, 0, -100);
        // lv_obj_add_event_cb(tempObj, GuiCreateBootUpdateSkipHandler, LV_EVENT_CLICKED, NULL);
    }
}
