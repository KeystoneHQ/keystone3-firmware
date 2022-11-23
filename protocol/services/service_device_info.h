/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: device info protocol service.
 * Author: leon sun
 * Create: 2023-6-29
 ************************************************************************************************/


#ifndef _SERVICE_DEVICE_INFO_H
#define _SERVICE_DEVICE_INFO_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "protocol_parse.h"


enum {
    COMMAND_ID_DEVICE_INFO_BASIC = 1,
    COMMAND_ID_DEVICE_INFO_RUNNING,
    COMMAND_ID_DEVICE_INFO_MAX
};

extern const ProtocolServiceCallbackFunc_t g_deviceInfoServiceFunc[];


#endif
