#include "stdio.h"
#include "string.h"
#include "crc.h"
#include "internal_protocol_parser.h"
#include "service_device_info.h"
#include "service_file_trans.h"
#include "user_memory.h"
#include "assert.h"

static struct ProtocolParser *global_parser = NULL;
static ProtocolSendCallbackFunc_t g_sendFunc = NULL;
static uint8_t g_protocolRcvBuffer[PROTOCOL_MAX_LENGTH];

static uint8_t *ExecuteService(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
static uint8_t *ProtocolParse(const uint8_t *inData, uint32_t inLen, uint32_t *outLen);

typedef struct {
    uint8_t serviceId;
    uint8_t commandIdNum;
    const ProtocolServiceCallbackFunc_t *func;
} ProtocolService_t;

static const ProtocolService_t g_ProtocolServiceList[] = {
    {SERVICE_ID_DEVICE_INFO, COMMAND_ID_DEVICE_INFO_MAX, g_deviceInfoServiceFunc},
    {SERVICE_ID_FILE_TRANS, COMMAND_ID_FILE_TRANS_MAX, g_fileTransInfoServiceFunc},
#ifdef WEB3_VERSION
    {SERVICE_ID_NFT_FILE_TRANS, COMMAND_ID_FILE_TRANS_MAX, g_nftFileTransInfoServiceFunc},
#endif
};

void InternalProtocol_Parse(const uint8_t *data, uint32_t len)
{
    if (g_sendFunc == NULL) {
        printf("err, g_sendFunc == NULL\n");
        return;
    }

    assert(len <= PROTOCOL_MAX_LENGTH);
    static uint32_t rcvLen = 0;
    uint32_t i, outLen;
    uint8_t *sendBuf;

    for (i = 0; i < len; i++) {
        if (global_parser->rcvCount >= PROTOCOL_MAX_LENGTH) {
            global_parser->rcvCount = 0;
            rcvLen = 0;
            continue;
        }

        if (global_parser->rcvCount == 0) {
            if (data[i] == PROTOCOL_HEADER) {
                g_protocolRcvBuffer[global_parser->rcvCount++] = data[i];
            }
        } else if (global_parser->rcvCount == 9) {
            g_protocolRcvBuffer[global_parser->rcvCount++] = data[i];
            rcvLen = ((uint32_t)g_protocolRcvBuffer[9] << 8) + g_protocolRcvBuffer[8];
            assert(rcvLen <= (PROTOCOL_MAX_LENGTH - 14));
        } else if (global_parser->rcvCount == rcvLen + 13) {
            g_protocolRcvBuffer[global_parser->rcvCount] = data[i];
            sendBuf = ProtocolParse(g_protocolRcvBuffer, rcvLen + 14, &outLen);
            if (sendBuf) {
                g_sendFunc(sendBuf, outLen);
                SRAM_FREE(sendBuf);
            }
            global_parser->rcvCount = 0;
            rcvLen = 0;
        } else {
            g_protocolRcvBuffer[global_parser->rcvCount++] = data[i];
        }
    }
}

static void RegisterSendFunc(ProtocolSendCallbackFunc_t sendFunc)
{
    if (g_sendFunc == NULL) {
        g_sendFunc = sendFunc;
    }
}

struct ProtocolParser *NewInternalProtocolParser()
{
    if (!global_parser) {
        global_parser = (struct ProtocolParser *)SRAM_MALLOC(sizeof(struct ProtocolParser));
        global_parser->name = INTERNAL_PROTOCOL_PARSER_NAME;
        global_parser->parse = InternalProtocol_Parse;
        global_parser->registerSendFunc = RegisterSendFunc;
        global_parser->rcvCount = 0;
    }
    return global_parser;
}

static uint8_t *ProtocolParse(const uint8_t *inData, uint32_t inLen, uint32_t *outLen)
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
        memcpy_s(&receivedCrc, sizeof(receivedCrc), inData + inLen - 4, 4);
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
    if (head == NULL || tlvData == NULL || outLen == NULL) {
        printf("err, invalid input parameters\n");
        return NULL;
    }

    uint32_t i;
    for (i = 0; i < sizeof(g_ProtocolServiceList) / sizeof(g_ProtocolServiceList[0]); i++) {
        if (g_ProtocolServiceList[i].serviceId == head->serviceId) {
            if (head->commandId >= g_ProtocolServiceList[i].commandIdNum) {
                printf("err, head->commandId=%d, commandIdNum=%d\n", head->commandId, g_ProtocolServiceList[i].commandIdNum);
                return NULL;
            }

            if (g_ProtocolServiceList[i].func[head->commandId] == NULL) {
                printf("err, no func for commandId=%d\n", head->commandId);
                return NULL;
            }

            return g_ProtocolServiceList[i].func[head->commandId](head, tlvData, outLen);
        }
    }

    printf("err, serviceId=%d not found\n", head->serviceId);
    return NULL;
}