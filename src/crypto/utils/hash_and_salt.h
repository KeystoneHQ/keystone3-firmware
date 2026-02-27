#ifndef _HASH_AND_SALT_H
#define _HASH_AND_SALT_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

void GenerateSaltSource(void);
void HashWithSalt(uint8_t *outData, const uint8_t *inData, uint32_t inLen, const char *saltString);
void HashWithSalt512(uint8_t *outData, const uint8_t *inData, uint32_t inLen, const char *saltString);

#endif
