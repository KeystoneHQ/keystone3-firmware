#include <stdio.h>
#include <stdlib.h>
#include "drv_rtc.h"
#include "time.h"

#define USE_EXTERN_32K
static void stampTimeToRealTime(uint32_t stampTime, Times *standardTime);

static char g_currentTimeBuf[32];

void SetCurrentStampTime(uint32_t stampTime)
{
    RTC_SetRefRegister(stampTime);
    RTC_ResetCounter();
}

uint32_t GetCurrentStampTime(void)
{
    return RTC_GetCounter() + RTC_GetRefRegister();
}

uint32_t GetRtcCounter(void)
{
    return RTC_GetCounter();
}

static void stampTimeToRealTime(uint32_t stampTime, Times *standardTime)
{
    time_t tick = (time_t)stampTime;
    struct tm tm;
    char tempBuf[32];
    tm = *localtime(&tick);
    strftime(tempBuf, sizeof(tempBuf), "%Y-%m-%d %H:%M:%S", &tm);

    standardTime->Year = atoi(tempBuf);
    standardTime->Mon = atoi(tempBuf + 5);
    standardTime->Day = atoi(tempBuf + 8);
    standardTime->Hour = atoi(tempBuf + 11);
    standardTime->Min = atoi(tempBuf + 14);
    standardTime->Second = atoi(tempBuf + 17);
}

const char *GetCurrentTime(void)
{
    Times standardTime;
    uint32_t stampTime = RTC_GetCounter() + RTC_GetRefRegister();
    stampTimeToRealTime(stampTime, &standardTime);

    sprintf(g_currentTimeBuf, "%04d-%02d-%02d %02d:%02d:%02d", standardTime.Year, standardTime.Mon, standardTime.Day,
            (standardTime.Hour + 8) > 24 ? standardTime.Hour + 8 - 24 : standardTime.Hour + 8, standardTime.Min, standardTime.Second);

    return g_currentTimeBuf;
}

static void RtcNvicConfiguration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);

    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void RtcInit(void)
{
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_BPU, ENABLE);

    RtcNvicConfiguration();

#ifdef USE_EXTERN_32K
    BPU->SEN_ANA0 |= BIT(10);
#endif

    RTC_SetAlarm(1);
    RTC_ClearITPendingBit();
    RTC_ITConfig(ENABLE);

    if (GetCurrentStampTime() < 946684800) {
        SetCurrentStampTime(946684800);
    }
}

void RTC_IRQHandler(void)
{
    RTC_ClearITPendingBit();
    NVIC_ClearPendingIRQ(RTC_IRQn);
}
