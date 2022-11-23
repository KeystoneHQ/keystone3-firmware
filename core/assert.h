/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Assert handler.
 * Author: leon sun
 * Create: 2023-4-7
 ************************************************************************************************/


#ifndef _ASSERT_H
#define _ASSERT_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

void ShowAssert(const char *file, uint32_t len);

#define ASSERT(expr) ((expr) ? (void)0 : ShowAssert(__FILE__, __LINE__))

//compatible for general interface
#define assert(expr) ((expr) ? (void)0 : ShowAssert(__FILE__, __LINE__))

#endif
