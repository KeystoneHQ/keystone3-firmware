#include <string.h>
#include <stdio.h>
#include "gui.h"
#include "version.h"

#define SOFTWARE_VERSION_MAX_LEN            (32)
#define STRINGIFY(x)                        #x
#define EXPAND(x)                           STRINGIFY(x)
#define BOOT_VERSION_ADDR                   0x01002000
#define BOOT_VERSION_HEAD                   "Boot v"

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
static bool GetBootSoftwareVersionFormData(uint32_t *major, uint32_t *minor, uint32_t *build, const uint8_t *data, uint32_t dataLen);

void GetSoftWareVersion(char *version)
{
#ifndef BTC_ONLY
    if (SOFTWARE_VERSION_BUILD % 2 == 0) {
        snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%s v%d.%d.%d", _("about_info_firmware_version_head"), SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
    } else {
        snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%s v%d.%d.%d(beta%d)", _("about_info_firmware_version_head"), SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD, SOFTWARE_VERSION_BETA);
    }
#else
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "Firmware v%d.%d.%d-BTC", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
#endif
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
#ifndef BTC_ONLY
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%d.%d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
#else
    snprintf(version, SOFTWARE_VERSION_MAX_LEN, "%d.%d.%d-BTC", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
#endif
}

const char *GetSoftwareVersionString(void)
{
    static char version[32] = {0};
#ifndef BTC_ONLY
    sprintf(version, "Firmware v%d.%d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
#else
    sprintf(version, "Firmware v%d.%d.%d-BTC", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
#endif
    return version;
}

bool NeedUpdateBoot(void)
{
#ifdef COMPILE_SIMULATOR
    return false;
#endif
    uint32_t major, minor, build;
    if (GetBootSoftwareVersion(&major, &minor, &build) == false) {
        return true;
    }
    printf("major: %d, minor: %d, build: %d\n", major, minor, build);
    return major == 0 && minor <= 1 && build <= 9;
}

bool GetBootSoftwareVersion(uint32_t *major, uint32_t *minor, uint32_t *build)
{
#ifdef COMPILE_SIMULATOR
    return false;
#endif
    uint8_t read[4096];
    memcpy(read, (void *)BOOT_VERSION_ADDR, 4096);
    return GetBootSoftwareVersionFormData(major, minor, build, read, 4096);
}

static bool GetBootSoftwareVersionFormData(uint32_t *major, uint32_t *minor, uint32_t *build, const uint8_t *data, uint32_t dataLen)
{
    uint32_t versionInfoOffset = UINT32_MAX, i, headLen;
    char *versionStr, read[64];
    int32_t ret;
    bool succ = false;

    headLen = strlen(BOOT_VERSION_HEAD);
    for (i = 0; i < dataLen - headLen - 32; i++) {
        if (data[i] == 'B') {
            if (strncmp((char *)&data[i], BOOT_VERSION_HEAD, headLen) == 0) {
                versionInfoOffset = i;
                break;
            }
        }
    }
    do {
        if (versionInfoOffset == UINT32_MAX) {
            printf("boot version string not found in fixed segment\n");
            break;
        }
        memcpy(read, &data[versionInfoOffset], 64);
        read[31] = '\0';
        if (strncmp(read, BOOT_VERSION_HEAD, headLen) != 0) {
            break;
        }
        versionStr = read + headLen;
        ret = sscanf(versionStr, "%d.%d.%d", major, minor, build);
        if (ret != 3) {
            break;
        }
        succ = true;
    } while (0);
    if (succ == false) {
        *major = 0;
        *minor = 0;
        *build = 0;
    }
    return succ;
}
