#ifndef _DRV_ILI9806_H
#define _DRV_ILI9806_H

#include "stdint.h"
#include "stdbool.h"

#define ILI9806_WIDTH               480
#define ILI9806_HEIGHT              800

void Ili9806Init(void);
bool Ili9806Busy(void);
void Ili9806Clear(uint16_t color);
void Ili9806Draw(uint16_t xStart, uint16_t Ystart, uint16_t xEnd, uint16_t yEnd, uint16_t *colors);

#endif
