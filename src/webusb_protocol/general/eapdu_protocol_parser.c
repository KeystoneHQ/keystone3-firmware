#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "eapdu_protocol_parser.h"
#include "keystore.h"
#include "data_parser_task.h"
#include "user_msg.h"
#include "user_memory.h"
#include "gui_views.h"
#include "user_delay.h"
#include "eapdu_services/service_resolve_ur.h"
#include "eapdu_services/service_check_lock.h"
#include "eapdu_services/service_echo_test.h"
#include "eapdu_services/service_export_address.h"
#include "eapdu_services/service_trans_usb_pubkey.h"
#include "eapdu_services/service_get_device_info.h"

static ProtocolSendCallbackFunc_t g_sendFunc = NULL;
static struct ProtocolParser *global_parser = NULL;

#define EAPDU_RESPONSE_STATUS_LENGTH 2
#define MAX_PACKETS 200
#define MAX_PACKETS_LENGTH 64
#define MAX_EAPDU_DATA_SIZE (MAX_PACKETS_LENGTH - OFFSET_CDATA)
#define MAX_EAPDU_RESPONSE_DATA_SIZE (MAX_PACKETS_LENGTH - OFFSET_CDATA - EAPDU_RESPONSE_STATUS_LENGTH)
// #define MAX_EAPDU_RESPONSE_DATA_SIZE 48

static uint8_t g_protocolRcvBuffer[MAX_PACKETS][MAX_PACKETS_LENGTH] __attribute__((section(".data_parser_section")));
static uint8_t g_packetLengths[MAX_PACKETS];
static uint8_t g_receivedPackets[MAX_PACKETS];
static uint8_t g_totalPackets = 0;

typedef enum {
    FRAME_INVALID_LENGTH,
    UNKNOWN_COMMAND,
    FRAME_INDEX_ERROR,
    FRAME_TOTAL_ERROR,
    DUPLICATE_FRAME,
    FRAME_CHECKSUM_OK,
} ParserStatusEnum;

void SendEApduResponse(EAPDUResponsePayload_t *payload)
{
    assert(payload != NULL);
    uint8_t packet[MAX_PACKETS_LENGTH];
    uint16_t totalPackets = (payload->dataLen + MAX_EAPDU_RESPONSE_DATA_SIZE - 1) / MAX_EAPDU_RESPONSE_DATA_SIZE;
    uint16_t packetIndex = 0;
    uint32_t offset = 0;

    while (payload->dataLen > 0) {
        uint16_t packetDataSize = payload->dataLen > MAX_EAPDU_RESPONSE_DATA_SIZE ? MAX_EAPDU_RESPONSE_DATA_SIZE : payload->dataLen;

        packet[OFFSET_CLA] = payload->cla;
        insert_16bit_value(packet, OFFSET_INS, payload->commandType);
        insert_16bit_value(packet, OFFSET_P1, totalPackets);
        insert_16bit_value(packet, OFFSET_P2, packetIndex);
        insert_16bit_value(packet, OFFSET_LC, payload->requestID);
        memcpy_s(packet + OFFSET_CDATA, MAX_PACKETS_LENGTH - OFFSET_CDATA, payload->data + offset, packetDataSize);
        insert_16bit_value(packet, OFFSET_CDATA + packetDataSize, payload->status);
        g_sendFunc(packet, OFFSET_CDATA + packetDataSize + EAPDU_RESPONSE_STATUS_LENGTH);
        offset += packetDataSize;
        payload->dataLen -= packetDataSize;
        packetIndex++;
        UserDelay(10);
    }
}

void SendEApduResponseError(uint8_t cla, CommandType ins, uint16_t requestID, StatusEnum status, char *error)
{
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "payload", error);
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    result->data = (uint8_t *)json_str;
    result->dataLen = strlen((char *)result->data);
    result->status = status;
    result->cla = cla;
    result->commandType = ins;
    result->requestID = requestID;
    SendEApduResponse(result);
    EXT_FREE(json_str);
    SRAM_FREE(result);
}

static void free_parser()
{
    g_totalPackets = 0;
    memset_s(g_receivedPackets, sizeof(g_receivedPackets), 0, sizeof(g_receivedPackets));
    memset_s(g_packetLengths, sizeof(g_receivedPackets), 0, sizeof(g_packetLengths));
    for (int i = 0; i < MAX_PACKETS; i++) {
        memset_s(g_protocolRcvBuffer, sizeof(g_protocolRcvBuffer[i]), 0, sizeof(g_protocolRcvBuffer[i]));
    }
}

static void EApduRequestHandler(EAPDURequestPayload_t *request)
{
    if (!request) {
        printf("Invalid request: NULL pointer\n");
        return;
    }
    printf("request: %u\n", request->commandType);
    switch (request->commandType) {
    case CMD_ECHO_TEST:
        EchoService(request);
        break;
    case CMD_RESOLVE_UR:
        ProcessURService(request);
        break;
    case CMD_CHECK_LOCK_STATUS:
        CheckDeviceLockStatusService(request);
        break;
    case CMD_EXPORT_ADDRESS:
        ExportAddressService(request);
        break;
    case CMD_GET_DEVICE_INFO:
        GetDeviceInfoService(request);
        break;
    case CMD_GET_DEVICE_USB_PUBKEY:
        GetDeviceUsbPubkeyService(request);
        break;
    default:
        printf("Invalid command: %u\n", request->commandType);
        break;
    }
}

