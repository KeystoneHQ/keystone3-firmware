#include "internal_protocol_parser.h"
#include "eapdu_protocol_parser.h"
#include "stdio.h"
#include "string.h"
#include "user_utils.h"
#include "protocol_codec.h"
#include "crc.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "log_print.h"
#include "service_file_trans.h"

#define PROTOCOL_PARSE_OVERTIME 500

uint8_t g_protocolRcvBuffer[PROTOCOL_MAX_LENGTH];

void ProtocolReceivedData(const uint8_t *data, uint32_t len, ProtocolSendCallbackFunc_t sendFunc)
{
    static uint32_t lastTick = 0;
    uint32_t tick;
    static struct ProtocolParser *currentParser = NULL;

    tick = osKernelGetTickCount();

    if (data[0] == EAPDU_PROTOCOL_HEADER && !GetIsReceivingFile())
    {
        currentParser = NewEApduProtocolParser();
    }
    else
    {
        currentParser = NewInternalProtocolParser();
    }

    if (currentParser->rcvCount != 0)
    {
        if (tick - lastTick > PROTOCOL_PARSE_OVERTIME)
        {
            currentParser->rcvCount = 0;
        }
    }
    lastTick = tick;
    currentParser->registerSendFunc(sendFunc);

    currentParser->parse(data, len);
}
