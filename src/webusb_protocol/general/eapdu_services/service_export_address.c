#include "service_export_address.h"
#include "user_msg.h"
#include "user_utils.h"
#include "gui.h"
#include "gui_lock_widgets.h"
#include "gui_home_widgets.h"
// #include "gui_lock_widgets.h"

/* DEFINES */

/* TYPEDEFS */
enum Chain {
    ETH,
};

enum {
    OFFSET_CHAIN = 0,
    OFFSET_WALLET = 2,
    OFFSET_TYPE = 4
};

struct EthParams {
    uint8_t n;
    ETHAccountType type;
    uint8_t chain;
    uint8_t wallet;
};

typedef struct {
    uint16_t requestID;
    uint8_t n;
    uint8_t wallet;
    ETHAccountType type;
} ExportAddressParams_t;

/* FUNC DECLARATION*/
static void ExportEthAddress(uint16_t requestID, uint8_t n, ETHAccountType type);

/* STATIC VARIABLES */
static ExportAddressParams_t *g_exportAddressParams = NULL;

static struct EthParams *NewParams()
{
    struct EthParams *params = (struct EthParams *)SRAM_MALLOC(sizeof(struct EthParams));
    params->n = -1;
    params->type = -1;
    params->chain = -1;
    params->wallet = -1;
    return params;
}

static bool IsValidParams(struct EthParams *params)
{
    if (params->n == -1 || params->type == -1 || params->chain == -1 || params->wallet == -1) {
        return false;
    }
    return true;
}

static struct EthParams *ParseParams(char *data)
{
    struct EthParams *params = NewParams();
    params->n = 0;
    params->chain = (uint8_t)extract_16bit_value(data, OFFSET_CHAIN);
    params->wallet = (uint8_t)extract_16bit_value(data, OFFSET_WALLET);
    params->type = (uint8_t)extract_16bit_value(data, OFFSET_TYPE);

    return params;
}

uint8_t GetExportWallet()
{
    if (g_exportAddressParams == NULL) {
        return DEFAULT;
    }
    return g_exportAddressParams->wallet;
}

void ExportAddressApprove()
{
    ExportEthAddress(g_exportAddressParams->requestID, g_exportAddressParams->n, g_exportAddressParams->type);
    SRAM_FREE(g_exportAddressParams);
    g_exportAddressParams = NULL;
}

void ExportAddressReject()
{
    SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, g_exportAddressParams->requestID, PRS_EXPORT_ADDRESS_REJECTED, "Export address is rejected");
    SRAM_FREE(g_exportAddressParams);
    g_exportAddressParams = NULL;
}

static void ExportEthAddress(uint16_t requestID, uint8_t n, ETHAccountType type)
{
#ifndef COMPILE_SIMULATOR
    UREncodeResult *urResult = GetUnlimitedMetamaskDataForAccountType(type);

    if (urResult->error_code != 0) {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, requestID, PRS_EXPORT_ADDRESS_ERROR, urResult->error_message);
        return;
    }

    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "payload", urResult->data);
    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    result->data = (uint8_t *)json_str;
    result->dataLen = strlen((char *)result->data);
    result->status = RSP_SUCCESS_CODE;
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_EXPORT_ADDRESS;
    result->requestID = requestID;

    SendEApduResponse(result);
    EXT_FREE(json_str);
    SRAM_FREE(result);
#endif
}

static bool CheckExportAcceptable(EAPDURequestPayload_t *payload)
{
    if (GuiLockScreenIsTop()) {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload->requestID, PRS_EXPORT_ADDRESS_DISALLOWED, "Export address is not allowed when the device is locked");
        return false;
    }
    // Only allow on specific pages
    if (!GuiHomePageIsTop()) {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload->requestID, PRS_EXPORT_ADDRESS_DISALLOWED, "Export address is just allowed on specific pages");
        return false;
    }
    return true;
}

void ExportAddressService(EAPDURequestPayload_t *payload)
{
    if (!CheckExportAcceptable(payload)) {
        return;
    }

    if (g_exportAddressParams != NULL) {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload->requestID, PRS_EXPORT_ADDRESS_BUSY, "Export address is busy, please try again later");
        SRAM_FREE(g_exportAddressParams);
        g_exportAddressParams = NULL;
        return;
    }

    struct EthParams *params = ParseParams((char *)payload->data);
    if (!IsValidParams(params)) {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
        return;
    }

    if (params->chain == ETH) {
        g_exportAddressParams = (ExportAddressParams_t *)SRAM_MALLOC(sizeof(ExportAddressParams_t));
        g_exportAddressParams->requestID = payload->requestID;
        g_exportAddressParams->n = params->n;
        g_exportAddressParams->type = params->type;
        g_exportAddressParams->wallet = params->wallet;

        EAPDUResultPage_t *resultPage = (EAPDUResultPage_t *)SRAM_MALLOC(sizeof(EAPDUResultPage_t));
        resultPage->command = CMD_EXPORT_ADDRESS;
        resultPage->error_code = 0;
        resultPage->error_message = "";
        GotoResultPage(resultPage);
        SRAM_FREE(resultPage);
    } else {
        SendEApduResponseError(EAPDU_PROTOCOL_HEADER, CMD_EXPORT_ADDRESS, payload->requestID, PRS_EXPORT_ADDRESS_UNSUPPORTED_CHAIN, "Unsupported chain");
    }

    SRAM_FREE(params);
}