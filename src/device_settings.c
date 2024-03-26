#include "device_setting.h"
#include "stdio.h"
#include "string.h"
#include "flash_address.h"
#include "drv_gd25qxx.h"
#include "keystore.h"
#include "flash_address.h"
#include "drv_gd25qxx.h"
#include "assert.h"
#include "user_memory.h"
#include "cjson/cJSON.h"
#include "user_utils.h"
#include "background_task.h"
#include "drv_lcd_bright.h"
#include "fingerprint_process.h"
#include "screen_manager.h"
#include "power_manager.h"
#include "account_manager.h"
#include "version.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

#define VERSION_MAX_LENGTH      32


#define KEY_VERSION                     "version"

#define KEY_SETUP_STEP                  "setup_step"
#define KEY_BRIGHT                      "bright"
#define KEY_AUTO_LOCK_SCREEN            "auto_lock_screen"
#define KEY_AUTO_POWER_OFF              "auto_power_off"
#define KEY_VIBRATION                   "vibration"
#define KEY_DARK_MODE                   "dark_mode"
#define KEY_USB_SWITCH                  "usb_switch"
#define KEY_LAST_VERSION                "last_version"

#define DEFAULT_SETUP_STEP              0
#define DEFAULT_BRIGHT                  15
#define DEFAULT_AUTO_LOCK_SCREEN        60
#define DEFAULT_AUTO_POWER_OFF          0
#define DEFAULT_VIBRATION               1
#define DEFAULT_DARK_MODE               1
#define DEFAULT_USB_SWITCH              1
#define DEFAULT_LAST_VERSION            0

typedef struct {
    uint32_t setupStep;
    uint32_t bright;
    uint32_t autoLockScreen;
    uint32_t autoPowerOff;
    uint32_t vibration;
    uint32_t darkMode;
    uint32_t usbSwitch;
    uint32_t lastVersion;
} DeviceSettings_t;

static int32_t SaveDeviceSettingsAsyncFunc(const void *inData, uint32_t inDataLen);
static void SaveDeviceSettingsSync(void);
static bool GetDeviceSettingsFromJsonString(const char *string);
static char *GetJsonStringFromDeviceSettings(void);
static int32_t GetIntValue(const cJSON *obj, const char *key, int32_t defaultValue);
static void GetStringValue(const cJSON *obj, const char *key, char *value, uint32_t maxLen);

static const char g_deviceSettingsVersion[] = "1.0.0";
DeviceSettings_t g_deviceSettings;

void DeviceSettingsInit(void)
{
    int32_t ret;
    uint32_t size;
    bool needRegenerate = false;
    char *jsonString = NULL;

    ret = Gd25FlashReadBuffer(SPI_FLASH_ADDR_NORMAL_PARAM, (uint8_t *)&size, sizeof(size));
    ASSERT(ret == 4);
    printf("device settings size=%d\n", size);
    do {
        if (size >= SPI_FLASH_SIZE_NORMAL_PARAM - 4) {
            needRegenerate = true;
            printf("device settings size err,%d\r\n", size);
            break;
        }
        jsonString = SRAM_MALLOC(size + 1);
        ret = Gd25FlashReadBuffer(SPI_FLASH_ADDR_NORMAL_PARAM + 4, (uint8_t *)jsonString, size);
        ASSERT(ret == size);
        jsonString[size] = 0;
        if (GetDeviceSettingsFromJsonString(jsonString) == false) {
            printf("GetDeviceSettingsFromJsonString false, need regenerate\r\n");
            printf("err jsonString=%s\r\n", jsonString);
            needRegenerate = true;
        }
    } while (0);

    if (needRegenerate) {
        g_deviceSettings.setupStep = DEFAULT_SETUP_STEP;
        g_deviceSettings.bright = DEFAULT_BRIGHT;
        g_deviceSettings.autoLockScreen = DEFAULT_AUTO_LOCK_SCREEN;
        g_deviceSettings.autoPowerOff = DEFAULT_AUTO_POWER_OFF;
        g_deviceSettings.vibration = DEFAULT_VIBRATION;
        g_deviceSettings.darkMode = DEFAULT_DARK_MODE;
        g_deviceSettings.usbSwitch = DEFAULT_USB_SWITCH;
        g_deviceSettings.lastVersion = DEFAULT_LAST_VERSION;
        SaveDeviceSettingsSync();
    }
}


/// @brief Save device settings to FLASH in background task.
void SaveDeviceSettings(void)
{
    AsyncExecute(SaveDeviceSettingsAsyncFunc, NULL, 0);
}


uint32_t GetSetupStep(void)
{
    return g_deviceSettings.setupStep;
}


void SetSetupStep(uint32_t setupStep)
{
    g_deviceSettings.setupStep = setupStep;
}


