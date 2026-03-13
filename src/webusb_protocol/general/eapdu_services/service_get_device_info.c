#include <string.h>
#include "keystore.h"
#include "define.h"
#include "service_check_lock.h"
#include "user_memory.h"
#include "version.h"
#include "gui.h"
#include "gui_lock_widgets.h"

void GetDeviceInfoService(EAPDURequestPayload_t *payload)
{
    char buffer[BUFFER_SIZE_32] = {0};
    uint8_t mfp[4] = {0};

    if (payload == NULL) {
        return;
    }

    if (GuiLockScreenIsTop()) {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_GET_DEVICE_INFO, payload->requestID, PRS_PARSING_DISALLOWED,
                               "Get device info is not allowed when the device is locked");
        return;
    }

    GetUpdateVersionNumber(buffer);
    GetMasterFingerPrint(mfp);
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    if (result == NULL) {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_GET_DEVICE_INFO, payload->requestID, RSP_FAILURE_CODE, "Internal memory error");
        return;
    }

    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        SRAM_FREE(result);
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_GET_DEVICE_INFO, payload->requestID, RSP_FAILURE_CODE, "Internal memory error");
        return;
    }

    cJSON_AddStringToObject(root, "firmwareVersion", buffer);
    snprintf_s(buffer, sizeof(buffer), "%02x%02x%02x%02x", mfp[0], mfp[1], mfp[2], mfp[3]);
    cJSON_AddStringToObject(root, "walletMFP", buffer);
    char *json_str = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    cJSON_Delete(root);
    if (json_str == NULL) {
        SRAM_FREE(result);
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_GET_DEVICE_INFO, payload->requestID, RSP_FAILURE_CODE, "Internal memory error");
        return;
    }

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
