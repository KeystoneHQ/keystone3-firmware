/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Protocol parse.
 * Author: leon sun
 * Create: 2023-6-29
 ************************************************************************************************/

#include "internal_protocol_parser.h"
#include "apdu_protocol_parser.h"
#include "stdio.h"
#include "string.h"
#include "user_utils.h"
#include "protocol_codec.h"
#include "crc.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "log_print.h"

#define PROTOCOL_PARSE_OVERTIME 500

uint8_t g_protocolRcvBuffer[PROTOCOL_MAX_LENGTH];

struct ProtocolParser *currentParser = NULL;

void ProtocolReceivedData(const uint8_t *data, uint32_t len, ProtocolSendCallbackFunc_t sendFunc)
{
    static uint32_t lastTick = 0;
    uint32_t tick;
    static struct ProtocolParser *currentParser = NULL;

    tick = osKernelGetTickCount();

    if (data[0] == APDU_PROTOCOL_HEADER)
    {
        currentParser = NewApduProtocolParser();
    }
    else
    {
        currentParser = NewInternalProtocolParser();
    }

    printf("ProtocolReceivedData start\n");

    if (currentParser->rcvCount != 0)
    {
        if (tick - lastTick > PROTOCOL_PARSE_OVERTIME)
        {
            currentParser->rcvCount = 0;
        }
    }
    lastTick = tick;
    printf("registerSendFunc start\n");
    currentParser->registerSendFunc(&sendFunc);
    printf("registerSendFunc end\n");

    currentParser->parse(data, len);
}
