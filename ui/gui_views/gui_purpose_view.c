/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_setup_view.c
 * Description:
 * author     : stone wang
 * data       : 2023-01-06 14:40
**********************************************************************/
#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_purpose_widgets.h"

static int32_t GuiPurposeViewInit(void)
{
    GuiPurposeAreaInit();
    return SUCCESS_CODE;
}

static int32_t GuiPurposeViewDeInit(void)
{
    GuiPurposeAreaDeInit();
    return SUCCESS_CODE;
}

int32_t GuiPurposeViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_purposeView.isActive);

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiPurposeViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiPurposeViewDeInit();
    case SIG_INIT_SDCARD_CHANGE:
        break;
    case GUI_EVENT_REFRESH:
        GuiPurposeAreaRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiPurposeAreaRestart();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_purposeView = {
    .id = SCREEN_PURPOSE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiPurposeViewEventProcess,
};

