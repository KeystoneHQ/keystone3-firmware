#include "stdio.h"
#include "stdlib.h"
#include "apdu_protocol_parser.h"

static ProtocolSendCallbackFunc_t *g_sendFunc = NULL;
static struct ProtocolParser *global_parser = NULL;
uint8_t g_index = 0;

#define MAX_PACKETS 8
#define MAX_PACKETS_LENGTH 64

uint8_t g_protocolRcvBuffer[MAX_PACKETS][MAX_PACKETS_LENGTH];
uint8_t g_packetLengths[MAX_PACKETS];
uint8_t g_receivedPackets[MAX_PACKETS];
uint8_t g_totalPackets = 0;

static void parse_apdu(const uint8_t *frame, uint32_t len)
{
    if (len < 4) {
        printf("Invalid APDU data\n");
        return;
    }

    uint8_t cla = frame[OFFSET_CLA];
    uint8_t ins = frame[OFFSET_INS];
    uint8_t p1 = frame[OFFSET_P1];  // total number of packets
    uint8_t p2 = frame[OFFSET_P2];  // index of the current packet
    uint8_t lc = frame[OFFSET_LC];
    uint8_t *data = frame + OFFSET_CDATA;
    uint8_t dataLen = len - OFFSET_CDATA;

    if (p1 > MAX_PACKETS || p2 >= p1) {
        printf("Invalid packet index or total number of packets\n");
        return;
    }

    if (p2 == 0 && g_totalPackets == 0) {  // the first packet
        g_totalPackets = p1;
        memset(g_receivedPackets, 0, sizeof(g_receivedPackets));
    } else if (p1 != g_totalPackets) {  // total number of packets should be the same in all packets
        printf("Inconsistent total number of packets\n");
        return;
    }

    if (g_receivedPackets[p2]) {  // duplicate packet
        printf("Duplicate packet\n");
        return;
    }

    memcpy(g_protocolRcvBuffer[p2], data, dataLen);
    g_packetLengths[p2] = dataLen;
    g_receivedPackets[p2] = 1;

    // check if all packets have arrived
    for (uint8_t i = 0; i < g_totalPackets; i++) {
        if (!g_receivedPackets[i]) {
            printf("Waiting for packet %d\n", i);
            return;  // not all packets have arrived yet
        }
    }

    printf("All packets have arrived. Received %d packets.\n", g_totalPackets);
    
    // Print all received packets
    printf("Received data: ");
    for (uint8_t i = 0; i < g_totalPackets; i++) {
        for (uint8_t j = 0; j < g_packetLengths[i]; j++) {
            printf("%02X ", g_protocolRcvBuffer[i][j]);
        }
    }
    printf("\n");

    // reset for the next set of packets
    g_totalPackets = 0;
}

void ApduProtocol_Parse(const uint8_t *frame, uint32_t len)
{
    parse_apdu(frame, len);
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