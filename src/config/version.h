#ifndef _VERSION_H
#define _VERSION_H

#define SD_CARD_OTA_BIN_PATH                "0:/keystone3.bin"

#define SOFTWARE_VERSION_MAX_LEN            (32)
#define SOFTWARE_VERSION_MAJOR              12
#define SOFTWARE_VERSION_MAJOR_OFFSET       10
#define SOFTWARE_VERSION_MINOR              2
#define SOFTWARE_VERSION_BUILD              21
#define SOFTWARE_VERSION_BETA               1
#define SOFTWARE_VERSION                    (SOFTWARE_VERSION_MAJOR * 10000 + SOFTWARE_VERSION_MINOR * 100 + SOFTWARE_VERSION_BUILD)
#ifdef WEB3_VERSION
#define SOFTWARE_VERSION_SUFFIX             ""
#endif

#ifdef CYPHERPUNK_VERSION
#define SOFTWARE_VERSION_SUFFIX             " - Cypherpunk"
#endif

#ifdef BTC_ONLY
#define SOFTWARE_VERSION_SUFFIX             " - BTC"
#endif

#if SOFTWARE_VERSION_MAJOR > 99 || SOFTWARE_VERSION_MINOR > 99 || SOFTWARE_VERSION_BUILD > 99
#error "Invalid software version"
#endif

void GetSoftWareVersion(char *version);
void GetSoftWareVersionNumber(char *version);
const char *GetSoftwareVersionString(void);
void GetUpdateVersionNumber(char *version);
bool GetBootSoftwareVersion(uint32_t *major, uint32_t *minor, uint32_t *build);
bool IsBootVersionMatch(void);
void GetBootVersionNumber(char *version);

#endif

