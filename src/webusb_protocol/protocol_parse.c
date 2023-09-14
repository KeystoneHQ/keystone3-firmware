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


#define PROTOCOL_PARSE_OVERTIME             500

uint8_t g_protocolRcvBuffer[PROTOCOL_MAX_LENGTH];

struct ProtocolParser* currentParser = NULL;

void ProtocolReceivedData(const uint8_t *data, uint32_t len, ProtocolSendCallbackFunc_t sendFunc)
{
    uint32_t tick, i, outLen;
    uint8_t *sendBuf;
    static struct ApduProtocolParser *currentParser = NULL;

    tick = osKernelGetTickCount();

    for (i = 0; i < len; i++) {
        if (currentParser == NULL) {
            if (data[i] == INTERNAL_PROTOCOL_HEADER) {
                currentParser = NewInternalProtocolParser();
                currentParser->base.parse(data + i, 1);
            }
            else if (data[i] == APDU_PROTOCOL_HEADER) {
                currentParser = NewApduProtocolParser();
                currentParser->base.parse(data + i, 1);
            }
            else {
                continue;
            }
            if (currentParser) {
                printf("current parser: %s\n", currentParser->base.name);
            }
        } else {
            currentParser->base.parse(data + i, 1);
        }

        if (currentParser->base.isFullFrameReceived()) {
            sendBuf = currentParser->base.getProcessedData(&outLen);
            if (sendBuf) {
                PrintArray("sendBuffer: ", sendBuf, outLen);
                sendFunc(sendBuf, outLen);
                SRAM_FREE(sendBuf);
            }
            currentParser->base.reset();
            SRAM_FREE(currentParser);
            currentParser = NULL;
        }
    }
}
