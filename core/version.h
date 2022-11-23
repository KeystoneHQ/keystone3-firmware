/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : version.h
 * Description:
 * author     : liuzepeng
 * data       : 2023-05-11 10:08
**********************************************************************/

#ifndef _VERSION_H
#define _VERSION_H

#define SOFTWARE_VERSION_MAJOR              0
#define SOFTWARE_VERSION_MINOR              8
#define SOFTWARE_VERSION_BUILD              8
#define SOFTWARE_VERSION                    (SOFTWARE_VERSION_MAJOR * 10000 + SOFTWARE_VERSION_MINOR * 100 + SOFTWARE_VERSION_BUILD)

void GetSoftWareVersion(char *version);
void GetSoftWareVersionNumber(char *version);
const char *GetSoftwareVersionString(void);

#endif /* _VERSION_H */