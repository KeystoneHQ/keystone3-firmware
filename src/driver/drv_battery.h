#ifndef _DRV_BATTARY_H
#define _DRV_BATTARY_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

/// @brief Battery init, including ADC init.
/// @param
void BatteryInit(void);
void BatteryOpen(void);



/// @brief Get battery voltage.
/// @param
/// @return Battery voltage, in millivolts.
uint32_t GetBatteryMilliVolt(void);

bool BatteryIntervalHandler(void);
uint8_t GetBatterPercent(void);
uint32_t GetBatteryInterval(void);
uint8_t GetBatteryPercentByMilliVolt(uint32_t milliVolt, bool discharge);

void BatteryTest(int argc, char *argv[]);

#define LOW_BATTERY_LIMIT               20
#define CHECK_BATTERY_LOW_POWER()       ((GetBatterPercent() <= LOW_BATTERY_LIMIT) ? ERR_KEYSTORE_SAVE_LOW_POWER : SUCCESS_CODE)

#endif
