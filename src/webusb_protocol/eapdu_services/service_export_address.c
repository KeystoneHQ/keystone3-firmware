#include "service_export_address.h"

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

static void ExportEthAddress(EAPDURequestPayload_t payload, uint8_t n, ETHAccountType type)
{
    UREncodeResult *urResult = GetMetamaskDataForAccountType(type);

    if (urResult->error_code != 0)
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload.requestID, PRS_EXPORT_ADDRESS_ERROR, urResult->error_message);
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
    result->requestID = payload.requestID;

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
    // TODO: Only allow on specific pages
    return true;
}

void *ExportAddressService(EAPDURequestPayload_t payload)
{
    if (!CheckExportAcceptable(payload))
    {
        return NULL;
    }

    struct EthParams *params = ParseParams(payload.data);

    if (!IsValidParams(params))
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload.requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
    }

    if (params->chain == ETH)
    {
        ExportEthAddress(payload, params->n, params->type);
    }
    else
    {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload.requestID, PRS_UNSUPPORTED_CHAIN, "Unsupported chain");
    }

    SRAM_FREE(params);
}