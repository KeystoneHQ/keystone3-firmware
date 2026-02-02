#include <stdio.h>
#include <string.h>
#include "user_memory.h"
#include "service_echo_test.h"
#include "log_print.h"
#include "librust_c.h"
#include "sha256.h"
#include "user_msg.h"

void GetDeviceUsbPubkeyService(EAPDURequestPayload_t *payload)
{
    EAPDUResponsePayload_t *result = (EAPDUResponsePayload_t *)SRAM_MALLOC(sizeof(EAPDUResponsePayload_t));
    result->cla = EAPDU_PROTOCOL_HEADER;
    result->commandType = CMD_GET_DEVICE_USB_PUBKEY;
    result->requestID = payload->requestID;

    printf("GetDeviceUsbPubkeyService: received %d bytes\n", payload->dataLen);
    PrintArray("Received data", payload->data, payload->dataLen);

    // Parse the payload format: [pubkey_length(1byte)] + [pubkey] + [signature(64bytes)]
    if (payload->dataLen < 1 + 33 + 64) {
        printf("GetDeviceUsbPubkeyService: payload too short (min 98 bytes required)\n");
        const char *errorMessage = "payload too short";
        result->data = errorMessage;
        result->dataLen = strlen(errorMessage);
        result->status = PRS_SET_PUBKEY_INVALID_PARAMS;
        SendEApduResponse(result);
        SRAM_FREE(result);
        return;
    }

    // Extract public key length
    uint8_t pubKeyLen = payload->data[0];
    printf("Public key length: %d\n", pubKeyLen);

    // Validate public key length
    if (pubKeyLen != 33 && pubKeyLen != 65) {
        printf("GetDeviceUsbPubkeyService: invalid pubkey length %d\n", pubKeyLen);
        const char *errorMessage = "invalid pubkey length";
        result->data = errorMessage;
        result->dataLen = strlen(errorMessage);
        result->status = PRS_SET_PUBKEY_INVALID_PARAMS;
        SendEApduResponse(result);
        SRAM_FREE(result);
        return;
    }

    // Check total length
    if (payload->dataLen < 1 + pubKeyLen + 64) {
        printf("GetDeviceUsbPubkeyService: payload length mismatch\n");
        const char *errorMessage = "payload length mismatch";
        result->data = errorMessage;
        result->dataLen = strlen(errorMessage);
        result->status = PRS_SET_PUBKEY_INVALID_PARAMS;
        SendEApduResponse(result);
        SRAM_FREE(result);
        return;
    }

    // Extract public key and signature
    uint8_t *pubKey = payload->data + 1;
    uint8_t *signature = payload->data + 1 + pubKeyLen;

    PrintArray("Public key", pubKey, pubKeyLen);
    PrintArray("Signature", signature, 64);

    // Verify signature: hash the public key and verify with the same public key (self-signed)
    // This proves that the sender has the corresponding private key
    uint8_t hash[32] = {0};
    struct sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, pubKey, pubKeyLen);
    sha256_done(&ctx, (struct sha256 *)hash);

    PrintArray("Public key hash", hash, 32);

    // Verify the signature using the received public key itself (self-signed verification)
    // This proves the sender owns the private key corresponding to this public key
    if (k1_verify_signature(signature, hash, pubKey) == false) {
        printf("GetDeviceUsbPubkeyService: signature verification failed\n");
        const char *errorMessage = "signature verification failed";
        result->data = errorMessage;
        result->dataLen = strlen(errorMessage);
        result->status = PRS_SET_PUBKEY_VERIFY_FAILED;
        SendEApduResponse(result);
        SRAM_FREE(result);
        return;
    }

    PubValueMsg(UI_MSG_NEXT_VIEW, 1);
    printf("pub key verified, proceed to next view\n");
    printf("GetDeviceUsbPubkeyService: signature verification SUCCESS\n");

    const char *successMessage = "verify pubkey success";
    result->data = successMessage;
    result->dataLen = strlen(successMessage);
    result->status = PRS_SET_PUBKEY_VERIFY_SUCCESS;
    SendEApduResponse(result);
    SRAM_FREE(result);
}