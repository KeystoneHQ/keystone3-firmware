/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Assert handler.
 * Author: leon sun
 * Create: 2023-4-7
 ************************************************************************************************/


#include "assert.h"
#include "presetting.h"
#include "log_print.h"

#ifdef COMPILE_SIMULATOR

void ShowAssert(const char* file, uint32_t len)
{
    printf("assert,file=%s\r\nline=%d\r\n", file, len);
    while (1);
}

#else

#include "draw_on_lcd.h"

LV_FONT_DECLARE(openSans_20);

void ShowAssert(const char *file, uint32_t len)
{
    PrintOnLcd(&openSans_20, 0xFFFF, "assert,file=%s\nline=%d\n\n", file, len);
    PrintErrorInfoOnLcd();
    while (1);
}

#endif

