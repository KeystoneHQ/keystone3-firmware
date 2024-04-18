#include "gui_style.h"
#include "lv_i18n_api.h"

lv_style_t g_mnemonicDarkStyle;
lv_style_t g_numBtnmStyle;
lv_style_t g_numBtnmDisabledStyle;
lv_style_t g_numBtnmCheckedStyle;
lv_style_t g_enterPassBtnmStyle;
lv_style_t g_enterPressBtnmStyle;
lv_style_t g_numShareStyle;
lv_style_t g_dividerLineStyle;
lv_style_t g_generalBtnPressStyle;

lv_font_t *g_defTitleFont = (lv_font_t *)&openSansEnTitle;                  //36
lv_font_t *g_defLittleTitleFont = (lv_font_t *)&openSansEnLittleTitle;      //28
lv_font_t *g_defTextFont = (lv_font_t *)&openSansEnText;                    //24
lv_font_t *g_defIllustrateFont = (lv_font_t *)&openSansEnIllustrate;        //20
lv_font_t *g_defBoldIllustratFont = (lv_font_t *)&openSansEnIllustrate;     //20 bold

static const GuiFontDesc_t g_langFontArr[] = {
    {LANG_EN, &openSansEnTitle, &openSansEnLittleTitle, &openSansEnText, &openSansEnIllustrate, &openSansEnIllustrate},
#ifdef RU_SUPPORT
    {LANG_RU, &rusTitle, &rusLittleTitle, &rusText, &rusIllustrate, &rusIllustrate},
#endif
#ifdef CN_SUPPORT
    {LANG_ZH_CN, &cnTitle, &openSansCnLittleTitle, &openSansCnText, &openSansCnIllustrate, &openSansCnIllustrate},
#endif
};

static void GuiBtnmStyleInit(void)
{
    lv_style_init(&g_mnemonicDarkStyle);
    lv_style_set_bg_color(&g_mnemonicDarkStyle, DARK_BG_COLOR);
    lv_style_set_radius(&g_mnemonicDarkStyle, 8);
    lv_style_set_shadow_width(&g_mnemonicDarkStyle, 0);
    lv_style_set_border_width(&g_mnemonicDarkStyle, 2);
    lv_style_set_outline_width(&g_mnemonicDarkStyle, 0);
    lv_style_set_text_color(&g_mnemonicDarkStyle, WHITE_COLOR);

    lv_style_init(&g_numBtnmStyle);
    lv_style_set_bg_color(&g_numBtnmStyle, DARK_GRAY_COLOR);
    lv_style_set_radius(&g_numBtnmStyle, 6);
    lv_style_set_shadow_width(&g_numBtnmStyle, 0);
    lv_style_set_border_width(&g_numBtnmStyle, 0);
    lv_style_set_outline_width(&g_numBtnmStyle, 0);
    lv_style_set_text_color(&g_numBtnmStyle, WHITE_COLOR);

    lv_style_init(&g_numBtnmDisabledStyle);
    lv_style_set_bg_color(&g_numBtnmDisabledStyle, ORANGE_COLOR);
    lv_style_set_bg_opa(&g_numBtnmDisabledStyle, LV_OPA_30);

    lv_style_init(&g_numBtnmCheckedStyle);
    lv_style_set_bg_color(&g_numBtnmCheckedStyle, ORANGE_COLOR);

    lv_style_init(&g_numShareStyle);
    lv_style_set_bg_color(&g_numShareStyle, DARK_BG_COLOR);
    lv_style_set_radius(&g_numShareStyle, 12);
    lv_style_set_shadow_width(&g_numShareStyle, 0);
    lv_style_set_border_width(&g_numShareStyle, 2);
    lv_style_set_border_color(&g_numShareStyle, DARK_BG_COLOR);
    lv_style_set_outline_width(&g_numShareStyle, 0);
    lv_style_set_text_color(&g_numShareStyle, WHITE_COLOR);

    lv_style_init(&g_enterPassBtnmStyle);
    lv_style_set_bg_color(&g_enterPassBtnmStyle, BLACK_COLOR);
    lv_style_set_radius(&g_enterPassBtnmStyle, 6);
    lv_style_set_shadow_width(&g_enterPassBtnmStyle, 0);
    lv_style_set_border_width(&g_enterPassBtnmStyle, 0);
    lv_style_set_outline_width(&g_enterPassBtnmStyle, 0);
    lv_style_set_text_color(&g_enterPassBtnmStyle, WHITE_COLOR);

    lv_style_init(&g_enterPressBtnmStyle);
    lv_style_set_bg_color(&g_enterPressBtnmStyle, WHITE_COLOR);
    lv_style_set_bg_opa(&g_enterPressBtnmStyle, LV_OPA_10 + LV_OPA_2);
    lv_style_set_radius(&g_enterPressBtnmStyle, 6);
    lv_style_set_shadow_width(&g_enterPressBtnmStyle, 0);
    lv_style_set_border_width(&g_enterPressBtnmStyle, 0);
    lv_style_set_outline_width(&g_enterPressBtnmStyle, 0);
    lv_style_set_text_color(&g_enterPressBtnmStyle, WHITE_COLOR);

    lv_style_init(&g_generalBtnPressStyle);
    lv_style_set_bg_color(&g_generalBtnPressStyle, WHITE_COLOR);
    lv_style_set_bg_opa(&g_generalBtnPressStyle, LV_OPA_10 + LV_OPA_2);
    lv_style_set_radius(&g_generalBtnPressStyle, 24);
    lv_style_set_shadow_width(&g_generalBtnPressStyle, 0);
    lv_style_set_border_width(&g_generalBtnPressStyle, 0);
    lv_style_set_outline_width(&g_generalBtnPressStyle, 0);
    lv_style_set_text_color(&g_generalBtnPressStyle, WHITE_COLOR);
}

static void GuiDividLineInit(void)
{
    lv_style_init(&g_dividerLineStyle);
    lv_style_set_line_width(&g_dividerLineStyle, 1);
    lv_style_set_line_color(&g_dividerLineStyle, WHITE_COLOR);
#ifndef BTC_ONLY
    lv_style_set_line_opa(&g_dividerLineStyle, LV_OPA_10 + LV_OPA_2);
#else
    lv_style_set_line_opa(&g_dividerLineStyle, LV_OPA_20);
#endif
    // lv_style_set_line_rounded(&g_dividerLineStyle, true);
    lv_style_set_radius(&g_dividerLineStyle, 0);
}

void GuiSetLanguageFont(uint8_t langIndex)
{
    g_defTitleFont = (lv_font_t *)g_langFontArr[langIndex].title;
    g_defLittleTitleFont = (lv_font_t *)g_langFontArr[langIndex].littleTitle;
    g_defTextFont = (lv_font_t *)g_langFontArr[langIndex].text;
    g_defIllustrateFont = (lv_font_t *)g_langFontArr[langIndex].illustrate;
    g_defBoldIllustratFont = (lv_font_t *)g_langFontArr[langIndex].boldIllustrate;
}

void GuiStyleInit(void)
{
    GuiBtnmStyleInit();
    GuiDividLineInit();
}
