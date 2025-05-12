#include <string.h>
#include "keystore.h"
#include "define.h"
#include "service_check_lock.h"
#include "user_memory.h"
#include "version.h"
#include "gui_views.h"
#include "gui_model.h"
#include "account_public_info.h"
#include "user_utils.h"
#include "gui_cosmos.h"

#define ADD_CJSON_RESULT(root, errorCode, errorMessage) \
    cJSON *jsonItem = cJSON_CreateObject(); \
    cJSON_AddNumberToObject(jsonItem, "errorCode", errorCode); \
    cJSON_AddStringToObject(jsonItem, "errorMessage", errorMessage); \
    cJSON_AddItemToObject(root, "result", jsonItem);

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
static void UsbGetFirmwareChecksum(EAPDURequestPayload_t *payload);
static void UsbSetDeviceConfig(EAPDURequestPayload_t *payload);
static void UsbGetDeviceConfig(EAPDURequestPayload_t *payload);
static void UsbSetWalletIconName(EAPDURequestPayload_t *payload);
static void UsbSetPassphrase(EAPDURequestPayload_t *payload);
static void UsbWalletSeedCheck(EAPDURequestPayload_t *payload);
static void UsbWalletChangePassword(EAPDURequestPayload_t *payload);
static void UsbSetWalletPassword(EAPDURequestPayload_t *payload);

static USBTestFuncMap_t usbTestFuncMap[] = {
    {CMD_GET_DEVICE_CONFIG, UsbGetDeviceConfig},
    {CMD_SET_DEVICE_CONFIG, UsbSetDeviceConfig},
    {CMD_GET_FIRMWARE_CHECKSUM, UsbGetFirmwareChecksum},

    {CMD_GET_DEVICE_WALLET_AMOUNT, UsbGetCurrentDeviceWalletAmount},
    {CMD_GET_CURRENT_WALLET_INFO, UsbGetCurrentWalletInfo},
    {CMD_UNLOCK_DEVICE, UsbUnlockDevice},
    {CMD_CREATE_WALLET, UsbCreateWallet},
    {CMD_IMPORT_WALLET, UsbImportWallet},
    {CMD_DELETE_WALLET, UsbDeleteWallet},
    {CMD_GET_WALLET_ADDRESS, UsbGetWalletAddress},
    {CMD_SET_WALLET_ICON_NAME, UsbSetWalletIconName},
    {CMD_SET_PASSPHRASE, UsbSetPassphrase},
    {CMD_WALLET_SEED_CHECK, UsbWalletSeedCheck},
    {CMD_WALLET_CHANGE_PASSWORD, UsbWalletChangePassword},
    {CMD_SET_WALLET_PASSWORD, UsbSetWalletPassword},
};

static bool g_isUsbTest = false;
#define check_usb_test() if (!g_isUsbTest) { return; }

