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


