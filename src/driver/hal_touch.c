#include "hal_touch.h"
#include "drv_i2c_io.h"
#include "drv_cst726.h"
#include "drv_ft6336.h"
#include "drv_gsl1691.h"
#include "drv_cst3xx.h"
#include "user_delay.h"
#include "err_code.h"

#define TOUCH_RST_PORT                  GPIOF
#define TOUCH_RST_PIN                   GPIO_Pin_2


HalTouchOpt_t g_halTouchOpt = {0};
static TouchPadIntCallbackFunc_t g_touchPadIntCallback;
static volatile bool g_touchOpen = false;

/// @brief Touch init.
/// @param[in] func Interrupt callback function, called when EXTINT gpio rasing/falling.
void TouchInit(TouchPadIntCallbackFunc_t func)
{
    uint8_t addr;
    GPIO_InitTypeDef gpioInit = {0};
    I2CIO_Cfg_t i2cioConfig;

    //Reset chip.
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = TOUCH_RST_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(TOUCH_RST_PORT, &gpioInit);
    GPIO_ResetBits(TOUCH_RST_PORT, TOUCH_RST_PIN);
    UserDelay(100);
    GPIO_SetBits(TOUCH_RST_PORT, TOUCH_RST_PIN);

    I2CIO_Init(&i2cioConfig, GPIOB, GPIO_Pin_0, GPIOB, GPIO_Pin_1);
    addr = I2CIO_SearchDevices(&i2cioConfig);
    switch (addr) {
    case CST726_I2C_ADDR: {
        g_halTouchOpt.Init = Cst726Init;
        g_halTouchOpt.Open = Cst726Open;
        g_halTouchOpt.GetStatus = Cst726GetStatus;
        printf("touch ic:cst726\r\n");
    }
    break;
    case FT6336_I2C_ADDR: {
        g_halTouchOpt.Init = Ft6336Init;
        g_halTouchOpt.Open = Ft6336Open;
        g_halTouchOpt.GetStatus = Ft6336GetStatus;
        printf("touch ic:ft6336\r\n");
    }
    break;
    case GSL1691_I2C_ADDR: {
        g_halTouchOpt.Init = Gsl1691Init;
        g_halTouchOpt.Open = Gsl1691Open;
        g_halTouchOpt.GetStatus = Gsl1691GetStatus;
        printf("touch ic:gsl1691\r\n");
    }
    break;
    case CST3XX_I2C_ADDR: {
        g_halTouchOpt.Init = Cst3xxInit;
        g_halTouchOpt.Open = Cst3xxOpen;
        g_halTouchOpt.GetStatus = Cst3xxGetStatus;
        printf("touch ic:cst3xx\r\n");
    }
    break;
    default: {
        printf("unknown touch ic\r\n");
    }
    break;
    }
    g_touchPadIntCallback = func;

    if (g_halTouchOpt.Init) {
        g_halTouchOpt.Init();
        g_touchOpen = true;
    }
}


void TouchOpen(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = TOUCH_RST_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(TOUCH_RST_PORT, &gpioInit);
    GPIO_ResetBits(TOUCH_RST_PORT, TOUCH_RST_PIN);
    UserDelay(20);
    GPIO_SetBits(TOUCH_RST_PORT, TOUCH_RST_PIN);
    UserDelay(20);
    if (g_halTouchOpt.Open) {
        g_halTouchOpt.Open();
        g_touchOpen = true;
    }
}


void TouchClose(void)
{
    g_touchOpen = false;
}


/// @brief
/// @param[out] status Touch status including coordinate and press status.
/// @return err code.
int32_t TouchGetStatus(TouchStatus_t *status)
{
    if (g_halTouchOpt.GetStatus && g_touchOpen) {
        return g_halTouchOpt.GetStatus(status);
    } else {
        return ERR_TOUCHPAD_NOREG;
    }
}


/// @brief Touch pad exti handler.
/// @param
void TouchPadIntHandler(void)
{
    if (g_touchPadIntCallback != NULL) {
        g_touchPadIntCallback();
    }
}


void TouchPadTest(int argc, char *argv[])
{
    TouchStatus_t status;

    printf("touch pad test!\n");

    if (strcmp(argv[0], "get_status") == 0) {
        TouchGetStatus(&status);
    } else if (strcmp(argv[0], "reset") == 0) {
        printf("touch pad reset\n");
        GPIO_ResetBits(TOUCH_RST_PORT, TOUCH_RST_PIN);
        UserDelay(100);
        GPIO_SetBits(TOUCH_RST_PORT, TOUCH_RST_PIN);
        g_halTouchOpt.Init();
    }  else if (strcmp(argv[0], "open") == 0) {
        printf("touch pad open\n");
        TouchClose();
        TouchOpen();
    } else {
        printf("touch pad cmd err\n");
    }
}

