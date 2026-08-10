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

#define COIN_TYPE_SIZE 4
#define SOLANA_COIN_TYPE 501U
#define SOLANA_MAX_DERIVATION_DEPTH 4U

static bool ParseCoinType(const uint8_t *data, uint32_t len, uint32_t *coinType)
{
    if (data == NULL || coinType == NULL || len < COIN_TYPE_SIZE) {
        return false;
    }

    *coinType = ((uint32_t)data[0] << 24) |
                ((uint32_t)data[1] << 16) |
                ((uint32_t)data[2] << 8) |
                ((uint32_t)data[3]);
    return true;
}

#ifdef WEB3_VERSION
static bool ParseSolDerivationPath(const uint8_t *data, uint32_t len, char *path, size_t pathSize)
{
    if (data == NULL || path == NULL || pathSize == 0 || len < 1) {
        return false;
    }

    uint8_t depth = data[0];
    if (depth == 0 || depth > SOLANA_MAX_DERIVATION_DEPTH) {
        return false;
    }
    uint32_t expectedLen = 1 + (depth * 4);

    if (len != expectedLen) {
        return false;
    }

    path[0] = '\0';

    for (uint8_t i = 0; i < depth; i++) {
        uint32_t offset = 1 + (i * 4);

        uint32_t component = ((uint32_t)data[offset] << 24) |
                             ((uint32_t)data[offset + 1] << 16) |
                             ((uint32_t)data[offset + 2] << 8) |
                             ((uint32_t)data[offset + 3]);

        bool isHardened = (component & 0x80000000) != 0;
        if (!isHardened) {
            return false;
        }

        component &= 0x7FFFFFFF;
        size_t used = strlen(path);
        size_t available = pathSize - used;
        int written;
        if (used == 0) {
            written = snprintf(path, pathSize, "%u'", component);
        } else {
            written = snprintf(path + used, available, "/%u'", component);
        }
        if (written < 0 || (size_t)written >= available) {
            return false;
        }
    }

    return true;
}
#endif

void GetDeviceUsbPubkeyService(EAPDURequestPayload_t *payload)
{
    EAPDUResponsePayload_t *result = NULL;
    cJSON *root = NULL;
    char *json_str = NULL;
    uint32_t coinType = 0;
    char path[BUFFER_SIZE_32] = {0};
    char *pubKey = NULL;

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

    if (!ParseCoinType(payload->data, payload->dataLen, &coinType)) {
        cJSON_AddStringToObject(root, "error", "Invalid payload");
        goto create_response;
    }

    if (coinType != SOLANA_COIN_TYPE) {
        cJSON_AddStringToObject(root, "error", "Unsupported coin type");
        goto create_response;
    }

#ifndef WEB3_VERSION
    // Solana public keys only exist in the multi-coins firmware.
    (void)path;
    (void)pubKey;
    cJSON_AddStringToObject(root, "error", "Unsupported coin type");
    goto create_response;
#else
    if (!ParseSolDerivationPath(payload->data + COIN_TYPE_SIZE,
                                payload->dataLen - COIN_TYPE_SIZE,
                                path, sizeof(path))) {
        cJSON_AddStringToObject(root, "error", "Failed to parse derivation path");
        goto create_response;
    }

    ChainType pubkeyIndex = CheckSolPathSupport(path);
    if (pubkeyIndex == XPUB_TYPE_NUM) {
        cJSON_AddStringToObject(root, "error", "Unsupported derivation path");
        goto create_response;
    }

    pubKey = GetCurrentAccountPublicKey(pubkeyIndex);
    if (pubKey == NULL) {
        cJSON_AddStringToObject(root, "error", "Failed to get public key");
        goto create_response;
    }

    cJSON_AddStringToObject(root, "pubkey", pubKey);
    cJSON_AddStringToObject(root, "derivationPath", path);
    cJSON_AddNumberToObject(root, "coinType", coinType);
#endif

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
    if (result != NULL) {
        SRAM_FREE(result);
    }
}
