/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: low power implement.
 * Author: leon sun
 * Create: 2023-5-25
 ************************************************************************************************/


#include "low_power.h"
#include "stdio.h"
#include "string.h"
#include "drv_sys.h"
#include "drv_exti.h"
#include "drv_lcd_bright.h"
#include "drv_uart.h"
#include "hal_lcd.h"
#include "hal_touch.h"
#include "drv_power.h"
#include "drv_ds28s60.h"
#include "drv_psram.h"
#include "drv_gd25qxx.h"
#include "drv_rtc.h"
#include "drv_button.h"
#include "drv_battery.h"
#include "drv_aw32001.h"
#include "drv_usb.h"
#include "fingerprint_process.h"
#include "fingerprint_task.h"
#include "ui_display_task.h"
#include "user_msg.h"
#include "background_task.h"
#include "user_fatfs.h"
#include "gui_views.h"
#include "gui_api.h"
#include "power_manager.h"
#include "screen_manager.h"

#define RTC_WAKE_UP_INTERVAL_DISCHARGE                          (60 * 30)           //30 minutes
#define RTC_WAKE_UP_INTERVAL_CHARGING                           (80)                //80 seconds

static void SetRtcWakeUp(uint32_t second);
static int32_t InitSdCardAfterWakeup(const void *inData, uint32_t inDataLen);

volatile LowPowerState g_lowPowerState = LOW_POWER_STATE_WORKING;

void LowPowerTest(int argc, char *argv[])
{
    if (strcmp(argv[0], "enter") == 0) {
        printf("enter low power\r\n");
        EnterLowPower();
        RecoverFromLowPower();
        printf("exit low power\r\n");
    } else if (strcmp(argv[0], "cpu_sleep") == 0) {
        printf("enter cpu sleep\r\n");
        EnterCpuSleep();
        printf("exit cpu sleep\r\n");
    } else {
        printf("error low power cmd\r\n");
    }
}

static void FpLowerPowerHandle(void *argument)
{
    uint32_t wakeUpCount = EnterLowPower();
    RecoverFromLowPower();
    ClearLockScreenTime();
    ClearShutdownTime();
    printf("wakeUpCount=%d\r\n", wakeUpCount);
}

void LowerPowerTimerStart(void)
{
    osTimerId_t lowPowerTimer = osTimerNew(FpLowerPowerHandle, osTimerOnce, NULL, NULL);
    osTimerStart(lowPowerTimer, 10);
}

/// @brief Enter low power.
/// @return wake up count.
uint32_t EnterLowPower(void)
{
    uint32_t sleepSecond, wakeUpSecond, wakeUpCount = 0;
    g_lowPowerState = LOW_POWER_STATE_DEEP_SLEEP;
    UsbDeInit();
    printf("enter deep sleep\r\n");
    sleepSecond = GetUsbPowerState() == USB_POWER_STATE_DISCONNECT ? RTC_WAKE_UP_INTERVAL_DISCHARGE : RTC_WAKE_UP_INTERVAL_CHARGING;
    printf("sleepSecond=%d\n", sleepSecond);
    TouchClose();
    UserDelay(10);
    SetLvglHandlerAndSnapShot(false);
    DisableAllHardware();
    ExtInterruptInit();
    SetRtcWakeUp(sleepSecond);
    wakeUpSecond = GetRtcCounter() + sleepSecond;
    EnterDeepSleep();
    while ((ButtonPress() == false) && (FingerPress() == false)) {
        // while (ButtonPress() == false) {
        RecoverFromDeepSleep();
        Uart0OpenPort();
        wakeUpCount++;
        if (GetRtcCounter() >= wakeUpSecond) {
            Gd25FlashOpen();
            Aw32001RefreshState();
            BatteryIntervalHandler();
            printf("rtc wake up interval\n");
            printf("GetUsbPowerState()=%d\n", GetUsbPowerState());
            sleepSecond = GetUsbPowerState() == USB_POWER_STATE_DISCONNECT ? RTC_WAKE_UP_INTERVAL_DISCHARGE : RTC_WAKE_UP_INTERVAL_CHARGING;
            printf("sleepSecond=%d\n", sleepSecond);
            AutoShutdownHandler(sleepSecond);
            SetRtcWakeUp(sleepSecond);
            wakeUpSecond = GetRtcCounter() + sleepSecond;
        }
        DisableAllHardware();
        ExtInterruptInit();
        EnterDeepSleep();
    }
    return wakeUpCount;
}

