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
#include "gui_nft_screen_widgets.h"
#include "gui_firmware_update_deny_widgets.h"
#include "gui_trans_nft_process_widgets.h"
#include "gui_firmware_update_widgets.h"
#include "gui_lock_widgets.h"
#include "presetting.h"
#include "anti_tamper.h"
#include "gui_global_resources.h"
#include "gui_about_info_widgets.h"
#include "account_manager.h"
#include "gui_setup_widgets.h"
#include "device_setting.h"
#include "drv_aw32001.h"
#include "usb_task.h"
#include "ui_display_task.h"

static int32_t GuiInitViewInit(void *param)
{
    bool isTamper = false;
    if (param != NULL) {
        isTamper = *(bool *)param;
    }
    printf("isTamper=%d..........\n", isTamper);
    GuiEnterPassLabelRefresh();
    GuiStyleInit();
    GuiStatusBarInit();
    GlobalResourcesInit();
    if (GetFactoryResult() == false) {
        GuiFrameOpenView(&g_inactiveView);
        return SUCCESS_CODE;
    }
    if (isTamper) {
        GuiFrameOpenView(&g_selfDestructView);
        return SUCCESS_CODE;
    }
    GuiModeGetAccount();
    return SUCCESS_CODE;
}

int32_t GUI_InitViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    static uint8_t walletNum;
    static uint16_t lockParam = SIG_LOCK_VIEW_VERIFY_PIN;
    uint16_t battState;
    uint32_t rcvValue;
    uint8_t percent = 0;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiInitViewInit(param);
    case GUI_EVENT_OBJ_DEINIT:
        printf("init view should not be closed");
        break;
    case SIG_INIT_GET_ACCOUNT_NUMBER:
        walletNum = *(uint8_t *)param;
        if (walletNum == 0) {
            GuiFrameOpenView(&g_setupView);
            if (IsUpdateSuccess()) {
                GuiFrameOpenView(&g_updateSuccessView);
            }
            if (NeedUpdateBoot()) {
                GuiFrameOpenView(&g_bootUpdateView);
            }
            break;
        } else {
            return GuiFrameOpenViewWithParam(&g_lockView, &lockParam, sizeof(lockParam));
        }
        break;
    case SIG_INIT_BATTERY:
        battState = *(uint16_t *)param;
        //printf("rcv battState=0x%04X\r\n", battState);
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
    case SIG_INIT_TRANSFER_NFT_SCREEN:
        rcvValue = *(uint32_t *)param;
        printf("rcvValue=%d\r\n", rcvValue);
        if (rcvValue != 0) {
            OpenMsgBox(&g_guiMsgBoxNftScreen);
        } else {
            CloseMsgBox(&g_guiMsgBoxNftScreen);
        }
        break;
    case SIG_INIT_USB_CONNECTION:
        rcvValue = *(uint32_t *)param;
        if (rcvValue != 0 && !GuiLockScreenIsTop() && GetUsbDetectState() && ((GetCurrentAccountIndex() != 0xFF) || GuiIsSetup())) {
            if (GetUsbState() == false) {
                OpenMsgBox(&g_guiMsgBoxUsbConnection);
            }
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
#ifdef WEB3_VERSION
            NftLockQuit();
#endif
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
    case SIG_INIT_SDCARD_CHANGE_IMG:
        rcvValue = *(uint32_t *)param;
        GuiStatusBarSetSdCard(!rcvValue, true);
        break;
    case SIG_INIT_SDCARD_CHANGE:
        rcvValue = *(uint32_t *)param;
        GuiStatusBarSetSdCard(!rcvValue, false);
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
            percent = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiUpdateCheckSumPercent(percent);
        break;
    case SIG_SETTING_SHA256_PERCENT:
        if (param != NULL) {
            percent = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        GuiFirmwareUpdateSha256Percent(percent);
        break;
    case SIG_STATUS_BAR_REFRESH:
        GuiStatusBarSetUsb();
        break;
#ifdef BTC_ONLY
    case SIG_STATUS_BAR_TEST_NET:
        GuiStatusBarSetTestNet();
        break;
#endif
#ifdef WEB3_VERSION
    case SIG_INIT_NFT_BIN:
        rcvValue = *(uint32_t *)param;
        if (rcvValue != 0) {
            OpenMsgBox(&g_guiMsgBoxTransNftProcess);
        } else {
            CloseMsgBox(&g_guiMsgBoxTransNftProcess);
        }
        break;
    case SIG_INIT_NFT_BIN_TRANS_FAIL:
        GuiNftTransferFailed();
        break;
#endif
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

