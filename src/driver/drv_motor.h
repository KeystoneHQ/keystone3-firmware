#ifndef _DRV_MOTOR_H
#define _DRV_MOTOR_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

enum {
    MOTOR_SHAKE_SHORT_TIME = 200,
    MOTOR_SHAKE_LONG_TIME = 500,
    MOTOR_SHAKE_ULTRA_LONG_TIME = 1000,
};

#define MOTOR_LEVEL_HIGH                100
#define MOTOR_LEVEL_MIDDLE              80
#define MOTOR_LEVEL_LOW                 60

void MotorCtrl(uint32_t level, uint32_t tick);
void MotorInit(void);

#endif
