#ifndef _DRV_QRDECODE_H
#define _DRV_QRDECODE_H

#include "stdint.h"
#include "stdbool.h"
#include "decodelib.h"

#define QRDECODE_BUFF_SIZE          (410 * 1024)

int32_t QrDecodeInit(uint8_t *pool);
void QrDecodeDeinit(void);

uint32_t QrDecodeGetCamTick(void);
uint32_t QrDecodeGetViewTick(void);
uint32_t QrDecodeGetDecodeTick(void);

int32_t QrDecodeProcess(char *result, uint32_t maxLen, uint8_t progress);

#endif
