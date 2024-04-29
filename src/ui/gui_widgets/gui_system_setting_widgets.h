#ifndef _GUI_SYSTEM_SETTING_WIDGETS_H
#define _GUI_SYSTEM_SETTING_WIDGETS_H

void GuiSystemSettingAreaInit(void);
void GuiSystemSettingAreaDeInit(void);
void GuiSystemSettingAreaRefresh(void);
void GuiSystemSettingAreaRestart(void);

void GuiSystemSettingVerifyPasswordSuccess(void);
void GuiSystemSettingVerifyPasswordErrorCount(void *param);
#endif