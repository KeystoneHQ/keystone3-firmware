#include "stdio.h"
#include "stdlib.h"
#include "apdu_protocol_parser.h"
#include "librust_c.h"
#include "keystore.h"

static ProtocolSendCallbackFunc_t *g_sendFunc = NULL;
static struct ProtocolParser *global_parser = NULL;

#define MAX_PACKETS 8
#define MAX_PACKETS_LENGTH 64

uint8_t g_protocolRcvBuffer[MAX_PACKETS][MAX_PACKETS_LENGTH];
uint8_t g_packetLengths[MAX_PACKETS];
uint8_t g_receivedPackets[MAX_PACKETS];
uint8_t g_totalPackets = 0;

static CommandResponse *ProcessEthereumTransactionSignature(uint8_t *data, uint16_t dataLen)
{
    CommandResponse *response = (CommandResponse *)malloc(sizeof(CommandResponse));
    response->command = SIGN_ETH_TX;
    response->length = 0;
    response->data = NULL;

    UREncodeResult *encodeResult;
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    struct URParseResult *urResult = parse_ur(data);
    encodeResult = eth_sign_tx(urResult->data, seed, len);
    printf("encodeResult->data: %s\n", encodeResult->data);
    printf("encodeResult->data length: %d\n", strlen(encodeResult->data)); 
    (*g_sendFunc)(encodeResult->data, strlen(encodeResult->data));

    return response;
}

static void reset()
{
    g_totalPackets = 0;
    memset(g_receivedPackets, 0, sizeof(g_receivedPackets));
    // reset g_packetLengths and g_protocolRcvBuffer
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
    // uint8_t ins = frame[OFFSET_INS];
    uint8_t p1 = frame[OFFSET_P1]; // total number of packets
    uint8_t p2 = frame[OFFSET_P2]; // index of the current packet
    uint8_t lc = frame[OFFSET_LC];
    uint8_t *data = frame + OFFSET_CDATA;
    uint8_t dataLen = len - OFFSET_CDATA;
    printf("\n");
    PrintArray("APDU data", data, dataLen);
    printf("\n");
    PrintArray("APDU frame", frame, len);
    printf("\n");

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
        printf("p1: %d, g_totalPackets: %d\n", p1, g_totalPackets);
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

    printf("All packets have arrived. Received %d packets.\n", g_totalPackets);

    // Print all received packets
    printf("Received data: ");

    // uint8_t *fullData = NULL;

    uint16_t fullDataLen = 0;
    for (uint16_t i = 0; i < g_totalPackets; i++)
    {
        fullDataLen += g_packetLengths[i];
        printf("g_packetLengths[i]: %d \n", g_packetLengths[i]);
    }
    printf("fullDataLen: %d\n", fullDataLen);
    uint8_t *fullData = (uint8_t *)malloc(fullDataLen + 1);
    uint16_t offset = 0;
    for (uint16_t i = 0; i < g_totalPackets; i++)
    {
        memcpy(fullData + offset, g_protocolRcvBuffer[i], g_packetLengths[i]);
        offset += g_packetLengths[i];
        PrintArray("---fullData---", fullData, offset);
    }
    fullData[fullDataLen] = '\0';
    printf("data char : %s\n", (char *)fullData);

    CommandResponse *result = (CommandResponse *)malloc(sizeof(CommandResponse));
    switch (frame[OFFSET_INS])
    {
    case SIGN_ETH_TX:
        printf("Processing SIGN_ETH_TX command\n");
        result = ProcessEthereumTransactionSignature(fullData, fullDataLen);
        free(fullData);
        break;

    default:
        printf('Invalid command\n');
        break;
    }

    g_totalPackets = 0;
    memset(g_receivedPackets, 0, sizeof(g_receivedPackets));
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