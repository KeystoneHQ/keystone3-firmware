#ifndef _MOTOR_MANAGER_H_
#define _MOTOR_MANAGER_H_

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

typedef enum {
    MICRO = 0,
    SLIGHT,
    SHORT,
    LONG,
    SUPER_LONG,
} VIBRATION_STRENGTH;

void Vibrate(VIBRATION_STRENGTH strength);
void UnlimitedVibrate(VIBRATION_STRENGTH strength);

#endif