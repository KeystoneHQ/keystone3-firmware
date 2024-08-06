#include <string.h>
#include "keystore.h"
#include "define.h"
#include "service_check_lock.h"
#include "user_memory.h"
#include "version.h"

void GetDeviceInfoService(EAPDURequestPayload_t *payload)
{
    char buffer[BUFFER_SIZE_32] = {0};
    uint8_t mfp[4] = {0};
    GetUpdateVersionNumber(buffer);
    GetMasterFingerPrint(mfp);
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));

    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "firmwareVersion", buffer);
    snprintf_s(buffer, sizeof(buffer), "%02x%02x%02x%02x", mfp[0], mfp[1], mfp[2], mfp[3]);
    cJSON_AddStringToObject(root, "walletMFP", buffer);
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