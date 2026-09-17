#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "mhscpu.h"
#include "cmsis_os.h"
#include "eapdu_protocol_parser.h"
#include "keystore.h"
#include "data_parser_task.h"
#include "user_msg.h"
#include "user_memory.h"
#include "gui_views.h"
#include "user_delay.h"
#include "eapdu_framing.h"
#include "mpu_sandbox_task.h"
#include "mpu_sandbox_ur_validator.h"
#include "eapdu_services/service_resolve_ur.h"
#include "eapdu_services/service_check_lock.h"
#include "eapdu_services/service_echo_test.h"
#include "eapdu_services/service_export_address.h"
#include "eapdu_services/service_get_device_info.h"
#include "eapdu_services/service_trans_usb_pubkey.h"

static ProtocolSendCallbackFunc_t g_sendFunc = NULL;
static uint32_t g_eapduRcvCount = 0;

#define EAPDU_RESPONSE_STATUS_LENGTH 2
#define MAX_PACKETS_LENGTH MAX_EAPDU_PACKET_SIZE
#define MAX_EAPDU_RESPONSE_DATA_SIZE (MAX_PACKETS_LENGTH - OFFSET_CDATA - EAPDU_RESPONSE_STATUS_LENGTH)
#define EAPDU_SANDBOX_RESULT_TIMEOUT_MS 1000U

static uint32_t GetRcvCount(void);
static void ResetRcvCount(void);
static const char *MpuSandboxUrValidationMessage(uint32_t status);

static const char *MpuSandboxUrValidationMessage(uint32_t status)
{
    switch ((MpuSandboxUrValidationStatus_t)status) {
    case MPU_SANDBOX_UR_INVALID_INPUT:
        return "UR validation failed";
    case MPU_SANDBOX_UR_INVALID_SCHEME:
        return "ur decode failed, reason: invalid scheme";
    case MPU_SANDBOX_UR_INVALID_TYPE:
        return "ur decode failed, reason: invalid type";
    case MPU_SANDBOX_UR_INVALID_MULTIPART:
        return "ur decode failed, reason: invalid multipart indices";
    case MPU_SANDBOX_UR_INVALID_BYTEWORDS_LENGTH:
        return "ur decode failed, reason: invalid length";
    case MPU_SANDBOX_UR_INVALID_BYTEWORDS_WORD:
        return "ur decode failed, reason: invalid word";
    case MPU_SANDBOX_UR_INVALID_CHECKSUM:
        return "ur decode failed, reason: invalid checksum";
    case MPU_SANDBOX_UR_CBOR_UNEXPECTED_EOF:
        return "cbor decode failed, reason: end of input bytes";
    case MPU_SANDBOX_UR_CBOR_NESTING_LIMIT:
        return "cbor decode failed, reason: nesting limit 32 exceeded";
    case MPU_SANDBOX_UR_CBOR_ITEM_LIMIT:
        return "cbor decode failed, reason: item limit 1024 exceeded";
    case MPU_SANDBOX_UR_CBOR_TRAILING_DATA:
        return "cbor decode failed, reason: trailing data";
    case MPU_SANDBOX_UR_CBOR_INVALID:
    default:
        return "cbor decode failed, reason: invalid structure";
    }
}

static bool WaitForSandboxResult(uint32_t requestId, MpuSandboxEapduResult_t *result,
                                 const uint8_t **payload)
{
    for (uint32_t wait = 0U; wait < EAPDU_SANDBOX_RESULT_TIMEOUT_MS; wait++) {
        if (MpuSandboxGetEapduResult(requestId, result, payload)) {
            return true;
        }
        osDelay(1U);
    }
    printf("EAPDU sandbox result timeout, request=%lu\r\n", (unsigned long)requestId);
    return false;
}

void SendEApduResponse(EAPDUResponsePayload_t *payload)
{
    assert(payload != NULL);
    if (payload == NULL || g_sendFunc == NULL) {
        return;
    }
    if (payload->data == NULL && payload->dataLen != 0U) {
        return;
    }

    uint8_t packet[MAX_PACKETS_LENGTH];
    uint16_t totalPackets = (payload->dataLen + MAX_EAPDU_RESPONSE_DATA_SIZE - 1) / MAX_EAPDU_RESPONSE_DATA_SIZE;
    uint16_t packetIndex = 0;
    uint32_t offset = 0;
    uint32_t remaining = payload->dataLen;
    if (totalPackets == 0U) {
        totalPackets = 1U;
    }

    do {
        uint16_t packetDataSize = remaining > MAX_EAPDU_RESPONSE_DATA_SIZE ? MAX_EAPDU_RESPONSE_DATA_SIZE : (uint16_t)remaining;

        packet[OFFSET_CLA] = payload->cla;
        insert_16bit_value(packet, OFFSET_INS, payload->commandType);
        insert_16bit_value(packet, OFFSET_P1, totalPackets);
        insert_16bit_value(packet, OFFSET_P2, packetIndex);
        insert_16bit_value(packet, OFFSET_LC, payload->requestID);
        if (packetDataSize > 0U) {
            memcpy_s(packet + OFFSET_CDATA, MAX_PACKETS_LENGTH - OFFSET_CDATA, payload->data + offset, packetDataSize);
        }
        insert_16bit_value(packet, OFFSET_CDATA + packetDataSize, payload->status);
        g_sendFunc(packet, OFFSET_CDATA + packetDataSize + EAPDU_RESPONSE_STATUS_LENGTH);
        offset += packetDataSize;
        remaining -= packetDataSize;
        packetIndex++;
        UserDelay(10);
    } while (remaining > 0U);
}

