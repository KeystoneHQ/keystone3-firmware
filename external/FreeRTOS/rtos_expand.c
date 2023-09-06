/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2023. All rights reserved.
 * Description: rtos扩展功能
 * Author: leon sun
 * Create: 2022-6-24
 ************************************************************************************************/

#include "rtos_expand.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "cmsis_os2.h"
#include "string.h"

volatile uint32_t FreeRTOSRunTimeTicks;
uint32_t FreeRTOSRunTimeTicksBak;
TaskStatus_t *g_taskStatusArray;

uint32_t FindIndexFromName(const char *taskname);

void PrintTasksStatus(void)
{
#if (configUSE_TRACE_FACILITY > 0)
    uint32_t totalRunTime;
    UBaseType_t   arraySize;
    TaskStatus_t  *statusArray;
    UBaseType_t sizeOfStack;
    UBaseType_t totalSizeofStack = 0;

    static uint32_t taskRunTime = 0;
    uint32_t millionPercent;

    arraySize = uxTaskGetNumberOfTasks(); //获取任务个数
    statusArray = pvPortMalloc(arraySize * sizeof(TaskStatus_t));
    if (statusArray != NULL) { //内存申请成功
        arraySize = uxTaskGetSystemState((TaskStatus_t *)statusArray,
                                         (UBaseType_t)arraySize,
                                         (uint32_t *)&totalRunTime);
        printf("\n\n");
        printf("total task num :  %d.\n", arraySize);
        printf("CurrentState enum: {eRunning = 0, eReady,eBlocked,eSuspended,eDeleted,eInvalid}\n");

        printf("    TaskName\t   Task Prio\t   CurrentState\t   SizeStk\t MinStk\t   FreeStkPercent\tCPUUsedPercent\n");
        for (uint8_t i = 0; i < arraySize; i++) {
            sizeOfStack = vTaskGetStackSize(xTaskGetHandle(statusArray[i].pcTaskName));
            totalSizeofStack += sizeOfStack * 4;
            taskRunTime = FindIndexFromName(statusArray[i].pcTaskName);
            millionPercent = (uint64_t)(statusArray[i].ulRunTimeCounter - taskRunTime) * 1000000 / (uint64_t)(totalRunTime - FreeRTOSRunTimeTicksBak);
            printf("%12s\t %9d\t %9d\t %8d\t %6d\t %8d\t %8d/%8d\t %8d/1M\r\n",
                   statusArray[i].pcTaskName,
                   (int)statusArray[i].uxCurrentPriority,
                   (int)statusArray[i].eCurrentState,
                   (int)sizeOfStack * 4,
                   (int)statusArray[i].usStackHighWaterMark * 4,
                   (int)((float)(int)statusArray[i].usStackHighWaterMark / sizeOfStack * 100),
                   (int)(statusArray[i].ulRunTimeCounter - taskRunTime),
                   (int)(totalRunTime - FreeRTOSRunTimeTicksBak),
                   millionPercent);
        }

        printf("total allocated stack size :  %d(%d K).\n", (int)totalSizeofStack, (int)totalSizeofStack / 1024);
    }
    vPortFree(statusArray);
#else
    printf(" Need to set configUSE_TRACE_FACILITY to 1.\n");
#endif
}


uint32_t FindIndexFromName(const char *taskname)
{
    uint8_t i;
    uint32_t arraySize;
    if (g_taskStatusArray == NULL) {
        return 0;
    }
    arraySize = uxTaskGetNumberOfTasks();
    for (i = 0; i < arraySize; i++) {
        if (strcmp(g_taskStatusArray[i].pcTaskName, taskname) == 0) {
            break;
        }
    }
    return g_taskStatusArray[i].ulRunTimeCounter;
}


void ClrRunTimeStats(void)
{
    uint32_t totalRunTime;
    UBaseType_t arraySize;

    arraySize = uxTaskGetNumberOfTasks(); //获取任务个数
    if (g_taskStatusArray) {
        vPortFree(g_taskStatusArray);
    }
    g_taskStatusArray = pvPortMalloc(sizeof(TaskStatus_t) * arraySize);
    uxTaskGetSystemState((TaskStatus_t *)g_taskStatusArray,
                         (UBaseType_t)arraySize,
                         (uint32_t *)&totalRunTime);
    FreeRTOSRunTimeTicksBak = FreeRTOSRunTimeTicks;
}


uint32_t GetRunTimeCounter(void)
{
    if (osKernelGetState() == osKernelRunning) {
        FreeRTOSRunTimeTicks = osKernelGetTickCount();
    }
    return FreeRTOSRunTimeTicks;
}


void ConfigureTimerForRunTimeStats(void)
{
    FreeRTOSRunTimeTicks = 0;
}
