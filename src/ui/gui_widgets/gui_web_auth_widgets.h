#ifndef _GUI_WEB_AUTH_WIDGETS_H
#define _GUI_WEB_AUTH_WIDGETS_H

#include <stdint.h>

typedef enum {
    WEB_AUTH_ENTRY_SETUP = 0,
    WEB_AUTH_ENTRY_SETTING,

    WEB_AUTH_ENTRY_BUTT,
} WEB_AUTH_ENTRY_ENUM;

void GuiWebAuthAreaInit();
void GuiWebAuthAreaDeInit();
void GuiWebAuthAreaRefresh();
void GuiWebAuthAreaRestart();
void GuiWebAuthSetEntry(uint8_t entry);
void GuiWebAuthShowKeyCheck(const char *keyCheck);
void GuiWebAuthShowVerificationId(uint32_t crc);

#endif
