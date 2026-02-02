#ifndef _SERVICE_TRANS_USB_PUBKEY_H
#define _SERVICE_TRANS_USB_PUBKEY_H

#include "eapdu_protocol_parser.h"

typedef struct {
    uint8_t pubkeyLen;
    char pubkey[65];
    char signature[64];
} PubkeySignatureVerifyParam_t;

void GetDeviceUsbPubkeyService(EAPDURequestPayload_t *payload);

#endif