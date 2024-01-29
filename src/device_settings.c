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

#define VERSION_MAX_LENGTH      32


#define KEY_VERSION                     "version"

#define KEY_SETUP_STEP                  "setup_step"
#define KEY_BRIGHT                      "bright"
#define KEY_AUTO_LOCK_SCREEN            "auto_lock_screen"
#define KEY_AUTO_POWER_OFF              "auto_power_off"
#define KEY_VIBRATION                   "vibration"
#define KEY_DARK_MODE                   "dark_mode"
#define KEY_USB_SWITCH                  "usb_switch"

#define DEFAULT_SETUP_STEP              0
#define DEFAULT_BRIGHT                  15
#define DEFAULT_AUTO_LOCK_SCREEN        60
#define DEFAULT_AUTO_POWER_OFF          0
#define DEFAULT_VIBRATION               1
#define DEFAULT_DARK_MODE               1
#define DEFAULT_USB_SWITCH              0

typedef struct {
    uint32_t setupStep;
    uint32_t bright;
    uint32_t autoLockScreen;            //second
    uint32_t autoPowerOff;              //second, value zero for never
    uint32_t vibration;
    uint32_t darkMode;
    uint32_t usbSwitch;
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

/// @brief Wipe device.
void WipeDevice(void)
{
    int ret = 0;
    // reset all account address index in receive page
    {
        void GuiResetAllUtxoAddressIndex(void);
        void GuiResetAllEthAddressIndex(void);
        void GuiResetAllStandardAddressIndex(void);

        GuiResetAllUtxoAddressIndex();
        GuiResetAllEthAddressIndex();
        GuiResetAllStandardAddressIndex();
    }

    uint32_t wipeFlag = DEVICE_WIPE_FLAG_MAGIC_NUM;
    Gd25FlashWriteBuffer(SPI_FLASH_ADDR_PROTECT_PARAM, (uint8_t *)&wipeFlag, sizeof(wipeFlag));
    SetShowPowerOffPage(false);
    FpWipeManageInfo();
    ErasePublicInfo();
    DestroyAccount(0);
    DestroyAccount(1);
    DestroyAccount(2);
    for (uint32_t addr = 0; addr < GD25QXX_FLASH_SIZE; addr += 1024 * 64) {
        Gd25FlashBlockErase(addr);
        printf("flash erase address: %#x\n", addr);
    }
}


/// @brief Device settings test.
/// @param argc Test arg count.
/// @param argv Test arg values.
void DeviceSettingsTest(int argc, char *argv[])
{
    if (strcmp(argv[0], "wipe") == 0) {
        printf("start wipe\n");
        WipeDevice();
        printf("wipe over\n");
    } else if (strcmp(argv[0], "info") == 0) {
        printf("device settings verion=%s\n", g_deviceSettingsVersion);
        printf("setupStep=%d\n", GetSetupStep());
        printf("bright=%d\n", GetBright());
        printf("autoLockScreen=%d\n", GetAutoLockScreen());
        printf("autoPowerOff=%d\n", GetAutoPowerOff());
        printf("vibration=%d\n", GetVibration());
        printf("darkMode=%d\n", GetDarkMode());
        printf("usbSwitch=%d\n", GetUSBSwitch());
    } else if (strcmp(argv[0], "set") == 0) {
        SetSetupStep(5);
        SetBright(50);
        SetAutoLockScreen(15);
        SetAutoPowerOff(1);
        SetVibration(0);
        SetDarkMode(0);
        SetUSBSwitch(0);
        SaveDeviceSettings();
        printf("set device settings test\n");
    } else {
        printf("device settings test arg err\n");
    }
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
    size = strlen(jsonString);
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
    } while (0);
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
        len = strlen(strTemp);
        if (len < maxLen) {
            strcpy(value, strTemp);
        } else {
            strcpy(value, "");
        }
    } else {
        printf("key:%s does not exist\r\n", key);
        strcpy(value, "");
    }
}
