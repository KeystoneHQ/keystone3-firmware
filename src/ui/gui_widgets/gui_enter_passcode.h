#ifndef _GUI_ENTER_PASSCODE_H
#define _GUI_ENTER_PASSCODE_H

#include "gui.h"
#include "gui_keyboard.h"
#define CREATE_PIN_NUM                                  6
#define CREATE_PASSWORD_NUM                             22

typedef enum {
    ENTER_PASSCODE_SET_PIN,
    ENTER_PASSCODE_SET_PASSWORD,
    ENTER_PASSCODE_REPEAT_PIN,
    ENTER_PASSCODE_REPEAT_PASSWORD,
    ENTER_PASSCODE_VERIFY_PIN,
    ENTER_PASSCODE_VERIFY_PASSWORD,
    ENTER_PASSCODE_LOCK_VERIFY_PIN,
    ENTER_PASSCODE_LOCK_VERIFY_PASSWORD,

    ENTER_PASSCODE_BUTT,
} ENTER_PASSCODE_ENUM;

typedef struct GuiEnterPasscodeItem {
    ENTER_PASSCODE_ENUM mode;
    lv_obj_t        *pinCont;
    lv_obj_t        *passWdCont;
    lv_obj_t        *numLed[CREATE_PIN_NUM];
    lv_obj_t        *errLabel;
    lv_obj_t        *repeatLabel;
    lv_obj_t        *fpErrLabel;
    lv_event_cb_t   setPassCb;
    uint8_t         currentNum;
    lv_obj_t        *titleLabel;
    lv_obj_t        *descLabel;
    KeyBoard_t      *kb;
    lv_obj_t        *btnm;
    lv_obj_t        *eyeImg;
    lv_obj_t        *scoreBar;
    lv_style_t      *scoreBarStyle;
    lv_obj_t        *scoreLevel;
    lv_obj_t        *lenOverLabel;
} GuiEnterPasscodeItem_t;

void *GuiCreateEnterPasscode(lv_obj_t *parent, lv_event_cb_t Cb, void *param, ENTER_PASSCODE_ENUM method);
void GuiDelEnterPasscode(void *obj, void *param);
void GuiEnterPassCodeStatus(GuiEnterPasscodeItem_t *item, bool en);
void GuiEnterPassLabelRefresh(void);
void SwitchPasswordModeHandler(lv_event_t *e);
void GuiUpdateEnterPasscodeParam(GuiEnterPasscodeItem_t *item, void *param);
uint8_t GetPassWordStrength(const char *password, uint8_t len);
void GuiFingerPrintStatus(GuiEnterPasscodeItem_t *item, bool en, uint8_t errCnt);
void PassWordPinSwitch(GuiEnterPasscodeItem_t *item);
void GuiEnterPassLabelRefresh(void);

#endif /* _GUI_ENTER_PASSCODE_H */

