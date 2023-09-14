#ifndef APDU_PROTOCOL_PARSER_H
#define APDU_PROTOCOL_PARSER_H

#include "protocol_parse.h"

#define APDU_PROTOCOL_HEADER             0x00

struct ApduProtocolParser
{
    struct ProtocolParser base;
    uint32_t rcvCount;
    uint32_t rcvLen;
    uint8_t g_protocolRcvBuffer[1024];
};

struct ApduProtocolParser* NewApduProtocolParser();

#endif