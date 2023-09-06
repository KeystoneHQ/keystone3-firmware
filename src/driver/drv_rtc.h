/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : drv_rtc.h
 * Description:
 * author     : stone wang
 * data       : 2023-01-04 10:12
**********************************************************************/

#ifndef _DRV_RTC_H
#define _DRV_RTC_H

#include "mhscpu.h"

typedef struct times {
    int Year;
    int Mon;
    int Day;
    int Hour;
    int Min;
    int Second;
} Times;

const char *GetCurrentTime(void);
void SetCurrentStampTime(uint32_t stampTime);
uint32_t GetRtcCounter(void);
uint32_t GetCurrentStampTime(void);
void RtcInit(void);

#endif /* _DRV_RTC_H */


