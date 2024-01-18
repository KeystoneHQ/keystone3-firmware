#ifndef _GUI_HINTBOX_H
#define _GUI_HINTBOX_H

void *GuiCreateHintBox(lv_obj_t *parent, uint16_t w, uint16_t h, bool en);
void *GuiCreateHintBoxWithoutTop(lv_obj_t *parent, uint16_t w, uint16_t h);
void *GuiCreateAnimHintBox(lv_obj_t *parent, uint16_t w, uint16_t h, uint16_t animH);
void GuiDeleteAnimHintBox(void);
uint16_t GetHintBoxReHeight(uint16_t oldHeight, lv_obj_t *obj);
void GuiStopAnimHintBox(void);
void *GuiCreateResultHintbox(lv_obj_t *parent, uint16_t h, const void *src, const char *titleText,
                             const char *descText, const char *leftBtnText, lv_color_t leftColor, const char *rightBtnText, lv_color_t rightColor);
void *GuiCreateUpdateHintbox(lv_obj_t *parent, uint16_t h, const void *src, const char *titleText,
                             const char *descText, const char *leftBtnText, lv_color_t leftColor,  const char *rightBtnText, lv_color_t rightColor, bool checkSumDone);
void *GuiCreateGeneralHintBox(lv_obj_t *parent, uint16_t h, const void *src, const char *titleText,
                              const char *desc1, const char *desc2, const char *leftBtnText, lv_color_t leftColor,
                              const char *rightBtnText, lv_color_t rightColor);
void *GuiGetHintBoxLeftBtn(lv_obj_t *parent);
void *GuiGetHintBoxRightBtn(lv_obj_t *parent);
void *GuiCreateErrorCodeHintbox(int32_t errCode, lv_obj_t **param);
void CloseHintBoxHandler(lv_event_t *e);


#define GuiCreateConfirmHintBox(parent, h, src, title, desc1, desc2, btnText, color) GuiCreateGeneralHintBox(parent, h, src, title, desc1, desc2, NULL, color, btnText, color)

#endif /* _GUI_HINTBOX_H */

