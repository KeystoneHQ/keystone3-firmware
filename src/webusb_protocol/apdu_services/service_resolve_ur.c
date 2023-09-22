#include "service_resolve_ur.h";
#include "user_delay.h";
#include "gui_chain.h";
#include "gui_views.h";

typedef struct
{
    uint8_t viewType;
    uint8_t urType;
} UrViewType_t;

static void BasicHandlerFunc(const void *data, uint32_t data_len, StatusEnum status)
{
    if (g_handleURCallback != NULL)
    {
        APDUResponsePayload_t *payload = (APDUResponsePayload_t *)malloc(sizeof(APDUResponsePayload_t));
        payload->data = (uint8_t *)data;
        payload->dataLen = data_len;
        payload->status = status;
        g_handleURCallback(APDU_PROTOCOL_HEADER, CMD_RESOLVE_UR, payload);
    }
};

static void HandleSuccessFunc(const void *data, uint32_t data_len)
{
    BasicHandlerFunc(data, data_len, RSP_SUCCESS_CODE);
};

static void HandleFailureFunc(const void *data, uint32_t data_len)
{
    BasicHandlerFunc(data, data_len, RSP_FAILURE_CODE);
};

static void HandleResultFunc(const void *data, uint32_t data_len, bool isSuccess)
{
    isSuccess ? HandleSuccessFunc(data, data_len) : HandleFailureFunc(data, data_len);
};

static uint8_t * DataParser(APDURequestPayload_t *payload)
{
    return payload->data;
}

void *ProcessUREvents(APDURequestPayload_t *payload, ResponseHandler *sendResponse)
{
    g_handleURCallback = sendResponse;

    struct URParseResult *urResult = parse_ur(DataParser(payload));
    UrViewType_t urViewType = {0, 0};
    urViewType.viewType = urResult->t;
    urViewType.urType = urResult->ur_type;
    GuiFrameOpenViewWithParam(&g_USBTransportView, HandleResultFunc, sizeof(HandleResultFunc));
    UserDelay(500);
    handleURResult(urResult, urViewType, false);
}
