//
//  rs1024.h
//
//  Copyright © 2020 by Blockchain Commons, LLC
//  Licensed under the "BSD-2-Clause Plus Patent License"
//

#ifndef RS1024_H
#define RS1024_H

#include <stdbool.h>
#include <stdint.h>

void rs1024_create_checksum(
    uint16_t *values, // data words (10 bit)
    uint32_t values_length, // length of the data array, including three checksum word
    bool extendable_backup_flag
);

uint8_t rs1024_verify_checksum(
    const uint16_t *values,  // data words
    uint32_t values_length, // length of the data array
    bool extendable_backup_flag
);

#endif /* RS1024_H */
