#ifndef _DRV_LCD_BRIGHT_H
#define _DRV_LCD_BRIGHT_H

#include "stdint.h"
#include "stdbool.h"

void LcdBrightInit(void);
void LcdBacklightOff(void);
void LcdBacklightOn(void);
void LcdFadesOut(void);
void SetLcdBright(uint32_t bright);

#endif
