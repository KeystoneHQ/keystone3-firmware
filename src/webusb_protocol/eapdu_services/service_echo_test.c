#include "service_echo_test.h"

void *EchoService(EAPDURequestPayload_t payload)
{
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)calloc(1, sizeof(EAPDUResponsePayload_t));
    result->data = &payload.data;
    result->dataLen = payload.dataLen;
    result->status = RSP_SUCCESS_CODE;
    SendEApduResponse(EAPDU_PROTOCOL_HEADER, CMD_ECHO_TEST, result);

    free(result);
}