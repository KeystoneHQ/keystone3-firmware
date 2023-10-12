#include "service_check_lock.h"

void *CheckDeviceLockStatusService(EAPDURequestPayload_t payload)
{
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)calloc(1, sizeof(EAPDUResponsePayload_t));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "payload", GuiLockScreenIsTop());
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    result->data = (uint8_t *)json_str;
    result->dataLen = strlen((char *)result->data);
    result->status = RSP_SUCCESS_CODE;

    SendEApduResponse(EAPDU_PROTOCOL_HEADER, CMD_CHECK_LOCK_STATUS, result);

    free(result);
}