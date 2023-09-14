#include "stdio.h"
#include "internal_protocol_parser.h"
#include "service_device_info.h"
#include "service_file_trans.h"
#include "service_sign_tx.h"
#include "user_memory.h"

static struct InternalProtocolParser* global_parser = NULL;

static uint8_t *ExecuteService(FrameHead_t *head, const uint8_t *tlvData, uint32_t *outLen);
static uint8_t *ProtocolParse(const uint8_t *inData, uint32_t inLen, uint32_t *outLen);

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

void InternalProtocol_Parse(const uint8_t *data, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++) {
        if (global_parser->rcvCount == 0) {
            if (data[i] == INTERNAL_PROTOCOL_HEADER) {
                global_parser->g_protocolRcvBuffer[global_parser->rcvCount] = data[i];
                global_parser->rcvCount++;
            }
        } else if (global_parser->rcvCount == 9) {
            global_parser->g_protocolRcvBuffer[global_parser->rcvCount] = data[i];
            global_parser->rcvCount++;
            global_parser->rcvLen = ((uint32_t)global_parser->g_protocolRcvBuffer[9] << 8) + global_parser->g_protocolRcvBuffer[8];
        } else if (global_parser->rcvCount == global_parser->rcvLen + 13) {
            global_parser->g_protocolRcvBuffer[global_parser->rcvCount] = data[i];
            global_parser->rcvCount = 0;
            global_parser->fullFrameReceived = true;
        } else {
            global_parser->g_protocolRcvBuffer[global_parser->rcvCount] = data[i];
            global_parser->rcvCount++;
        }
    }
}

void InternalProtocol_Reset()
{
    global_parser->rcvCount = 0;
    global_parser->rcvLen = 0;
    global_parser->fullFrameReceived = false;
}

bool InternalProtocol_IsFullFrameReceived()
{
    return global_parser->fullFrameReceived;
}

uint8_t* InternalProtocol_GetProcessedData(uint32_t* outLen)
{
    *outLen = global_parser->rcvLen;
    uint8_t *processedData = ProtocolParse(global_parser->g_protocolRcvBuffer, global_parser->rcvLen, &outLen);
    PrintArray("InternalProtocol_GetProcessedData: ", processedData, *outLen);
    return processedData;
}

struct InternalProtocolParser* NewInternalProtocolParser()
{
    if (!global_parser) {
        global_parser = (struct InternalProtocolParser*)SRAM_MALLOC(sizeof(struct InternalProtocolParser));
        global_parser->base.parse = InternalProtocol_Parse;
        global_parser->base.reset = InternalProtocol_Reset;
        global_parser->base.isFullFrameReceived = InternalProtocol_IsFullFrameReceived;
        global_parser->base.getProcessedData = InternalProtocol_GetProcessedData;
        global_parser->base.name = "InternalProtocolParser";
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