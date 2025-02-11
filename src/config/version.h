#ifndef _VERSION_H
#define _VERSION_H

#define SD_CARD_OTA_BIN_PATH                "0:/keystone3.bin"

#define SOFTWARE_VERSION_MAX_LEN            (32)
#ifdef WEB3_VERSION
#define SOFTWARE_VERSION_MAJOR              2
#define SOFTWARE_VERSION_MAJOR_OFFSET       0
#define SOFTWARE_VERSION_MINOR              0
#define SOFTWARE_VERSION_BUILD              0
#define SOFTWARE_VERSION_BETA               0
#define SOFTWARE_VERSION                    (SOFTWARE_VERSION_MAJOR * 10000 + SOFTWARE_VERSION_MINOR * 100 + SOFTWARE_VERSION_BUILD)
#define SOFTWARE_VERSION_SUFFIX             ""
#endif /* _VERSION_H */

#ifdef CYPHERPUNK_VERSION
#include "version_cypherpunk.h"
#endif

#ifdef BTC_ONLY
#include "version_btc_only.h"
#endif

#if SOFTWARE_VERSION_MAJOR > 99 || SOFTWARE_VERSION_MINOR > 99 || SOFTWARE_VERSION_BUILD > 99
#error "Invalid software version"
#endif

void GetSoftWareVersion(char *version);
void GetSoftWareVersionNumber(char *version);
const char *GetSoftwareVersionString(void);
void GetUpdateVersionNumber(char *version);
bool GetBootSoftwareVersion(uint32_t *major, uint32_t *minor, uint32_t *build);

#endif

