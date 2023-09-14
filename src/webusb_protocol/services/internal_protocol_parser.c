#include "stdio.h"
#include "internal_protocol_parser.h"

static struct InternalProtocolParser* global_parser = NULL;

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
    return global_parser->g_protocolRcvBuffer;
}

struct InternalProtocolParser* NewInternalProtocolParser()
{
    if (!global_parser) {
        global_parser = (struct InternalProtocolParser*)SRAM_MALLOC(sizeof(struct InternalProtocolParser));
        global_parser->base.parse = InternalProtocol_Parse;
        global_parser->base.reset = InternalProtocol_Reset;
        global_parser->base.isFullFrameReceived = InternalProtocol_IsFullFrameReceived;
        global_parser->base.getProcessedData = InternalProtocol_GetProcessedData;
    }
    return global_parser;
}