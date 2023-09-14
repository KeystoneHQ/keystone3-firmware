#ifndef INTERNAL_PROTOCOL_PARSER_H
#define INTERNAL_PROTOCOL_PARSER_H

#include "protocol_parse.h"

#define INTERNAL_PROTOCOL_HEADER             0x6B

struct InternalProtocolParser
{
    struct ProtocolParser base;
    uint32_t lastTick;
    uint32_t rcvCount;
    uint32_t rcvLen;
    bool fullFrameReceived;
    uint8_t g_protocolRcvBuffer[1024];
};

struct InternalProtocolParser* NewInternalProtocolParser();

#endif