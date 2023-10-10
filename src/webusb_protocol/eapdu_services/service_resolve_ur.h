#include "stdio.h"
#include "stdlib.h"
#include "eapdu_protocol_parser.h"
#include "librust_c.h"
#include "keystore.h"

typedef void ResponseHandler(uint8_t cla, uint8_t ins, EAPDUResponsePayload_t *payload);
void *ProcessUREvents(EAPDURequestPayload_t *payload);
void HandleURResultViaUSBFunc(const void *data, uint32_t data_len, bool isSuccess);
