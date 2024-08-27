#ifndef _DEVICE_SETTINGS_H
#define _DEVICE_SETTINGS_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

#define LOW_BATTERY_PERCENT                     20
#define DEVICE_WIPE_FLAG_MAGIC_NUM              0x1234ABCD
#define MIN_BRIGHT                              4
#define MAX_BRIGHT                              50

void DeviceSettingsInit(void);
void SaveDeviceSettings(void);

uint32_t GetSetupStep(void);
void SetSetupStep(uint32_t setupStep);

uint32_t GetBright(void);
void SetBright(uint32_t bight);

uint32_t GetAutoLockScreen(void);
void SetAutoLockScreen(uint32_t autoLockScreen);

uint32_t GetAutoPowerOff(void);
void SetAutoPowerOff(uint32_t autoPowerOff);

uint32_t GetVibration(void);
void SetVibration(uint32_t vibration);

uint32_t GetDarkMode(void);
void SetDarkMode(uint32_t darkMode);

uint32_t GetUSBSwitch(void);
void SetUSBSwitch(uint32_t usbSwitch);

uint32_t GetLanguage(void);
void SetLanguage(uint32_t language);

bool GetNftScreenSaver(void);
void SetNftScreenSaver(bool enable);
bool IsNftScreenValid(void);
void SetNftBinValid(bool en);

bool IsUpdateSuccess(void);
void WipeDevice(void);
void DeviceSettingsTest(int argc, char *argv[]);

#endif
