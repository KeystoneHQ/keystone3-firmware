#include <string.h>
#include <stdio.h>
#include "version.h"

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

#define SOFTWARE_VERSION_STR "Firmware v" EXPAND(SOFTWARE_VERSION_MAJOR + 10) "." EXPAND(SOFTWARE_VERSION_MINOR) "." EXPAND(SOFTWARE_VERSION_BUILD)
#ifdef COMPILE_SIMULATOR
const char g_softwareVersionString[] = SOFTWARE_VERSION_STR;
#else
const char g_softwareVersionString[] __attribute__((section(".fixSection"))) = SOFTWARE_VERSION_STR;
#endif

void GetSoftWareVersion(char *version)
{
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "Firmware v%d.%d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
}

void GetSoftWareVersionNumber(char *version)
{
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%d.%d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
}

const char *GetSoftwareVersionString(void)
{
    return g_softwareVersionString;
}

