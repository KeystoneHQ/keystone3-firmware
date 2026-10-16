#ifndef _GUI_HINTBOX_H
#define _GUI_HINTBOX_H
#include "lv_event.h"
#include "lv_obj.h"
#include "gui_qr_hintbox.h"
typedef struct {
    const char *name;
    const void *src;
    lv_event_cb_t callBack;
    void *param;
} MoreInfoTable_t;

typedef struct {
    lv_obj_t *container;
    lv_obj_t *closeButton;
    lv_obj_t *checkBox;
    lv_obj_t *continueButton;
} BackupConfirmationHintBox_t;

void *GuiCreateHintBox(uint16_t h);
void *GuiCreateHintBoxWithoutTop(lv_obj_t *parent, uint16_t w, uint16_t h);
void *GuiCreateAnimHintBox(uint16_t w, uint16_t h, uint16_t animH);
void GuiDeleteAnimHintBox(void);
uint16_t GetHintBoxReHeight(uint16_t oldHeight, lv_obj_t *obj);
void GuiStopAnimHintBox(void);
void *GuiCreateResultHintbox(uint16_t h, const void *src, const char *titleText,
                             const char *descText, const char *leftBtnText,
                             lv_color_t leftColor, const char *rightBtnText,
                             lv_color_t rightColor);
void *GuiCreateUpdateHintbox(const void *src, const char *titleText,
                             const char *descText, const char *leftBtnText,
                             lv_color_t leftColor, const char *rightBtnText,
                             lv_color_t rightColor, bool checkSumDone);
void *GuiCreateGeneralHintBox(const void *src, const char *titleText,
                              const char *desc1, const char *desc2,
                              const char *leftBtnText, lv_color_t leftColor,
                              const char *rightBtnText, lv_color_t rightColor);
#ifdef CYPHERPUNK_VERSION
lv_obj_t *GuiCreateZcashSeedWarning(lv_event_cb_t backHandler, lv_event_cb_t continueHandler);
#endif
void *GuiCreateBackupConfirmationHintBox(BackupConfirmationHintBox_t *hintBox);
lv_obj_t *GuiCreateSdCardErrorHintBox(bool cardMissing, lv_event_cb_t closeCallback);
void *GuiGetHintBoxLeftBtn(lv_obj_t *parent);
void *GuiGetHintBoxRightBtn(lv_obj_t *parent);
void CloseHintBoxHandler(lv_event_t *e);
void GuiHintBoxResize(lv_obj_t *obj, uint16_t height);
void *GuiCreateMoreInfoHintBox(const void *src, const char *titleText,
                               MoreInfoTable_t *table, uint8_t cnt,
                               bool isCling, void *parent);
void GuiCreateTooltipHintBox(const char *titleText, const char *descText, const char *link);
void CloseTooltipHintBoxHandler();

#define GuiCreateConfirmHintBox(src, title, desc1, desc2, btnText, color)      \
  GuiCreateGeneralHintBox(src, title, desc1, desc2, NULL, color, btnText, color)

#endif /* _GUI_HINTBOX_H */
