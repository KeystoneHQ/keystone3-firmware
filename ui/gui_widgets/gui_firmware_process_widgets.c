/**************************************************************************************************
 * Copyright (c) Keystone 2020-2025. All rights reserved.
 * Description: Firmware process UI widgets.
 * Author: leon sun
 * Create: 2023-7-19
 ************************************************************************************************/

#include "gui_firmware_process_widgets.h"
#include "lvgl.h"
#include "gui_obj.h"
#include "gui_resource.h"
#include "screen_manager.h"

static void GuiFirmwareProcessInit(void);
static void GuiFirmwareProcessDeInit(void);

const GuiMsgBox_t g_guiMsgBoxFirmwareProcess = {
    GuiFirmwareProcessInit,
    GuiFirmwareProcessDeInit,
    GUI_FIRMWARE_PROCESS_PRIORITY,
};

static lv_obj_t *container;
static bool g_lockable;

static void GuiFirmwareProcessInit(void)
{
    lv_obj_t *label;
    g_lockable = IsPreviousLockScreenEnable();
    SetLockScreen(false);
    if (container != NULL) {
        lv_obj_del(container);
    }
    container = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);

    label = GuiCreateTextLabel(container, "Updating");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 403);
    label = GuiCreateNoticeLabel(container, "Approximately 3 minutes");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 457);
    label = GuiCreateNoticeLabel(container, "Do not unplug the USB cable while the installation process is underway");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label, 360);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 660);
    GuiCreateCircleAroundAnimation(container, -51);
}


static void GuiFirmwareProcessDeInit(void)
{
    if (container != NULL) {
        GuiStopCircleAroundAnimation();
        lv_obj_del(container);
    }
    container = NULL;
    SetLockScreen(g_lockable);
}

