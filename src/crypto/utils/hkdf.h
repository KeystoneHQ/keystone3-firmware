/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: HASH and salt.
 * Author: leon sun
 * Create: 2023-3-7
 ************************************************************************************************/

#include "stdint.h"

void hkdf(uint8_t *password, const uint8_t *salt, uint8_t *output, uint32_t iterations);