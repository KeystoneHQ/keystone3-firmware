#ifndef _QRDECODE_TASK_H
#define _QRDECODE_TASK_H

#include "stdint.h"
#include "stdbool.h"
#include "librust_c.h"

#define QR_DECODE_STRING_LEN   1536

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
void HandleDefaultViewType(URParseResult *urResult, URParseMultiResult *urMultiResult, UrViewType_t urViewType, bool is_multi);
#ifdef COMPILE_SIMULATOR
void handleURResult(URParseResult *urResult, URParseMultiResult *urMultiResult, UrViewType_t urViewType, bool is_multi);
#endif

#endif

