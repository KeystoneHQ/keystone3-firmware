#include "stdio.h"
#include "stdlib.h"
#include "apdu_protocol_parser.h"
#include "librust_c.h"
#include "keystore.h"

typedef void ResponseHandler(uint8_t cla, uint8_t ins, uint8_t *data, uint32_t dataLen);
static ResponseHandler *g_handleURCallback = NULL;

void *ProcessUREvents(uint8_t *data, uint32_t dataLen, ResponseHandler *sendResponse);