static ParserStatusEnum CheckFrameValidity(EAPDUFrame_t *eapduFrame)
{
    if (eapduFrame->p1 > MAX_PACKETS) {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, eapduFrame->ins, eapduFrame->lc, PRS_INVALID_TOTAL_PACKETS, "Invalid total number of packets");
        free_parser();
        return FRAME_TOTAL_ERROR;
    } else if (eapduFrame->p2 >= eapduFrame->p1) {
        printf("Invalid packet index\n");
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, eapduFrame->ins, eapduFrame->lc, PRS_INVALID_INDEX, "Invalid packet index");
        free_parser();
        return FRAME_INDEX_ERROR;
    } else if (g_receivedPackets[eapduFrame->p2]) {
        // duplicate packet
        printf("Duplicate frame\n");
        return DUPLICATE_FRAME;
    }

    return FRAME_CHECKSUM_OK;
}

static EAPDUFrame_t *FrameParser(const uint8_t *frame, uint32_t len)
{
    if (!frame || len < OFFSET_CDATA) {
        printf("Invalid frame data\n");
        return NULL;
    }
    EAPDUFrame_t *eapduFrame = (EAPDUFrame_t *)SRAM_MALLOC(sizeof(EAPDUFrame_t));
    eapduFrame->cla = frame[OFFSET_CLA];
    eapduFrame->ins = extract_16bit_value(frame, OFFSET_INS);
    eapduFrame->p1 = extract_16bit_value(frame, OFFSET_P1);
    eapduFrame->p2 = extract_16bit_value(frame, OFFSET_P2);
    eapduFrame->lc = extract_16bit_value(frame, OFFSET_LC);
    eapduFrame->data = (uint8_t *)(frame + OFFSET_CDATA);
    eapduFrame->dataLen = len - OFFSET_CDATA;
    return eapduFrame;
}

void EApduProtocolParse(const uint8_t *frame, uint32_t len)
{
    if (len < 4) { // Ensure frame has minimum length
        printf("Invalid EAPDU data: too short\n");
        free_parser();
        return;
    }
    EAPDUFrame_t *eapduFrame = FrameParser(frame, len);
    if (!eapduFrame || CheckFrameValidity(eapduFrame) != FRAME_CHECKSUM_OK) {
        SRAM_FREE(eapduFrame);
        return;
    }
    if (eapduFrame->p2 == 0 && g_totalPackets == 0) {
        g_totalPackets = eapduFrame->p1;
        assert(g_totalPackets <= MAX_PACKETS);
        memset_s(g_receivedPackets, sizeof(g_receivedPackets), 0, sizeof(g_receivedPackets));
    }
    assert(eapduFrame->dataLen <= MAX_PACKETS_LENGTH && eapduFrame->p2 < MAX_PACKETS);
    memcpy_s(g_protocolRcvBuffer[eapduFrame->p2], sizeof(g_protocolRcvBuffer[eapduFrame->p2]), eapduFrame->data, eapduFrame->dataLen);
    g_packetLengths[eapduFrame->p2] = eapduFrame->dataLen;
    g_receivedPackets[eapduFrame->p2] = 1;
    for (uint8_t i = 0; i < g_totalPackets; i++) {
        if (!g_receivedPackets[i]) {
            printf("Waiting for more packets, missing: %d\n", i);
            SRAM_FREE(eapduFrame);
            return;
        }
    }
    uint32_t fullDataLen = 0;
    uint32_t offset = 0;
    uint8_t *fullData = NULL;
    for (uint16_t i = 0; i < g_totalPackets; i++) {
        fullDataLen += g_packetLengths[i];
    }

    fullData = (uint8_t *)SRAM_MALLOC(fullDataLen + 1);
    for (uint32_t i = 0; i < g_totalPackets; i++) {
        memcpy_s(fullData + offset, fullDataLen - offset, g_protocolRcvBuffer[i], g_packetLengths[i]);
        offset += g_packetLengths[i];
    }
    fullData[fullDataLen] = '\0';
    EAPDURequestPayload_t *request = (EAPDURequestPayload_t *)SRAM_MALLOC(sizeof(EAPDURequestPayload_t));
    request->data = fullData;
    request->dataLen = fullDataLen;
    request->requestID = eapduFrame->lc;
    request->commandType = eapduFrame->ins;
    request->cla = eapduFrame->cla;
    EApduRequestHandler(request);
    SRAM_FREE(eapduFrame);
    SRAM_FREE(fullData);
    SRAM_FREE(request);
    free_parser();
}

static void RegisterSendFunc(ProtocolSendCallbackFunc_t sendFunc)
{
    if (g_sendFunc == NULL) {
        g_sendFunc = sendFunc;
    }
}

struct ProtocolParser *NewEApduProtocolParser()
{
    if (!global_parser) {
        global_parser = (struct ProtocolParser *)SRAM_MALLOC(sizeof(struct ProtocolParser));
        global_parser->name = EAPDU_PROTOCOL_PARSER_NAME;
        global_parser->parse = EApduProtocolParse;
        global_parser->registerSendFunc = RegisterSendFunc;
        global_parser->rcvCount = 0;
    }
    printf("global_parser = %p\n", global_parser);
    return global_parser;
}

void GotoResultPage(EAPDUResultPage_t *resultPageParams)
{
    if (resultPageParams != NULL) {
        if (GuiCheckIfTopView(&g_USBTransportView)) {
            return;
        }
        if (resultPageParams == NULL) {
            PubValueMsg(UI_MSG_USB_TRANSPORT_VIEW, 0);
        } else {
            PubBufferMsg(UI_MSG_USB_TRANSPORT_VIEW, resultPageParams, sizeof(EAPDUResultPage_t));
        }
    }
}