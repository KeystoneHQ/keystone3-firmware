#ifndef _ASSERT_H
#define _ASSERT_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"
#include "diagnostic_location.h"

void ShowAssert(const char *file, uint32_t line);

#define ASSERT(expr) ((expr) ? (void)0 : ShowAssert(DIAGNOSTIC_FILE, DIAGNOSTIC_LINE))

//compatible for general interface
#define assert(expr) ASSERT(expr)

#endif
