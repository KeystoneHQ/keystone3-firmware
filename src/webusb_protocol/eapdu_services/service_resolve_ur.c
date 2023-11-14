#include "service_resolve_ur.h";
#include "user_delay.h";
#include "gui_chain.h";
#include "user_msg.h";

#define REQUEST_ID_IDLE 0
static uint16_t g_requestID = REQUEST_ID_IDLE;

typedef struct
{
    uint8_t viewType;
    uint8_t urType;
} UrViewType_t;

static void BasicHandlerFunc(const void *data, uint32_t data_len, uint16_t requestID, StatusEnum status)
{
    EAPDUResponsePayload_t *payload = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "payload", (char *)data);
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    payload->data = (uint8_t *)json_str;
    payload->dataLen = strlen((char *)payload->data);
    payload->status = status;
    payload->cla = EAPDU_PROTOCOL_HEADER;
    payload->commandType = CMD_RESOLVE_UR;
    payload->requestID = requestID;

    SendEApduResponse(payload);

    g_requestID = REQUEST_ID_IDLE;
    SRAM_FREE(payload);
};

uint16_t GetCurrentUSParsingRequestID()
{
    return g_requestID;
};

void HandleURResultViaUSBFunc(const void *data, uint32_t data_len, uint16_t requestID, StatusEnum status)
{
    BasicHandlerFunc(data, data_len, requestID, status);
    EAPDUResultPage_t *resultPage = (EAPDUResultPage_t *)SRAM_MALLOC(sizeof(EAPDUResultPage_t));
    resultPage->command = CMD_RESOLVE_UR;
    resultPage->error_code = status;
    resultPage->error_message = (char *)data;
    if (status == PRS_PARSING_DISALLOWED)
    {
        return NULL;
    }
    GotoResultPage(resultPage);
};

static uint8_t *DataParser(EAPDURequestPayload_t *payload)
{
    return payload->data;
}

static bool CheckURAcceptable(EAPDURequestPayload_t payload)
{
    char *data = "";
    if (GuiLockScreenIsTop())
    {
        data = "Device is locked";
        HandleURResultViaUSBFunc(data, strlen(data), g_requestID, PRS_PARSING_DISALLOWED);
        return false;
    }
    // Only allow URL parsing on specific pages
    if (!GuiHomePageIsTop())
    {
        data = "Export address is just allowed on specific pages";
        HandleURResultViaUSBFunc(data, strlen(data), g_requestID, PRS_PARSING_DISALLOWED);
        return false;
    }
    return true;
}

void *ProcessURService(EAPDURequestPayload_t payload)
{
    if (g_requestID != REQUEST_ID_IDLE)
    {
        const char *data = "Previous request is not finished";
        HandleURResultViaUSBFunc(data, strlen(data), payload.requestID, PRS_PARSING_DISALLOWED);
        return NULL;
    }
    else
    {
        g_requestID = payload.requestID;
    }

    if (!CheckURAcceptable(payload))
    {
        return NULL;
    }
    struct URParseResult *urResult = parse_ur(DataParser(&payload));
    if (urResult->error_code != 0)
    {
        HandleURResultViaUSBFunc(urResult->error_message, strlen(urResult->error_message), g_requestID, PRS_PARSING_ERROR);
        return NULL;
    }
    UrViewType_t urViewType = {0, 0};
    urViewType.viewType = urResult->t;
    urViewType.urType = urResult->ur_type;
    UIDisplay_t data = {0};
    HandleDefaultViewType(urResult, urViewType, false);
    PtrT_TransactionCheckResult checkResult = CheckUrResult(urViewType.viewType);
    if (checkResult != NULL && checkResult->error_code == 0)
    {
        PubValueMsg(UI_MSG_PREPARE_RECEIVE_UR_USB, urViewType.viewType);
    } else {
        EAPDUResultPage_t *resultPage = (EAPDUResultPage_t *)SRAM_MALLOC(sizeof(EAPDUResultPage_t));
        resultPage->command = CMD_RESOLVE_UR;
        resultPage->error_code = checkResult->error_code;
        resultPage->error_message = checkResult->error_message;
        GotoResultPage(resultPage);
        HandleURResultViaUSBFunc(checkResult->error_message, strlen(checkResult->error_message), g_requestID, PRS_PARSING_ERROR);
        SRAM_FREE(resultPage);
    }
}

