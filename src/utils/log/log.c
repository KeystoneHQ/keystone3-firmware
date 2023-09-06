/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: log stored in SPI flash.
 * Author: leon sun
 * Create: 2023-4-11
 ************************************************************************************************/

#include "log.h"
#include "stdio.h"
#include <stdlib.h>
#include <stdarg.h>
#include "err_code.h"
#include "user_utils.h"
#include "flash_address.h"
#include "drv_gd25qxx.h"
#include "user_memory.h"
#include "drv_rtc.h"
#include "user_msg.h"
#include "crc.h"
#include "log_print.h"
#include "user_fatfs.h"
#include "log_task.h"
#include "gui_api.h"
#include "gui_views.h"

#define LOG_NAME_MAX_LEN            64
#define LOG_DATA_HEAD_SIZE          8
#define NEXT_SECTOR_MARK            0x0000

#define LOG_MAX_STRING_LEN          1024

typedef struct {
    uint16_t event;
    uint16_t length     : 11;
    uint16_t checksum   : 4;
    uint16_t dataType   : 1;
    uint32_t timeStamp;
    uint8_t *pData;
} LogData_t;


static void WriteLogAsync(LogData_t *pLogData);
static void WriteLogSync(LogData_t *pLogData);
static uint32_t FindLogOffsetAddr(void);
static void EraseNextSector(uint32_t length);
static uint32_t GetNextSectorAddr(uint32_t addr);

static uint32_t g_logAddr;
static bool g_logInit = false;
static char g_logName[LOG_NAME_MAX_LEN];

void LogSetLogName(char *name)
{
    strcpy(g_logName, name);
}

void WriteLogEvent(uint32_t event)
{
    if (g_logInit == false) {
        printf("write log before log init!!\r\n");
        return;
    }
    //printf("WriteLogEvent,event=%d\r\n", event);
    LogData_t logData = {0};
    logData.event = event;
    logData.timeStamp = GetCurrentStampTime();
    if (LogLoopStart()) {
        WriteLogAsync(&logData);
    } else {
        WriteLogSync(&logData);
    }
}


void WriteLogValue(uint32_t event, uint32_t value)
{
    if (g_logInit == false) {
        printf("write log before log init!!\r\n");
        return;
    }
    //printf("WriteLogValue,event=%d,value=%d\r\n", event, value);
    LogData_t logData = {0};
    logData.event = event;
    logData.length = 1;
    logData.timeStamp = GetCurrentStampTime();
    logData.pData = (uint8_t *)&value;
    if (LogLoopStart()) {
        WriteLogAsync(&logData);
    } else {
        WriteLogSync(&logData);
    }
}


void WriteLogFormat(uint32_t event, const char *format, ...)
{
    if (g_logInit == false) {
        printf("write log before log init!!\r\n");
        return;
    }
    char *str = SRAM_MALLOC(LOG_MAX_STRING_LEN);
    va_list argList;
    va_start(argList, format);
    //printf("WriteLogFormat,event=%d\r\n", event);
    vsprintf(str, format, argList);
    LogData_t logData = {0};
    logData.event = event;
    logData.dataType = 1;
    logData.length = strlen(str) / 4 + 1;
    logData.timeStamp = GetCurrentStampTime();
    logData.pData = (uint8_t *)str;
    //printf("length=%d\r\n", logData.length);
    if (LogLoopStart()) {
        WriteLogAsync(&logData);
    } else {
        WriteLogSync(&logData);
    }

    va_end(argList);
    SRAM_FREE(str);
}


void LogExport(void)
{
    PubValueMsg(LOG_MSG_EXPORT, 0);
}


void LogErase(void)
{
    PubValueMsg(LOG_MSG_ERASE, 0);
}


void LogInit(void)
{
    g_logAddr = FindLogOffsetAddr();
    printf("g_logAddr=0x%08lX\r\n", g_logAddr);
    g_logInit = true;
}


void WriteLogDataToFlash(const void *data, uint32_t length)
{
    //printf("WriteLogDataToFlash,g_logAddr=0x%08X,length=%d\r\n", g_logAddr, length);
    if (length > GD25QXX_SECTOR_SIZE) {
        printf("length err");
    }
    EraseNextSector(length);
    Gd25FlashWriteBuffer(g_logAddr, data, length);
    g_logAddr += length;
    //printf("new g_logAddr=0x%08X\r\n", g_logAddr);
}


