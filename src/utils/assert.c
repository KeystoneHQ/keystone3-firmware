#include "stdio.h"
#include "define.h"
#include "assert.h"
#include "stdio.h"
#include "presetting.h"
#include "log_print.h"
#include "drv_gd25qxx.h"
#include "flash_address.h"
#include "safe_str_lib.h"

#ifdef COMPILE_SIMULATOR
#include "stdio.h"
void ShowAssert(const char* file, uint32_t line)
{
#ifdef BUILD_PRODUCTION
    (void)file;
    (void)line;
    printf("assert\r\n");
#else
    printf("assert,file=%s\r\nline=%u\r\n", file, (unsigned int)line);
#endif
    while (1);
}
#else

#include "draw_on_lcd.h"
#include "cmsis_os.h"

LV_FONT_DECLARE(openSans_20);

void ShowAssert(const char *file, uint32_t line)
{
    char assertStr[BUFFER_SIZE_256];
    osKernelLock();
#ifdef BUILD_PRODUCTION
    (void)file;
    (void)line;
    snprintf_s(assertStr, BUFFER_SIZE_256, "assert");
#else
    snprintf_s(assertStr, BUFFER_SIZE_256, "assert,file=%s,line=%u", file, (unsigned int)line);
#endif
    Gd25FlashWriteBufferNoMutex(SPI_FLASH_ADDR_ERR_INFO, (uint8_t *)assertStr, strnlen_s(assertStr, sizeof(assertStr) - 1) + 1);
    PrintOnLcd(&openSans_20, 0xFFFF, "The error was caused by a failed data request.\nYour assets remain safe.\n");
#ifndef BUILD_PRODUCTION
    snprintf_s(assertStr, BUFFER_SIZE_256, "assert\nline: %u\nfile:\n", (unsigned int)line);
    PrintOnLcd(&openSans_20, 0xFFFF, assertStr);
    PrintOnLcd(&openSans_20, 0xFFFF, file);
    PrintOnLcd(&openSans_20, 0xFFFF, "\n\n");
#endif
    PrintErrorInfoOnLcd();
    RestartCountdownOnLcd();
}

#endif
