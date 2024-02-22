#include "assert.h"
#include "stdio.h"
#include "presetting.h"
#include "log_print.h"
#include "drv_gd25qxx.h"
#include "flash_address.h"
#include "safe_str_lib.h"

#ifdef COMPILE_SIMULATOR

#ifdef COMPILE_MAC_SIMULATOR
#include "stdio.h"
void ShowAssert(const char* file, uint32_t len)
{
    printf("assert,file=%s\r\nline=%d\r\n", file, len);
    while (1);
}
#endif

#else

#include "draw_on_lcd.h"

LV_FONT_DECLARE(openSans_20);

void ShowAssert(const char *file, uint32_t len)
{
    char assertStr[256];
    PrintOnLcd(&openSans_20, 0xFFFF, "assert,file=%s\nline=%d\n\n", file, len);
    PrintErrorInfoOnLcd();
    sprintf(assertStr, "assert,file=%s,line=%d", file, len);
    Gd25FlashWriteBufferNoMutex(SPI_FLASH_ADDR_ERR_INFO, (uint8_t *)assertStr, strnlen_s(assertStr, sizeof(assertStr) - 1) + 1);
    while (1);
}

#endif

