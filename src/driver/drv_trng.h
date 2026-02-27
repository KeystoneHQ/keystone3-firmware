#ifndef _DRV_TRNG_H
#define _DRV_TRNG_H

#include "stdint.h"
#include "stdbool.h"

void TrngInit(void);
void TrngGet(void *buf, uint32_t len);

#endif
