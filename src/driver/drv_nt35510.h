/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: NT35510 8080驱动芯片.
 * Author: leon sun
 * Create: 2022-11-24
 ************************************************************************************************/


#ifndef _DRV_NT35510_H
#define _DRV_NT35510_H

#include "stdint.h"
#include "stdbool.h"

#define NT35510_WIDTH               480
#define NT35510_HEIGHT              800

void Nt35510Init(void);
bool Nt35510Busy(void);
void Nt35510Clear(uint16_t color);
void Nt35510Draw(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t *colors);

#endif
