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
void ShowAssert(const char* file, uint32_t len)
{
    printf("assert,file=%s\r\nline=%d\r\n", file, len);
    while (1);
}
#else

#include "draw_on_lcd.h"
#include "cmsis_os.h"

LV_FONT_DECLARE(openSans_20);

void ShowAssert(const char *file, uint32_t len)
{
    char assertStr[BUFFER_SIZE_256];

    osKernelLock();

    snprintf_s(assertStr, BUFFER_SIZE_256, "assert,file=%s,line=%d", file, (int)len);
    Gd25FlashWriteBufferNoMutex(SPI_FLASH_ADDR_ERR_INFO, (uint8_t *)assertStr, strnlen_s(assertStr, sizeof(assertStr) - 1) + 1);

    PrintOnLcd(&openSans_20, 0xFFFF, "The error was caused by a failed data request.\nYour assets remain safe.\n");
    PrintErrorInfoOnLcd();
    RestartCountdownOnLcd();
}

#endif

