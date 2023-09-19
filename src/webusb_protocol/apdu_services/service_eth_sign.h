#include "stdio.h"
#include "stdlib.h"
#include "apdu_protocol_parser.h"
#include "librust_c.h"
#include "keystore.h"

Response *ProcessEthereumTransactionSignature(uint8_t *data, uint16_t dataLen);