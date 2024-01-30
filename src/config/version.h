#ifndef _VERSION_H
#define _VERSION_H

#define SOFTWARE_VERSION_MAJOR              1
#define SOFTWARE_VERSION_MINOR              2
#define SOFTWARE_VERSION_BUILD              10
#define SOFTWARE_VERSION                    (SOFTWARE_VERSION_MAJOR * 10000 + SOFTWARE_VERSION_MINOR * 100 + SOFTWARE_VERSION_BUILD)

#if SOFTWARE_VERSION_MAJOR > 99 || SOFTWARE_VERSION_MINOR > 99 || SOFTWARE_VERSION_BUILD > 99
#error "Invalid software version"
#endif

void GetSoftWareVersion(char *version);
void GetSoftWareVersionNumber(char *version);
const char *GetSoftwareVersionString(void);

#endif /* _VERSION_H */