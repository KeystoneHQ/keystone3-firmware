/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Directly draw items on LCD with out GUI handler.
 * Author: leon sun
 * Create: 2023-4-8
 ************************************************************************************************/


#ifndef _DRAW_ON_LCD_H
#define _DRAW_ON_LCD_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "lv_font.h"
#include "lv_img_buf.h"

void PrintOnLcd(const lv_font_t *font, uint16_t color, const char *format, ...);
int16_t DrawStringOnLcd(uint16_t x, uint16_t y, const char *string, uint16_t color, const lv_font_t *font);
void DrawProgressBarOnLcd(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint8_t progress, uint16_t color);
void DrawImageOnLcd(uint16_t x, uint16_t y, const lv_img_dsc_t *imgDsc);
void DrawBootLogoOnLcd(void);

#endif
