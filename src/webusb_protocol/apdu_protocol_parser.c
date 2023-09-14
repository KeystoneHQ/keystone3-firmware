#include "stdio.h"
#include "apdu_protocol_parser.h"

static ProtocolSendCallbackFunc_t *g_sendFunc = NULL;
static struct ProtocolParser *global_parser = NULL;

void ApduProtocol_Parse(const uint8_t *data, uint32_t len)
{
}

static void RegisterSendFunc(ProtocolSendCallbackFunc_t *sendFunc)
{
    g_sendFunc = sendFunc;
}

struct ProtocolParser *NewApduProtocolParser()
{
    if (!global_parser)
    {
        global_parser = (struct ProtocolParser *)malloc(sizeof(struct ProtocolParser));
        global_parser->name = APDU_PROTOCOL_PARSER_NAME;
        global_parser->parse = ApduProtocol_Parse;
        global_parser->registerSendFunc = RegisterSendFunc;
        global_parser->rcvCount = 0;
    }
    return global_parser;
}