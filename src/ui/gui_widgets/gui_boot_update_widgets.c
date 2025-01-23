#include "gui_views.h"
#include "gui_boot_update_widgets.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "presetting.h"
#include "gui_model.h"

static lv_obj_t *g_bootUpdateCont = NULL;

void GuiCreateBootUpdateHandler(lv_event_t * e)
{
    printf("GuiCreateBootUpdateHandler\n");
    GuiModelUpdateBoot();
}

static void JumpToApp(void)
{
#define APP_ADDR                            (0x1001000 + 0x80000)
    typedef void (*jumpApp)(void);
    volatile uint32_t *ptr = (uint32_t *)APP_ADDR;
    jumpApp app;

    if (*ptr != 0xffffffff) {
        DisableAllHardware();
        
        __disable_irq();
        for (int i = 0; i < 8; i++) {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }
        
        SCB->VTOR = APP_ADDR;
        
        app = (jumpApp)(*(__IO uint32_t*)(APP_ADDR + 4));
        __set_MSP(*(__IO uint32_t*)APP_ADDR);
        __DSB();
        __ISB();
        
        app();
    }
}

void GuiCreateBootUpdateSkipHandler(lv_event_t * e)
{
    printf("GuiCreateBootUpdateSkipHandler\n");
    GuiCloseCurrentWorkingView();
}

void GuiCreateBootUpdateRestartToAppHandler(lv_event_t * e)
{
    printf("GuiCreateBootUpdateRestartToAppHandler\n");
    JumpToApp();
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

    lv_obj_t *btn = GuiCreateBtn(g_bootUpdateCont, "Restart");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -36);
    lv_obj_add_event_cb(btn, GuiCreateBootUpdateRestartToAppHandler, LV_EVENT_CLICKED, NULL);
}

void GuiBootUpdateInit(void)
{
    lv_obj_t *tempObj;
    printf("GuiBootUpdateInit\n");
    if (g_bootUpdateCont == NULL) {
        g_bootUpdateCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
        tempObj = GuiCreateImg(g_bootUpdateCont, &imgFirmwareUp);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 180);
        tempObj = GuiCreateLittleTitleLabel(g_bootUpdateCont, "Update Bootloader");
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 284);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, "The device will update the bootloader to enhance security. Please ensure:\nthe device has at least 80% battery \nPower adapter is connectedThis process may take up to 5 minutes. DO NOT disconnect power or perform any operations, or device damage may occur.");
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 336);
        
        tempObj = GuiCreateLabelWithFontAndTextColor(g_bootUpdateCont, "support@keyst.one", &openSansEnIllustrate, 0x1BE0C6);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 570);
        char serialNumber[SERIAL_NUMBER_MAX_LEN];
        char buff[BUFFER_SIZE_128];
        GetSerialNumber(serialNumber);
        snprintf_s(buff, sizeof(buff), "SN:%s", serialNumber);
        tempObj = GuiCreateNoticeLabel(g_bootUpdateCont, buff);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 620);

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
