#ifndef _GUI_WIPE_DEVICE_WIDGETS_H
#define _GUI_WIPE_DEVICE_WIDGETS_H

#include <stdbool.h>

void GuiWipeDeviceWidgetsInit();
void GuiWipeDeviceWidgetsDeInit();
void GuiWipeDeviceWidgetsRefresh();
void GuiWipeDeviceWidgetsRestart();
void GuiWipeDeviceSetForced(bool forced);

#endif
