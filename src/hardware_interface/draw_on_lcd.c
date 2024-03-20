#include "draw_on_lcd.h"
#include "stdio.h"
#include "string.h"
#include "lv_font.h"
#include "lv_color.h"
#include "hal_lcd.h"
#include "assert.h"
#include "user_delay.h"
#include "math.h"
#include "stdlib.h"
#include "ui_display_task.h"
#include "stdarg.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "drv_lcd_bright.h"


#define TEXT_LINE_GAP               3
#define DRAW_MAX_STRING_LEN         256
#define PAGE_MARGINS                15

typedef struct {
    uint16_t green_h : 3;
    uint16_t red : 5;
    uint16_t blue : 5;
    uint16_t green_l : 3;
} LcdDrawColor_t;


static void DrawLetterOnLcd(uint16_t x, uint16_t y, uint16_t width, uint16_t height, lv_font_glyph_dsc_t *dsc, const uint8_t *map_p, uint16_t color);
static void GetTrueColors(uint16_t *trueColors, const uint8_t *colorData, uint32_t pixelNum, lv_img_cf_t colorFormat);


static LcdDrawColor_t g_bgColor = {0};

void PrintOnLcd(const lv_font_t *font, uint16_t color, const char *format, ...)
{
    static bool backInit = false;
    static uint16_t yCursor = PAGE_MARGINS;
    uint8_t *gram = GetLvglGramAddr();
    char str[DRAW_MAX_STRING_LEN];

    LcdDrawColor_t *pColor;
    va_list argList;
    va_start(argList, format);
    if (backInit == false) {
        backInit = true;
        osKernelLock();
        //g_bgColor.blue = 31;
        while (LcdBusy());
        for (uint32_t i = 0; i < GetLvglGramSize() / 2; i++) {
            pColor = (LcdDrawColor_t *)&gram[i * 2];
            memcpy(pColor, &g_bgColor, sizeof(LcdDrawColor_t));
        }
        //memset(gram, 0, GetLvglGramSize());
        LcdDraw(0, 0, LCD_DISPLAY_WIDTH - 1, 400 - 1, (uint16_t *)gram);
        while (LcdBusy());
        LcdDraw(0, 400, LCD_DISPLAY_WIDTH, 800 - 1, (uint16_t *)gram);
        while (LcdBusy());
    }
    vsprintf(str, format, argList);
    printf(str);
    yCursor = DrawStringOnLcd(PAGE_MARGINS, yCursor, str, color, font);
    va_end(argList);
}


/// @brief Draw string on lcd.
/// @param[in] x Coordinate x.
/// @param[in] y Coordinate y.
/// @param[in] string
/// @param[in] color
/// @param[in] font
int16_t DrawStringOnLcd(uint16_t x, uint16_t y, const char *string, uint16_t color, const lv_font_t *font)
{
    const uint8_t *bitmap;
    uint16_t charWidth, charHeight, xCursor;
    uint32_t len = strlen(string);
    lv_font_glyph_dsc_t dsc;

    charHeight = lv_font_get_line_height(font);
    xCursor = x;
    for (uint32_t i = 0; i < len; i++) {
        if (string[i] == '\r') {
            continue;
        }
        if (string[i] == '\n') {
            //new line.
            xCursor = x;
            y += (charHeight + TEXT_LINE_GAP);
            continue;
        }
        charWidth = lv_font_get_glyph_width(font, string[i], string[i + 1]);
        bitmap = lv_font_get_glyph_bitmap(font, string[i]);
        lv_font_get_glyph_dsc(font, &dsc, string[i], string[i + 1]);
        //printf("%c:\r\n", string[i]);
        //printf("charWidth=%d,adv_w=%d,box_h=%d,box_w=%d,bpp=%d,is_placeholder=%d,ofs_x=%d,ofs_y=%d\r\n", charWidth, dsc.adv_w, dsc.box_h, dsc.box_w, dsc.bpp, dsc.is_placeholder, dsc.ofs_x, dsc.ofs_y);
        if (xCursor + charWidth >= LCD_DISPLAY_WIDTH - PAGE_MARGINS - 1) {
            //new line
            xCursor = x;
            y += (charHeight + TEXT_LINE_GAP);
        }
        DrawLetterOnLcd(xCursor, y, charWidth, charHeight, &dsc, bitmap, color);
        xCursor += charWidth;
    }
    //return y + charHeight + TEXT_LINE_GAP;
    return y;
}


