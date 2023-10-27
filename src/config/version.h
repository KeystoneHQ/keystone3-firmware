/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : version.h
 * Description:
 * author     : liuzepeng
 * data       : 2023-05-11 10:08
**********************************************************************/

#ifndef _VERSION_H
#define _VERSION_H

#define SOFTWARE_VERSION_MAJOR              1
#define SOFTWARE_VERSION_MINOR              1
#define SOFTWARE_VERSION_BUILD              0
#define SOFTWARE_VERSION                    (SOFTWARE_VERSION_MAJOR * 10000 + SOFTWARE_VERSION_MINOR * 100 + SOFTWARE_VERSION_BUILD)

#if SOFTWARE_VERSION_MAJOR > 99 || SOFTWARE_VERSION_MINOR > 99 || SOFTWARE_VERSION_BUILD > 99
#error "Invalid software version"
#endif

void GetSoftWareVersion(char *version);
void GetSoftWareVersionNumber(char *version);
const char *GetSoftwareVersionString(void);

#endif /* _VERSION_H */