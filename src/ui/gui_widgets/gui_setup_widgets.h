#ifndef _GUI_SETUP_WIDGETS_H
#define _GUI_SETUP_WIDGETS_H

#include <stdint.h>
#include "device_setting.h"

typedef enum {
    WALLET_METHOD_CREATE,
    WALLET_METHOD_IMPORT,

    WALLET_METHOD_BUTT,
} WALLET_METHOD_ENUM;

extern SETUP_PHASE_ENUM lastShutDownPage;

uint8_t GuiSetupNextTile(void);
uint8_t GuiSetupPrevTile(void);
void GuiSetupAreaDeInit(void);
void GuiSetupAreaDeInit(void);
void GuiSetupAreaInit(void);
void GuiSetupAreaRestart(void);
void GuiSetupAreaRefresh(void);
bool GuiIsSetup(void);

uint8_t GuiSetSetupPhase(SETUP_PHASE_ENUM pahaseEnum);
bool GuiJudgeCurrentPahse(SETUP_PHASE_ENUM pahaseEnum);

#endif /* _GUI_SETUP_WIDGETS_H */
