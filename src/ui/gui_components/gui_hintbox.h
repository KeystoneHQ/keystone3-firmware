/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_hintbox.h
 * Description:
 * author     : stone wang
 * data       : 2023-02-02 16:20
**********************************************************************/

#ifndef _GUI_HINTBOX_H
#define _GUI_HINTBOX_H

void *GuiCreateHintBox(lv_obj_t *parent, uint16_t w, uint16_t h, bool en);
void *GuiCreateHintBoxWithoutTop(lv_obj_t *parent, uint16_t w, uint16_t h);
void *GuiCreateAnimHintBox(lv_obj_t *parent, uint16_t w, uint16_t h, uint16_t animH);
void GuiDeleteAnimHintBox(void);
uint16_t GetHintBoxReHeight(uint16_t oldHeight, lv_obj_t *obj);
void *GuiCreateResultHintbox(lv_obj_t *parent, uint16_t h, const void *src, const char *titleText,
                             const char *descText, const char *leftBtnText, lv_color_t leftColor, const char *rightBtnText, lv_color_t rightColor);
void *GuiGetHintBoxLeftBtn(lv_obj_t *parent);
void *GuiGetHintBoxRightBtn(lv_obj_t *parent);
void *GuiCreateErrorCodeHintbox(int32_t errCode, lv_obj_t **param);
void CloseHintBoxHandler(lv_event_t *e);

#endif /* _GUI_HINTBOX_H */

