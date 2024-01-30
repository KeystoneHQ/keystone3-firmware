#ifndef _GUI_SETUP_WIDGETS_H
#define _GUI_SETUP_WIDGETS_H

#include <stdint.h>

typedef enum {
    WALLET_METHOD_CREATE,
    WALLET_METHOD_IMPORT,

    WALLET_METHOD_BUTT,
} WALLET_METHOD_ENUM;

typedef enum {
    SETUP_PAHSE_WELCOME = 0,
    SETUP_PAHSE_LANGUAGE,
    SETUP_PAHSE_WEB_AUTH,
    SETUP_PAHSE_FIRMWARE_UPDATE,
    SETUP_PAHSE_CREATE_WALLET,
    SETUP_PAHSE_DONE,
} SETUP_PHASE_ENUM;

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
void GuiCreateLanguageWidget(lv_obj_t *parent, uint16_t offset);

#endif /* _GUI_SETUP_WIDGETS_H */

