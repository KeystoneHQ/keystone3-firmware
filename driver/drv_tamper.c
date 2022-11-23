/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: tamper driver.
 * Author: leon sun
 * Create: 2023-2-21
 ************************************************************************************************/

#include "drv_tamper.h"
#include "stdio.h"
#include "mhscpu.h"
#include "user_delay.h"


#define TAMPER_READ_COUNT           1000

#define TAMPER_PORT                 GPIOA
#define TAMPER_PIN                  GPIO_Pin_2


TamperProcessCallbackFunc_t g_tamperProcessCallback;

/// @brief Tamper init.
/// @param
void TamperInit(TamperProcessCallbackFunc_t func)
{
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInit.GPIO_Pin = TAMPER_PIN;
    gpioInit.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(TAMPER_PORT, &gpioInit);

    UserDelay(1);
    g_tamperProcessCallback = func;
    g_tamperProcessCallback();
}



/// @brief Read tamper input GPIO.
/// @return true:fine. false:tamper occurred.
bool ReadTamperInput(void)
{
    uint32_t count = 0;
    uint8_t lastReadLevel, readLevel;

    lastReadLevel = GPIO_ReadInputDataBit(TAMPER_PORT, TAMPER_PIN);
    while (1) {
        readLevel = GPIO_ReadInputDataBit(TAMPER_PORT, TAMPER_PIN);
        if (readLevel == lastReadLevel) {
            count++;
        } else {
            count = 0;
            lastReadLevel = readLevel;
        }
        if (count >= TAMPER_READ_COUNT) {
            break;
        }
        UserDelayUs(10);
    }
    return readLevel == Bit_SET;
}



