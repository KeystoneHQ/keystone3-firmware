#include "gui_obj.h"
#include "gui_views.h"
#include "gui_style.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_enter_passcode.h"
#include "gui_pop_message_box.h"
#include "gui_power_option_widgets.h"
#include "gui_firmware_process_widgets.h"
#include "gui_usb_connection_widgets.h"
#include "gui_low_battery_widgets.h"
#include "gui_firmware_update_deny_widgets.h"
#include "gui_firmware_update_widgets.h"
#include "gui_lock_widgets.h"
#include "presetting.h"
#include "anti_tamper.h"
#include "gui_global_resources.h"
#include "gui_about_info_widgets.h"

static int32_t GuiInitViewInit(void)
{
    LanguageInit();
    GuiEnterPassLabelInit();
    GuiStyleInit();
    GuiStatusBarInit();
    GlobalResourcesInit();
    if (GetFactoryResult() == false) {
        GuiFrameOpenView(&g_inactiveView);
        return SUCCESS_CODE;
    }
    if (Tampered()) {
        GuiFrameOpenView(&g_selfDestructView);
        return SUCCESS_CODE;
    }
    GuiModeGetAmount();
    // GuiFrameOpenView(&g_settingView);
    // GuiFrameOpenView(&g_connectWalletView);
    return SUCCESS_CODE;
}

int32_t GUI_InitViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t walletNum;
    static uint16_t lockParam = SIG_LOCK_VIEW_VERIFY_PIN;
    uint16_t battState;
    uint32_t rcvValue;
    uint8_t checkSumPercent = 0;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiInitViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        printf("init view should not be closed");
        break;
    case SIG_INIT_GET_ACCOUNT_NUMBER:
        walletNum = *(uint8_t *)param;
        if (walletNum == 0) {
            return GuiFrameOpenView(&g_setupView);
        } else {
            return GuiFrameOpenViewWithParam(&g_lockView, &lockParam, sizeof(lockParam));
        }
        break;
    case SIG_INIT_BATTERY:
        battState = *(uint16_t *)param;
        printf("rcv battState=0x%04X\r\n", battState);
        GuiStatusBarSetBattery(battState & 0xFF, (battState & 0x8000) != 0);
        break;
    case SIG_INIT_FIRMWARE_UPDATE_DENY:
        rcvValue = *(uint32_t *)param;
        if (rcvValue != 0) {
            OpenMsgBox(&g_guiMsgBoxFirmwareUpdateDeny);
        } else {
            CloseMsgBox(&g_guiMsgBoxFirmwareUpdateDeny);
        }
        break;
    case SIG_INIT_LOW_BATTERY:
        rcvValue = *(uint32_t *)param;
        if (rcvValue != 0) {
            OpenMsgBox(&g_guiMsgBoxLowBattery);
        } else {
            CloseMsgBox(&g_guiMsgBoxLowBattery);
        }
        break;
    case SIG_INIT_USB_CONNECTION:
        rcvValue = *(uint32_t *)param;
        if (rcvValue != 0 && !GuiLockScreenIsTop()) {
            OpenMsgBox(&g_guiMsgBoxUsbConnection);
        } else {
            CloseMsgBox(&g_guiMsgBoxUsbConnection);
        }
        break;
    case SIG_INIT_USB_STATE_CHANGE:
        GuiStatusBarSetUsb();
        break;
    case SIG_INIT_POWER_OPTION:
        rcvValue = *(uint32_t *)param;
        if (rcvValue != 0) {
            OpenMsgBox(&g_guiMsgBoxPowerOption);
        } else {
            CloseMsgBox(&g_guiMsgBoxPowerOption);
        }
        break;
    case SIG_INIT_FIRMWARE_PROCESS:
        rcvValue = *(uint32_t *)param;
        if (rcvValue != 0) {
            OpenMsgBox(&g_guiMsgBoxFirmwareProcess);
        } else {
            CloseMsgBox(&g_guiMsgBoxFirmwareProcess);
        }
        break;
    case SIG_INIT_CLOSE_CURRENT_MSG_BOX:
        CloseCurrentMsgBox();
        break;
    case SIG_INIT_SDCARD_CHANGE:
        rcvValue = *(uint32_t *)param;
        GuiStatusBarSetSdCard(!rcvValue);
        break;
    case SIG_INIT_SD_CARD_OTA_COPY_SUCCESS:
        GuiFirmwareSdCardCopyResult(true);
        break;
    case SIG_INIT_SD_CARD_OTA_COPY_FAIL:
        GuiFirmwareSdCardCopyResult(false);
        break;
    case SIG_INIT_SD_CARD_OTA_COPY:
        GuiFirmwareSdCardCopy();
        break;
    case SIG_FIRMWARE_VERIFY_PASSWORD_FAIL: 
        GuiFirmwareUpdateVerifyPasswordErrorCount(param);
        break;
    case SIG_SETTING_CHECKSUM_PERCENT:
        if (param != NULL) {
            checkSumPercent = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiUpdateCheckSumPercent(checkSumPercent);
        break;
    case SIG_SETTING_SHA256_PERCENT:
        if (param != NULL) {
            checkSumPercent = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiFirmwareUpdateSha256Percent(checkSumPercent);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_initView = {
    .id = SCREEN_INIT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GUI_InitViewEventProcess,
};

