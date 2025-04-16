#include <string.h>
#include "keystore.h"
#include "define.h"
#include "service_check_lock.h"
#include "user_memory.h"
#include "version.h"
#include "gui_views.h"
#include "gui_model.h"

typedef void (*HandleUSBTestFunc_t)(EAPDURequestPayload_t *payload);

typedef struct {
    uint32_t commandType;
    HandleUSBTestFunc_t handleFunc;
} USBTestFuncMap_t;

static void UsbGetCurrentDeviceWalletAmount(EAPDURequestPayload_t *payload);
static void UsbGetCurrentWalletInfo(EAPDURequestPayload_t *payload);
static void UsbUnlockDevice(EAPDURequestPayload_t *payload);
static void UsbCreateWallet(EAPDURequestPayload_t *payload);
static void UsbImportWallet(EAPDURequestPayload_t *payload);
static void UsbDeleteWallet(EAPDURequestPayload_t *payload);
static void UsbGetWalletAddress(EAPDURequestPayload_t *payload);

static USBTestFuncMap_t usbTestFuncMap[] = {
    {CMD_GET_DEVICE_WALLET_AMOUNT, UsbGetCurrentDeviceWalletAmount},
    {CMD_GET_CURRENT_WALLET_INFO, UsbGetCurrentWalletInfo},
    {CMD_UNLOCK_DEVICE, UsbUnlockDevice},
    {CMD_CREATE_WALLET, UsbCreateWallet},
    {CMD_IMPORT_WALLET, UsbImportWallet},
    {CMD_DELETE_WALLET, UsbDeleteWallet},
    {CMD_GET_WALLET_ADDRESS, UsbGetWalletAddress},
};

bool HandleUSBTestFunc(EAPDURequestPayload_t *payload)
{
    for (int i = 0; i < sizeof(usbTestFuncMap) / sizeof(usbTestFuncMap[0]); i++) {
        if (usbTestFuncMap[i].commandType == payload->commandType) {
            usbTestFuncMap[i].handleFunc(payload);
            return true;
        }
    }
    return false;
}

static void BuildBaseResponse(EAPDURequestPayload_t *payload, uint8_t *data, uint32_t dataLen)
{
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    result->data = data;
    result->dataLen = dataLen;
    result->status = RSP_SUCCESS_CODE;
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = payload->commandType;
    result->requestID = payload->requestID;

    SendEApduResponse(result);
    SRAM_FREE(result);
}

static void UsbGetCurrentDeviceWalletAmount(EAPDURequestPayload_t *payload)
{
    uint8_t walletAmount;
    GetExistAccountNum(&walletAmount);
    uint8_t blankAccountIndex;
    GetBlankAccountIndex(&blankAccountIndex);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "walletAmount", walletAmount);
    cJSON_AddNumberToObject(root, "blankAccountIndex", blankAccountIndex);
    char *jsonStr = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    printf("json_str = %s\n", jsonStr);
    cJSON_Delete(root);
    BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
    EXT_FREE(jsonStr);
}

static void UsbGetCurrentWalletInfo(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    uint8_t iconIndex = 0;
    uint8_t currentAccountIndex = GetCurrentAccountIndex();
    char walletName[WALLET_NAME_MAX_LEN + 1];
    strcpy_s(walletName, WALLET_NAME_MAX_LEN + 1, GetWalletName());
    uint8_t mfp[4] = {0};
    char buffer[BUFFER_SIZE_32] = {0};
    GetMasterFingerPrint(mfp);
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "iconIndex", iconIndex);
    cJSON_AddStringToObject(root, "walletName", walletName);
    cJSON_AddNumberToObject(root, "currentWalletIndex", currentAccountIndex);
    snprintf_s(buffer, sizeof(buffer), "%02x%02x%02x%02x", mfp[0], mfp[1], mfp[2], mfp[3]);
    cJSON_AddStringToObject(root, "walletMFP", buffer);
    char *jsonStr = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    printf("jsonStr = %s\n", jsonStr);
    cJSON_Delete(root);
    BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
    EXT_FREE(jsonStr);
}

static void UsbUnlockDevice(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char *password = cJSON_GetObjectItem(root, "password");
        if (password == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char *passwordStr = cJSON_GetStringValue(password);
        printf("password = ..%s..\n", passwordStr);
        static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
        SecretCacheSetPassword(passwordStr);
        GuiModelVerifyAccountPassWord(&single);
        BuildBaseResponse(payload, NULL, 0);
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }
}

static void UsbCreateWallet(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        cJSON *walletName = cJSON_GetObjectItem(root, "walletName");
        if (walletName == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char *walletNameStr = cJSON_GetStringValue(walletName);
        printf("walletName = %s\n", walletNameStr);

        cJSON *walletIconIndex = cJSON_GetObjectItem(root, "walletIconIndex");
        if (walletIconIndex == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        uint8_t walletIconIndexValue = cJSON_GetNumberValue(walletIconIndex);
    
        BuildBaseResponse(payload, NULL, 0);
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }
}

static void UsbImportWallet(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        cJSON *walletName = cJSON_GetObjectItem(root, "walletName");
        if (walletName == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char *walletNameStr = cJSON_GetStringValue(walletName);
        printf("walletName = %s\n", walletNameStr);

        cJSON *walletIconIndex = cJSON_GetObjectItem(root, "walletIconIndex");
        if (walletIconIndex == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        uint8_t walletIconIndexValue = cJSON_GetNumberValue(walletIconIndex);

        BuildBaseResponse(payload, NULL, 0);
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }
}

static void UsbDeleteWallet(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        cJSON *walletIndex = cJSON_GetObjectItem(root, "walletIndex");
        if (walletIndex == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        uint8_t walletIndexValue = cJSON_GetNumberValue(walletIndex);
        // todo delete target wallet
        uint8_t currentAccountIndex = GetCurrentAccountIndex();
        if (currentAccountIndex != walletIndexValue) {
            cJSON *result = cJSON_CreateObject();
            cJSON_AddStringToObject(result, "result", "can not delete other wallet");
            char *jsonStr = cJSON_PrintBuffered(result, BUFFER_SIZE_1024, false);
            printf("jsonStr = %s\n", jsonStr);
            cJSON_Delete(result);
            BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
            EXT_FREE(jsonStr);
            break;
        } else {
            BuildBaseResponse(payload, NULL, 0);
        }
    } while (0);
}

static void UsbGetWalletAddress(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        cJSON *path = cJSON_GetObjectItem(root, "hdPath");
        if (path == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        char *hdPath = cJSON_GetStringValue(path);
        printf("hdPath = %s\n", hdPath);

        cJSON *chainType = cJSON_GetObjectItem(root, "chainType");
        if (chainType == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        uint8_t chainTypeValue = cJSON_GetNumberValue(chainType);
        char *xPub = GetCurrentAccountPublicKey(chainTypeValue);
        printf("xPub = %s\n", xPub);
        SimpleResponse_c_char *result = utxo_get_address(hdPath, xPub);

        cJSON *address = cJSON_CreateObject();
        cJSON_AddStringToObject(address, "address", result->data);
        char *jsonStr = cJSON_PrintBuffered(address, BUFFER_SIZE_1024, false);
        printf("jsonStr = %s\n", jsonStr);
        cJSON_Delete(address);
        EXT_FREE(jsonStr);
        BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
        free_simple_response_c_char(result);
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }
}
