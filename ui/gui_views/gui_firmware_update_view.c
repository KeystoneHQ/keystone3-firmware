/**************************************************************************************************
 * Copyright (c) Keystone 2020-2025. All rights reserved.
 * Description: Gui firmware update view.
 * Author: leon sun
 * Create: 2023-7-18
 ************************************************************************************************/

#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_firmware_update_widgets.h"
#include "gui_status_bar.h"

static int32_t GuiFirmwareUpdateViewInit(void *param)
{
    GuiFirmwareUpdateInit(param);
    return SUCCESS_CODE;
}

static int32_t GuiFirmwareUpdateViewDeInit(void)
{
    GuiFirmwareUpdateDeInit();
    return SUCCESS_CODE;
}

int32_t GuiFirmwareUpdateViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint32_t rcvValue;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiFirmwareUpdateViewInit(param);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiFirmwareUpdateViewDeInit();
    case GUI_EVENT_DISACTIVE:
        break;
    case SIG_INIT_SDCARD_CHANGE:
        rcvValue = *(uint32_t *)param;
        GuiStatusBarSetSdCard(!rcvValue);
    case GUI_EVENT_REFRESH:
        GuiFirmwareUpdateRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiFirmwareUpdatePrevTile();
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        GuiFirmwareSdCardCopy();
        break;
    case SIG_INIT_SD_CARD_OTA_COPY_SUCCESS:
        GuiFirmwareSdCardCopyResult(true);
        break;
    case SIG_INIT_SD_CARD_OTA_COPY_FAIL:
        GuiFirmwareSdCardCopyResult(false);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_firmwareUpdateView = {
    .id = SCREEN_FIRMWARE_UPDATE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiFirmwareUpdateViewEventProcess,
};

