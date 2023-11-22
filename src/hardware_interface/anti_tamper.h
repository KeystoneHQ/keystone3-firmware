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
