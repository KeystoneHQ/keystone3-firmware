#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include "gui.h"

#if 0
typedef enum {
    BUTTON_TYPE_LABEL = 0,
    BUTTON_TYPE_IMG,
    BUTTON_TYPE_CONT,

    BUTTON_TYPE_BUTT,
} BUTTON_TYPE_ENUM;
#endif

typedef struct GuiButton {
    lv_obj_t *obj;
    lv_align_t align;
    GuiPosition_t position;
} GuiButton_t;

void *GuiCreateButton(lv_obj_t *parent, uint16_t w, uint16_t h, GuiButton_t *member,
                      uint8_t cnt, lv_event_cb_t buttonCb, void *param);
void *GuiCreateImgLabelButton(lv_obj_t *parent, const char *text, const void *src, uint16_t width,
                              lv_event_cb_t buttonCb, void *param);
void *GuiCreateStatusCoinButton(lv_obj_t *parent, const char *text, const void *src);
void *GuiUpdateStatusCoinButton(lv_obj_t *button, const char *text, const void *src);
void *GuiCreateImgLabelAdaptButton(lv_obj_t *parent, const char *text, const void *src,
                                   lv_event_cb_t buttonCb, void *param);
void *GuiCreateSelectButton(lv_obj_t *parent, const char *text, const void *src,
                            lv_event_cb_t buttonCb, void *param, bool isCling);
void *GuiCreateImgButton(lv_obj_t *parent, const void *src, uint16_t width,
                         lv_event_cb_t buttonCb, void *param);
void *GuiSettingItemButton(lv_obj_t *parent, uint16_t width, const char *text, const char *descText, const void *src,
                           lv_event_cb_t buttonCb, void *param);


#endif /* _GUI_BUTTON_H */

