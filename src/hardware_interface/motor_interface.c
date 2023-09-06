#include "drv_motor.h"

void MicroVibration(void)
{
    //MotorCtrl(30, 200);
}

void SlightVibration(void)
{
    MotorCtrl(MOTOR_LEVEL_LOW, MOTOR_SHAKE_SHORT_TIME);
}

void ShortVibration(void)
{
    MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_SHORT_TIME);
}

void LongVibration(void)
{
    MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_LONG_TIME);
}

void SuperLongVibration(void)
{
    MotorCtrl(MOTOR_LEVEL_MIDDLE, MOTOR_SHAKE_ULTRA_LONG_TIME);
}