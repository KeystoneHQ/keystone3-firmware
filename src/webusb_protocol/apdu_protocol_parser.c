#include "stdio.h"
#include "stdlib.h"
#include "apdu_protocol_parser.h"
#include "librust_c.h"
#include "keystore.h"
#include "service_eth_sign.h"

static ProtocolSendCallbackFunc_t g_sendFunc = NULL;
static struct ProtocolParser *global_parser = NULL;

#define MAX_PACKETS 8
#define MAX_PACKETS_LENGTH 64
#define MAX_PACKETS_DATA_LENGTH 59
#define MAX_APDU_DATA_SIZE (MAX_PACKETS_LENGTH - OFFSET_CDATA)

uint8_t g_protocolRcvBuffer[MAX_PACKETS][MAX_PACKETS_LENGTH];
uint8_t g_packetLengths[MAX_PACKETS];
uint8_t g_receivedPackets[MAX_PACKETS];
uint8_t g_totalPackets = 0;

static void send_apdu_response(uint8_t cla, uint8_t ins, uint8_t *data, uint32_t dataLen)
{
    uint8_t packet[MAX_PACKETS_LENGTH];
    uint8_t totalPackets = (dataLen + MAX_APDU_DATA_SIZE - 1) / MAX_APDU_DATA_SIZE;
    uint8_t packetIndex = 0;
    uint32_t offset = 0;
    while (dataLen > 0)
    {
        uint8_t packetDataSize = dataLen > MAX_APDU_DATA_SIZE ? MAX_APDU_DATA_SIZE : dataLen;

        packet[OFFSET_CLA] = cla;
        packet[OFFSET_INS] = ins;
        packet[OFFSET_P1] = totalPackets;
        packet[OFFSET_P2] = packetIndex;
        packet[OFFSET_LC] = packetDataSize;
        memcpy(packet + OFFSET_CDATA, data + offset, packetDataSize);
        g_sendFunc(packet, OFFSET_CDATA + packetDataSize);
        offset += packetDataSize;
        dataLen -= packetDataSize;
        packetIndex++;
    }
}

static void reset()
{
    g_totalPackets = 0;
    memset(g_receivedPackets, 0, sizeof(g_receivedPackets));
    memset(g_packetLengths, 0, sizeof(g_packetLengths));
    memset(g_protocolRcvBuffer, 0, sizeof(g_protocolRcvBuffer));
}

static void parse_apdu(const uint8_t *frame, uint32_t len)
{
    if (len < 4)
    {
        printf("Invalid APDU data\n");
        reset();
        return;
    }

    uint8_t cla = frame[OFFSET_CLA];
    uint8_t p1 = frame[OFFSET_P1]; // total number of packets
    uint8_t p2 = frame[OFFSET_P2]; // index of the current packet
    uint8_t lc = frame[OFFSET_LC];
    uint8_t *data = frame + OFFSET_CDATA;
    uint8_t dataLen = len - OFFSET_CDATA;

    if (p1 > MAX_PACKETS || p2 >= p1)
    {
        printf("Invalid packet index or total number of packets\n");
        reset();
        return;
    }

    if (p2 == 0 && g_totalPackets == 0)
    { // the first packet
        g_totalPackets = p1;
        printf("Total number of packets: %d\n", g_totalPackets);
        memset(g_receivedPackets, 0, sizeof(g_receivedPackets));
    }
    else if (p1 != g_totalPackets)
    { // total number of packets should be the same in all packets
        printf("Inconsistent total number of packets\n");
        reset();
        return;
    }

    if (g_receivedPackets[p2])
    { // duplicate packet
        printf("Duplicate packet\n");
        return;
    }

    memcpy(g_protocolRcvBuffer[p2], data, dataLen);
    g_packetLengths[p2] = dataLen;
    g_receivedPackets[p2] = 1;

    // check if all packets have arrived
    for (uint8_t i = 0; i < g_totalPackets; i++)
    {
        if (!g_receivedPackets[i])
        {
            printf("Waiting for packet %d\n", i);
            return; // not all packets have arrived yet
        }
    }

    uint32_t fullDataLen = 0;
    for (uint16_t i = 0; i < g_totalPackets; i++)
    {
        fullDataLen += g_packetLengths[i];
    }
    uint8_t *fullData = (uint8_t *)malloc(fullDataLen + 1);
    uint32_t offset = 0;
    for (uint32_t i = 0; i < g_totalPackets; i++)
    {
        memcpy(fullData + offset, g_protocolRcvBuffer[i], g_packetLengths[i]);
        offset += g_packetLengths[i];
    }
    fullData[fullDataLen] = '\0';

    Response *result = (Response *)malloc(sizeof(Response));
    switch (frame[OFFSET_INS])
    {
    case CMD_ECHO_TEST:
        send_apdu_response(APDU_PROTOCOL_HEADER, CMD_ECHO_TEST, fullData, fullDataLen);
        break;
    case CMD_SIGN_ETH_TX:
        ProcessEthereumTransactionSignature(fullData, fullDataLen, send_apdu_response);
        break;
    case CMD_CHECK_LOCK_STATUS:
        result->data = (uint8_t *)malloc(1);
        result->data[0] = GuiLockScreenIsTop();
        result->length = 1;
        send_apdu_response(APDU_PROTOCOL_HEADER, CMD_CHECK_LOCK_STATUS, result->data, result->length);
        break;
    default:
        printf('Invalid command\n');
        break;
    }

    free(fullData);
    free(result);
    reset();
}

void ApduProtocol_Parse(const uint8_t *frame, uint32_t len)
{
    parse_apdu(frame, len);
}

static void RegisterSendFunc(ProtocolSendCallbackFunc_t sendFunc)
{
    if (g_sendFunc == NULL)
    {
        g_sendFunc = sendFunc;
    }
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