/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Protocol parse.
 * Author: leon sun
 * Create: 2023-6-29
 ************************************************************************************************/


#ifndef _PROTOCOL_PARSE_H
#define _PROTOCOL_PARSE_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "protocol_codec.h"

enum {
    SERVICE_ID_DEVICE_INFO              = 1,
    SERVICE_ID_FILE_TRANS,
    SERVICE_ID_SIGN_TX,
    SERVICE_ID_MAX
};

#define TYPE_GENERAL_RESULT_ACK             0xFF


typedef uint8_t *(*ProtocolServiceCallbackFunc_t)(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
typedef void (*ProtocolSendCallbackFunc_t)(const uint8_t *data, uint32_t len);

void ProtocolReceivedData(const uint8_t *data, uint32_t len, ProtocolSendCallbackFunc_t sendFunc);

struct ProtocolParser
{
    char *name;
    uint32_t rcvCount;
    void (*parse)(const uint8_t *data, uint32_t len);
    void (*registerSendFunc)(ProtocolSendCallbackFunc_t sendFunc);
};

#endif
