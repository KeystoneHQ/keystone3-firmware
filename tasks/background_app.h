/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: background app.
 * Author: leon sun
 * Create: 2023-3-25
 ************************************************************************************************/

#ifndef _BACKGROUND_APP_H
#define _BACKGROUND_APP_H

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    SYSTEM_RESET_TYPE_REBOOT,
    SYSTEM_RESET_TYPE_POWEROFF,
} SystemResetType;


void BackGroundAppInit(void);
void ChangerRefreshState(void);
void ExecuteSystemReset(SystemResetType type);

#endif

