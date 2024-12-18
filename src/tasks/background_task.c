#include "background_task.h"
#include "background_app.h"
#include "drv_aw32001.h"
#include "drv_battery.h"
#include "drv_tamper.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "user_msg.h"
#include "user_fatfs.h"
#include "user_delay.h"
#include "err_code.h"
#include "gui_api.h"
#include "gui_views.h"
#include "drv_usb.h"
#include "usb_task.h"
#include "anti_tamper.h"
#include "device_setting.h"

static void BackgroundTask(void *argument);
static void RebootTimerFunc(void *argument);

osThreadId_t g_backgroundTaskHandle;
static osTimerId_t g_rebootTimer = NULL;

void CreateBackgroundTask(void)
{
    const osThreadAttr_t backgroundTask_attributes = {
        .name = "BackgroundTask",
        .stack_size = 1024 * 8,
        .priority = (osPriority_t)osPriorityBelowNormal,
    };
    g_backgroundTaskHandle = osThreadNew(BackgroundTask, NULL, &backgroundTask_attributes);
}

/// @brief Notify the background task to reboot the system.
void SystemReboot(void)
{
    if (g_rebootTimer == NULL) {
        uint32_t *arg = SRAM_MALLOC(sizeof(uint32_t));
        *arg = SYSTEM_RESET_TYPE_REBOOT;
        g_rebootTimer = osTimerNew(RebootTimerFunc, osTimerOnce, arg, NULL);
        osTimerStart(g_rebootTimer, 3000);
    }
    PubValueMsg(BACKGROUND_MSG_RESET, SYSTEM_RESET_TYPE_REBOOT);
}

/// @brief Notify the background task to poweroff the system.
void SystemPoweroff(void)
{
    if (g_rebootTimer == NULL) {
        uint32_t *arg = SRAM_MALLOC(sizeof(uint32_t));
        *arg = SYSTEM_RESET_TYPE_POWEROFF;
        g_rebootTimer = osTimerNew(RebootTimerFunc, osTimerOnce, arg, NULL);
        osTimerStart(g_rebootTimer, 3000);
    }
    PubValueMsg(BACKGROUND_MSG_RESET, SYSTEM_RESET_TYPE_POWEROFF);
}

static void BackgroundTask(void *argument)
{
    Message_t rcvMsg;
    osStatus_t ret;
    uint16_t battState;
    bool sdCardState = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7);
    BackGroundAppInit();
    while (1) {
        ret = osMessageQueueGet(g_backgroundQueue, &rcvMsg, NULL, 10000);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case BACKGROUND_MSG_MINUTE: {
            // battery measurement, etc.
            if (GetChargeState() != CHARGE_STATE_NOT_CHARGING) {
                // printf("charger insert minute @bg_task\r\n");
                ChangerRefreshState();
            }
        }
        break;
        case BACKGROUND_MSG_CHANGER_INSERT: {
            osDelay(200);
            // printf("charger insert @bg_task\r\n");
            ChangerRefreshState();
            battState = GetBatterPercent();
            if (GetUsbPowerState() == USB_POWER_STATE_CONNECT) {
                battState |= 0x8000;
            }
            if (GetUsbDetectState() == false) {
                CloseUsb();
                GuiApiEmitSignalWithValue(SIG_INIT_USB_CONNECTION, 0);
                GuiApiEmitSignalWithValue(SIG_INIT_PULLOUT_USB, 0);
            } else if (GetUSBSwitch()) {
#if (USB_POP_WINDOW_ENABLE == 1)
                GuiApiEmitSignalWithValue(SIG_INIT_USB_CONNECTION, 1);
#else
                OpenUsb();
#endif
            }
            GuiApiEmitSignal(SIG_INIT_BATTERY, &battState, sizeof(battState));
        }
        break;
        case BACKGROUND_MSG_RESET: {
            printf("reset device type=%d\r\n", rcvMsg.value);
            ExecuteSystemReset((SystemResetType)rcvMsg.value);
        }
        break;
        case BACKGROUND_MSG_BATTERY_INTERVAL: {
            if (rcvMsg.value != 0 || BatteryIntervalHandler()) {
                battState = GetBatterPercent();
                if (GetUsbPowerState() == USB_POWER_STATE_CONNECT) {
                    battState |= 0x8000;
                }
                //printf("send battState=0x%04X\r\n", battState);
                GuiApiEmitSignal(SIG_INIT_BATTERY, &battState, sizeof(battState));
            }
        }
        break;
        case BACKGROUND_MSG_TAMPER: {
            TamperBackgroundHandler();
        }
        break;
        case BACKGROUND_MSG_SD_CARD_CHANGE: {
            UserDelay(100);
            if (sdCardState == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7)) {
                break;
            } else {
                sdCardState = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7);
                if (sdCardState == false) {
                    ret = MountSdFatfs();
                    uint32_t freeSize = FatfsGetSize("0:");
                    if (freeSize > 0) {
                        GuiApiEmitSignalWithValue(SIG_INIT_SDCARD_CHANGE, sdCardState);
                    }
                } else {
                    UnMountSdFatfs();
                    GuiApiEmitSignalWithValue(SIG_INIT_SDCARD_CHANGE, sdCardState);
                }
            }
        }
        break;
        default:
            break;
        }
        if (rcvMsg.buffer != NULL) {
            SRAM_FREE(rcvMsg.buffer);
        }
    }
}

static void RebootTimerFunc(void *argument)
{
    uint32_t *arg = argument;
    ExecuteSystemReset(*arg);
    g_rebootTimer = NULL;
}
