#ifndef _GUI_SYSTEM_SETTING_WIDGETS_H
#define _GUI_SYSTEM_SETTING_WIDGETS_H

void GuiSystemSettingAreaInit();
void GuiSystemSettingAreaDeInit();
void GuiSystemSettingAreaRefresh();

void GuiSystemSettingVerifyPasswordSuccess(void);
void GuiSystemSettingVerifyPasswordErrorCount(void *param);
#endif