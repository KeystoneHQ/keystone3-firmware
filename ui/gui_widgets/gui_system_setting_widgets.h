#ifndef _GUI_SYSTEM_SETTING_WIDGETS_H
#define _GUI_SYSTEM_SETTING_WIDGETS_H

void GuiSystemSettingAreaInit();
void GuiSystemSettingAreaDeInit();
void GuiSystemSettingAreaRefresh();

void GuiSystemSettingVerifyPasswordResult(bool result);
void GuiSystemSettingVerifyPasswordErrorCount(void *param);
#endif