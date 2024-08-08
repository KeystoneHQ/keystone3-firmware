#ifndef _GUI_ATTENTION_H
#define _GUI_ATTENTION_H

#include "lvgl.h"
#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_hintbox.h"
#include "gui_views.h"
#include "drv_battery.h"
#include "drv_aw32001.h"

typedef enum {
    ATTENTION_LOW_POWER = 0,
    ATTENTION_CONFIRM,
} ATTENTION_HINTBOX_TYPE;

void GuiCreateAttentionHintbox(uint16_t confirmSign);
void GuiCreateHardwareCallInvaildParamHintbox(char *title, char *context);

void GuiCloseAttentionHintbox();
void GuiCreateInitializatioCompleteHintbox();

#endif