#include "background_app.h"
#include "drv_aw32001.h"
#include "drv_battery.h"
#include "user_msg.h"
#include "mhscpu.h"
#include "power_manager.h"
#include "drv_lcd_bright.h"
#include "drv_usb.h"
#include "low_power.h"

static void ChangerInsertIntCallback(void);
static void MinuteTimerFunc(void *argument);
static void BatteryTimerFunc(void *argument);

static osTimerId_t g_minuteTimer, g_batteryTimer;
static bool g_powerOff = false;

void BackGroundAppInit(void)
{
    RegisterChangerInsertCallback(ChangerInsertIntCallback);
    g_minuteTimer = osTimerNew(MinuteTimerFunc, osTimerPeriodic, NULL, NULL);
    g_batteryTimer = osTimerNew(BatteryTimerFunc, osTimerPeriodic, NULL, NULL);
    osTimerStart(g_minuteTimer, 60 * 1000);
    osTimerStart(g_batteryTimer, GetBatteryInterval());
    ChangerRefreshState();
    PubValueMsg(BACKGROUND_MSG_BATTERY_INTERVAL, 0);
    PowerManagerInit();
}

/// @brief Executed in background task.
/// @param
void ChangerRefreshState(void)
{
    Aw32001RefreshState();
    //todo: notify UI to refresh the charging state display.
}

/// @brief Restarts the device using the specified reset type.
/// @param type The reset type to use.
void ExecuteSystemReset(SystemResetType type)
{
    switch (type) {
    case SYSTEM_RESET_TYPE_REBOOT: {
        LcdFadesOut();
        NVIC_SystemReset();

    }
    break;
    case SYSTEM_RESET_TYPE_POWEROFF: {
        if (!g_powerOff) {
            g_powerOff = true;
            LcdFadesOut();
            Aw32001PowerOff();
            DisableAllHardware();
        }
    }
    break;
    default:
        break;
    }
}

/// @brief Note: Executed in ISR, notify the background task to refresh state.
/// @param
static void ChangerInsertIntCallback(void)
{
    PubValueMsg(BACKGROUND_MSG_CHANGER_INSERT, 0);
}

static void MinuteTimerFunc(void *argument)
{
    PubValueMsg(BACKGROUND_MSG_MINUTE, 0);
}

static void BatteryTimerFunc(void *argument)
{
    PubValueMsg(BACKGROUND_MSG_BATTERY_INTERVAL, 0);
    osTimerStart(g_batteryTimer, GetBatteryInterval());
}
