#ifndef _VERSION_H
#define _VERSION_H

#define CONCAT(x, y) x ## y
#define PLUS10(x) CONCAT(1, x)

#define SOFTWARE_VERSION_MAX_LEN            (32)
#define SOFTWARE_VERSION_MAJOR              1
#define SOFTWARE_VERSION_MINOR              1
#define SOFTWARE_VERSION_BUILD              2
#define SOFTWARE_VERSION                    (SOFTWARE_VERSION_MAJOR * 10000 + SOFTWARE_VERSION_MINOR * 100 + SOFTWARE_VERSION_BUILD)

#define SOFTWARE_VERSION_BTC_ONLY_MAJOR     PLUS10(SOFTWARE_VERSION_MAJOR)

#if SOFTWARE_VERSION_MAJOR > 99 || SOFTWARE_VERSION_MINOR > 99 || SOFTWARE_VERSION_BUILD > 99
#error "Invalid software version"
#endif

void GetSoftWareVersion(char *version);
void GetSoftWareVersionNumber(char *version);
void GetUpdateVersionNumber(char *version);
const char *GetSoftwareVersionString(void);

#endif /* _VERSION_H */
