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
