#include "stdio.h"
#include "stdlib.h"
#include "eapdu_protocol_parser.h"
#include "librust_c.h"
#include "keystore.h"
#include "eapdu_services/service_resolve_ur.h"
#include "eapdu_services/service_check_lock.h"
#include "eapdu_services/service_echo_test.h"

static ProtocolSendCallbackFunc_t g_sendFunc = NULL;
static struct ProtocolParser *global_parser = NULL;

#define EAPDU_RESPONSE_STATUS_LENGTH 2
#define MAX_PACKETS 32
#define MAX_PACKETS_LENGTH 64
#define MAX_EAPDU_DATA_SIZE (MAX_PACKETS_LENGTH - OFFSET_CDATA)
#define MEX_EAPDU_RESPONSE_DATA_SIZE (MAX_PACKETS_LENGTH - OFFSET_CDATA - EAPDU_RESPONSE_STATUS_LENGTH)

uint8_t g_protocolRcvBuffer[MAX_PACKETS][MAX_PACKETS_LENGTH];
uint8_t g_packetLengths[MAX_PACKETS];
uint8_t g_receivedPackets[MAX_PACKETS];
uint8_t g_totalPackets = 0;

static uint16_t extract_16bit_value(const uint8_t *frame, int offset)
{
    return ((uint16_t)frame[offset] << 8) | frame[offset + 1];
}

static void insert_16bit_value(uint8_t *frame, int offset, uint16_t value)
{
    frame[offset] = (uint8_t)(value >> 8);
    frame[offset + 1] = (uint8_t)(value & 0xFF);
}

void SendEApduResponse(uint8_t cla, CommandType ins, EAPDUResponsePayload_t *payload)
{
    uint8_t packet[MAX_PACKETS_LENGTH];
    uint16_t totalPackets = (payload->dataLen + MEX_EAPDU_RESPONSE_DATA_SIZE - 1) / MEX_EAPDU_RESPONSE_DATA_SIZE;
    uint16_t packetIndex = 0;
    uint32_t offset = 0;
    while (payload->dataLen > 0)
    {
        uint16_t packetDataSize = payload->dataLen > MEX_EAPDU_RESPONSE_DATA_SIZE ? MEX_EAPDU_RESPONSE_DATA_SIZE : payload->dataLen;

        packet[OFFSET_CLA] = cla;
        insert_16bit_value(packet, OFFSET_INS, ins);
        insert_16bit_value(packet, OFFSET_P1, totalPackets);
        insert_16bit_value(packet, OFFSET_P2, packetIndex);
        insert_16bit_value(packet, OFFSET_LC, packetDataSize);
        memcpy(packet + OFFSET_CDATA, payload->data + offset, packetDataSize);
        insert_16bit_value(packet, OFFSET_CDATA + packetDataSize, payload->status);
        g_sendFunc(packet, OFFSET_CDATA + packetDataSize + EAPDU_RESPONSE_STATUS_LENGTH);
        offset += packetDataSize;
        payload->dataLen -= packetDataSize;
        packetIndex++;
    }
}

static void free_parser()
{
    g_totalPackets = 0;
    memset(g_receivedPackets, 0, sizeof(g_receivedPackets));
    memset(g_packetLengths, 0, sizeof(g_packetLengths));
    memset(g_protocolRcvBuffer, 0, sizeof(g_protocolRcvBuffer));
}

static void EApduRequestHandler(EAPDURequestPayload_t *request, CommandType command)
{
    switch (command)
    {
    case CMD_ECHO_TEST:
        EchoService(*request);
        break;
    case CMD_RESOLVE_UR:
        ProcessURService(*request);
        break;
    case CMD_CHECK_LOCK_STATUS:
        CheckDeviceLockStatusService(*request);
        break;
    default:
        printf('Invalid command\n');
        break;
    }
}

static ParserStatusEnum CheckFrameValidity(EAPDUFrame_t *eapduFrame)
{
    if (eapduFrame->p1 > MAX_PACKETS)
    {
        printf("Invalid packet index or total number of packets\n");
        free_parser();
        return FRAME_TOTAL_ERROR;
    }
    else if (eapduFrame->p2 >= eapduFrame->p1)
    {
        printf("Invalid packet index or total number of packets\n");
        free_parser();
        return FRAME_INDEX_ERROR;
    }
    else if (g_receivedPackets[eapduFrame->p2])
    { // duplicate packet
        printf("Duplicate frame\n");
        return DUPLICATE_FRAME;
    }

    return FRAME_CHECKSUM_OK;
}

static EAPDUFrame_t *FrameParser(const uint8_t *frame, uint32_t len)
{
    EAPDUFrame_t *eapduFrame = (EAPDUFrame_t *)malloc(sizeof(EAPDUFrame_t));
    eapduFrame->cla = frame[OFFSET_CLA];
    eapduFrame->ins = extract_16bit_value(frame, OFFSET_INS);
    eapduFrame->p1 = extract_16bit_value(frame, OFFSET_P1);
    eapduFrame->p2 = extract_16bit_value(frame, OFFSET_P2);
    eapduFrame->lc = extract_16bit_value(frame, OFFSET_LC);
    eapduFrame->data = frame + OFFSET_CDATA;
    eapduFrame->dataLen = len - OFFSET_CDATA;
    return eapduFrame;
}

void EApduProtocolParse(const uint8_t *frame, uint32_t len)
{
    if (len < 4)
    {
        printf("Invalid EAPDU data\n");
        free_parser();
        return;
    }

    EAPDUFrame_t *eapduFrame = FrameParser(frame, len);

    if (CheckFrameValidity(eapduFrame) != FRAME_CHECKSUM_OK)
    {
        return;
    }

    if (eapduFrame->p2 == 0 && g_totalPackets == 0)
    { // the first packet
        g_totalPackets = eapduFrame->p1;
        printf("Total number of packets: %d\n", g_totalPackets);
        memset(g_receivedPackets, 0, sizeof(g_receivedPackets));
    }

    memcpy(g_protocolRcvBuffer[eapduFrame->p2], eapduFrame->data, eapduFrame->dataLen);
    g_packetLengths[eapduFrame->p2] = eapduFrame->dataLen;
    g_receivedPackets[eapduFrame->p2] = 1;

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

    EAPDURequestPayload_t *request = (EAPDURequestPayload_t *)malloc(sizeof(EAPDURequestPayload_t));
    request->data = fullData;
    request->dataLen = fullDataLen;
    EApduRequestHandler(request, eapduFrame->ins);

    free(eapduFrame);
    free(fullData);
    free(request);
    free_parser();
}

static void RegisterSendFunc(ProtocolSendCallbackFunc_t sendFunc)
{
    if (g_sendFunc == NULL)
    {
        g_sendFunc = sendFunc;
    }
}

struct ProtocolParser *NewEApduProtocolParser()
{
    if (!global_parser)
    {
        global_parser = (struct ProtocolParser *)malloc(sizeof(struct ProtocolParser));
        global_parser->name = EAPDU_PROTOCOL_PARSER_NAME;
        global_parser->parse = EApduProtocolParse;
        global_parser->registerSendFunc = RegisterSendFunc;
        global_parser->rcvCount = 0;
    }
    return global_parser;
}