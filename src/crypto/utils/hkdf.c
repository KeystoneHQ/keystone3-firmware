#include "hkdf.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "librust_c.h"
#include "log_print.h"

void hkdf(uint8_t *password, const uint8_t *salt, uint8_t *output, uint32_t iterations)
{
    SimpleResponse_u8 *simpleResponse = pbkdf2_rust(password, (uint8_t *)salt, iterations);
    if (simpleResponse == NULL) {
        printf("get hdk return NULL\r\n");
        return;
    }
    uint8_t *kdfResult = simpleResponse->data;
    memcpy(output, kdfResult, 32);
    free_simple_response_u8(simpleResponse);
}

void hkdf64(uint8_t *password, const uint8_t *salt, uint8_t *output, uint32_t iterations)
{
    SimpleResponse_u8 *simpleResponse = pbkdf2_rust_64(password, (uint8_t *)salt, iterations);
    if (simpleResponse == NULL) {
        printf("get hdk return NULL\r\n");
        return;
    }
    uint8_t *kdfResult = simpleResponse->data;
    memcpy(output, kdfResult, 64);
    free_simple_response_u8(simpleResponse);
}