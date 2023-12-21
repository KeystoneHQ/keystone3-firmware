#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_firmware_update_widgets.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_lock_widgets.h"

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
    uint8_t checkSumPercent = 0;

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
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiFirmwareSdCardCopy();
        break;
    case SIG_FIRMWARE_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *) passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiLockScreenErrorCount(param);
                return SUCCESS_CODE;
            }
        }
        GuiFirmwareUpdateVerifyPasswordErrorCount(param);
        break;
    case SIG_INIT_SD_CARD_OTA_COPY_SUCCESS:
        GuiFirmwareSdCardCopyResult(true);
        break;
    case SIG_INIT_SD_CARD_OTA_COPY_FAIL:
        GuiFirmwareSdCardCopyResult(false);
        break;
    case SIG_SETTING_SHA256_PERCENT:
        if (param != NULL) {
            checkSumPercent = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiFirmwareUpdateSha256Percent(checkSumPercent);
        break;
    case SIG_SETTING_SHA256_PERCENT_ERROR:
        GuiFirmwareUpdateSha256Percent(0xFF);
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

