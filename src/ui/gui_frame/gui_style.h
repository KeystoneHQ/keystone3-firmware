#ifndef _GUI_STYLE_H
#define _GUI_STYLE_H

#include "gui_resource.h"

#define GRAY_COLOR                          lv_color_hex(0x27282D)
#define WHITE_COLOR                         lv_color_hex(0xFFFFFF)
#define BLACK_COLOR                         lv_color_hex(0x0)
#define ORANGE_COLOR                        lv_color_hex(0xF5870A)
#define RED_COLOR                           lv_color_hex(0xF55831)
#define DARK_BG_COLOR                       lv_color_hex(0x333333)
#define WHITE_COLOR_OPA12                   lv_color_hex(0x4b4b4b)
#define WHITE_COLOR_OPA20                   lv_color_hex(0x5c5c5c)
#define WHITE_COLOR_OPA64                   lv_color_hex(0xA4A4A4)
#define BLUE_COLOR                          lv_color_hex(0x5C87FF)
#define BLUE_GREEN_COLOR                    lv_color_hex(0x1BE0C6)
#define GREEN_COLOR                         lv_color_hex(0x2EA374)
#define DARK_GRAY_COLOR                     lv_color_hex(0x666666)
#define DEEP_ORANGE_COLOR                   lv_color_hex(0xF55831)
#define LIGHT_BLUE_COLOR                    lv_color_hex(0x1BE0C6)

typedef struct GuiFontDesc {
    uint8_t langIndex;
    const lv_font_t *title;
    const lv_font_t *littleTitle;
    const lv_font_t *text;
    const lv_font_t *illustrate;
    const lv_font_t *boldIllustrate;
} GuiFontDesc_t;

extern lv_style_t g_mnemonicDarkStyle;
extern lv_style_t g_numBtnmStyle;
extern lv_style_t g_numBtnmDisabledStyle;
extern lv_style_t g_numBtnmCheckedStyle;
extern lv_style_t g_dividerLineStyle;
extern lv_style_t g_numShareStyle;
extern lv_style_t g_enterPassBtnmStyle;
extern lv_style_t g_enterPressBtnmStyle;
extern lv_style_t g_generalBtnPressStyle;

extern lv_font_t *g_defTitleFont;
extern lv_font_t *g_defLittleTitleFont;
extern lv_font_t *g_defTextFont;
extern lv_font_t *g_defIllustrateFont;
extern lv_font_t *g_defBoldIllustratFont;

void GuiStyleInit(void);
void GuiSetLanguageFont(uint8_t langIndex);

#endif /* _GUI_STYLE_H */

