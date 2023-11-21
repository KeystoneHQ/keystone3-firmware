/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: HASH and salt.
 * Author: leon sun
 * Create: 2023-3-7
 ************************************************************************************************/

#include "hkdf.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "librust_c.h"
#include "log_print.h"


void hkdf(uint8_t *password, const uint8_t *salt, uint8_t *output, uint32_t iterations) {
    SimpleResponse_u8 *simpleResponse = pbkdf2_rust(password, salt, iterations);
    if (simpleResponse == NULL) {
        printf("get hdk return NULL\r\n");
        return;
    }
    uint8_t *kdfResult = simpleResponse->data;
    memcpy(output, kdfResult, 32);
    free_simple_response_u8(simpleResponse);
}