/// @brief Draw progress bar on lcd.
/// @param[in] x Coordinate x.
/// @param[in] y Coordinate y.
/// @param[in] progress 0-100.
/// @param[in] color
void DrawProgressBarOnLcd(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint8_t progress, uint16_t color)
{
    uint16_t pixelMap[length * width];
    uint16_t row, col;
    int32_t pow1, pow2, radius, radiusPow, centerX;
    bool progressPositive, outOfArc;

    radius = width / 2;
    radiusPow = radius * radius;
    for (col = 0; col < length; col++) {
        progressPositive = (col < length * progress / 100);
        for (row = 0; row < width; row++) {
            outOfArc = false;
            if (col < radius || col > length - radius) {
                //arc calc
                centerX = col < radius ? radius : length - radius;
                pow1 = abs(centerX - col);
                pow1 = pow1 * pow1;
                pow2 = abs((int32_t)radius - row);
                pow2 = pow2 * pow2;
                //printf("radius=%d,col=%d,row=%d,pow1=%d,pow2=%d,radiusPow=%d\r\n", radius, col, row, pow1, pow2, radiusPow);
                if (pow1 + pow2 > radiusPow) {
                    outOfArc = true;
                }
            }
            if (outOfArc) {
                pixelMap[row * length + col] = 0;           //black
            } else if (progressPositive) {
                pixelMap[row * length + col] = color;       //color
            } else {
                pixelMap[row * length + col] = 0x8631;      //dark
            }
        }
    }
    while (LcdBusy());
    LcdDraw(x, y, x + length - 1, y + width - 1, pixelMap);
    while (LcdBusy());
}


void DrawImageOnLcd(uint16_t x, uint16_t y, const lv_img_dsc_t *imgDsc)
{
    uint16_t *colors;

    colors = SRAM_MALLOC(imgDsc->header.w * imgDsc->header.h * 2);
    GetTrueColors(colors, imgDsc->data, imgDsc->header.w * imgDsc->header.h, imgDsc->header.cf);
    LcdDraw(x, y, x + imgDsc->header.w - 1, y + imgDsc->header.h - 1, colors);
    while (LcdBusy());
    SRAM_FREE(colors);
}


extern const lv_img_dsc_t imgBootLogo;
#ifdef BTC_ONLY
extern const lv_img_dsc_t imgBootBtc;
#endif

void DrawBootLogoOnLcd(void)
{
#ifndef BTC_ONLY
    DrawImageOnLcd(192, 300, &imgBootLogo);
#else
    DrawImageOnLcd(192, 245, &imgBootLogo);
    DrawImageOnLcd(150, 712, &imgBootBtc);
#endif
    UserDelay(100);
    SetLcdBright(50);
}


