/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : group.h
 * Description:
 * author     : stone wang
 * data       : 2023-02-22 11:01
**********************************************************************/
#ifndef _SLIP39_GROUP_H
#define _SLIP39_GROUP_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t count;
    uint8_t threshold;
    const char **passwords;
} GroupDesc_t;

// detailed information, https://github.com/satoshilabs/slips/blob/master/slip-0039.md#format-of-the-share-mnemonic
typedef struct {
    uint16_t identifier;
    uint8_t iteration;
    uint8_t groupIndex;
    uint8_t groupThreshold;
    uint8_t groupCount;
    uint8_t memberIndex;
    uint8_t memberThreshold;
    uint8_t valueLength;
    uint8_t value[32];
} Slip39Shared_t;

#endif /* _SLIP39_GROUP_H */

