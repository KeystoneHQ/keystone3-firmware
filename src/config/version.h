#ifndef BTC_ONLY
#ifndef _VERSION_H
#define _VERSION_H

#define SD_CARD_OTA_BIN_PATH                "0:/keystone3.bin"

#define SOFTWARE_VERSION_MAX_LEN            (32)
#define SOFTWARE_VERSION_MAJOR              1
#define SOFTWARE_VERSION_MINOR              5
#define SOFTWARE_VERSION_BUILD              2
#define SOFTWARE_VERSION_BETA               0
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
