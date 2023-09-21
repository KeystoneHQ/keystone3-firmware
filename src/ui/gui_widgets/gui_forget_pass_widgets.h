#ifndef _GUI_FORGET_PASS_WIDGETS_H
#define _GUI_FORGET_PASS_WIDGETS_H

void GuiForgetAnimContDel(int errCode);
void GuiForgetPassInit(void *param);
void GuiForgetPassRefresh(void);
int8_t GuiForgetPassNextTile(uint8_t tileIndex);
int8_t GuiForgetPassPrevTile(uint8_t tileIndex);
void GuiForgetPassPassCode(bool result, uint8_t tileIndex);
void GuiForgetPassSetPinPass(const char* buf);
void GuiForgetPassRepeatPinPass(const char* buf);
void GuiForgetPassDeInit(void);
void GuiForgetPassResetPass(bool en, int errCode);
void GuiForgetPassVerifyResult(bool en, int errCode);
void GuiForgetPassUpdateKeyboard(void);

#endif /* _GUI_FORGET_PASS_WIDGETS_H */

