#ifndef _GUI_LOCK_DEVICE_WIDGETS_H
#define _GUI_LOCK_DEVICE_WIDGETS_H

void GuiLockDeviceInit(void *param);
void GuiLockDeviceRefresh(void);
void GuiLockDeviceDeInit(void);
void GuiDelALLWalletSetup(void);
void ResetSuccess(void);
void GuiClearAllTop(void);
void GuiLockScreenWipeDevice(void);

uint16_t GuiGetLockTimeByLeftErrorCount(uint16_t leftErrorCount);
#endif /* _GUI_LOCK_DEVICE_WIDGETS_H */
