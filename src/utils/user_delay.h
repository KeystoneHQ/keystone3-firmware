/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: User delay.
 * Author: leon sun
 * Create: 2023-1-18
 ************************************************************************************************/


#ifndef _USER_DELAY_H
#define _USER_DELAY_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

void UserDelay(uint32_t ms);
void UserDelayUs(uint32_t us);
void PretendDoingSomething(uint32_t i);

#endif