bool HandleUSBTestFunc(EAPDURequestPayload_t *payload)
{
    for (int i = 0; i < sizeof(usbTestFuncMap) / sizeof(usbTestFuncMap[0]); i++) {
        if (usbTestFuncMap[i].commandType == payload->commandType) {
            g_isUsbTest = true;
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

static void BuildBaseResponseWithResult(EAPDURequestPayload_t *payload, const char *key, const char *value, uint32_t commandType)
{
    if (payload == NULL) {
        payload = (EAPDURequestPayload_t *)SRAM_MALLOC(sizeof(EAPDURequestPayload_t));
        payload->commandType = commandType;
        payload->requestID = 0;
    }

    cJSON *root = cJSON_CreateObject();
    ADD_CJSON_RESULT(root, RSP_SUCCESS_CODE, "success");
    cJSON_AddStringToObject(root, key, value);
    char *jsonStr = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    printf("jsonStr = %s\n", jsonStr);
    cJSON_Delete(root);
    BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
    EXT_FREE(jsonStr);
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
    ADD_CJSON_RESULT(root, RSP_SUCCESS_CODE, "success");
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

    printf("payload->data = %s\n", (char *)payload->data);
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

        uint8_t entropy[ENTROPY_MAX_LEN];
        char entropyStr[128] = {0};
        uint8_t entropyLen = 0;
        SecretCacheSetPassword(passwordStr);
        GetAccountEntropy(currentAccountIndex, entropy, &entropyLen, passwordStr);
        printf("entropyLen = %d\n", entropyLen);
        ByteArrayToHexStr(entropy, entropyLen, entropyStr);
        printf("entropyStr = %s\n", entropyStr);
        cJSON_Delete(root);

        root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "iconIndex", iconIndex);
        cJSON_AddStringToObject(root, "walletName", walletName);
        cJSON_AddNumberToObject(root, "currentIndex", currentAccountIndex);
        snprintf_s(buffer, sizeof(buffer), "%02x%02x%02x%02x", mfp[0], mfp[1], mfp[2], mfp[3]);
        cJSON_AddStringToObject(root, "walletMFP", buffer);
        cJSON_AddStringToObject(root, "entropy", entropyStr);
    } while (0);
    
    ADD_CJSON_RESULT(root, RSP_SUCCESS_CODE, "success");
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

        char walletName[WALLET_NAME_MAX_LEN + 1];
        GetStringValue(root, "walletName", walletName, sizeof(walletName));
        if (strlen(walletName) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        printf("walletName = %s\n", walletName);

        uint8_t walletIconIndexValue = GetIntValue(root, "iconIndex", 0xFF);
        if (walletIconIndexValue == 0xFF) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        printf("walletIconIndexValue = %d\n", walletIconIndexValue);

        uint8_t wordCnt = GetIntValue(root, "wordCount", 0xFF);
        if (wordCnt == 0xFF) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char password[PASSWORD_MAX_LEN + 1];
        GetStringValue(root, "password", password, sizeof(password));
        if (strlen(password) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        GuiSetEmojiIconIndex(walletIconIndexValue);
        GuiSetCurrentKbWalletName(walletName);
        SecretCacheSetNewPassword(password);
        GuiModelBip39UpdateMnemonic(wordCnt);
        GuiModelWriteSe();
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

        char walletName[WALLET_NAME_MAX_LEN + 1];
        GetStringValue(root, "walletName", walletName, sizeof(walletName));
        printf("walletName = %s\n", walletName);
        if (strlen(walletName) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        GuiSetCurrentKbWalletName(walletName);

        uint8_t walletIconIndexValue = GetIntValue(root, "iconIndex", 0xFF);
        printf("walletIconIndexValue = %d\n", walletIconIndexValue);
        if (walletIconIndexValue == 0xFF) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char mnemonic[MNEMONIC_MAX_LEN + 1];
        GetStringValue(root, "mnemonic", mnemonic, sizeof(mnemonic));
        printf("mnemonic = %s\n", mnemonic);
        if (strlen(mnemonic) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        int wordCnt = GetIntValue(root, "wordCount", 0xFF);
        if (wordCnt == 0xFF) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char *password = cJSON_GetObjectItem(root, "password");
        if (password == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char *passwordStr = cJSON_GetStringValue(password);
        printf("password = %s\n", passwordStr);
        if (strlen(passwordStr) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        SecretCacheSetNewPassword(passwordStr);
        SecretCacheSetMnemonic(mnemonic);
        GuiSetEmojiIconIndex(walletIconIndexValue);
        Bip39Data_t bip39 = {
            .wordCnt = wordCnt,
            .forget = false,
        };
        GuiModelBip39CalWriteSe(bip39);
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
        printf("walletIndexValue = %d, currentAccountIndex = %d\n", walletIndexValue, currentAccountIndex);
        if (currentAccountIndex != walletIndexValue) {
            cJSON *result = cJSON_CreateObject();
            ADD_CJSON_RESULT(result, PRS_EXPORT_HARDWARE_CALL_SUCCESS, "can not delete other wallet");
            char *jsonStr = cJSON_PrintBuffered(result, BUFFER_SIZE_1024, false);
            printf("jsonStr = %s\n", jsonStr);
            cJSON_Delete(result);
            BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
            EXT_FREE(jsonStr);
            break;
        } else {
            GuiModelSettingDelWalletDesc();
            BuildBaseResponseWithResult(payload, "DeleteWallet", "success", CMD_DELETE_WALLET);
        }
    } while (0);
}

static char g_tempAddressBuff[BUFFER_SIZE_128] = {0};
static char *GetCurrentPathAddress(char *hdPath, char *rootPath, uint8_t chainType, uint32_t index)
{
    memset(g_tempAddressBuff, 0, sizeof(g_tempAddressBuff));
    char *xPub = GetCurrentAccountPublicKey(chainType);
    SimpleResponse_c_char *result = NULL;
    switch (chainType) {
        case XPUB_TYPE_BTC:
        case XPUB_TYPE_BTC_LEGACY:
        case XPUB_TYPE_BTC_NATIVE_SEGWIT:
        case XPUB_TYPE_BTC_TAPROOT:
        case XPUB_TYPE_LTC:
        case XPUB_TYPE_DASH:
        case XPUB_TYPE_BCH:
            result = utxo_get_address(hdPath, xPub);
            break;
        case XPUB_TYPE_ETH_BIP44_STANDARD:
        case XPUB_TYPE_ETH_LEDGER_LEGACY:
        case XPUB_TYPE_ETH_LEDGER_LIVE_0:
        case XPUB_TYPE_ETH_LEDGER_LIVE_1:
        case XPUB_TYPE_ETH_LEDGER_LIVE_2:
        case XPUB_TYPE_ETH_LEDGER_LIVE_3:
        case XPUB_TYPE_ETH_LEDGER_LIVE_4:
        case XPUB_TYPE_ETH_LEDGER_LIVE_5:
        case XPUB_TYPE_ETH_LEDGER_LIVE_6:
        case XPUB_TYPE_ETH_LEDGER_LIVE_7:
        case XPUB_TYPE_ETH_LEDGER_LIVE_8:
        case XPUB_TYPE_ETH_LEDGER_LIVE_9:
        case XPUB_TYPE_AVAX_BIP44_STANDARD:
            result = eth_get_address(hdPath, xPub, rootPath);
            break;
        case XPUB_TYPE_TRX:
            snprintf_s(hdPath, BUFFER_SIZE_128, "m/44'/195'/0'/0/%u", index);
            result = tron_get_address(hdPath, xPub);
            break;
        case XPUB_TYPE_SCRT:
        case XPUB_TYPE_IOV:
        case XPUB_TYPE_CRO:
        case XPUB_TYPE_KAVA:
        case XPUB_TYPE_TERRA:
        case XPUB_TYPE_XRP:
        case XPUB_TYPE_THOR:
            snprintf_s(hdPath, BUFFER_SIZE_128, "m/44'/118'/0'/0/%u", index);
            const CosmosChain_t *chain = GuiGetCosmosChain(chainType);
            result = cosmos_get_address(hdPath, xPub, rootPath, (char*)chain->prefix);
            break;
        case XPUB_TYPE_AVAX_X_P:
            snprintf_s(hdPath, sizeof(hdPath), "%s/0/%u", "m/44'/9000'/0'", index);
            strcpy_s(rootPath, sizeof(rootPath), "m/44'/9000'/0'");
            result = avalanche_get_x_p_address(hdPath, xPub, rootPath);
            break;
        case XPUB_TYPE_SOL_BIP44_0:
        case XPUB_TYPE_SOL_BIP44_1:
        case XPUB_TYPE_SOL_BIP44_2:
        case XPUB_TYPE_SOL_BIP44_3:
        case XPUB_TYPE_SOL_BIP44_4:
        case XPUB_TYPE_SOL_BIP44_5:
        case XPUB_TYPE_SOL_BIP44_6:
        case XPUB_TYPE_SOL_BIP44_7:
        case XPUB_TYPE_SOL_BIP44_8:
        case XPUB_TYPE_SOL_BIP44_9:
        case XPUB_TYPE_SOL_BIP44_10:
        case XPUB_TYPE_SOL_BIP44_11:
        case XPUB_TYPE_SOL_BIP44_12:
        case XPUB_TYPE_SOL_BIP44_13:
        case XPUB_TYPE_SOL_BIP44_14:
        case XPUB_TYPE_SOL_BIP44_15:
        case XPUB_TYPE_SOL_BIP44_16:
        case XPUB_TYPE_SOL_BIP44_17:
        case XPUB_TYPE_SOL_BIP44_18:
        case XPUB_TYPE_SOL_BIP44_19:
        case XPUB_TYPE_SOL_BIP44_20:
        case XPUB_TYPE_SOL_BIP44_21:
        case XPUB_TYPE_SOL_BIP44_22:
        case XPUB_TYPE_SOL_BIP44_23:
        case XPUB_TYPE_SOL_BIP44_24:
        case XPUB_TYPE_SOL_BIP44_25:
        case XPUB_TYPE_SOL_BIP44_26:
        case XPUB_TYPE_SOL_BIP44_27:
        case XPUB_TYPE_SOL_BIP44_28:
        case XPUB_TYPE_SOL_BIP44_29:
        case XPUB_TYPE_SOL_BIP44_30:
        case XPUB_TYPE_SOL_BIP44_31:
        case XPUB_TYPE_SOL_BIP44_32:
        case XPUB_TYPE_SOL_BIP44_33:
        case XPUB_TYPE_SOL_BIP44_34:
        case XPUB_TYPE_SOL_BIP44_35:
        case XPUB_TYPE_SOL_BIP44_36:
        case XPUB_TYPE_SOL_BIP44_37:
        case XPUB_TYPE_SOL_BIP44_38:
        case XPUB_TYPE_SOL_BIP44_39:
        case XPUB_TYPE_SOL_BIP44_40:
        case XPUB_TYPE_SOL_BIP44_41:
        case XPUB_TYPE_SOL_BIP44_42:
        case XPUB_TYPE_SOL_BIP44_43:
        case XPUB_TYPE_SOL_BIP44_44:
        case XPUB_TYPE_SOL_BIP44_45:
        case XPUB_TYPE_SOL_BIP44_46:
        case XPUB_TYPE_SOL_BIP44_47:
        case XPUB_TYPE_SOL_BIP44_48:
        case XPUB_TYPE_SOL_BIP44_49:
        case XPUB_TYPE_SOL_BIP44_ROOT:
        case XPUB_TYPE_SOL_BIP44_CHANGE_0:
        case XPUB_TYPE_SOL_BIP44_CHANGE_1:
        case XPUB_TYPE_SOL_BIP44_CHANGE_2:
        case XPUB_TYPE_SOL_BIP44_CHANGE_3:
        case XPUB_TYPE_SOL_BIP44_CHANGE_4:
        case XPUB_TYPE_SOL_BIP44_CHANGE_5:
        case XPUB_TYPE_SOL_BIP44_CHANGE_6:
        case XPUB_TYPE_SOL_BIP44_CHANGE_7:
        case XPUB_TYPE_SOL_BIP44_CHANGE_8:
        case XPUB_TYPE_SOL_BIP44_CHANGE_9:
        case XPUB_TYPE_SOL_BIP44_CHANGE_10:
        case XPUB_TYPE_SOL_BIP44_CHANGE_11:
        case XPUB_TYPE_SOL_BIP44_CHANGE_12:
        case XPUB_TYPE_SOL_BIP44_CHANGE_13:
        case XPUB_TYPE_SOL_BIP44_CHANGE_14:
        case XPUB_TYPE_SOL_BIP44_CHANGE_15:
        case XPUB_TYPE_SOL_BIP44_CHANGE_16:
        case XPUB_TYPE_SOL_BIP44_CHANGE_17:
        case XPUB_TYPE_SOL_BIP44_CHANGE_18:
        case XPUB_TYPE_SOL_BIP44_CHANGE_19:
        case XPUB_TYPE_SOL_BIP44_CHANGE_20:
        case XPUB_TYPE_SOL_BIP44_CHANGE_21:
        case XPUB_TYPE_SOL_BIP44_CHANGE_22:
        case XPUB_TYPE_SOL_BIP44_CHANGE_23:
        case XPUB_TYPE_SOL_BIP44_CHANGE_24:
        case XPUB_TYPE_SOL_BIP44_CHANGE_25:
        case XPUB_TYPE_SOL_BIP44_CHANGE_26:
        case XPUB_TYPE_SOL_BIP44_CHANGE_27:
        case XPUB_TYPE_SOL_BIP44_CHANGE_28:
        case XPUB_TYPE_SOL_BIP44_CHANGE_29:
        case XPUB_TYPE_SOL_BIP44_CHANGE_30:
        case XPUB_TYPE_SOL_BIP44_CHANGE_31:
        case XPUB_TYPE_SOL_BIP44_CHANGE_32:
        case XPUB_TYPE_SOL_BIP44_CHANGE_33:
        case XPUB_TYPE_SOL_BIP44_CHANGE_34:
        case XPUB_TYPE_SOL_BIP44_CHANGE_35:
        case XPUB_TYPE_SOL_BIP44_CHANGE_36:
        case XPUB_TYPE_SOL_BIP44_CHANGE_37:
        case XPUB_TYPE_SOL_BIP44_CHANGE_38:
        case XPUB_TYPE_SOL_BIP44_CHANGE_39:
        case XPUB_TYPE_SOL_BIP44_CHANGE_40:
        case XPUB_TYPE_SOL_BIP44_CHANGE_41:
        case XPUB_TYPE_SOL_BIP44_CHANGE_42:
        case XPUB_TYPE_SOL_BIP44_CHANGE_43:
        case XPUB_TYPE_SOL_BIP44_CHANGE_44:
        case XPUB_TYPE_SOL_BIP44_CHANGE_45:
        case XPUB_TYPE_SOL_BIP44_CHANGE_46:
        case XPUB_TYPE_SOL_BIP44_CHANGE_47:
        case XPUB_TYPE_SOL_BIP44_CHANGE_48:
        case XPUB_TYPE_SOL_BIP44_CHANGE_49:
            result = solana_get_address(xPub);
            break;
        case XPUB_TYPE_SUI_0:
        case XPUB_TYPE_SUI_1:
        case XPUB_TYPE_SUI_2:
        case XPUB_TYPE_SUI_3:
        case XPUB_TYPE_SUI_4:
        case XPUB_TYPE_SUI_5:
        case XPUB_TYPE_SUI_6:
        case XPUB_TYPE_SUI_7:
        case XPUB_TYPE_SUI_8:
        case XPUB_TYPE_SUI_9:
            result = sui_generate_address(xPub);
            break;
        case XPUB_TYPE_APT_0:
        case XPUB_TYPE_APT_1:
        case XPUB_TYPE_APT_2:
        case XPUB_TYPE_APT_3:
        case XPUB_TYPE_APT_4:
        case XPUB_TYPE_APT_5:
        case XPUB_TYPE_APT_6:
        case XPUB_TYPE_APT_7:
        case XPUB_TYPE_APT_8:
        case XPUB_TYPE_APT_9:
            result = aptos_generate_address(xPub);
            break;
        case XPUB_TYPE_ADA_0:
        case XPUB_TYPE_ADA_1:
        case XPUB_TYPE_ADA_2:
        case XPUB_TYPE_ADA_3:
        case XPUB_TYPE_ADA_4:
        case XPUB_TYPE_ADA_5:
        case XPUB_TYPE_ADA_6:
        case XPUB_TYPE_ADA_7:
        case XPUB_TYPE_ADA_8:
        case XPUB_TYPE_ADA_9:
        case XPUB_TYPE_ADA_10:
        case XPUB_TYPE_ADA_11:
        case XPUB_TYPE_ADA_12:
        case XPUB_TYPE_ADA_13:
        case XPUB_TYPE_ADA_14:
        case XPUB_TYPE_ADA_15:
        case XPUB_TYPE_ADA_16:
        case XPUB_TYPE_ADA_17:
        case XPUB_TYPE_ADA_18:
        case XPUB_TYPE_ADA_19:
        case XPUB_TYPE_ADA_20:
        case XPUB_TYPE_ADA_21:
        case XPUB_TYPE_ADA_22:
        case XPUB_TYPE_ADA_23:
        case XPUB_TYPE_LEDGER_ADA_0:
        case XPUB_TYPE_LEDGER_ADA_1:
        case XPUB_TYPE_LEDGER_ADA_2:
        case XPUB_TYPE_LEDGER_ADA_3:
        case XPUB_TYPE_LEDGER_ADA_4:
        case XPUB_TYPE_LEDGER_ADA_5:
        case XPUB_TYPE_LEDGER_ADA_6:
        case XPUB_TYPE_LEDGER_ADA_7:
        case XPUB_TYPE_LEDGER_ADA_8:
        case XPUB_TYPE_LEDGER_ADA_9:
        case XPUB_TYPE_LEDGER_ADA_10:
        case XPUB_TYPE_LEDGER_ADA_11:
        case XPUB_TYPE_LEDGER_ADA_12:
        case XPUB_TYPE_LEDGER_ADA_13:
        case XPUB_TYPE_LEDGER_ADA_14:
        case XPUB_TYPE_LEDGER_ADA_15:
        case XPUB_TYPE_LEDGER_ADA_16:
        case XPUB_TYPE_LEDGER_ADA_17:
        case XPUB_TYPE_LEDGER_ADA_18:
        case XPUB_TYPE_LEDGER_ADA_19:
        case XPUB_TYPE_LEDGER_ADA_20:
        case XPUB_TYPE_LEDGER_ADA_21:
        case XPUB_TYPE_LEDGER_ADA_22:
        case XPUB_TYPE_LEDGER_ADA_23:
            result = cardano_get_base_address(xPub, 0, 1);
            break;
        case XPUB_TYPE_ARWEAVE:
            result = arweave_get_address(xPub);
            break;
        case XPUB_TYPE_STELLAR_0:
        case XPUB_TYPE_STELLAR_1:
        case XPUB_TYPE_STELLAR_2:
        case XPUB_TYPE_STELLAR_3:
        case XPUB_TYPE_STELLAR_4:
            result = stellar_get_address(xPub);
            break;
        case XPUB_TYPE_TON_BIP39:
        case XPUB_TYPE_TON_NATIVE:
            result = ton_get_address(xPub);
            break;
        default:
            break;
    }
    if (result == NULL) {
        return NULL;
    }
    if (result->error_code == 0) {
        strcpy_s(g_tempAddressBuff, sizeof(g_tempAddressBuff), result->data);
    }
    free_simple_response_c_char(result);
    return g_tempAddressBuff;
}

static void UsbGetWalletAddress(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    printf("payload->data = %s\n", payload->data);
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        char hdPath[128] = {0};
        char rootPath[128] = {0};
        GetStringValue(root, "hdPath", hdPath, sizeof(hdPath));
        printf("hdPath = %s\n", hdPath);
        GetStringValue(root, "rootPath", rootPath, sizeof(rootPath));
        printf("rootPath = %s\n", rootPath);
        if (strlen(hdPath) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        int32_t chainTypeValue = GetIntValue(root, "chainType", 0xFF);
        printf("chainTypeValue = %d\n", chainTypeValue);
        if (chainTypeValue == 0xFF) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        int32_t index = GetIntValue(root, "index", 0xFF);
        printf("index = %d\n", index);
        if (index == 0xFF) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        cJSON *address = cJSON_CreateObject();
        cJSON_AddStringToObject(address, "address", GetCurrentPathAddress(hdPath, rootPath, chainTypeValue, index));
        char *jsonStr = cJSON_PrintBuffered(address, BUFFER_SIZE_1024, false);
        printf("jsonStr = %s\n", jsonStr);
        cJSON_Delete(address);
        EXT_FREE(jsonStr);
        BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }
}

static void UsbGetFirmwareChecksum(EAPDURequestPayload_t *payload)
{
    GuiModelCalculateCheckSum();
}

void UsdImportWalletResult(bool success, uint8_t newAccount)
{
    check_usb_test();
    uint8_t *entropy;
    uint32_t entropyLen;

    EAPDURequestPayload_t *payload = (EAPDURequestPayload_t *)SRAM_MALLOC(sizeof(EAPDURequestPayload_t));
    payload->commandType = CMD_IMPORT_WALLET;
    payload->requestID = 0;

    cJSON *root = cJSON_CreateObject();
    ADD_CJSON_RESULT(root, RSP_SUCCESS_CODE, "success");
    cJSON_AddNumberToObject(root, "newAccount", newAccount);
    char *jsonStr = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    printf("jsonStr = %s\n", jsonStr);
    cJSON_Delete(root);
    BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
    EXT_FREE(jsonStr);
    g_isUsbTest = false;
}

void UsdCreateWalletResult(bool success, uint8_t newAccount)
{
    check_usb_test();
    uint8_t *entropy;
    uint32_t entropyLen;
    char entropyStr[128] = {0};

    entropy = SecretCacheGetEntropy(&entropyLen);
    ByteArrayToHexStr(entropy, entropyLen, entropyStr);
    EAPDURequestPayload_t *payload = (EAPDURequestPayload_t *)SRAM_MALLOC(sizeof(EAPDURequestPayload_t));
    payload->commandType = CMD_CREATE_WALLET;
    payload->requestID = 0;

    cJSON *root = cJSON_CreateObject();
    ADD_CJSON_RESULT(root, RSP_SUCCESS_CODE, "success");
    cJSON_AddNumberToObject(root, "newAccount", newAccount);
    cJSON_AddStringToObject(root, "entropy", entropyStr);
    char *jsonStr = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    printf("jsonStr = %s\n", jsonStr);
    cJSON_Delete(root);
    BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
    EXT_FREE(jsonStr);
    g_isUsbTest = false;
}

void UsbGetFirmwareChecksumResult(void)
{
    check_usb_test();
    char hash[128] = {0};
    SecretCacheGetChecksum(hash);
    cJSON *root = cJSON_CreateObject();
    ADD_CJSON_RESULT(root, RSP_SUCCESS_CODE, "success");
    cJSON_AddStringToObject(root, "checksum", hash);
    char *jsonStr = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    printf("jsonStr = %s\n", jsonStr);
    cJSON_Delete(root);
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    result->data = (uint8_t *)jsonStr;
    result->dataLen = strlen((char *)result->data);
    result->status = RSP_SUCCESS_CODE;
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_GET_FIRMWARE_CHECKSUM;
    result->requestID = 0;
    SendEApduResponse(result);
    EXT_FREE(jsonStr);
    SRAM_FREE(result);
    g_isUsbTest = false;
}

void UsbUnlockDeviceSuccess(bool success)
{
    check_usb_test();
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "unlockState", success);
    uint8_t currentAccountIndex = GetCurrentAccountIndex();
    cJSON_AddNumberToObject(root, "currentIndex", currentAccountIndex);
    char *jsonStr = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    printf("jsonStr = %s\n", jsonStr);
    cJSON_Delete(root);
    result->data = (uint8_t *)jsonStr;
    result->dataLen = strlen((char *)result->data);
    result->status = RSP_SUCCESS_CODE;
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_UNLOCK_DEVICE;
    result->requestID = 0;

    SendEApduResponse(result);
    EXT_FREE(jsonStr);
    SRAM_FREE(result);
    g_isUsbTest = false;
}

static void UsbGetDeviceConfig(EAPDURequestPayload_t *payload)
{
    char *jsonStr = GetJsonStringFromDeviceSettings();
    BuildBaseResponse(payload, (uint8_t *)jsonStr, strlen(jsonStr));
    EXT_FREE(jsonStr);
}

static void UsbSetDeviceConfig(EAPDURequestPayload_t *payload)
{
    BuildBaseResponseWithResult(payload, "GetDeviceConfig", "success", CMD_GET_DEVICE_CONFIG);
}

static void UsbSetWalletIconName(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    int32_t ret = ERR_GENERAL_FAIL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        char name[128] = {0};
        GetStringValue(root, "walletName", name, sizeof(name));
        if (strlen(name) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        int32_t iconIndex = GetIntValue(root, "iconIndex", 0xFF);
        if (iconIndex == 0xFF) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        WalletDesc_t wallet = {
            .iconIndex = iconIndex,
        };
        strcpy_s(wallet.name, WALLET_NAME_MAX_LEN + 1, name);
        GuiModelSettingSaveWalletDesc(&wallet);
        ret = SUCCESS_CODE;
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }

    if (ret != SUCCESS_CODE) {
        BuildBaseResponseWithResult(payload, "SetWalletIconName", "failed", CMD_SET_WALLET_ICON_NAME);
    } else {
        BuildBaseResponseWithResult(payload, "SetWalletIconName", "success", CMD_SET_WALLET_ICON_NAME);
    }
}

static void UsbSetPassphrase(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    int32_t ret = ERR_GENERAL_FAIL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char *passphrase = cJSON_GetStringValue(cJSON_GetObjectItem(root, "passphrase"));
        if (strlen(passphrase) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        SecretCacheSetPassphrase(passphrase);
        GuiModelSettingWritePassphrase();
        ret = SUCCESS_CODE;
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }

    if (ret != SUCCESS_CODE) {
        BuildBaseResponseWithResult(payload, "SetPassphrase", "failed", CMD_SET_PASSPHRASE);
    }
}

static void UsbWalletSeedCheck(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    int32_t ret = ERR_GENERAL_FAIL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        
        char mnemonic[MNEMONIC_MAX_LEN + 1];
        GetStringValue(root, "mnemonic", mnemonic, sizeof(mnemonic));
        if (strlen(mnemonic) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }
        printf("mnemonic = %s\n", mnemonic);
        SecretCacheSetMnemonic(mnemonic);

        int wordCnt = GetIntValue(root, "wordCount", 0xFF);
        if (wordCnt == 0xFF) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        GuiModelBip39RecoveryCheck(wordCnt);
        ret = SUCCESS_CODE;
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }

    if (ret != SUCCESS_CODE) {
        UsdWalletSeedCheckResult(false);
    }
}

void UsdWalletSeedCheckResult(bool success)
{
    check_usb_test();
    BuildBaseResponseWithResult(NULL, "WalletSeedCheck", success ? "success" : "failed", CMD_WALLET_SEED_CHECK);
    g_isUsbTest = false;
}

static void UsbWalletChangePassword(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    int32_t ret = ERR_GENERAL_FAIL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char oldPassword[128] = {0};
        GetStringValue(root, "oldPassword", oldPassword, sizeof(oldPassword));
        if (strlen(oldPassword) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char newPassword[128] = {0};
        GetStringValue(root, "newPassword", newPassword, sizeof(newPassword));
        if (strlen(newPassword) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        printf("newPassword = %s\n", newPassword);
        printf("oldPassword = %s\n", oldPassword);

        SecretCacheSetNewPassword(newPassword);
        SecretCacheSetPassword(oldPassword);
        GuiModelChangeAccountPassWord();
        ret = SUCCESS_CODE;
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }

    if (ret != SUCCESS_CODE) {
        BuildBaseResponseWithResult(payload, "WalletChangePassword", "failed", CMD_WALLET_CHANGE_PASSWORD);
    }
}

void UsbWalletChangePasswordResult(bool success)
{
    check_usb_test();
    BuildBaseResponseWithResult(NULL, "WalletChangePassword", success ? "success" : "failed", CMD_WALLET_CHANGE_PASSWORD);
    g_isUsbTest = false;
}

static void UsbSetWalletPassword(EAPDURequestPayload_t *payload)
{
    cJSON *root = NULL;
    int32_t ret = ERR_GENERAL_FAIL;
    do {
        root = cJSON_Parse((char *)payload->data);
        if (root == NULL) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        char password[128] = {0};
        GetStringValue(root, "password", password, sizeof(password));
        if (strlen(password) == 0) {
            SendEApduResponseError(EAPDU_PROTOCOL_HEADER, payload->commandType, payload->requestID, PRS_EXPORT_ADDRESS_INVALID_PARAMS, "Invalid params");
            break;
        }

        printf("password = %s\n", password);

        SecretCacheSetPassword(password);
        ret = SUCCESS_CODE;
    } while (0);
    if (root != NULL) {
        cJSON_Delete(root);
    }

    if (ret != SUCCESS_CODE) {
        BuildBaseResponseWithResult(payload, "SetWalletPassword", "failed", CMD_SET_WALLET_PASSWORD);
    } else {
        BuildBaseResponseWithResult(payload, "SetWalletPassword", "success", CMD_SET_WALLET_PASSWORD);
    }
}
