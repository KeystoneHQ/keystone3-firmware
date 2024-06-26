#include "screen_manager.h"
#include "drv_lcd_bright.h"
#include "drv_button.h"
#include "drv_usb.h"
#include "gui_api.h"
#include "gui_views.h"
#include "qrdecode_task.h"
#include "low_power.h"
#include "keystore.h"
#include "gui_power_option_widgets.h"
#include "fingerprint_task.h"
#include "user_msg.h"
#include "device_setting.h"
#include "power_manager.h"
#include "gui_lock_widgets.h"
#include "account_manager.h"
#include "fingerprint_process.h"

#define LOCK_SCREEN_TICK                                    1000
#define LOCK_SCREEN_TIME_OUT                                60 * 1000

static void ShortPressHandler(void);
static void ReleaseHandler(void);
static void LockScreen(void);
static void LockScreenTimerFunc(void *argument);
void SetFpLowPowerMode(void);

static volatile bool g_pageLockScreenEnable;
static volatile bool g_lockScreenEnable;
static osTimerId_t g_lockScreenTimer;
static volatile uint32_t g_lockScreenTick;
static volatile uint32_t g_lockTimeOut;

static volatile bool g_lockTimeState = false;
static volatile bool g_lockDeivceTimeAlive = false;

void ScreenManagerInit(void)
{
    //Register short press and release button event.
    RegisterButtonEvent(BUTTON_EVENT_SHORT_PRESS, ShortPressHandler);
    RegisterButtonEvent(BUTTON_EVENT_RELEASE, ReleaseHandler);
    g_pageLockScreenEnable = true;
    g_lockScreenEnable = true;
    g_lockScreenTimer = osTimerNew(LockScreenTimerFunc, osTimerPeriodic, NULL, NULL);
    g_lockScreenTick = 0;
    g_lockTimeOut = GetAutoLockScreen() * 1000;
    osTimerStart(g_lockScreenTimer, LOCK_SCREEN_TICK);
}

void SetPageLockScreen(bool enable)
{
    g_pageLockScreenEnable = enable;
}

void SetLockScreen(bool enable)
{
    g_lockScreenEnable = enable;
    if (enable) {
        LcdBacklightOn();
        GuiApiEmitSignal(SIG_STATUS_BAR_REFRESH, NULL, 0);
#ifdef BTC_ONLY
        GuiApiEmitSignal(SIG_STATUS_BAR_TEST_NET, NULL, 0);
#endif
    }
}

void SetLockTimeState(bool enable)
{
    g_lockTimeState = enable;
}

bool IsPreviousLockScreenEnable(void)
{
    return g_lockScreenEnable;
}

void ClearLockScreenTime(void)
{
    g_lockScreenTick = 0;
}

void SetLockTimeOut(uint32_t timeOut)
{
    g_lockTimeOut = timeOut * 1000;
    ClearLockScreenTime();
}

void SetLockDeviceAlive(bool alive)
{
    g_lockDeivceTimeAlive = alive;
}

static void ShortPressHandler(void)
{

}

static void ReleaseHandler(void)
{
    LockScreen();
}

static void LockScreen(void)
{
    if (!g_pageLockScreenEnable) {
        printf("current page lock screen is disabled\n");
        return;
    }

    if (!g_lockScreenEnable) {
        printf("lock screen is disabled\n");
        return;
    }

    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    uint8_t accountNum = 1;

    if (FpModuleIsExist()) {
        SetFpLowPowerMode();
    }
    ClearLockScreenTime();
    ClearShutdownTime();
    LcdBacklightOff();
    GuiApiEmitSignal(SIG_INIT_CLOSE_CURRENT_MSG_BOX, NULL, 0);
    GetExistAccountNum(&accountNum);
    if (accountNum > 0 && !g_lockTimeState) {
        LogoutCurrentAccount();
        GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_UNLOCK);
        GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
    }
#ifdef BTC_ONLY
    GuiApiEmitSignal(SIG_STATUS_BAR_TEST_NET, NULL, 0);
#endif

    if (g_lockDeivceTimeAlive) {
        printf("lock device page is alive\n");
        GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_CLEAR_ALL_TOP, NULL, 0);
    }

    if (!FpModuleIsExist()) {
        uint32_t wakeUpCount = EnterLowPower();
        RecoverFromLowPower();
        ClearLockScreenTime();
        ClearShutdownTime();
        printf("wakeUpCount=%d\r\n", wakeUpCount);
    }
}

static void LockScreenTimerFunc(void *argument)
{
    g_lockScreenTick += LOCK_SCREEN_TICK;
    if (g_lockScreenTick >= g_lockTimeOut && g_lockTimeOut != 0) {
        LockScreen();
    }
}