uint32_t GetBright(void)
{
    return g_deviceSettings.bright;
}


void SetBright(uint32_t bight)
{
    SetLcdBright(bight);
    g_deviceSettings.bright = bight;
}


uint32_t GetAutoLockScreen(void)
{
    return g_deviceSettings.autoLockScreen;
}


void SetAutoLockScreen(uint32_t autoLockScreen)
{
    SetLockTimeOut(autoLockScreen);
    g_deviceSettings.autoLockScreen = autoLockScreen;
}


uint32_t GetAutoPowerOff(void)
{
    return g_deviceSettings.autoPowerOff;
}


void SetAutoPowerOff(uint32_t autoPowerOff)
{
    SetShutdownTimeOut(autoPowerOff);
    g_deviceSettings.autoPowerOff = autoPowerOff;
}


uint32_t GetVibration(void)
{
    return g_deviceSettings.vibration;
}


void SetVibration(uint32_t vibration)
{
    g_deviceSettings.vibration = vibration;
}


uint32_t GetDarkMode(void)
{
    return g_deviceSettings.darkMode;
}


void SetDarkMode(uint32_t darkMode)
{
    g_deviceSettings.darkMode = darkMode;
}


uint32_t GetUSBSwitch(void)
{
    return g_deviceSettings.usbSwitch;
}


void SetUSBSwitch(uint32_t usbSwitch)
{
    g_deviceSettings.usbSwitch = usbSwitch;
}

bool IsUpdateSuccess(void)
{
    uint32_t currentVersion = SOFTWARE_VERSION_MAJOR << 16 | SOFTWARE_VERSION_MINOR << 8 | SOFTWARE_VERSION_BUILD;
    bool isUpdate = false;
    if (g_deviceSettings.lastVersion == DEFAULT_LAST_VERSION) {
        g_deviceSettings.lastVersion = currentVersion;
        SaveDeviceSettings();
        isUpdate = false;
    }

    if (g_deviceSettings.lastVersion < currentVersion) {
        g_deviceSettings.lastVersion = currentVersion;
        SaveDeviceSettings();
        isUpdate = true;
    }
    printf("g_deviceSettings.lastVersion=%#x\n", g_deviceSettings.lastVersion);

    return isUpdate;
}


/// @brief Wipe device.
void WipeDevice(void)
{

}


/// @brief Device settings test.
/// @param argc Test arg count.
/// @param argv Test arg values.
void DeviceSettingsTest(int argc, char *argv[])
{
}


static int32_t SaveDeviceSettingsAsyncFunc(const void *inData, uint32_t inDataLen)
{
    SaveDeviceSettingsSync();
    return 0;
}


static void SaveDeviceSettingsSync(void)
{
    char *jsonString;
    uint32_t size;

    jsonString = GetJsonStringFromDeviceSettings();
    printf("jsonString=%s\n", jsonString);
    Gd25FlashSectorErase(SPI_FLASH_ADDR_NORMAL_PARAM);      //Only one sector for device settings.
    size = strnlen_s(jsonString, SPI_FLASH_SIZE_NORMAL_PARAM - 4 - 1);
    ASSERT(size < SPI_FLASH_SIZE_NORMAL_PARAM - 4);
    Gd25FlashWriteBuffer(SPI_FLASH_ADDR_NORMAL_PARAM, (uint8_t *)&size, 4);
    Gd25FlashWriteBuffer(SPI_FLASH_ADDR_NORMAL_PARAM + 4, (uint8_t *)jsonString, size + 1);
    EXT_FREE(jsonString);
}


