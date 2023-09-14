#include "apdu_protocol_parser.h"

void ApduProtocol_Parse(const uint8_t *data, uint32_t len)
{
    // ... put your original parsing logic here ...
}

void ApduProtocol_Reset()
{
    // ... reset all the fields to their initial state ...
}

bool ApduProtocol_IsFullFrameReceived()
{
    // ... return whether a full frame is received ...
}

uint8_t* ApduProtocol_GetProcessedData(uint32_t* outLen)
{
    // ... return the processed data and its length ...
}

struct ApduProtocolParser* NewApduProtocolParser()
{
    struct ApduProtocolParser* parser = (struct ApduProtocolParser*)malloc(sizeof(struct ApduProtocolParser));
    parser->base.parse = ApduProtocol_Parse;
    parser->base.reset = ApduProtocol_Reset;
    parser->base.isFullFrameReceived = ApduProtocol_IsFullFrameReceived;
    parser->base.getProcessedData = ApduProtocol_GetProcessedData;
    return parser;
}