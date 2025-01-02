#include <string.h>
#include <stdio.h>
#include "gui.h"
#include "version.h"

#define SOFTWARE_VERSION_MAX_LEN            (32)
#define STRINGIFY(x)                        #x
#define EXPAND(x)                           STRINGIFY(x)

#ifndef BTC_ONLY
#define SOFTWARE_VERSION_STR "Firmware v" EXPAND(SOFTWARE_VERSION_MAJOR) "." EXPAND(SOFTWARE_VERSION_MINOR) "." EXPAND(SOFTWARE_VERSION_BUILD)
#else
#define SOFTWARE_VERSION_STR "Firmware v" EXPAND(SOFTWARE_VERSION_BTC_ONLY_MAJOR) "." EXPAND(SOFTWARE_VERSION_MINOR) "." EXPAND(SOFTWARE_VERSION_BUILD)
#endif
#ifdef COMPILE_SIMULATOR
const char g_softwareVersionString[] = SOFTWARE_VERSION_STR;
#else
const char g_softwareVersionString[] __attribute__((section(".fixSection"))) = SOFTWARE_VERSION_STR;
#endif

void GetSoftWareVersion(char *version)
{
    if (SOFTWARE_VERSION_BUILD % 2 == 0) {
        snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%s v%d.%d.%d - %s", _("about_info_firmware_version_head"), SOFTWARE_VERSION_MAJOR - SOFTWARE_VERSION_MAJOR_OFFSET, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD, SOFTWARE_VERSION_SUFFIX);
    } else {
        snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%s v%d.%d.%d(beta%d) - %s", _("about_info_firmware_version_head"), SOFTWARE_VERSION_MAJOR - SOFTWARE_VERSION_MAJOR_OFFSET, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD, SOFTWARE_VERSION_SUFFIX);
    }
}

void GetUpdateVersionNumber(char *version)
{
#ifndef BTC_ONLY
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%d.%d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
#else
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%d.%d.%d-B", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
#endif
}

void GetSoftWareVersionNumber(char *version)
{
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%d.%d.%d - %s", SOFTWARE_VERSION_MAJOR - SOFTWARE_VERSION_MAJOR_OFFSET, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD, SOFTWARE_VERSION_SUFFIX);
}

const char *GetSoftwareVersionString(void)
{
    static char version[32] = {0};
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "v%d.%d.%d - %s", SOFTWARE_VERSION_MAJOR - SOFTWARE_VERSION_MAJOR_OFFSET, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD, SOFTWARE_VERSION_SUFFIX);
    return version;
}