void RecoverFromLowPower(void)
{
    RecoverFromDeepSleep();
    SetGpioPullUp(GPIOD, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
    Uart0OpenPort();
    PowerInit();
    TouchOpen();
    //Uart1OpenPort();
    Uart2OpenPort();
    FingerPrintGroupSetBit(FINGER_PRINT_EVENT_RESTART);
    PsramOpen();
    Gd25FlashOpen();
    DS28S60_Open();
    LcdInit();
    LcdClear(0x0000);
    PubValueMsg(BACKGROUND_MSG_BATTERY_INTERVAL, 1);
    SetLvglHandlerAndSnapShot(true);
    g_lowPowerState = LOW_POWER_STATE_WORKING;
    LcdBacklightOn();
    UsbInit();
    AsyncExecute(InitSdCardAfterWakeup, NULL, 0);
}

void EnterDeepSleep(void)
{
    GPIO_WakeEvenConfig(GPIO_PortSourceGPIOE, GPIO_Pin_14, ENABLE);     //woken up by button.
    GPIO_WakeEvenConfig(GPIO_PortSourceGPIOF, GPIO_Pin_14, ENABLE);     //woken up by fingerprint.
    //GPIO_WakeEvenConfig(GPIO_PortSourceGPIOF, GPIO_Pin_15, ENABLE);     //woken up by USB.
    GPIO_WakeModeConfig(GPIO_WakeMode_Now);
    //for (int32_t u32Count = 0; u32Count < 0xFFFFF; u32Count++);
    SYSCTRL->MSR_CR1 |= BIT(27);
    //for (int32_t u32Count = 0; u32Count < 0xFFFFF; u32Count++);
    /************************* power down ROM *************************/
    SYSCTRL->ANA_CTRL |= BIT(7);
    //for (int32_t u32Count = 0; u32Count < 0xFFFFF; u32Count++);
    /************************ power down LDO25 ************************/
    SYSCTRL->LDO25_CR |= (BIT(4) | BIT(5));
    /************************** for usb ************************/
    *(uint32_t *)(0x40000C00 + 0x0060) = 0x01;
    *(uint32_t *)(0x40000C00 + 0x0000) = 0x4100;
    UserDelayUs(1000);
    SYSCTRL_EnterSleep(SleepMode_DeepSleep);
}

void RecoverFromDeepSleep(void)
{
    /************************ for LDO25 ************************/
    SYSCTRL->LDO25_CR &= ~(BIT(4) | BIT(5));
    /************************* for ROM *************************/
    SYSCTRL->ANA_CTRL &= ~BIT(7);
    /************************* for MSR *************************/
    SYSCTRL->MSR_CR1 &= ~BIT(27);
    /************************** for usb ************************/
    *(uint32_t *)(0x40000C00 + 0x0000) = 0x0;
    *(uint32_t *)(0x40000C00 + 0x0060) = 0x81;
}

void EnterCpuSleep(void)
{
    SYSCTRL_EnterSleep(SleepMode_CpuOff);
}

void SetAllGpioLow(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInit.GPIO_Pin = GPIO_Pin_All;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOA, &gpioInit);
    GPIO_Init(GPIOB, &gpioInit);
    GPIO_Init(GPIOC, &gpioInit);
    GPIO_Init(GPIOD, &gpioInit);
    GPIO_Init(GPIOE, &gpioInit);
    GPIO_Init(GPIOF, &gpioInit);
    GPIO_Init(GPIOG, &gpioInit);
    GPIO_Init(GPIOH, &gpioInit);
    GPIO_Write(GPIOA, 0x0);             //FFFF:1.55mA
    GPIO_Write(GPIOB, 0x0);             //FFFF:1.62mA
    GPIO_Write(GPIOC, 0x0);             //FFFF:1.78mA
    GPIO_Write(GPIOD, 0x0);             //FFFF:1.57mA
    GPIO_Write(GPIOE, 0x0);             //FFFF:1.52mA
    GPIO_Write(GPIOF, 0x0);             //FFFF:1.53mA
    GPIO_Write(GPIOE, 0x0);             //FFFF:1.54mA
    GPIO_Write(GPIOG, 0x0);             //FFFF:2.20mA

    SetGpioFloat(GPIOE, GPIO_Pin_11);
    SetGpioFloat(GPIOD, GPIO_Pin_12 | GPIO_Pin_13);
    SetGpioFloat(GPIOF, GPIO_Pin_14);
    SetGpioFloat(GPIOA, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
    SetGpioPullUp(GPIOD, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
}

void DisableAllHardware(void)
{
    while (!UART_IsTXEmpty(UART0));
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_GPIO, ENABLE);
    SetGpioLow(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11);
    SetGpioLow(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
    SetGpioLow(GPIOC, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
    SetGpioLow(GPIOD, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
    SetGpioLow(GPIOE, GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_13);
    SetGpioLow(GPIOF, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_13);
    SetGpioLow(GPIOG, GPIO_Pin_0 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12);
    SetGpioLow(GPIOH, GPIO_Pin_All);

    // Finger, reset
    // SetGpioLow(GPIOE, GPIO_Pin_11);
    // SetGpioLow(GPIOD, GPIO_Pin_12);
    // SetGpioLow(GPIOD, GPIO_Pin_13);
    // SetGpioLow(GPIOA, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
    // SetGpioLow(GPIOF, GPIO_Pin_14);

    //PSRAM
    SetGpioHigh(GPIOG, GPIO_Pin_6);

    //SPI2, FLASH
    //CS
    SetGpioHigh(GPIOB, GPIO_Pin_3);
    //WP
    SetGpioHigh(GPIOE, GPIO_Pin_1);
    //HOLD
    SetGpioHigh(GPIOA, GPIO_Pin_3);

    //SPI3, SE
    SetGpioHigh(GPIOD, GPIO_Pin_9);

    //SetGpioFloat(GPIOA, GPIO_Pin_2);
    SetGpioFloat(GPIOB, GPIO_Pin_0 | GPIO_Pin_1);
    //SetGpioFloat(GPIOC, GPIO_Pin_2 | GPIO_Pin_4);
    SetGpioFloat(GPIOD, GPIO_Pin_12 | GPIO_Pin_13);
    SetGpioFloat(GPIOE, GPIO_Pin_15);
    SetGpioFloat(GPIOF, GPIO_Pin_0);
    SetGpioFloat(GPIOG, GPIO_Pin_11 | GPIO_Pin_12);
}


LowPowerState GetLowPowerState(void)
{
    return g_lowPowerState;
}


void SetGpioFloat(GPIO_TypeDef *GpioX, uint32_t pin)
{
    GPIO_InitTypeDef gpioInit;

    gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = pin;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GpioX, &gpioInit);
}


void SetGpioPullUp(GPIO_TypeDef *GpioX, uint32_t pin)
{
    GPIO_InitTypeDef gpioInit;

    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    gpioInit.GPIO_Pin = pin;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GpioX, &gpioInit);
}


void SetGpioLow(GPIO_TypeDef *GpioX, uint32_t pin)
{
    GPIO_InitTypeDef gpioInit;

    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInit.GPIO_Pin = pin;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GpioX, &gpioInit);

    for (uint32_t i = 0; i < 16; i++) {
        if ((pin & (1 << i)) != 0) {
            GPIO_ResetBits(GpioX, 1 << i);
        }
    }
}


void SetGpioHigh(GPIO_TypeDef *GpioX, uint32_t pin)
{
    GPIO_InitTypeDef gpioInit;

    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInit.GPIO_Pin = pin;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GpioX, &gpioInit);

    for (uint32_t i = 0; i < 16; i++) {
        if ((pin & (1 << i)) != 0) {
            GPIO_SetBits(GpioX, 1 << i);
        }
    }
}


static void SetRtcWakeUp(uint32_t second)
{
    RTC_SetAlarm(GetRtcCounter() + second);
    //RTC_SetRefRegister(0);
    RTC_ITConfig(ENABLE);
    GPIO->WAKE_TYPE_EN |= BIT(12);
}

static int32_t InitSdCardAfterWakeup(const void *inData, uint32_t inDataLen)
{
    bool sdCardState = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7);
    if (sdCardState == false) {
        MountSdFatfs();
        uint32_t freeSize = FatfsGetSize("0:");
        if (freeSize > 0) {
            GuiApiEmitSignalWithValue(SIG_INIT_SDCARD_CHANGE, sdCardState);
        }
    } else {
        UnMountSdFatfs();
        GuiApiEmitSignalWithValue(SIG_INIT_SDCARD_CHANGE, sdCardState);
    }
    return 0;
}

