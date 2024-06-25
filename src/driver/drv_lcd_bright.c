#include "drv_lcd_bright.h"
#include "stdio.h"
#include "mhscpu.h"
#include "user_delay.h"

//#define LCD_BRIGHT_PWM_PORT                 GPIOA
//#define LCD_BRIGHT_PWM_PIN                  GPIO_Pin_2
//#define LCD_BRIGHT_PWM_TIM                  TIM_2
#define LCD_BRIGHT_PWM_PORT                 GPIOF
#define LCD_BRIGHT_PWM_PIN                  GPIO_Pin_3
#define LCD_BRIGHT_PWM_TIM                  TIM_7

#define LCD_PWM_HZ                          10000

typedef enum {
    LCD_BRIGHT_STATE_INIT,
    LCD_BRIGHT_STATE_GPIO,
    LCD_BRIGHT_STATE_PWM,
    LCD_BRIGHT_STATE_OFF,
} LcdBrightState_t;

static LcdBrightState_t lcdBrightState = LCD_BRIGHT_STATE_INIT;
static uint32_t g_lastBright = 0;

static void LcdBrightAsGpio(bool set);
static void LcdBrightAsPwm(uint32_t bright);

/// @brief Init.
/// @param
void LcdBrightInit(void)
{
    SetLcdBright(0);
}

/// @brief Set lcd bright.
/// @param bright 0-100
void SetLcdBright(uint32_t bright)
{
    if (bright > 100) {
        bright = 100;
    }
    g_lastBright = bright;
    if (bright == 0 || bright == 100) {
        LcdBrightAsGpio(bright != 0);
    } else {
        LcdBrightAsPwm(bright);
    }
}

void LcdBacklightOff(void)
{
    LcdBrightAsGpio(0);
    lcdBrightState = LCD_BRIGHT_STATE_OFF;
}

void LcdBacklightOn(void)
{
    if (lcdBrightState == LCD_BRIGHT_STATE_OFF) {
        lcdBrightState = LCD_BRIGHT_STATE_INIT;
        SetLcdBright(g_lastBright);
    }
}

void LcdFadesOut(void)
{
    uint32_t bright;

    bright = g_lastBright;
    if (bright >= 100) {
        bright = 99;
    }
    while (bright > 0) {
        LcdBrightAsPwm(bright);
        bright--;
        UserDelay(5);
    }
    LcdBrightAsGpio(0);
}

static void LcdBrightAsGpio(bool set)
{
    GPIO_InitTypeDef gpioInit = {0};

    if (lcdBrightState != LCD_BRIGHT_STATE_GPIO) {
        lcdBrightState = LCD_BRIGHT_STATE_GPIO;
        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
        gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
        gpioInit.GPIO_Pin = LCD_BRIGHT_PWM_PIN;
        gpioInit.GPIO_Remap = GPIO_Remap_1;
        GPIO_Init(LCD_BRIGHT_PWM_PORT, &gpioInit);
    }
    if (set) {
        GPIO_SetBits(LCD_BRIGHT_PWM_PORT, LCD_BRIGHT_PWM_PIN);
    } else {
        GPIO_ResetBits(LCD_BRIGHT_PWM_PORT, LCD_BRIGHT_PWM_PIN);
    }
}

static void LcdBrightAsPwm(uint32_t bright)
{
    SYSCTRL_ClocksTypeDef clocks;
    TIM_PWMInitTypeDef TIM_PWMSetStruct;
    uint32_t lowPeriod, highPeriod, period;

    SYSCTRL_GetClocksFreq(&clocks);
    period = clocks.PCLK_Frequency / LCD_PWM_HZ;
    highPeriod = period * bright / 100 - 1;
    lowPeriod = period - highPeriod - 2;

    if (lcdBrightState != LCD_BRIGHT_STATE_PWM) {
        lcdBrightState = LCD_BRIGHT_STATE_PWM;
        SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_TIMM0, ENABLE);

        TIM_PWMSetStruct.TIM_LowLevelPeriod = lowPeriod;
        TIM_PWMSetStruct.TIM_HighLevelPeriod = highPeriod;
        TIM_PWMSetStruct.TIMx = LCD_BRIGHT_PWM_TIM;
        TIM_PWMInit(TIMM0, &TIM_PWMSetStruct);
        GPIO_PinRemapConfig(LCD_BRIGHT_PWM_PORT, LCD_BRIGHT_PWM_PIN, GPIO_Remap_2);
        TIM_Cmd(TIMM0, LCD_BRIGHT_PWM_TIM, ENABLE);
    } else {
        TIM_SetPWMPeriod(TIMM0, LCD_BRIGHT_PWM_TIM, lowPeriod, highPeriod);
    }
}
