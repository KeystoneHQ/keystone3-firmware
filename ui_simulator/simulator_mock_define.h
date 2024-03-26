#ifndef _SIMULATOR_MOCK_DEFINE_H
#define _SIMULATOR_MOCK_DEFINE_H
#include "user_memory.h"

#define memcpy_s(dest, destsz, src, count) memcpy(dest, src, count)
#define strnlen_s(sstr, smax) strnlen(sstr, smax)
#define strncat_s(str, max, src, len) strncat(str, src, len)
#define strcat_s(str, max, src) strcat(str, src)
#define strncpy_s(str, max, src, len) strncpy(str, src, len)

#endif