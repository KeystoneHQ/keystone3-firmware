#ifndef _SDCARD_MANAGER_H_
#define _SDCARD_MANAGER_H_

#include "stdbool.h"
#include "stdint.h"

int FileWrite(const char *filename, const uint8_t *content, uint32_t len);

#endif