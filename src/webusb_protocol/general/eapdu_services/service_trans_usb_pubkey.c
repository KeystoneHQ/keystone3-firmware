#include "service_echo_test.h"
#include "eapdu_protocol_parser.h"
#include "utils/define.h"
#include "cJSON.h"
#include "account_public_info.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "user_memory.h"

static char* ParseSolDerivationPath(const uint8_t *data, uint32_t len) 
{
    if (data == NULL || len < 1) {
        return NULL;
    }
    
    uint8_t depth = data[0];
    printf("depth = %u.......\n", depth);
    uint32_t expectedLen = 1 + (depth * 4);
    
    if (len < expectedLen) {
        return NULL;
    }
    
    char *path = (char *)SRAM_MALLOC(BUFFER_SIZE_32);
    if (path == NULL) {
        return NULL;
    }
    
    // Initialize path as empty string (no "m/" prefix)
    path[0] = '\0';
    
    for (uint8_t i = 0; i < depth; i++) {
        uint32_t offset = 1 + (i * 4);
        
        // Parse 4-byte component in big-endian format
        uint32_t component = ((uint32_t)data[offset] << 24) |
                           ((uint32_t)data[offset + 1] << 16) |
                           ((uint32_t)data[offset + 2] << 8) |
                           ((uint32_t)data[offset + 3]);
        
        printf("component = %u.......\n", component);
        bool isHardened = (component & 0x80000000) != 0;
        
        if (isHardened) {
            component &= 0x7FFFFFFF;
            if (strlen(path) == 0) {
                // First component, don't add leading slash
                snprintf(path, BUFFER_SIZE_32, "%u'", component);
            } else {
                // Subsequent components, add leading slash
                snprintf(path + strlen(path), BUFFER_SIZE_32 - strlen(path), "/%u'", component);
            }
        } else {
            if (strlen(path) == 0) {
                // First component, don't add leading slash
                snprintf(path, BUFFER_SIZE_32, "%u", component);
            } else {
                // Subsequent components, add leading slash
                snprintf(path + strlen(path), BUFFER_SIZE_32 - strlen(path), "/%u", component);
            }
        }
    }
    
    return path;
}

void GetDeviceUsbPubkeyService(EAPDURequestPayload_t *payload)
{
    EAPDUResponsePayload_t *result = NULL;
    cJSON *root = NULL;
    char *path = NULL;
    char *json_str = NULL;
    
    result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    if (result == NULL) {
        printf("Failed to allocate result structure\n");
        goto cleanup;
    }
    
    root = cJSON_CreateObject();
    if (root == NULL) {
        printf("Failed to create JSON object\n");
        goto cleanup;
    }
    
    path = ParseSolDerivationPath(payload->data, payload->dataLen);
    if (path == NULL) {
        cJSON_AddStringToObject(root, "error", "Failed to parse derivation path");
        goto create_response;
    }
    
    ChainType pubkeyIndex = CheckSolPathSupport(path);
    if (pubkeyIndex == XPUB_TYPE_NUM) {
        cJSON_AddStringToObject(root, "error", "Unsupported derivation path");
        goto create_response;
    }
    
    char *pubKey = GetCurrentAccountPublicKey(pubkeyIndex);
    if (pubKey == NULL) {
        cJSON_AddStringToObject(root, "error", "Failed to get public key");
        goto create_response;
    }
    
    cJSON_AddStringToObject(root, "pubkey", pubKey);
    cJSON_AddStringToObject(root, "derivationPath", path);
    
create_response:
    json_str = cJSON_PrintBuffered(root, BUFFER_SIZE_1024, false);
    if (json_str == NULL) {
        printf("Failed to generate JSON string\n");
        goto cleanup;
    }
    
    result->data = (uint8_t *)json_str;
    result->dataLen = strlen((char *)result->data);
    result->status = RSP_SUCCESS_CODE;
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_GET_DEVICE_USB_PUBKEY;
    result->requestID = payload->requestID;
    
    SendEApduResponse(result);
    
cleanup:
    if (json_str != NULL) {
        EXT_FREE(json_str);
    }
    if (root != NULL) {
        cJSON_Delete(root);
    }
    if (path != NULL) {
        SRAM_FREE(path);
    }
    if (result != NULL) {
        SRAM_FREE(result);
    }
}