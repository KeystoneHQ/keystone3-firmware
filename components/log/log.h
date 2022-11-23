/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: log stored in SPI flash.
 * Author: leon sun
 * Create: 2023-4-11
 ************************************************************************************************/

#ifndef _LOG_H
#define _LOG_H

#include "stdint.h"
#include "stdbool.h"

//EVENT ID
enum {
    EVENT_ID_BOOT                       = 1,
    EVENT_ID_REBOOT,
    EVENT_ID_ERROR,
    EVENT_ID_BATTERY,
};

enum {
    ERROR_LOG_NOT_ENOUGH_SPACE          = 1,
    ERROR_LOG_HAVE_NO_SD_CARD,
    ERROR_LOG_EXPORT_ERROR,
};


void WriteLogEvent(uint32_t event);
void WriteLogValue(uint32_t event, uint32_t value);
void WriteLogFormat(uint32_t event, const char *format, ...);
void LogExport(void);
void LogErase(void);


void LogInit(void);
void WriteLogDataToFlash(const void *data, uint32_t length);
void LogExportSync(void);
void LogEraseSync(void);
void LogTest(int argc, char *argv[]);
void LogSetLogName(char *name);

#endif

