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
    MotorCtrl(40, 200);
}
static void ShortVibration(void)
{
    MotorCtrl(80, 200);
}
static void LongVibration(void)
{
    MotorCtrl(80, 500);

}
static void SuperLongVibration(void)
{
    MotorCtrl(80, 1000);
}
