#include <stdio.h>
#include "motor_manager.h"
#include "drv_motor.h"
#include "device_setting.h"
#include "motor_interface.h"

void Vibrate(VIBRATION_STRENGTH strength)
{
    if (!GetVibration()) {
        return;
    }

    switch (strength) {
    case MICRO:
        MicroVibration();
        break;
    case SLIGHT:
        SlightVibration();
        break;
    case SHORT:
        ShortVibration();
        break;
    case LONG:
        LongVibration();
        break;
    case SUPER_LONG:
        SuperLongVibration();
        break;
    default:
        printf("not support current strength %d\n", strength);
        break;
    }
}

void UnlimitedVibrate(VIBRATION_STRENGTH strength)
{
    switch (strength) {
    case MICRO:
        MicroVibration();
        break;
    case SLIGHT:
        SlightVibration();
        break;
    case SHORT:
        ShortVibration();
        break;
    case LONG:
        LongVibration();
        break;
    case SUPER_LONG:
        SuperLongVibration();
        break;
    default:
        printf("not support current strength %d\n", strength);
        break;
    }
}