
#ifndef _FINGERPRINT_CRC_H
#define _FINGERPRINT_CRC_H

#include "stdint.h"
#include "stdbool.h"
#include "crc.h"

uint32_t crc32_update_fast(const uint8_t *message, uint16_t n_bytes);

#endif