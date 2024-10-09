#ifndef _GUI_SYSTEM_SETTING_WIDGETS_H
#define _GUI_SYSTEM_SETTING_WIDGETS_H

void GuiSystemSettingAreaInit(void);
void GuiSystemSettingAreaDeInit(void);
void GuiSystemSettingAreaRefresh(void);
void GuiSystemSettingLanguage(void *param);

void GuiSystemSettingVerifyPasswordSuccess(void);
void GuiSystemSettingVerifyPasswordErrorCount(void *param);
#endif