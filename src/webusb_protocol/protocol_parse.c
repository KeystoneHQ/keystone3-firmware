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
#include "service_device_info.h"
#include "service_file_trans.h"
#include "service_sign_tx.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "log_print.h"


#define PROTOCOL_PARSE_OVERTIME             500

uint8_t g_protocolRcvBuffer[PROTOCOL_MAX_LENGTH];

struct ProtocolParser* currentParser = NULL;

static uint8_t *ExecuteService(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);


typedef struct {
    uint8_t serviceId;
    uint8_t commandIdNum;
    //uint8_t funcNum;
    const ProtocolServiceCallbackFunc_t *func;
} ProtocolService_t;

static const ProtocolService_t g_ProtocolServiceList[] = {
    {SERVICE_ID_DEVICE_INFO,        COMMAND_ID_DEVICE_INFO_MAX,     g_deviceInfoServiceFunc},
    {SERVICE_ID_FILE_TRANS,         COMMAND_ID_FILE_TRANS_MAX,      g_fileTransInfoServiceFunc},
    {SERVICE_ID_SIGN_TX,            COMMAND_ID_SIGN_TX_MAX,         g_signTxServiceFunc},
};

void ProtocolReceivedData(const uint8_t *data, uint32_t len, ProtocolSendCallbackFunc_t sendFunc)
{
    uint32_t tick, i, outLen;
    uint8_t *sendBuf;

    tick = osKernelGetTickCount();

    for (i = 0; i < len; i++) {
        if (currentParser == NULL) {
            if (data[i] == INTERNAL_PROTOCOL_HEADER) {
                currentParser = (struct ProtocolParser*)NewInternalProtocolParser();
            } else if (data[i] == APDU_PROTOCOL_HEADER) {
                currentParser = (struct ProtocolParser*)NewApduProtocolParser();
            } else {
                continue;
            }
        }

        currentParser->parse(data + i, 1);

        if (currentParser->isFullFrameReceived()) {
            sendBuf = currentParser->getProcessedData(&outLen);
            if (sendBuf) {
                PrintArray("sendBuf", sendBuf, outLen);
                sendFunc(sendBuf, outLen);
                SRAM_FREE(sendBuf);
            }
            currentParser->reset();
            free(currentParser);
            currentParser = NULL;
        }
    }
}

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
        } else {
            currentParser->base.parse(data + i, 1);
        }

        if (currentParser->base.isFullFrameReceived()) {
            sendBuf = currentParser->base.getProcessedData(&outLen);
            if (sendBuf) {
                PrintArray("sendBuf", sendBuf, outLen);
                sendFunc(sendBuf, outLen);
                SRAM_FREE(sendBuf);
            }
            currentParser->base.reset();
            SRAM_FREE(currentParser);
            currentParser = NULL;
        }
    }
}

// void ProtocolReceivedData(const uint8_t *data, uint32_t len, ProtocolSendCallbackFunc_t sendFunc)
// {
//     static uint32_t lastTick = 0, rcvCount = 0, rcvLen = 0;
//     uint32_t tick, i, outLen;
//     uint8_t *sendBuf;

//     tick = osKernelGetTickCount();
//     if (rcvCount != 0) {
//         if (tick - lastTick > PROTOCOL_PARSE_OVERTIME) {
//             printf("protocol over time, rcvCount=%d\n", rcvCount);
//             PrintArray("g_protocolRcvBuffer", g_protocolRcvBuffer, rcvCount);
//             rcvCount = 0;
//         }
//     }
//     lastTick = tick;

//     for (i = 0; i < len; i++) {
//         if (rcvCount == 0) {
//             if (data[i] == PROTOCOL_HEADER) {
//                 g_protocolRcvBuffer[rcvCount] = data[i];
//                 rcvCount++;
//                 printf("head\n");
//             }
//         } else if (rcvCount == 9) {
//             //length
//             g_protocolRcvBuffer[rcvCount] = data[i];
//             rcvCount++;
//             rcvLen = ((uint32_t)g_protocolRcvBuffer[9] << 8) + g_protocolRcvBuffer[8];
//             printf("rcvLen=%d\n", rcvLen);
//         } else if (rcvCount == rcvLen + 13) {
//             g_protocolRcvBuffer[rcvCount] = data[i];
//             rcvCount = 0;
//             printf("full frame,len=%d\n", rcvLen + 14);
//             sendBuf = ProtocolParse(g_protocolRcvBuffer, rcvLen + 14, &outLen);
//             if (sendBuf) {
//                 PrintArray("sendBuf", sendBuf, outLen);
//                 sendFunc(sendBuf, outLen);
//                 SRAM_FREE(sendBuf);
//             }
//         } else {
//             g_protocolRcvBuffer[rcvCount] = data[i];
//             rcvCount++;
//         }
//     }
// }


uint8_t *ProtocolParse(const uint8_t *inData, uint32_t inLen, uint32_t *outLen)
{
    uint8_t *outData = NULL;
    FrameHead_t *pHead;
    uint32_t receivedCrc, calculatedCrc;

    do {
        *outLen = 0;
        if (inData == NULL || inLen < sizeof(FrameHead_t) + 4) {
            printf("invalid inData\n");
            break;
        }
        pHead = (FrameHead_t *)inData;
        if (pHead->head != PROTOCOL_HEADER) {
            printf("invalid head\n");
            break;
        }
        if (pHead->protocolVersion != 0) {
            printf("invalid version\n");
            break;
        }
        if (pHead->protocolVersion != 0) {
            printf("invalid version\n");
            break;
        }
        if (pHead->length + sizeof(FrameHead_t) + 4 != inLen) {
            printf("inLen err\n");
            break;
        }
        PrintFrameHead(pHead);
        memcpy(&receivedCrc, inData + inLen - 4, 4);
        calculatedCrc = crc32_ieee(0, inData, inLen - 4);
        if (receivedCrc != calculatedCrc) {
            printf("crc err,receivedCrc=0x%08X,calculatedCrc=0x%08X\n", receivedCrc, calculatedCrc);
            break;
        }
        outData = ExecuteService(pHead, inData + sizeof(FrameHead_t), outLen);
    } while (0);

    return outData;
}


static uint8_t *ExecuteService(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen)
{
    uint32_t i;

    for (i = 0; i < sizeof(g_ProtocolServiceList) / sizeof(g_ProtocolServiceList[0]); i++) {
        if (g_ProtocolServiceList[i].serviceId == head->serviceId) {
            if (g_ProtocolServiceList[i].func[head->commandId] == NULL) {
                printf("err, no func\n");
                return NULL;
            }
            if (head->commandId >= g_ProtocolServiceList[i].commandIdNum) {
                printf("err,head->commandId=%d,commandIdNum=%d\n", head->commandId, g_ProtocolServiceList[i].commandIdNum);
                return NULL;
            }
            return g_ProtocolServiceList[i].func[head->commandId](head, tlvData, outLen);
        }
    }
    return NULL;
}

