//
//  rs1024.h
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#ifndef RS1024_H
#define RS1024_H

#include <stdint.h>

uint32_t rs1024_polymod(
    const uint16_t *values,    // values - 10 bit words
    uint32_t values_length // number of entries in the values array
);

void rs1024_create_checksum(
    uint16_t *values, // data words (10 bit)
    uint32_t n          // length of the data array, including three checksum word
);

uint8_t rs1024_verify_checksum(
    const uint16_t *values,  // data words
    uint32_t n         // length of the data array
);

#endif /* RS1024_H */