static void DrawLetterOnLcd(uint16_t x, uint16_t y, uint16_t width, uint16_t height, lv_font_glyph_dsc_t *dsc, const uint8_t *map_p, uint16_t color)
{
    uint16_t row, col, i, gapX, gapY, gapW, gapH, j;
    uint8_t pixel, r, b, g;
    LcdDrawColor_t color16, *pColor;
    LcdDrawColor_t pixelColor;
    uint32_t maxHeight = height > (height - dsc->box_h - dsc->ofs_y) ? height : (height - dsc->box_h - dsc->ofs_y);
    uint16_t pixelMap[width * maxHeight];

    memcpy(&color16, &color, sizeof(LcdDrawColor_t));
    r = color16.red;
    g = (color16.green_h << 3) + color16.green_l;
    b = color16.blue;

    if (height > dsc->box_h && dsc->box_w > 0 && dsc->box_h > 0) {
        while (LcdBusy());
        for (j = 0; j < (height - dsc->box_h - dsc->ofs_y) * width; j++) {
            pColor = (LcdDrawColor_t *)&pixelMap[j];
            memcpy(pColor, &g_bgColor, sizeof(LcdDrawColor_t));
        }
        //memset(pixelMap, 0x00, (height - dsc->box_h - dsc->ofs_y) * width * 2);
        LcdDraw(x, y, x + width - 1, y + (height - dsc->box_h - dsc->ofs_y) - 1, pixelMap);
        while (LcdBusy());
    }
    i = 0;
    for (row = 0; row < dsc->box_h; row++) {
        for (col = 0; col < dsc->box_w; col++) {
            pixel = (map_p[i / 2] >> ((i % 2) == 0 ? 4 : 0)) & 0x0F;
            if (pixel == 0) {
                memcpy(&pixelMap[i], &g_bgColor, sizeof(uint16_t));
            } else {
                pixelColor.red = ((r * pixel / 15) & 0x1F) + g_bgColor.red;
                pixelColor.green_l = ((g * pixel / 15) & 0x07) + g_bgColor.green_l;
                pixelColor.green_h = (((g * pixel / 15) >> 3) & 0x07) + g_bgColor.green_h;
                pixelColor.blue = ((b * pixel / 15) & 0x1F) + g_bgColor.blue;
                memcpy(&pixelMap[i], &pixelColor, sizeof(uint16_t));
            }
            i++;
        }
    }
    if (dsc->box_w > 0 && dsc->box_h > 0) {
        while (LcdBusy());
        LcdDraw(x, y + height - dsc->box_h - dsc->ofs_y, x + dsc->box_w - 1, y + height - dsc->ofs_y - 1, pixelMap);
        while (LcdBusy());
        //printf("x=%d,y=%d,box_w=%d,box_h=%d,width=%d\r\n", x, y, dsc->box_w, dsc->box_h, width);
    }
    if (width > dsc->box_w) {
        gapX = x + dsc->box_w;
        gapY = y;
        gapW = width - dsc->box_w;
        gapH = height;
        //wait for DMA send over
        while (LcdBusy());
        for (j = 0; j < gapW * gapH; j++) {
            pColor = (LcdDrawColor_t *)&pixelMap[j];
            memcpy(pColor, &g_bgColor, sizeof(LcdDrawColor_t));
        }
        //memset(pixelMap, 0x00, gapW * gapH * 2);
        LcdDraw(gapX, gapY, gapX + gapW - 1, gapY + gapH - 1, pixelMap);
        while (LcdBusy());
        //printf("width=%d,box_w=%d,gapX=%d,gapY=%d,gapW=%d,gapH=%d\r\n", width, dsc->box_w, gapX, gapY, gapW, gapH);
    }
}


static void GetTrueColors(uint16_t *trueColors, const uint8_t *colorData, uint32_t pixelNum, lv_img_cf_t colorFormat)
{
    lv_color16_t pixel;
    uint16_t green;
    uint32_t i;
    if (colorFormat == LV_IMG_CF_TRUE_COLOR_ALPHA) {
        for (i = 0; i < pixelNum; i++) {
            memcpy(&pixel.full, &colorData[i * 3], 2);
            pixel.ch.blue = pixel.ch.blue * colorData[i * 3 + 2] / 255;
            pixel.ch.red = pixel.ch.red * colorData[i * 3 + 2] / 255;
            green = (pixel.ch.green_h << 3) + pixel.ch.green_l;
            green = green * colorData[i * 3 + 2] / 255;
            pixel.ch.green_h = green >> 3;
            pixel.ch.green_l = green & 0x0007;
            trueColors[i] = pixel.full;
        }
    } else if (colorFormat == LV_IMG_CF_TRUE_COLOR) {
        memcpy(trueColors, colorData, pixelNum * 2);
    }
}