void LogExportSync(void)
{
    LogData_t logData, *pLogData;
    uint32_t readAddr, writeIndex, sectorCount, tick, sectorNow;
    uint8_t *logFileData;
    int32_t ret;

    logFileData = EXT_MALLOC(SPI_FLASH_SIZE_LOG);
    readAddr = g_logAddr;
    sectorCount = 0;
    while (1) {
        //find the earliest log.
        if (sectorCount++ >= SPI_FLASH_SIZE_LOG / GD25QXX_SECTOR_SIZE) {
            printf("log not found\r\n");
            EXT_FREE(logFileData);
            return;
        }
        readAddr = GetNextSectorAddr(readAddr);
        Gd25FlashReadBuffer(readAddr, (uint8_t *)&logData, LOG_DATA_HEAD_SIZE);
        if (logData.event != 0xFFFF) {
            break;
        }
    }
    printf("the earliest log addr=0x%08X\r\n", readAddr);
    writeIndex = 0;
    sectorCount = 0;
    sectorNow = readAddr / GD25QXX_SECTOR_SIZE * GD25QXX_SECTOR_SIZE;
    while (1) {
        if (sectorNow != readAddr / GD25QXX_SECTOR_SIZE * GD25QXX_SECTOR_SIZE) {
            sectorNow = readAddr / GD25QXX_SECTOR_SIZE * GD25QXX_SECTOR_SIZE;
            printf("sectorCount=%d\r\n", sectorCount);
            if (sectorCount++ > SPI_FLASH_SIZE_LOG / GD25QXX_SECTOR_SIZE) {
                printf("log export overlap\r\n");
                LogEraseSync();
                writeIndex = 0;
                break;
            }
        }
        Gd25FlashReadBuffer(readAddr, logFileData + writeIndex, LOG_DATA_HEAD_SIZE);
        pLogData = (LogData_t *)(logFileData + writeIndex);
        if (pLogData->event == NEXT_SECTOR_MARK) {
            readAddr = GetNextSectorAddr(readAddr);
            printf("goto next sector:0x%08X\r\n", readAddr);
            continue;
        }
        //printf("event=%d,length=%d,dataType=%d,checksum=0x%04X,timeStamp=%d\r\n", pLogData->event, pLogData->length, pLogData->dataType, pLogData->checksum, pLogData->timeStamp);
        readAddr += LOG_DATA_HEAD_SIZE;
        writeIndex += LOG_DATA_HEAD_SIZE;
        if (pLogData->length > 0) {
            Gd25FlashReadBuffer(readAddr, logFileData + writeIndex, pLogData->length * 4);
            readAddr += pLogData->length * 4;
            writeIndex += pLogData->length * 4;
        }
        //printf("readAddr=0x%08X,writeIndex=0x%08X\r\n", readAddr, writeIndex);
        if (readAddr == g_logAddr) {
            printf("complete a cycle reading on log zone\r\n");
            break;
        }
        if (pLogData->event == 0xFFFF) {
            printf("read data restart from head\r\n");
            readAddr = SPI_FLASH_ADDR_LOG;
        }
    }
    //PrintArray("logFileData", logFileData, writeIndex);
    // printf("delete old log file\r\n");
    printf("logname = %s\n", g_logName);
    // ret = FatfsFileDelete(logName);
    // printf("delete old log file ret=%d\r\n", ret);
    do {
        uint32_t leftSize = FatfsGetSize("0:");
        printf("start save log file,size=%d left size = \r\n", writeIndex, leftSize);
        if (writeIndex >= leftSize) {
            GuiApiEmitSignalWithValue(SIG_SETTING_LOG_EXPORT_NOT_ENOUGH_SPACE, ERROR_LOG_NOT_ENOUGH_SPACE);
            break;
        }

        if (writeIndex > 0) {
            tick = osKernelGetTickCount();
            ret = FatfsFileWrite(g_logName, logFileData, writeIndex);
            tick = osKernelGetTickCount() - tick;
            if (FatfsFileGetSize(g_logName) == writeIndex) {
                GuiApiEmitSignalWithValue(SIG_SETTING_LOG_EXPORT_SUCCESS, 0);
            } else {
                GuiApiEmitSignalWithValue(SIG_SETTING_LOG_EXPORT_FAIL, ERROR_LOG_EXPORT_ERROR);
            }
            printf("save log file ret=%d,used tick=%d,rate=%dbytes/s\r\n", ret, tick, writeIndex * 1000 / tick);
        }
    } while (0);
    EXT_FREE(logFileData);
}


void LogEraseSync(void)
{
    uint32_t addr;
    int32_t ret;

    printf("erase log flash zone\r\n");
    for (addr = SPI_FLASH_ADDR_LOG; addr < SPI_FLASH_ADDR_LOG + SPI_FLASH_SIZE_LOG; addr += GD25QXX_SECTOR_SIZE) {
        ret = Gd25FlashSectorErase(addr);
        CHECK_ERRCODE_BREAK("Gd25FlashSectorErase", ret);
    }
    g_logAddr = SPI_FLASH_ADDR_LOG;
    printf("erase log flash zone over!\r\n");
}


