/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: TRNG驱动.
 * Author: leon sun
 * Create: 2022-11-14
 ************************************************************************************************/

#ifndef _DRV_TRNG_H
#define _DRV_TRNG_H

#include "stdint.h"
#include "stdbool.h"

void TrngInit(void);
void TrngGet(void *buf, uint32_t len);

#endif
