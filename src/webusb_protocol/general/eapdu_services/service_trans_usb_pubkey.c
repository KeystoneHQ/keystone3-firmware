#include "user_memory.h"
#include "service_echo_test.h"

void GetDeviceUsbPubkeyService(EAPDURequestPayload_t *payload)
{
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    result->data = payload->data;
    result->dataLen = payload->dataLen;
    result->status = RSP_SUCCESS_CODE;
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_GET_DEVICE_USB_PUBKEY;
    result->requestID = payload->requestID;

    SendEApduResponse(result);

    SRAM_FREE(result);
}