#include "lv_i18n.h"
#include "lv_i18n_api.h"
#include "device_setting.h"

extern const lv_i18n_language_pack_t lv_i18n_language_pack[];

typedef struct {
    LANG_TYPE_ENUM langId;
    const char *langCode;
} LanguageInfo_t;

const LanguageInfo_t g_languageList[] = {
    {LANG_EN, "en"},
    {LANG_RU, "ru"},
    {LANG_ZH_CN, "zh"},
};

static uint8_t g_curLangIndex = LANG_RU;

void LanguageInit(void)
{
    lv_i18n_init(lv_i18n_language_pack);
    g_curLangIndex = GetLanguage();
    lv_i18n_set_locale(g_languageList[g_curLangIndex].langCode);
    GuiSetLanguageFont(g_languageList[g_curLangIndex].langId);
}

void LanguageSwitch(uint8_t langIndex)
{
    lv_i18n_init(lv_i18n_language_pack);
    lv_i18n_set_locale(g_languageList[langIndex].langCode);
    GuiSetLanguageFont(langIndex);
    SetLanguage(langIndex);
    SaveDeviceSettings();
    g_curLangIndex = langIndex;
}

uint8_t LanguageGetIndex(void)
{
    return g_curLangIndex;
}
