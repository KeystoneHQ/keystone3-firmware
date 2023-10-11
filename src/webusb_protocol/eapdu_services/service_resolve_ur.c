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
    EAPDUResponsePayload_t *payload = (EAPDUResponsePayload_t *)malloc(sizeof(EAPDUResponsePayload_t));
    payload->data = data;
    payload->dataLen = data_len;
    payload->status = status;
    SendEApduResponse(EAPDU_PROTOCOL_HEADER, CMD_RESOLVE_UR, payload);
    free(payload);
};

static void HandleSuccessFunc(const void *data, uint32_t data_len)
{
    BasicHandlerFunc(data, data_len, RSP_SUCCESS_CODE);
};

static void HandleFailureFunc(const void *data, uint32_t data_len)
{
    BasicHandlerFunc(data, data_len, RSP_FAILURE_CODE);
};

void HandleURResultViaUSBFunc(const void *data, uint32_t data_len, bool isSuccess)
{
    isSuccess ? HandleSuccessFunc(data, data_len) : HandleFailureFunc(data, data_len);
};

static uint8_t *DataParser(EAPDURequestPayload_t *payload)
{
    return payload->data;
}

void *ProcessURService(EAPDURequestPayload_t payload)
{
    struct URParseResult *urResult = parse_ur(DataParser(&payload));
    UrViewType_t urViewType = {0, 0};
    urViewType.viewType = urResult->t;
    urViewType.urType = urResult->ur_type;
    UIDisplay_t data = {0};
    PubValueMsg(UI_MSG_PREPARE_RECEIVE_UR_USB, 0);
    handleURResult(urResult, urViewType, false);
}
