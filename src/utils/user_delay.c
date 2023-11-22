#ifndef COMPILE_MAC_SIMULATOR

#include "user_delay.h"
#include "cmsis_os.h"
#include "mhscpu.h"

static volatile uint32_t g_timerTick = 0;

uint32_t actor = 10000000;

void UserDelay(uint32_t ms)
{
    uint32_t i, tick, countPerMs;
    if (osKernelGetState() == osKernelRunning) {
        osDelay(ms);
    } else {
        countPerMs = SYSCTRL->HCLK_1MS_VAL / 4;
        for (tick = 0; tick < ms; tick++) {
            for (i = 0; i < countPerMs; i++) {
                PretendDoingSomething(i);
            }
        }
    }
}


void UserDelayUs(uint32_t us)
{
    volatile uint32_t i, tick, countPerUs;
    countPerUs = SYSCTRL->HCLK_1MS_VAL / 4000;
    for (tick = 0; tick < us; tick++) {
        for (i = 0; i < countPerUs; i++) {
            PretendDoingSomething(i);
        }
    }
}


void PretendDoingSomething(uint32_t i)
{
    if (i > actor) {
        printf("!!\r\n");
    }
}

#endif