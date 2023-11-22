#ifndef _HAL_LCD_H
#define _HAL_LCD_H

#include "stdint.h"
#include "stdbool.h"

#define LCD_DISPLAY_WIDTH               480
#define LCD_DISPLAY_HEIGHT              800


typedef struct {
    void (*Init)(void);
    bool (*Busy)(void);
    void (*Clear)(uint16_t color);
    void (*Draw)(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t *colors);
} HalLcdOpt_t;

void LcdCheck(void);
void LcdInit(void);
bool LcdBusy(void);
void LcdClear(uint16_t color);
void LcdDraw(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t *colors);
void LcdFullScreen(uint16_t color);
void LcdTest(int argc, char *argv[]);

#endif
