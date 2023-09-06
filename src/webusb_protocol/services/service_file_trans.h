/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: file trans protocol service.
 * Author: leon sun
 * Create: 2023-7-7
 ************************************************************************************************/


#ifndef _SERVICE_FILE_TRANS_H
#define _SERVICE_FILE_TRANS_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "protocol_parse.h"


enum {
    COMMAND_ID_FILE_TRANS_INFO = 1,
    COMMAND_ID_FILE_TRANS_CONTENT,
    COMMAND_ID_FILE_TRANS_COMPLETE,
    COMMAND_ID_FILE_TRANS_MAX,
};

extern const ProtocolServiceCallbackFunc_t g_fileTransInfoServiceFunc[];


#endif
