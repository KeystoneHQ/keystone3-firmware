#include "service_export_address.h"
#include "user_msg.h";

static void ExportEthAddress(uint16_t requestID, uint8_t n, ETHAccountType type);

enum Chain
{
    ETH,
};

struct EthParams
{
    uint8_t n;
    ETHAccountType type;
    uint8_t chain;
};

static struct EthParams *NewParams()
{
    struct EthParams *params = (struct EthParams *)SRAM_MALLOC(sizeof(struct EthParams));
    params->n = -1;
    params->type = -1;
    params->chain = -1;
    return params;
}

static bool IsValidParams(struct EthParams *params)
{
    if (params->n == -1 || params->type == -1 || params->chain == -1)
    {
        return false;
    }
    return true;
}

static struct EthParams *ParseParams(char *data)
{
    cJSON *json = cJSON_Parse(data);
    struct EthParams *params = NewParams();

    cJSON *n = cJSON_GetObjectItem(json, "n");
    cJSON *type = cJSON_GetObjectItem(json, "type");
    cJSON *chain = cJSON_GetObjectItem(json, "chain");

    if (n != NULL && n->type == cJSON_Number)
    {
        params->n = n->valueint;
    }
    if (type != NULL && type->type == cJSON_Number)
    {
        params->type = type->valueint;
    }
    if (chain != NULL && chain->type == cJSON_Number)
    {
        params->chain = chain->valueint;
    }
    cJSON_Delete(json);
    return params;
}

typedef struct
{
    uint16_t requestID;
    uint8_t n;
    ETHAccountType type;
} ExportAddressParams_t;

static ExportAddressParams_t *g_exportAddressParams = NULL;

void ExportAddressApprove()
{
    ExportEthAddress(g_exportAddressParams->requestID, g_exportAddressParams->n, g_exportAddressParams->type);
    SRAM_FREE(g_exportAddressParams);
    g_exportAddressParams = NULL;
    GuiCLoseCurrentWorkingView();
}

void ExportAddressReject()
{
    SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, g_exportAddressParams->requestID, PRS_EXPORT_ADDRESS_REJECTED, "Export address is rejected");
    SRAM_FREE(g_exportAddressParams);
    g_exportAddressParams = NULL;
    GuiCLoseCurrentWorkingView();
}

static void ExportEthAddress(uint16_t requestID, uint8_t n, ETHAccountType type)
{
    UREncodeResult *urResult = GetUnlimitedMetamaskDataForAccountType(type);

    if (urResult->error_code != 0)
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, requestID, PRS_EXPORT_ADDRESS_ERROR, urResult->error_message);
        return;
    }

    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "payload", urResult->data);
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    result->data = (uint8_t *)json_str;
    result->dataLen = strlen(result->data);
    result->status = RSP_SUCCESS_CODE;
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_EXPORT_ADDRESS;
    result->requestID = requestID;

    SendEApduResponse(result);

    SRAM_FREE(result);
}

static bool CheckExportAcceptable(EAPDURequestPayload_t payload)
{
    if (GuiLockScreenIsTop())
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload.requestID, PRS_EXPORT_ADDRESS_DISALLOWED, "Export address is not allowed when the device is locked");
        return false;
    }
    // Only allow on specific pages
    if (!GuiHomePageIsTop())
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload.requestID, PRS_EXPORT_ADDRESS_DISALLOWED, "Export address is just allowed on specific pages");
        return false;
    }
    return true;
}

void *ExportAddressService(EAPDURequestPayload_t payload)
{
    if (!CheckExportAcceptable(payload))
    {
        return NULL;
    }

    if (g_exportAddressParams != NULL)
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload.requestID, PRS_EXPORT_ADDRESS_BUSY, "Export address is busy, please try again later");
        ExportAddressReject();
        return NULL;
    }

    struct EthParams *params = ParseParams(payload.data);

    if (!IsValidParams(params))
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload.requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
        return NULL;
    }

    if (params->chain == ETH)
    {
        g_exportAddressParams = (ExportAddressParams_t *)SRAM_MALLOC(sizeof(ExportAddressParams_t));
        g_exportAddressParams->requestID = payload.requestID;
        g_exportAddressParams->n = params->n;
        g_exportAddressParams->type = params->type;
        PubValueMsg(UI_MSG_USB_TRANSPORT_VIEW, 0);
    }
    else
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload.requestID, PRS_EXPORT_ADDRESS_UNSUPPORTED_CHAIN, "Unsupported chain");
    }

    SRAM_FREE(params);
}