#ifndef _GUI_ABOUT_INFO_WIDGETS_H
#define _GUI_ABOUT_INFO_WIDGETS_H

void GuiAboutInfoWidgetsInit();
void GuiAboutInfoWidgetsDeInit();
void GuiAboutInfoWidgetsRefresh();
void GuiAboutInfoWidgetsRestart();
void GuiAboutWidgetsLogExport(bool en, int32_t errCode);
void GuiUpdateCheckSumPercent(uint8_t percent);
void GuiStopFirmwareCheckSumHandler(lv_event_t *e);

#endif