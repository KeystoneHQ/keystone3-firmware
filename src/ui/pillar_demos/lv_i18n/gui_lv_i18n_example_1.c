#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "gui.h"
#include "lv_i18n.h"
#include "lv_i18n_api.h"

#define DEFAULT_CONFIG_PATH "C:\\list.txt"
void gui_lv_i18n_example_1(void)
{
    // LanguageInit();
    LanguageSwitch(LANG_ZH_CN);
    lv_obj_t *cont = GuiCreateContainer(480, 800);
    lv_obj_t *label = GuiCreateTextLabel(cont, _("language_title"));
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -40);

    LanguageSwitch(LANG_EN);
    label = GuiCreateLabel(cont, _("title1"));
    printf("%s...\n", _("title1"));
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label, &openSans_20, LV_PART_MAIN);
}

