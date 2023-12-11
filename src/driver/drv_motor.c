#include "drv_motor.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "mhscpu.h"


#define MOTOR_PWM_PORT                      GPIOA
#define MOTOR_PWM_PIN                       GPIO_Pin_4
#define MOTOR_PWM_TIM                       TIM_4

#define MOTOR_PWM_HZ                        10000


typedef enum {
    MOTOR_STATE_IDLE,
    MOTOR_STATE_BUSY,
} MotorStateType;

static osTimerId_t g_motorTimer = NULL;
static uint32_t g_motorLevel = 0;

static void MotorTimerFunc(void *argument);
static void MotorAsGpio(bool set);
static void MotorAsPwm(uint32_t pwm);

void MotorInit(void)
{
    g_motorTimer = osTimerNew(MotorTimerFunc, osTimerOnce, NULL, NULL);
}

/// @brief Control the motor to run for a specified time.
/// @param[in] level level value(0-100).
/// @param[in] tick Motor running time(ms).
void MotorCtrl(uint32_t level, uint32_t tick)
{
    if (level == 0) {
        return;
    }
    g_motorLevel = level;
    if (osTimerIsRunning(g_motorTimer)) {
        MotorAsGpio(false);
        osTimerStop(g_motorTimer);
    }
    MotorAsPwm(level);
    osTimerStart(g_motorTimer, tick);
}

static void MotorTimerFunc(void *argument)
{
    MotorAsGpio(false);
}


static void MotorAsGpio(bool set)
{
    GPIO_InitTypeDef gpioInit = {0};

    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = MOTOR_PWM_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(MOTOR_PWM_PORT, &gpioInit);
    if (set) {
        GPIO_SetBits(MOTOR_PWM_PORT, MOTOR_PWM_PIN);
    } else {
        GPIO_ResetBits(MOTOR_PWM_PORT, MOTOR_PWM_PIN);
    }
}


static void MotorAsPwm(uint32_t pwm)
{
    SYSCTRL_ClocksTypeDef clocks;
    TIM_PWMInitTypeDef TIM_PWMSetStruct;
    uint32_t lowPeriod, highPeriod, period;

    SYSCTRL_GetClocksFreq(&clocks);
    period = clocks.PCLK_Frequency / MOTOR_PWM_HZ;
    highPeriod = period * pwm / 100 - 1;
    lowPeriod = period - highPeriod - 2;
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TIMM0, ENABLE);
    TIM_PWMSetStruct.TIM_LowLevelPeriod = lowPeriod;
    TIM_PWMSetStruct.TIM_HighLevelPeriod = highPeriod;
    TIM_PWMSetStruct.TIMx = MOTOR_PWM_TIM;
    TIM_PWMInit(TIMM0, &TIM_PWMSetStruct);
    GPIO_PinRemapConfig(MOTOR_PWM_PORT, MOTOR_PWM_PIN, GPIO_Remap_2);
    TIM_Cmd(TIMM0, MOTOR_PWM_TIM, ENABLE);
}

