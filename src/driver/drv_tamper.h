#ifndef _DRV_TAMPER_H
#define _DRV_TAMPER_H

#include "stdint.h"
#include "stdbool.h"

typedef void (*TamperProcessCallbackFunc_t)(void);

void TamperInit(TamperProcessCallbackFunc_t func);
bool ReadTamperInput(void);

#endif
