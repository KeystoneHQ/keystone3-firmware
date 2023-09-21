/**************************************************************************************************
 * Copyright (c) Keystone 2020-2025. All rights reserved.
 * Description: Anti-tamper.
 * Author: leon sun
 * Create: 2023-8-2
 ************************************************************************************************/


#include "anti_tamper.h"
#include "drv_tamper.h"
#include "stdio.h"
#include "mhscpu.h"
#include "drv_uart.h"
#include "drv_ds28s60.h"
#include "user_delay.h"
#include "drv_sys.h"
#include "user_msg.h"
#include "low_power.h"
#include "drv_atecc608b.h"
#include "log_print.h"
#include "user_utils.h"
#include "drv_battery.h"
#include "background_task.h"


#define TAMPER_MARK                 0x5A


static void TamperEraseInfo(void);


/// @brief Called when startup.
void TamperStartup(void)
{
    if (ReadTamperInput()) {
        printf("tamper ok\r\n");
    } else {
        printf("tamper detected!!!\r\n");
        SYSCTRL_PLLConfig(SYSCTRL_PLL_108MHz);
#ifdef BUILD_PRODUCTION
#endif
        DisableAllHardware();
        SetGpioLow(GPIOE, GPIO_Pin_11);         //reset fp.
        //Set all ext int float
        SetGpioFloat(GPIOA, GPIO_Pin_2);
        SetGpioPullUp(GPIOD, GPIO_Pin_7);
        SetGpioPullUp(GPIOE, GPIO_Pin_14);
        SetGpioFloat(GPIOF, GPIO_Pin_1 | GPIO_Pin_15);
        SetGpioPullUp(GPIOF, GPIO_Pin_14);
        TamperEraseInfo();
        BatteryOpen();
        if (GetBatteryMilliVolt() < 3400) {
            printf("battery low, do not startup\n");
            DisableAllHardware();
            SetGpioLow(GPIOE, GPIO_Pin_11);         //reset fp.
            //Set all ext int float
            SetGpioFloat(GPIOA, GPIO_Pin_2);
            SetGpioPullUp(GPIOD, GPIO_Pin_7);
            SetGpioPullUp(GPIOE, GPIO_Pin_14);
            SetGpioFloat(GPIOF, GPIO_Pin_1 | GPIO_Pin_15);
            SetGpioPullUp(GPIOF, GPIO_Pin_14);
            while (1) {
                EnterDeepSleep();
            }
        }
    }
}


/// @brief Tamper ext interrupt handler.
void TamperIntHandler(void)
{
    printf("tamper interrupt\r\n");
    PubValueMsg(BACKGROUND_MSG_TAMPER, 0);
}


/// @brief Tamper handler that processed in background task when the tamper interrupt occurred.
void TamperBackgroundHandler(void)
{
    printf("tamper background handler\r\n");
    TamperEraseInfo();
    SystemReboot();
}


/// @brief Check the device whether be tampered.
/// @return True if the device has been tampered.
bool Tampered(void)
{
    uint8_t pageData[32];
    static bool tampered = false;
    static bool checked = false;
    if (checked) {
        return tampered;
    }
    Atecc608bEncryptRead(15, 0, pageData);
    PrintArray("pageData", pageData, 32);
    for (uint32_t i = 0; i < 32; i++) {
        if (pageData[i] != TAMPER_MARK) {
            printf("pageData[%d]=%d\n", i, pageData[i]);
            tampered = false;
            checked = true;
            return tampered;
        }
    }
    tampered = true;
    checked = true;
    return tampered;
}


void TamperTest(int argc, char *argv[])
{
    printf("tamper read=%d\r\n", ReadTamperInput());
}


static void TamperEraseInfo(void)
{
    uint8_t pageData[32];
    //In order to ensure that the current of the button cell can make the device erase secrets,
    //the redundant peripherals are turned off.
    DisableAllHardware();
    Uart0OpenPort();
    Atecc608bInit();
    memset(pageData, TAMPER_MARK, 32);
    Atecc608bEncryptWrite(15, 0, pageData);
    DS28S60_Init();
    CLEAR_ARRAY(pageData);
    for (uint32_t i = 0; i < 36; i++) {
        printf("erase index=%d\n", i);
        DS28S60_HmacEncryptWrite(pageData, i);
    }
    printf("erase index=88\n");
    DS28S60_HmacEncryptWrite(pageData, 88);
    printf("erase over\n");
}


void ClearTamperFlag(void)
{
    uint8_t pageData[32] = {0};
    Atecc608bEncryptWrite(15, 0, pageData);
}
