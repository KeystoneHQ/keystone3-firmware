#include <stdio.h>
#include "motor_manager.h"
#include "drv_motor.h"
#include "device_setting.h"

static void MicroVibration(void);
static void SlightVibration(void);
static void ShortVibration(void);
static void LongVibration(void);
static void SuperLongVibration(void);


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

static void MicroVibration(void)
{
    //MotorCtrl(30, 200);
}
static void SlightVibration(void)
{
    MotorCtrl(MOTOR_LEVEL_LOW, MOTOR_SHAKE_SHORT_TIME);
}
static void ShortVibration(void)
{
    MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_SHORT_TIME);
}
static void LongVibration(void)
{
    MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_LONG_TIME);

}
static void SuperLongVibration(void)
{
    MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_ULTRA_LONG_TIME);
}
