#include "power_manager.h"
#include "stdio.h"
#include "drv_aw32001.h"
#include "drv_button.h"
#include "drv_lcd_bright.h"
#include "mhscpu.h"
#include "gui_views.h"
#include "gui_api.h"
#include "device_setting.h"

static volatile uint32_t g_shutdownTick;
static volatile uint32_t g_shutdownTimeOut;

static volatile bool g_isShowPowerOptionPage;

static void PowerOption(void);

void PowerManagerInit(void)
{
    RegisterButtonEvent(BUTTON_EVENT_LONG_PRESS, PowerOption);

    g_shutdownTick = 0;
    g_shutdownTimeOut = GetAutoPowerOff() * 60 * 60;
    g_isShowPowerOptionPage = true;
}

void SetShowPowerOffPage(bool isShow)
{
    g_isShowPowerOptionPage = isShow;
}

static void PowerOption(void)
{
    if (g_isShowPowerOptionPage) {
        SystemReboot();
    }
}

void ClearShutdownTime(void)
{
    g_shutdownTick = 0;
}

void SetShutdownTimeOut(uint32_t timeOut)
{
    g_shutdownTimeOut = timeOut * 60 * 60;
    ClearShutdownTime();
}

void AutoShutdownHandler(uint32_t time)
{
    g_shutdownTick += time;
    printf("AutoShutdownHandler g_shutdownTick is %d diffTime is %d g_shutdownTimeOut is %d\n", g_shutdownTick, time, g_shutdownTimeOut);
    if (g_shutdownTick >= g_shutdownTimeOut && g_shutdownTimeOut != 0) {
        Aw32001PowerOff();
    }
}
