#include "hal_lcd.h"
#include "stdio.h"
#include "string.h"
#include "drv_ili9806.h"
#include "drv_nt35510.h"
#include "drv_parallel8080.h"
#include "hardware_version.h"
#include "user_memory.h"


HalLcdOpt_t g_lcdOpt;

void LcdCheck(void)
{
    if (GetHardwareVersion() == VERSION_EVT0) {
        g_lcdOpt.Init = Nt35510Init;
        g_lcdOpt.Clear = Nt35510Clear;
        g_lcdOpt.Draw = Nt35510Draw;
    } else {
        g_lcdOpt.Init = Ili9806Init;
        g_lcdOpt.Clear = Ili9806Clear;
        g_lcdOpt.Draw = Ili9806Draw;
    }
}

void LcdInit(void)
{
    g_lcdOpt.Init();
    LcdFullScreen(0);
}


bool LcdBusy(void)
{
    return Parallel8080Busy();
}


void LcdClear(uint16_t color)
{
    g_lcdOpt.Clear(color);
}


void LcdDraw(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t *colors)
{
    g_lcdOpt.Draw(xStart, yStart, xEnd, yEnd, colors);
}


void LcdFullScreen(uint16_t color)
{
    uint32_t i;
    uint16_t *colors = SRAM_MALLOC(LCD_DISPLAY_WIDTH * 80 * 2);

    for (i = 0; i < LCD_DISPLAY_WIDTH * 80; i++) {
        colors[i] = color;
    }

    for (i = 0; i < LCD_DISPLAY_HEIGHT; i += 80) {
        g_lcdOpt.Draw(0, i, LCD_DISPLAY_WIDTH - 1, i + 80 - 1, colors);
        while (LcdBusy());
    }
    SRAM_FREE(colors);
}


void LcdTest(int argc, char *argv[])
{
    uint32_t color;
    if (strcmp(argv[0], "full_color") == 0) {
        sscanf(argv[1], "%X", &color);
        LcdFullScreen((uint16_t)color);
        printf("lcd full_color=0x%X\r\n", color);
    } else {
        printf("lcd test input err\r\n");
    }
}

