#ifndef LV_I18N_API_H
#define LV_I18N_API_H

#include "gui_style.h"

typedef enum {
    LANG_EN,
    LANG_RU,
    LANG_ZH_CN,
    LANG_KO,
#ifndef BTC_ONLY
    LANG_ES,
    LANG_DE,
    LANG_JA,
#endif
    LANG_TYPE_BUTT,
} LANG_TYPE_ENUM;

void LanguageInit(void);
void LanguageSwitch(uint8_t langIndex);
uint8_t LanguageGetIndex(void);

#endif
