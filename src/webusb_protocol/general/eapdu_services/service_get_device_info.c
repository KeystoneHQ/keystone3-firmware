#include "define.h"
#include "service_check_lock.h"
#include "user_memory.h"

void GetDeviceInfoService(EAPDURequestPayload_t *payload)
{
    char version[BUFFER_SIZE_32] = {0};
    GetUpdateVersionNumber(version);
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "payload", version);
    char *json_str = cJSON_Print(root);
    printf("json_str = %s\n", json_str);
    cJSON_Delete(root);
    result->data = (uint8_t *)json_str;
    result->dataLen = strlen((char *)result->data);
    result->status = RSP_SUCCESS_CODE;
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_GET_DEVICE_INFO;
    result->requestID = payload->requestID;

    SendEApduResponse(result);
    EXT_FREE(json_str);
    SRAM_FREE(result);
}