void SendEApduResponseError(uint8_t cla, CommandType ins, uint16_t requestID, StatusEnum status, char *error)
{
    if (error == NULL) {
        error = "unknown error";
    }
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    if (result == NULL) {
        return;
    }
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        SRAM_FREE(result);
        return;
    }
    cJSON_AddStringToObject(root, "payload", error);
    char *json_str = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    cJSON_Delete(root);
    if (json_str == NULL) {
        SRAM_FREE(result);
        return;
    }
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
    g_eapduRcvCount = 0;
    (void)MpuSandboxResetEapduFraming();
}

static void EApduRequestHandler(EAPDURequestPayload_t *request)
{
    if (!request) {
        printf("Invalid request: NULL pointer\n");
        return;
    }
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
#ifdef WEB3_VERSION
    case CMD_EXPORT_ADDRESS:
        ExportAddressService(request);
        break;
#endif
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

void EApduProtocolParse(const uint8_t *frame, uint32_t len)
{
    MpuSandboxEapduResult_t framingResult;
    EAPDURequestPayload_t request;
    const uint8_t *payload = NULL;
    uint32_t sandboxRequestId;

    g_eapduRcvCount++;
    if (!MpuSandboxSubmit(MPU_SANDBOX_OP_EAPDU_FRAME, frame, len, &sandboxRequestId)) {
        printf("EAPDU sandbox busy or invalid input\n");
        g_eapduRcvCount = 0;
        return;
    }
    if (!WaitForSandboxResult(sandboxRequestId, &framingResult, &payload)) {
        printf("EAPDU sandbox failed closed, request=%lu\r\n",
               (unsigned long)sandboxRequestId);
        __DSB();
        NVIC_SystemReset();
        for (;;) {
        }
    }
    if (framingResult.timed_out != 0U) {
        printf("EAPDU reassembly timeout\n");
    }

    switch ((EapduFramingStatus_t)framingResult.framing_status) {
    case EAPDU_FRAMING_ERROR_SHORT_FRAME:
        printf("Invalid EAPDU data: too short\n");
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
        g_eapduRcvCount = 0;
        return;

    case EAPDU_FRAMING_ERROR_TOTAL_PACKETS:
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, framingResult.command_type, framingResult.request_id,
                               PRS_INVALID_TOTAL_PACKETS, "Invalid total number of packets");
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
        g_eapduRcvCount = 0;
        return;

    case EAPDU_FRAMING_ERROR_PACKET_INDEX:
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, framingResult.command_type, framingResult.request_id,
                               PRS_INVALID_INDEX, "Invalid packet index");
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
        g_eapduRcvCount = 0;
        return;

    case EAPDU_FRAMING_ERROR_PACKET_SIZE:
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, framingResult.command_type, framingResult.request_id,
                               PRS_INVALID_INDEX, "Invalid packet length/index");
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
        g_eapduRcvCount = 0;
        return;

    case EAPDU_FRAMING_ERROR_TOTAL_MISMATCH:
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, framingResult.command_type, framingResult.request_id,
                               PRS_INVALID_TOTAL_PACKETS, "Mismatched total packets");
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
        g_eapduRcvCount = 0;
        return;

    case EAPDU_FRAMING_ERROR_METADATA_MISMATCH:
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, framingResult.command_type, framingResult.request_id,
                               PRS_INVALID_INDEX, "Mismatched frame metadata");
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
        g_eapduRcvCount = 0;
        return;

    case EAPDU_FRAMING_DUPLICATE:
        printf("Duplicate frame\n");
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, false);
        return;

    case EAPDU_FRAMING_WAITING:
        printf("Waiting for more packets, missing: %u\n", framingResult.first_missing_packet);
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, false);
        return;

    case EAPDU_FRAMING_COMPLETE:
        break;

    default:
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
        g_eapduRcvCount = 0;
        return;
    }

    request.data = (uint8_t *)payload;
    request.dataLen = framingResult.payload_length;
    request.requestID = framingResult.request_id;
    request.commandType = framingResult.command_type;
    request.cla = framingResult.cla;
    printf("request->dataLen=%u\nrequestID=%u\ncommandType=%u\ncla=%u\n", (unsigned int)request.dataLen, request.requestID, request.commandType, request.cla);
    if ((request.commandType == CMD_RESOLVE_UR) &&
            (framingResult.ur_validation_status != MPU_SANDBOX_UR_OK)) {
        ProcessURValidationError(&request,
                                 MpuSandboxUrValidationMessage(
                                     framingResult.ur_validation_status));
        (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
        g_eapduRcvCount = 0;
        return;
    }
    EApduRequestHandler(&request);
    (void)MpuSandboxReleaseEapduResult(sandboxRequestId, true);
    g_eapduRcvCount = 0;
}

static void RegisterSendFunc(ProtocolSendCallbackFunc_t sendFunc)
{
    if (g_sendFunc == NULL) {
        g_sendFunc = sendFunc;
    }
}

static uint32_t GetRcvCount(void)
{
    return g_eapduRcvCount;
}

static void ResetRcvCount(void)
{
    free_parser();
}

const struct ProtocolParser *NewEApduProtocolParser()
{
    static const struct ProtocolParser g_eapduParser = {
        .name = EAPDU_PROTOCOL_PARSER_NAME,
        .parse = EApduProtocolParse,
        .registerSendFunc = RegisterSendFunc,
        .getRcvCount = GetRcvCount,
        .resetRcvCount = ResetRcvCount,
    };
    return &g_eapduParser;
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
