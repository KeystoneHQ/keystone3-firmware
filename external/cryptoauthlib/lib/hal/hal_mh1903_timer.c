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
