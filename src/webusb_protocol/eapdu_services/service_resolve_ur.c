#include "service_resolve_ur.h";
#include "user_delay.h";
#include "gui_chain.h";
#include "gui_views.h";
#include "user_msg.h";

typedef struct
{
    uint8_t viewType;
    uint8_t urType;
} UrViewType_t;

static void BasicHandlerFunc(const void *data, uint32_t data_len, StatusEnum status)
{
    EAPDUResponsePayload_t *payload = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "payload", (char *)data);
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    payload->data = (uint8_t *)json_str;
    payload->dataLen = strlen((char *)payload->data);
    payload->status = status;

    SendEApduResponse(EAPDU_PROTOCOL_HEADER, CMD_RESOLVE_UR, payload);

    SRAM_FREE(payload);
};

void HandleURResultViaUSBFunc(const void *data, uint32_t data_len, StatusEnum status)
{
    BasicHandlerFunc(data, data_len, status);
};

static uint8_t *DataParser(EAPDURequestPayload_t *payload)
{
    return payload->data;
}

void *ProcessURService(EAPDURequestPayload_t payload)
{
    if (GuiLockScreenIsTop()) {
        const char *data = "Device is locked";
        HandleURResultViaUSBFunc(data, strlen(data), PRS_PARSING_DISALLOWED);
        return NULL;
    }
    struct URParseResult *urResult = parse_ur(DataParser(&payload));
    if (urResult->error_code != 0) {
        HandleURResultViaUSBFunc(urResult->error_message, strlen(urResult->error_message), PRS_PARSING_ERROR);
        return NULL;
    }
    UrViewType_t urViewType = {0, 0};
    urViewType.viewType = urResult->t;
    urViewType.urType = urResult->ur_type;
    UIDisplay_t data = {0};
    PubValueMsg(UI_MSG_PREPARE_RECEIVE_UR_USB, 0);
    handleURResult(urResult, urViewType, false);
}
