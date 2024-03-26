
#ifndef _GUI_STATUS_BAR_H
#define _GUI_STATUS_BAR_H

#include "gui.h"

#ifdef BTC_ONLY
#define WALLPAPER_ENABLE            0
#else
#define WALLPAPER_ENABLE            0
#endif
typedef enum {
    NVS_BAR_RETURN = 0,
    NVS_BAR_CLOSE,
    NVS_BAR_MANAGE,

    NVS_LEFT_BUTTON_BUTT,
} NVS_LEFT_BUTTON_ENUM;

typedef enum {
    NVS_BAR_MID_WORD_SELECT = NVS_LEFT_BUTTON_BUTT + 1,
    NVS_BAR_MID_LABEL,
    NVS_BAR_MID_COIN,

    NVS_MID_BUTTON_BUTT,
} NVS_MID_BUTTON_ENUM;

typedef enum {
    NVS_BAR_WORD_SELECT = NVS_MID_BUTTON_BUTT + 1,
    NVS_BAR_WORD_RESET,
    NVS_BAR_QUESTION_MARK,
    NVS_BAR_MORE_INFO,
    NVS_BAR_SKIP,
    NVS_BAR_SEARCH,
    NVS_BAR_NEW_SKIP,
    NVS_BAR_UNDO,

    NVS_RIGHT_BUTTON_BUTT,
} NVS_RIGHT_BUTTON_ENUM;

typedef struct NavBarWidget {
    lv_obj_t *navBar;

    lv_obj_t *leftBtn;
    lv_obj_t *midBtn;
    lv_obj_t *rightBtn;
} NavBarWidget_t;

#ifdef BTC_ONLY
void GuiStatusBarSetTestNet(void);
#endif

#endif /* _GUI_STATUS_BAR_H */

