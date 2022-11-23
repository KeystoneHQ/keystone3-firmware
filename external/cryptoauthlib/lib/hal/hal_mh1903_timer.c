/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: mh1903 timer hal for atca.
 * Author: leon sun
 * Create: 2023-1-31
 ************************************************************************************************/

#include "atca_hal.h"
#include "user_delay.h"

void hal_delay_us(uint32_t delay)
{
    UserDelayUs(delay);
}

void hal_delay_ms(uint32_t msec)
{
    UserDelay(msec);
}
