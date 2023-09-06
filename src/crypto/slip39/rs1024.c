//
//  rs1024.c
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#include "rs1024.h"

//////////////////////////////////////////////////
// rs1024 checksum functions

static const uint32_t generator[] = {
    0x00E0E040,
    0x01C1C080,
    0x03838100,
    0x07070200,
    0x0E0E0009,
    0x1C0C2412,
    0x38086C24,
    0x3090FC48,
    0x21B1F890,
    0x03F3F120,
};

static const uint8_t customization[] = {
    's', 'h', 'a', 'm', 'i', 'r',
};


// We need 30 bits of checksum to get 3 words worth (CHECKSUM_LENGTH_WORDS)
uint32_t rs1024_polymod(
    const uint16_t *values,    // values - 10 bit words
    uint32_t values_length // number of entries in the values array
)
{
    // there are a bunch of hard coded magic numbers in this
    // that would have to be changed if the value of CHECKSUM_LENGTH_WORDS
    // were to change.

    // unsigned ints are assumed to be 32 bits, which is enough to hold
    // CHECKSUM_LENGTH_WORDS * RADIX_BITS
    uint32_t chk = 1;

    // initialize with the customization string
    for (uint32_t i = 0; i < 6; ++i) {
        // 20 = (CHESUM_LENGTH_WORDS - 1) * RADIX_BITS
        uint32_t b = chk >> 20;
        // 0xFFFFF = (1 << ((CHECKSUM_LENGTH_WORDS-1)*RADIX_BITS)) - 1
        // 10 = RADIX_BITS
        chk = ((chk & 0xFFFFF) << 10) ^ customization[i];
        for (unsigned int j = 0; j < 10; ++j, b >>= 1) {
            chk ^= generator[j] * (b & 1);
        }
    }

    // continue with the values
    for (uint32_t i = 0; i < values_length; ++i) {
        uint32_t b = chk >> 20;
        chk = ((chk & 0xFFFFF) << 10) ^ values[i];
        for (unsigned int j = 0; j < 10; ++j, b >>= 1) {
            chk ^= generator[j] * (b & 1);
        }
    }

    return chk;
}


void rs1024_create_checksum(
    uint16_t *values, // data words (10 bit)
    uint32_t n          // length of the data array, including three checksum word
)
{
    // Set the last three words to zero
    values[n - 3] = 0;
    values[n - 2] = 0;
    values[n - 1] = 0;

    // compute the checkum
    uint32_t polymod = rs1024_polymod(values, n) ^ 1;

    // fix up the last three words to make the checksum come out to the right number
    values[n - 3] = (polymod >> 20) & 1023;
    values[n - 2] = (polymod >> 10) & 1023;
    values[n - 1] = (polymod) & 1023;
}


uint8_t rs1024_verify_checksum(
    const uint16_t *values,  // data words
    uint32_t n         // length of the data array
)
{
    return rs1024_polymod(values, n) == 1;
}
