#ifndef BTC_ONLY
#ifndef _VERSION_H
#define _VERSION_H

#define SOFTWARE_VERSION_MAX_LEN            (32)
#define SOFTWARE_VERSION_MAJOR              1
#define SOFTWARE_VERSION_MINOR              4
#define SOFTWARE_VERSION_BUILD              5
#define SOFTWARE_VERSION_BETA               1
#define SOFTWARE_VERSION                    (SOFTWARE_VERSION_MAJOR * 10000 + SOFTWARE_VERSION_MINOR * 100 + SOFTWARE_VERSION_BUILD)

#if SOFTWARE_VERSION_MAJOR > 99 || SOFTWARE_VERSION_MINOR > 99 || SOFTWARE_VERSION_BUILD > 99
#error "Invalid software version"
#endif

void GetSoftWareVersion(char *version);
void GetSoftWareVersionNumber(char *version);
const char *GetSoftwareVersionString(void);
void GetUpdateVersionNumber(char *version);

#endif /* _VERSION_H */

#else
#include "version_btc_only.h"
#endif