void LogTest(int argc, char *argv[])
{
    uint32_t event;
    if (strcmp(argv[0], "export") == 0) {
        LogExport();
    } else if (strcmp(argv[0], "write_string") == 0) {
        VALUE_CHECK(argc, 3);
        sscanf(argv[1], "%d", &event);
        WriteLogFormat(event, "test:%s", argv[2]);
    } else if (strcmp(argv[0], "erase") == 0) {
        LogErase();
    } else {
        printf("log cmd err\r\n");
    }
}



static void WriteLogAsync(LogData_t *pLogData)
{
    uint8_t *data;
    uint32_t dataLength;

    pLogData->checksum = crc16_ccitt((uint8_t *)&pLogData->timeStamp, 4);
    dataLength = LOG_DATA_HEAD_SIZE + pLogData->length * 4;

    data = SRAM_MALLOC(dataLength);
    memcpy(data, pLogData, LOG_DATA_HEAD_SIZE);
    memcpy(data + LOG_DATA_HEAD_SIZE, pLogData->pData, pLogData->length * 4);
    PubBufferMsg(LOG_MSG_WRITE, data, dataLength);
    SRAM_FREE(data);
}


static void WriteLogSync(LogData_t *pLogData)
{
    uint8_t *data;
    uint32_t dataLength;

    pLogData->checksum = crc16_ccitt((uint8_t *)&pLogData->timeStamp, 4);
    dataLength = LOG_DATA_HEAD_SIZE + pLogData->length * 4;

    data = SRAM_MALLOC(dataLength);
    memcpy(data, pLogData, LOG_DATA_HEAD_SIZE);
    memcpy(data + LOG_DATA_HEAD_SIZE, pLogData->pData, pLogData->length * 4);
    WriteLogDataToFlash(data, dataLength);
    SRAM_FREE(data);
}


static uint32_t FindLogOffsetAddr(void)
{
    LogData_t logData;
    uint32_t addr;
    for (addr = SPI_FLASH_ADDR_LOG; addr < SPI_FLASH_ADDR_LOG + SPI_FLASH_SIZE_LOG;) {
        Gd25FlashReadBuffer(addr, (uint8_t *)&logData, LOG_DATA_HEAD_SIZE);         //Read a log head
        //printf("addr=0x%08lX,event=0x%04lX\r\n", addr, logData.event);
        if (logData.event == 0xFFFF) {
            return addr;
        }
        if (logData.event == NEXT_SECTOR_MARK) {
            addr = GetNextSectorAddr(addr);
            //printf("goto next sector\r\n");
            if (addr == SPI_FLASH_ADDR_LOG) {
                printf("log overlap, clear log flash zone.\r\n");
                break;
            }
            continue;
        }
        if (logData.checksum != (crc16_ccitt((uint8_t *)&logData.timeStamp, 4) & 0x000F)) {
            printf("log check error, clear log flash zone.\r\n");
            break;
        }
        addr = addr + LOG_DATA_HEAD_SIZE + logData.length * 4;
    }
    //err, need erase the whole log flash zone.
    LogEraseSync();
    return SPI_FLASH_ADDR_LOG;
}



static void EraseNextSector(uint32_t length)
{
    uint32_t nextSectorAddr;
    uint16_t nextSectorMark = NEXT_SECTOR_MARK;
    if (g_logAddr % GD25QXX_SECTOR_SIZE + length >= GD25QXX_SECTOR_SIZE) {
        //The current log or the next log will across sectors, need erase next sector
        nextSectorAddr = GetNextSectorAddr(g_logAddr);
        printf("erase next sector,addr=0x%08lX\r\n", nextSectorAddr);
        Gd25FlashSectorErase(nextSectorAddr);
        if (g_logAddr % GD25QXX_SECTOR_SIZE + length > GD25QXX_SECTOR_SIZE) {
            Gd25FlashWriteBuffer(g_logAddr, (uint8_t *)&nextSectorMark, sizeof(nextSectorMark));
        }
        g_logAddr = nextSectorAddr;
    }
}


static uint32_t GetNextSectorAddr(uint32_t addr)
{
    uint32_t nextSectorAddr;

    nextSectorAddr = (addr + GD25QXX_SECTOR_SIZE) / GD25QXX_SECTOR_SIZE * GD25QXX_SECTOR_SIZE;
    if (nextSectorAddr >= SPI_FLASH_ADDR_LOG + SPI_FLASH_SIZE_LOG) {
        nextSectorAddr = SPI_FLASH_ADDR_LOG;
    }
    return nextSectorAddr;
}

