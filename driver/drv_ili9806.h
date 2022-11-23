/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: ILI9806 8080 lcd drvier.
 * Author: leon sun
 * Create: 2023-3-31
 ************************************************************************************************/


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
