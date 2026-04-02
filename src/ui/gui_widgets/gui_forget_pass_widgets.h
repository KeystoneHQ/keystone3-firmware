#ifndef _GUI_FORGET_PASS_WIDGETS_H
#define _GUI_FORGET_PASS_WIDGETS_H

void GuiForgetAnimContDel(bool isReset);
void GuiForgetPassInit(void *param);
void GuiForgetPassRefresh(void);
int8_t GuiForgetPassNextTile(uint8_t tileIndex);
int8_t GuiForgetPassPrevTile(uint8_t tileIndex);
void GuiForgetPassPassCode(bool result, uint8_t tileIndex);
void GuiForgetPassSetPinPass(const char* buf);
void GuiForgetPassRepeatPinPass(const char* buf);
void GuiForgetPassDeInit(void);
void GuiForgetPassResetPass(bool en, int errCode);
void GuiForgetPassTonBip39Success(void);
void GuiForgetPassVerifyResult(bool en, int errCode);
void GuiForgetPassUpdateKeyboard(void);
bool GuiIsForgetPass(void);

#ifdef WEB3_VERSION
void GuiForgetPassTonSuccess(void);
#endif

#endif /* _GUI_FORGET_PASS_WIDGETS_H */

