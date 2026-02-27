#ifndef _SERVICE_TRANS_USB_PUBKEY_H
#define _SERVICE_TRANS_USB_PUBKEY_H

#include "eapdu_protocol_parser.h"

typedef struct {
    uint8_t pubkeyLen;
    uint8_t pubkey[65];
    uint8_t signature[64];
} PubkeySignatureVerifyParam_t;

void GetDeviceUsbPubkeyService(EAPDURequestPayload_t *payload);
void SendUsbPubkeySetResult(StatusEnum status, const char *message);
uint16_t GetCurrentUsbPubkeyRequestID(void);

#endif
