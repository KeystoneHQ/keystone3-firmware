#ifndef _DRV_RTC_H
#define _DRV_RTC_H

typedef struct times {
    int Year;
    int Mon;
    int Day;
    int Hour;
    int Min;
    int Second;
} Times;

void SetCurrentStampTime(uint32_t stampTime);
uint32_t GetRtcCounter(void);
uint32_t GetCurrentStampTime(void);
void RtcInit(void);
void StampTimeToUtcTime(int64_t timeStamp, char *utcTime, int maxLen);

#endif /* _DRV_RTC_H */


