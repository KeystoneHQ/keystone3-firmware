#include "drv_button.h"
#include "mhscpu.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "user_delay.h"
#include "user_msg.h"
#include "low_power.h"

#define BUTTON_INT_PORT                 GPIOE
#define BUTTON_INT_PIN                  GPIO_Pin_14

#define BUTTON_TIMER_TICK               10
#define SHORT_PRESS_BUTTON_TICK         50
#define LONG_PRESS_BUTTON_TICK          2000

static osTimerId_t g_buttonTickTimer;
static volatile bool g_buttonTimerBusy;
static volatile uint32_t g_buttonPressTick;

static ButtonEventCallbackFunc_t g_shortPressEventCallback;
static ButtonEventCallbackFunc_t g_releaseEventCallback;
static ButtonEventCallbackFunc_t g_longPressEventCallback;

static void ButtonTickTimerFunc(void *argument);

/// @brief Button init.
/// @param
void ButtonInit(void)
{
    g_shortPressEventCallback = NULL;
    g_releaseEventCallback = NULL;
    g_longPressEventCallback = NULL;
    g_buttonTimerBusy = false;
    g_buttonPressTick = 0;

    g_buttonTickTimer = osTimerNew(ButtonTickTimerFunc, osTimerPeriodic, NULL, NULL);
}

/// @brief Register a call back function for the specific button event.
/// @param[in] event The specific button event.
/// @param[in] func Callback function.
void RegisterButtonEvent(ButtonEventType event, ButtonEventCallbackFunc_t func)
{
    switch (event) {
    case BUTTON_EVENT_SHORT_PRESS: {
        g_shortPressEventCallback = func;
    }
    break;
    case BUTTON_EVENT_RELEASE: {
        g_releaseEventCallback = func;
    }
    break;
    case BUTTON_EVENT_LONG_PRESS: {
        g_longPressEventCallback = func;
    }
    break;
    default:
        break;
    }
}

void ButtonIntHandler(void)
{
    //printf("btn int\r\n");
    if (GetLowPowerState() != LOW_POWER_STATE_WORKING) {
        return;
    }
    if (g_buttonTimerBusy == false) {
        osTimerStart(g_buttonTickTimer, BUTTON_TIMER_TICK);
        g_buttonTimerBusy = true;
    }
}

static void ButtonTickTimerFunc(void *argument)
{
    g_buttonPressTick += BUTTON_TIMER_TICK;
    if (GPIO_ReadInputDataBit(BUTTON_INT_PORT, BUTTON_INT_PIN) == Bit_SET) {
        //Release button
        osTimerStop(g_buttonTickTimer);
        g_buttonTimerBusy = false;
        if (g_buttonPressTick > SHORT_PRESS_BUTTON_TICK && g_buttonPressTick < LONG_PRESS_BUTTON_TICK) {
            //release event
            if (g_releaseEventCallback) {
                g_releaseEventCallback();
            }
        }
        g_buttonPressTick = 0;
    } else {
        //Press button continually
        if (g_buttonPressTick == SHORT_PRESS_BUTTON_TICK) {
            //Short press event
            if (g_shortPressEventCallback) {
                NVIC_SystemReset();
                g_shortPressEventCallback();
            }
        } else if (g_buttonPressTick == LONG_PRESS_BUTTON_TICK) {
            //Long press event
            if (g_longPressEventCallback) {
                g_longPressEventCallback();
            }
        }
    }
}

/// @brief Get the state if the button is pressed now.
/// @return true-press, false-not press.
bool ButtonPress(void)
{
    return GPIO_ReadInputDataBit(BUTTON_INT_PORT, BUTTON_INT_PIN) == Bit_RESET;
}
