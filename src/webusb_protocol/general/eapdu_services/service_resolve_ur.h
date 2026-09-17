#include "stdio.h"
#include "stdlib.h"
#include "eapdu_protocol_parser.h"
#include "librust_c.h"
#include "keystore.h"

void ProcessURService(EAPDURequestPayload_t *payload);
void ProcessURValidationError(EAPDURequestPayload_t *payload, const char *error_message);
void HandleURResultViaUSBFunc(const void *data, uint32_t data_len, uint16_t requestID, StatusEnum status);
void HandleURResultViaUSBAsyncFunc(const void *data, uint32_t data_len, uint16_t requestID, StatusEnum status);
uint16_t GetCurrentUSParsingRequestID();
void ClearUSBRequestId(void);

typedef struct {
    uint32_t dataLen;
    uint16_t requestID;
    StatusEnum status;
    uint8_t data[];
} USBURResultMsg_t;
