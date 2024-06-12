#ifndef _FETCH_SENSITIVE_DATA_TASK_H
#define _FETCH_SENSITIVE_DATA_TASK_H

#include "stdint.h"
#include "stdbool.h"

#if 0
typedef int32_t (*BackgroundAsyncFunc_t)(const void *inData, uint32_t inDataLen);
typedef void *(*BackgroundAsyncRunnable_t)(void);
typedef int32_t (*BackgroundAsyncFuncWithRunnable_t)(const void *inData, uint32_t inDataLen, BackgroundAsyncRunnable_t runnable);

typedef struct {
    BackgroundAsyncFunc_t func;
    void *inData;
    uint32_t inDataLen;
    uint32_t delay;
} BackgroundAsync_t;

typedef struct {
    BackgroundAsyncFuncWithRunnable_t func;
    void *inData;
    uint32_t inDataLen;
    BackgroundAsyncRunnable_t runnable;
} BackgroundRunnable_t;

void CreateFetchSensitiveDataTask(void);

int32_t AsyncExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen);
int32_t AsyncExecuteWithPtr(BackgroundAsyncFunc_t func, const void *inData);
int32_t AsyncDelayExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen, uint32_t delay);
int32_t AsyncExecuteRunnable(BackgroundAsyncFuncWithRunnable_t func, const void *inData, uint32_t inDataLen, BackgroundAsyncRunnable_t runnable);
#endif
#endif
