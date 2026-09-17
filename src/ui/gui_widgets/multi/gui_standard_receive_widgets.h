#ifndef _GUI_STANDARD_RECEIVE_WIDGETS_H
#define _GUI_STANDARD_RECEIVE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    uint8_t chain;
    char address[44];
} StandardReceiveParams_t;

void GuiStandardReceiveInit(uint8_t chain, const char *address);
void GuiStandardReceiveDeInit(void);
void GuiStandardReceiveRefresh(void);
void GuiStandardReceivePrevTile(void);
void GuiResetCurrentStandardAddressIndex(uint8_t index);
void GuiResetAllStandardAddressIndex(void);
#endif
