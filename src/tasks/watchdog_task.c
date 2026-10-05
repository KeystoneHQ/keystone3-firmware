#include "watchdog_task.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "mhscpu.h"

#define WATCHDOG_TIMEOUT_MS       10000U
#define WATCHDOG_FEED_INTERVAL_MS 1000U

static void WatchdogTask(void *argument);

static StaticTask_t g_watchdogTaskBuffer __attribute__((section("privileged_data")));
static osThreadId_t g_watchdogTaskHandle __attribute__((section("privileged_data")));
static StackType_t g_watchdogTaskStack[configMINIMAL_STACK_SIZE]
    __attribute__((section("privileged_data")));

bool WatchdogTaskCreate(void)
{
    const osThreadAttr_t watchdogTaskAttributes = {
        .name = "WatchdogTask",
        .cb_mem = &g_watchdogTaskBuffer,
        .cb_size = sizeof(g_watchdogTaskBuffer),
        .stack_mem = g_watchdogTaskStack,
        .stack_size = sizeof(g_watchdogTaskStack),
        .priority = (osPriority_t)osPriorityRealtime7,
    };

    if (g_watchdogTaskHandle != NULL) {
        return true;
    }

    g_watchdogTaskHandle = osThreadNew(WatchdogTask, NULL, &watchdogTaskAttributes);
    if (g_watchdogTaskHandle == NULL) {
        return false;
    }

    WDT_SetReload(SYSCTRL->PCLK_1MS_VAL * WATCHDOG_TIMEOUT_MS);
    WDT_ModeConfig(WDT_Mode_CPUReset);
    WDT_ReloadCounter();
    WDT_Enable();
    return true;
}

static void WatchdogTask(void *argument)
{
    (void)argument;

    while(1) {
        WDT_ReloadCounter();
        osDelay(WATCHDOG_FEED_INTERVAL_MS);
    }
}
