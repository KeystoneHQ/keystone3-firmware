#ifndef LV_I18N_API_H
#define LV_I18N_API_H

#include "gui_style.h"

typedef enum {
    LANG_EN,
#ifdef RU_SUPPORT
    LANG_RU,
#endif
#ifdef CN_SUPPORT
    LANG_ZH_CN,
#endif

    LANG_TYPE_BUTT,
} LANG_TYPE_ENUM;

void LanguageInit(void);
void LanguageSwitch(uint8_t langIndex);
uint8_t LanguageGetIndex(void);

#endif
