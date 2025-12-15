#include "user_memory.h"
#include "service_echo_test.h"
#include "data_parser_task.h"
#include "log_print.h"
#include <stdio.h>

void GetDeviceUsbPubkeyService(EAPDURequestPayload_t *payload)
{
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_GET_DEVICE_USB_PUBKEY;
    result->requestID = payload->requestID;

    printf("GetDeviceUsbPubkeyService: received %d bytes\n", payload->dataLen);
    PrintArray("Received pubkey", payload->data, payload->dataLen);

    // Check if we received a valid public key (33 bytes compressed or 65 bytes uncompressed)
    if (payload->dataLen == 33 || payload->dataLen == 65) {
        // Generate device public key using ECDH with the received web public key
        uint8_t *devicePubKey = GetDeviceParserPubKey(payload->data, payload->dataLen);
        
        if (devicePubKey != NULL) {
            // Return the device's public key (33 bytes compressed)
            result->data = (uint8_t *)SRAM_MALLOC(33);
            memcpy_s(result->data, 33, devicePubKey, 33);
            result->dataLen = 33;
            result->status = RSP_SUCCESS_CODE;
            printf("GetDeviceUsbPubkeyService: returning device pubkey\n");
            PrintArray("Device pubkey", result->data, result->dataLen);
        } else {
            printf("GetDeviceUsbPubkeyService: failed to generate device pubkey\n");
            result->data = NULL;
            result->dataLen = 0;
            result->status = RSP_FAILURE_CODE;
        }
    } else {
        printf("GetDeviceUsbPubkeyService: invalid pubkey length %d\n", payload->dataLen);
        result->data = NULL;
        result->dataLen = 0;
        result->status = RSP_FAILURE_CODE;
    }

    SendEApduResponse(result);

    if (result->data != NULL) {
        SRAM_FREE(result->data);
    }
    SRAM_FREE(result);
}