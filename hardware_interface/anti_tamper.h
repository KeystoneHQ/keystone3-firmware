/**************************************************************************************************
 * Copyright (c) Keystone 2020-2025. All rights reserved.
 * Description: Anti-tamper.
 * Author: leon sun
 * Create: 2023-8-2
 ************************************************************************************************/


#ifndef _ANTI_TAMPER_H
#define _ANTI_TAMPER_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

void TamperStartup(void);
void TamperIntHandler(void);
void TamperBackgroundHandler(void);
bool Tampered(void);

void TamperTest(int argc, char *argv[]);

#endif
