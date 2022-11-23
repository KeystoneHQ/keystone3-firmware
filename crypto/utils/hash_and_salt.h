/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: HASH and salt.
 * Author: leon sun
 * Create: 2023-3-7
 ************************************************************************************************/


#ifndef _HASH_AND_SALT_H
#define _HASH_AND_SALT_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"


void GenerateSaltSource(void);
void HashWithSalt(uint8_t *outData, const uint8_t *inData, uint32_t inLen, const char *saltString);



#endif