static bool GetDeviceSettingsFromJsonString(const char *string)
{
    cJSON *rootJson;
    bool ret = true;
    char versionString[VERSION_MAX_LENGTH];

    do {
        rootJson = cJSON_Parse(string);
        if (rootJson == NULL) {
            printf("device settings json parse fail\n");
            ret = false;
            break;
        }
        GetStringValue(rootJson, KEY_VERSION, versionString, VERSION_MAX_LENGTH);
        if (strcmp(versionString, g_deviceSettingsVersion) != 0) {
            printf("saved version:%s\r\n", versionString);
            printf("g_deviceSettingsVersion:%s\r\n", g_deviceSettingsVersion);
            ret = false;
            break;
        }
        g_deviceSettings.setupStep = GetIntValue(rootJson, KEY_SETUP_STEP, DEFAULT_SETUP_STEP);
        g_deviceSettings.bright = GetIntValue(rootJson, KEY_BRIGHT, DEFAULT_BRIGHT);
        g_deviceSettings.autoLockScreen = GetIntValue(rootJson, KEY_AUTO_LOCK_SCREEN, DEFAULT_AUTO_LOCK_SCREEN);
        g_deviceSettings.autoPowerOff = GetIntValue(rootJson, KEY_AUTO_POWER_OFF, DEFAULT_AUTO_POWER_OFF);
        g_deviceSettings.vibration = GetIntValue(rootJson, KEY_VIBRATION, DEFAULT_VIBRATION);
        g_deviceSettings.darkMode = GetIntValue(rootJson, KEY_DARK_MODE, DEFAULT_DARK_MODE);
        g_deviceSettings.usbSwitch = GetIntValue(rootJson, KEY_USB_SWITCH, DEFAULT_USB_SWITCH);
        g_deviceSettings.lastVersion = GetIntValue(rootJson, KEY_LAST_VERSION, DEFAULT_LAST_VERSION);
    } while (0);
    printf("g_deviceSettings.setupStep=%d\n", g_deviceSettings.setupStep);
    printf("g_deviceSettings.bright=%d\n", g_deviceSettings.bright);
    printf("g_deviceSettings.autoLockScreen=%d\n", g_deviceSettings.autoLockScreen);
    printf("g_deviceSettings.autoPowerOff=%d\n", g_deviceSettings.autoPowerOff);
    printf("g_deviceSettings.vibration=%d\n", g_deviceSettings.vibration);
    printf("g_deviceSettings.darkMode=%d\n", g_deviceSettings.darkMode);
    printf("g_deviceSettings.usbSwitch=%d\n", g_deviceSettings.usbSwitch);
    printf("g_deviceSettings.lastVersion=%d\n", g_deviceSettings.lastVersion);
    cJSON_Delete(rootJson);

    return ret;
}

static char *GetJsonStringFromDeviceSettings(void)
{
    cJSON *rootJson;
    char *retStr;

    rootJson = cJSON_CreateObject();
    cJSON_AddItemToObject(rootJson, KEY_VERSION, cJSON_CreateString(g_deviceSettingsVersion));
    cJSON_AddItemToObject(rootJson, KEY_SETUP_STEP, cJSON_CreateNumber(g_deviceSettings.setupStep));
    cJSON_AddItemToObject(rootJson, KEY_BRIGHT, cJSON_CreateNumber(g_deviceSettings.bright));
    cJSON_AddItemToObject(rootJson, KEY_AUTO_LOCK_SCREEN, cJSON_CreateNumber(g_deviceSettings.autoLockScreen));
    cJSON_AddItemToObject(rootJson, KEY_AUTO_POWER_OFF, cJSON_CreateNumber(g_deviceSettings.autoPowerOff));
    cJSON_AddItemToObject(rootJson, KEY_VIBRATION, cJSON_CreateNumber(g_deviceSettings.vibration));
    cJSON_AddItemToObject(rootJson, KEY_DARK_MODE, cJSON_CreateNumber(g_deviceSettings.darkMode));
    cJSON_AddItemToObject(rootJson, KEY_USB_SWITCH, cJSON_CreateNumber(g_deviceSettings.usbSwitch));
    cJSON_AddItemToObject(rootJson, KEY_LAST_VERSION, cJSON_CreateNumber(g_deviceSettings.lastVersion));
    retStr = cJSON_Print(rootJson);
    RemoveFormatChar(retStr);
    cJSON_Delete(rootJson);

    return retStr;
}


/**
 * @brief       Get integer value from cJSON object.
 * @param[in]   obj : cJSON object.
 * @param[in]   key : key name.
 * @param[in]   defaultValue : if key does not exist, return this value.
 * @retval      integer value to get.
 */
static int32_t GetIntValue(const cJSON *obj, const char *key, int32_t defaultValue)
{
    cJSON *intJson = cJSON_GetObjectItem((cJSON *)obj, key);
    if (intJson != NULL) {
        return (uint32_t)intJson->valuedouble;
    }
    printf("key:%s does not exist\r\n", key);
    return defaultValue;
}


/**
 * @brief       Get string value from cJSON object.
 * @param[in]   obj : cJSON object.
 * @param[in]   key : key name.
 * @param[out]  value : return string value, if the acquisition fails, the string will be cleared.
 * @retval
 */
static void GetStringValue(const cJSON *obj, const char *key, char *value, uint32_t maxLen)
{
    cJSON *json;
    uint32_t len;
    char *strTemp;

    json = cJSON_GetObjectItem((cJSON *)obj, key);
    if (json != NULL) {
        strTemp = json->valuestring;
        len = strnlen_s(strTemp, maxLen);
        if (len < maxLen) {
            strcpy_s(value, maxLen, strTemp);
        } else {
            value[0] = '\0';
        }
    } else {
        printf("key:%s does not exist\r\n", key);
        value[0] = '\0';
    }
}
