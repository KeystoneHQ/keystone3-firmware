#include "service_resolve_ur.h"
#include "user_delay.h"
#include "gui_chain.h"
#include "user_msg.h"
#include "qrdecode_task.h"
#include "gui_lock_widgets.h"
#include "gui_resolve_ur.h"
#include "gui_views.h"
#include "general_msg.h"
#include "gui_key_derivation_request_widgets.h"

/* DEFINES */
#define REQUEST_ID_IDLE 0xFFFF

/* TYPEDEFS */

/* FUNC DECLARATION*/
static void BasicHandlerFunc(const void *data, uint32_t data_len, uint16_t requestID, StatusEnum status);
static bool CheckURAcceptable(void);
static void GotoFailPage(StatusEnum error_code, const char *error_message);
bool GuiIsSetup(void);

/* STATIC VARIABLES */
static uint16_t g_requestID = REQUEST_ID_IDLE;

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
    EXT_FREE(json_str);

    g_requestID = REQUEST_ID_IDLE;
    SRAM_FREE(payload);
};

static bool CheckURAcceptable(void)
{
    if (GuiLockScreenIsTop()) {
        const char *data = "Device is locked";
        HandleURResultViaUSBFunc(data, strlen(data), g_requestID, PRS_PARSING_DISALLOWED);
        return false;
    }
    if (GetMnemonicType() == MNEMONIC_TYPE_TON) {
        const char *data = "Ton wallet is not supported";
        HandleURResultViaUSBFunc(data, strlen(data), g_requestID, PRS_PARSING_DISALLOWED);
        return false;
    }
    // Only allow URL parsing on specific pages
    if (GuiIsSetup()) {
        const char *data = "Export address is just allowed on specific pages";
        HandleURResultViaUSBFunc(data, strlen(data), g_requestID, PRS_PARSING_DISALLOWED);
        return false;
    }
    return true;
}

static void GotoFailPage(StatusEnum error_code, const char *error_message)
{
    EAPDUResultPage_t *resultPage = (EAPDUResultPage_t *)SRAM_MALLOC(sizeof(EAPDUResultPage_t));
    resultPage->command = CMD_RESOLVE_UR;
    resultPage->error_code = error_code;
    resultPage->error_message = (char *)error_message;
    GotoResultPage(resultPage);
    HandleURResultViaUSBFunc(error_message, strlen(error_message), g_requestID, error_code);
    SRAM_FREE(resultPage);
}

void HandleURResultViaUSBFunc(const void *data, uint32_t data_len, uint16_t requestID, StatusEnum status)
{
    StatusEnum sendStatus = status;
    if (status == PRS_EXPORT_HARDWARE_CALL_SUCCESS) {
        sendStatus = RSP_SUCCESS_CODE;
    }

    BasicHandlerFunc(data, data_len, requestID, sendStatus);
    EAPDUResultPage_t *resultPage = (EAPDUResultPage_t *)SRAM_MALLOC(sizeof(EAPDUResultPage_t));
    resultPage->command = CMD_RESOLVE_UR;
    resultPage->error_code = status;
    resultPage->error_message = (char *)data;
    if (status == PRS_PARSING_DISALLOWED || status == PRS_PARSING_REJECTED || status == PRS_PARSING_VERIFY_PASSWORD_ERROR || status == PRS_EXPORT_HARDWARE_CALL_SUCCESS) {
        return;
    }
    GotoResultPage(resultPage);
    SRAM_FREE(resultPage);
};

uint16_t GetCurrentUSParsingRequestID()
{
    return g_requestID;
};

void ClearUSBRequestId(void)
{
    g_requestID = REQUEST_ID_IDLE;
}

static bool IsRequestAllowed(uint32_t requestID)
{
    if (g_requestID != REQUEST_ID_IDLE) {
        const char *data = "Previous request is not finished";
        HandleURResultViaUSBFunc(data, strlen(data), requestID, PRS_PARSING_DISALLOWED);
        return false;
    }
    
    if (!CheckURAcceptable()) {
        return false;
    }
    
    return true;
}

static void HandleHardwareCall(struct URParseResult *urResult)
{
    if (GuiCheckIfTopView(&g_keyDerivationRequestView) || GuiHomePageIsTop()) {
        GuiSetKeyDerivationRequestData(urResult, NULL, false);
        PubValueMsg(UI_MSG_USB_HARDWARE_VIEW, 0);
    }
    
    const char *data = "Export address is just allowed on specific pages";
    HandleURResultViaUSBFunc(data, strlen(data), g_requestID, PRS_PARSING_DISALLOWED);
    g_requestID = REQUEST_ID_IDLE;
}

static bool HandleNormalCall(void)
{
    if (GuiHomePageIsTop()) {
        return true;
    }
    
    if (GuiCheckIfTopView(&g_USBTransportView)) {
        PubValueMsg(UI_MSG_USB_TRANSPORT_NEXT_VIEW, 0);
        UserDelay(200);
        return true;
    }
    
    const char *data = "Export address is just allowed on specific pages";
    HandleURResultViaUSBFunc(data, strlen(data), g_requestID, PRS_PARSING_DISALLOWED);
    g_requestID = REQUEST_ID_IDLE;
    return false;
}

static void HandleCheckResult(PtrT_TransactionCheckResult checkResult, UrViewType_t urViewType)
{
    if (checkResult != NULL && checkResult->error_code == 0) {
        PubValueMsg(UI_MSG_PREPARE_RECEIVE_UR_USB, urViewType.viewType);
    } else if (checkResult != NULL &&
              (checkResult->error_code == MasterFingerprintMismatch ||
               checkResult->error_code == BitcoinNoMyInputs)) {
        const char *data = _("usb_transport_mismatched_wallet_desc");
        GotoFailPage(PRS_PARSING_MISMATCHED_WALLET, data);
    } else {
        GotoFailPage(PRS_PARSING_ERROR, checkResult->error_message);
    }
}

void ProcessURService(EAPDURequestPayload_t *payload)
{
#ifndef COMPILE_SIMULATOR
    struct URParseResult *urResult = NULL;
    PtrT_TransactionCheckResult checkResult = NULL;
    do {
        if (!IsRequestAllowed(payload->requestID)) {
            break;
        }
        g_requestID = payload->requestID;
        
        urResult = parse_ur((char *)payload->data);
        if (urResult->error_code != 0) {
            HandleURResultViaUSBFunc(urResult->error_message, strlen(urResult->error_message), g_requestID, PRS_PARSING_ERROR);
            break;
        }
        
        UrViewType_t urViewType = {
            .viewType = urResult->t,
            .urType = urResult->ur_type
        };
        
        if (urResult->ur_type == QRHardwareCall) {
            HandleHardwareCall(urResult);
            break;
        } else if (!HandleNormalCall()) {
            break;
        }
        
        if (!CheckViewTypeIsAllow(urViewType.viewType)) {
            const char *data = "this view type is not supported";
            HandleURResultViaUSBFunc(data, strlen(data), g_requestID, RSP_FAILURE_CODE);
            break;
        }
        
        HandleDefaultViewType(urResult, NULL, urViewType, false);
        checkResult = CheckUrResult(urViewType.viewType);
        HandleCheckResult(checkResult, urViewType);
    } while (0);

    if (urResult) {
        free_ur_parse_result(urResult);
    }
    if (checkResult) {
        free_TransactionCheckResult(checkResult);
    }
#endif
}