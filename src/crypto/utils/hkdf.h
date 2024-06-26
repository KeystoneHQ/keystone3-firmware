#include "stdint.h"

void hkdf(uint8_t *password, const uint8_t *salt, uint8_t *output, uint32_t iterations);
void hkdf64(uint8_t *password, const uint8_t *salt, uint8_t *output, uint32_t iterations);