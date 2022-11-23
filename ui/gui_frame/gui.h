#ifndef _GUI_H
#define _GUI_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "define.h"
#include "err_code.h"
#include "gui_resource.h"
#include "gui_style.h"
#include "gui_obj.h"
#include "lv_i18n.h"
#include "lv_i18n_api.h"
#include "user_utils.h"

#define GUI_STATUS_BAR_HEIGHT               (48)
#define GUI_NAV_BAR_HEIGHT                  (96)
#define GUI_MAIN_AREA_OFFSET                (GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT)

#define GUI_DEL_OBJ(obj)                    if (obj != NULL) { \
    lv_obj_del(obj);                        \
    obj = NULL;                             \
}

typedef struct GuiPosition {
    int16_t x;
    int16_t y;
} GuiPosition_t;

static bool inline GuiDarkMode(void)
{
    return true;
}

#define GUI_ASSERT(expr)                                        \
    do {                                                        \
        if(!(expr)) {                                           \
            printf("Asserted at expression: %s", #expr);        \
        }                                                       \
    } while(0)

#endif /* _GUI_H */

