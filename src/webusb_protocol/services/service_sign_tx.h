#ifndef _SERVICE_SIGN_TX_H
#define _SERVICE_SIGN_TX_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "protocol_parse.h"

enum {
    COMMAND_ID_SIGN_TX_REQ = 1,
    COMMAND_ID_SIGN_TX_RESP,
    COMMAND_ID_SIGN_TX_MAX,
};

extern const ProtocolServiceCallbackFunc_t g_signTxServiceFunc[];


#endif