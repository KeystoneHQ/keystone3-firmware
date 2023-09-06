/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: QR decode task
 * Author: leon sun
 * Create: 2022-12-5
 ************************************************************************************************/

#ifndef _QRDECODE_TASK_H
#define _QRDECODE_TASK_H

#include "stdint.h"
#include "stdbool.h"

#define QR_DECODE_STRING_LEN   1024

typedef enum {
    QR_DECODE_STATE_OFF,
    QR_DECODE_STATE_ON,
} QrDecodeStateType;

typedef struct {
    uint8_t viewType;
    uint8_t urType;
} UrViewType_t;

void CreateQrDecodeTask(void);
void StartQrDecode(void);
void StopQrDecode(void);
void QrDecodeTouchQuit(void);
void ProcessQr(uint32_t count);

#endif

