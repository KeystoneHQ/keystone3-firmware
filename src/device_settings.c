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
#include "lv_i18n_api.h"
#include "fetch_sensitive_data_task.h"
#include "ctaes.h"
#include "drv_mpu.h"

#define VERSION_MAX_LENGTH      32

#define KEY_VERSION                     "version"

#define KEY_SETUP_STEP                  "setup_step"
#define KEY_BRIGHT                      "bright"
#define KEY_AUTO_LOCK_SCREEN            "auto_lock_screen"
#define KEY_AUTO_POWER_OFF              "auto_power_off"
#define KEY_VIBRATION                   "vibration"
#define KEY_PERMIT_SIGN                 "permit_sign"
#define KEY_DARK_MODE                   "dark_mode"
#define KEY_USB_SWITCH                  "usb_switch"
#define KEY_LAST_VERSION                "last_version"
#define KEY_LANGUAGE                    "language"
#define KEY_NFT_SCREEN                  "nftEnable"
#define KEY_NFT_VALID                   "nftValid"
#define KEY_ENABLE_BLIND_SIGNING        "enableBlindSigning"
#define DEFAULT_SETUP_STEP              0
#define DEFAULT_BRIGHT                  15
#define DEFAULT_AUTO_LOCK_SCREEN        60
#define DEFAULT_AUTO_POWER_OFF          0
#define DEFAULT_VIBRATION               1
#define DEFAULT_PERMIT_SIGN             0
#define DEFAULT_DARK_MODE               1
#define DEFAULT_USB_SWITCH              1
#define DEFAULT_LAST_VERSION            0
#define DEFAULT_LANGUAGE                LANG_EN
typedef struct {
    uint32_t setupStep;
    uint32_t bright;
    uint32_t autoLockScreen;
    uint32_t autoPowerOff;
    uint32_t vibration;
    uint32_t permitSign;
    uint32_t darkMode;
    uint32_t usbSwitch;
    uint32_t lastVersion;
    uint32_t language;
    bool nftEnable;
    bool nftValid;
    bool enableBlindSigning;
} DeviceSettings_t;

static int32_t SaveDeviceSettingsAsyncFunc(const void *inData, uint32_t inDataLen);
static void SaveDeviceSettingsSync(void);
static bool GetDeviceSettingsFromJsonString(const char *string);
static char *GetJsonStringFromDeviceSettings(void);
static void AesEncryptBuffer(uint8_t *cipher, uint32_t sz, uint8_t *plain);
static void AesDecryptBuffer(uint8_t *plain, uint32_t sz, uint8_t *cipher);

typedef struct {
    // boot check
    uint8_t bootCheckFlag[16];

    // recovery mode switch
    uint8_t recoveryModeSwitch[16];
} BootParam_t;

static BootParam_t g_bootParam;
static const char g_deviceSettingsVersion[] = "1.0.0";
DeviceSettings_t g_deviceSettings;
static const uint8_t g_integrityFlag[16] = {
    0x01, 0x09, 0x00, 0x03,
    0x01, 0x09, 0x00, 0x03,
    0x01, 0x09, 0x00, 0x03,
    0x01, 0x09, 0x00, 0x03,
};
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
        g_deviceSettings.permitSign = DEFAULT_PERMIT_SIGN;
        g_deviceSettings.darkMode = DEFAULT_DARK_MODE;
        g_deviceSettings.usbSwitch = DEFAULT_USB_SWITCH;
        g_deviceSettings.lastVersion = DEFAULT_LAST_VERSION;
        g_deviceSettings.language = DEFAULT_LANGUAGE;
        g_deviceSettings.nftEnable = false;
        g_deviceSettings.nftValid = false;
        g_deviceSettings.enableBlindSigning = false;
        SaveDeviceSettingsSync();
    }

    // init boot param
    InitBootParam();
}

void InitBootParam(void)
{
    BootParam_t bootParam;
    bool needSave = false;
    uint8_t cipher[sizeof(g_bootParam)] = {0};
    uint8_t plain[sizeof(g_bootParam)] = {0};
    Gd25FlashReadBuffer(BOOT_SECURE_PARAM_FLAG, (uint8_t *)&bootParam, sizeof(bootParam));
    PrintArray("bootParam.bootCheckFlag", bootParam.bootCheckFlag, sizeof(bootParam.bootCheckFlag));
    PrintArray("bootParam.recoveryModeSwitch", bootParam.recoveryModeSwitch, sizeof(bootParam.recoveryModeSwitch));
    if (CheckAllFF(bootParam.bootCheckFlag, sizeof(bootParam.bootCheckFlag))) {
        memcpy(g_bootParam.bootCheckFlag, g_integrityFlag, sizeof(bootParam.bootCheckFlag));
        needSave = true;
    }
    if (CheckAllFF(bootParam.recoveryModeSwitch, sizeof(bootParam.recoveryModeSwitch))) {
    }

    if (needSave) {
        SaveBootParam();
    } else {
        AesDecryptBuffer(&g_bootParam, sizeof(g_bootParam), &bootParam);
        if (memcmp(g_bootParam.recoveryModeSwitch, g_integrityFlag, sizeof(g_bootParam.recoveryModeSwitch)) != 0) {
            PrintArray("bootParam.recoveryModeSwitch", g_bootParam.recoveryModeSwitch, sizeof(g_bootParam.recoveryModeSwitch));
            memset(g_bootParam.recoveryModeSwitch, 0, sizeof(g_bootParam.recoveryModeSwitch));
            if (SaveBootParam() != SUCCESS_CODE) {
                printf("SaveBootParam failed\n");
            } else {
                printf("SaveBootParam success\n");
            }
            Gd25FlashReadBuffer(BOOT_SECURE_PARAM_FLAG, (uint8_t *)&bootParam, sizeof(bootParam));
            AesDecryptBuffer(&g_bootParam, sizeof(g_bootParam), &bootParam);
        }
        PrintArray("bootParam.bootCheckFlag", g_bootParam.bootCheckFlag, sizeof(g_bootParam.bootCheckFlag));
        PrintArray("bootParam.recoveryModeSwitch", g_bootParam.recoveryModeSwitch, sizeof(g_bootParam.recoveryModeSwitch));
    }
}

void ResetBootParam(void)
{
    memset(g_bootParam.recoveryModeSwitch, 0, sizeof(g_bootParam.recoveryModeSwitch));
    memcpy(g_bootParam.bootCheckFlag, g_integrityFlag, sizeof(g_bootParam.bootCheckFlag));
    SaveBootParam();
}

int SaveBootParam(void)
{
    int ret = SUCCESS_CODE;
    uint8_t cipher[sizeof(g_bootParam)] = {0};
    uint8_t plain[sizeof(g_bootParam)] = {0};
    uint8_t verifyBuffer[sizeof(g_bootParam)] = {0};

    if (sizeof(g_bootParam) > BOOT_SECURE_PARAM_FLAG_SIZE) {
        return ERR_GD25_BAD_PARAM;
    }

    memcpy(plain, &g_bootParam, sizeof(g_bootParam));

    AesEncryptBuffer(cipher, sizeof(cipher), plain);

    if (Gd25FlashSectorErase(BOOT_SECURE_PARAM_FLAG) != SUCCESS_CODE) {
        ret = ERR_GD25_BAD_PARAM;
        goto cleanup;
    }

    Gd25FlashWriteBuffer(BOOT_SECURE_PARAM_FLAG, cipher, sizeof(cipher));

    Gd25FlashReadBuffer(BOOT_SECURE_PARAM_FLAG, verifyBuffer, sizeof(verifyBuffer));

    if (memcmp(cipher, verifyBuffer, sizeof(cipher)) != 0) {
        ret = ERR_GD25_BAD_PARAM;
    }

cleanup:
    memset(plain, 0, sizeof(plain));
    memset(cipher, 0, sizeof(cipher));
    memset(verifyBuffer, 0, sizeof(verifyBuffer));

    return ret;
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

uint32_t GetPermitSign(void)
{
    return g_deviceSettings.permitSign;
}

void SetPermitSign(uint32_t permitSign)
{
    g_deviceSettings.permitSign = permitSign;
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

static void AesEncryptBuffer(uint8_t *cipher, uint32_t sz, uint8_t *plain)
{
    AES128_CBC_ctx aesCtx;
    uint8_t key128[16] = {0};
    uint8_t iv[16] = {0};

    MpuSetOtpProtection(false);
    OTP_PowerOn();
    memcpy(key128, (uint32_t *)(0x40009128), 16);
    memcpy(iv, (uint32_t *)(0x40009138), 16);

    AES128_CBC_ctx ctx;
    AES128_CBC_init(&ctx, key128, iv);
    AES128_CBC_encrypt(&ctx, sz / 16, cipher, plain);
    MpuSetOtpProtection(true);
}

static void AesDecryptBuffer(uint8_t *plain, uint32_t sz, uint8_t *cipher)
{
    AES128_CBC_ctx ctx;
    uint8_t key128[16] = {0};
    uint8_t iv[16] = {0};

    MpuSetOtpProtection(false);
    OTP_PowerOn();
    memcpy(key128, (uint32_t *)(0x40009128), 16);
    memcpy(iv, (uint32_t *)(0x40009138), 16);
    AES128_CBC_init(&ctx, key128, iv);
    AES128_CBC_decrypt(&ctx, sz / 16, plain, cipher);
    MpuSetOtpProtection(true);
}

bool GetBootSecureCheckFlag(void)
{
    return (memcmp(g_bootParam.bootCheckFlag, g_integrityFlag, sizeof(g_bootParam.bootCheckFlag)) == 0);
}

void SetBootSecureCheckFlag(bool isSet)
{
    if (isSet) {
        memcpy(g_bootParam.bootCheckFlag, g_integrityFlag, sizeof(g_integrityFlag));
    } else {
        memset(g_bootParam.bootCheckFlag, 0, sizeof(g_bootParam.bootCheckFlag));
    }
    SaveBootParam();
}

bool GetRecoveryModeSwitch(void)
{
    return (memcmp(g_bootParam.recoveryModeSwitch, g_integrityFlag, sizeof(g_bootParam.recoveryModeSwitch)) == 0);
}

void SetRecoveryModeSwitch(bool isSet)
{
    if (isSet) {
        memcpy(g_bootParam.recoveryModeSwitch, g_integrityFlag, sizeof(g_bootParam.recoveryModeSwitch));
    } else {
        memset(g_bootParam.recoveryModeSwitch, 0, sizeof(g_bootParam.recoveryModeSwitch));
    }
    SaveBootParam();
    PrintArray("g_bootParam.recoveryModeSwitch", g_bootParam.recoveryModeSwitch, sizeof(g_bootParam.recoveryModeSwitch));
    printf("get recovery mode switch=%d\n", GetRecoveryModeSwitch());
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

bool IsNftScreenValid(void)
{
    return g_deviceSettings.nftValid;
}

void SetNftBinValid(bool en)
{
    g_deviceSettings.nftValid = en;
    if (en == false) {
        g_deviceSettings.nftEnable = false;
    }
}

bool GetNftScreenSaver(void)
{
    return g_deviceSettings.nftEnable;
}

void SetNftScreenSaver(bool enable)
{
    g_deviceSettings.nftEnable = enable;
}

bool GetEnableBlindSigning(void)
{
    return g_deviceSettings.enableBlindSigning;
}

void SetEnableBlindSigning(bool enable)
{
    g_deviceSettings.enableBlindSigning = enable;
}

uint32_t GetLanguage(void)
{
    return g_deviceSettings.language;
}

void SetLanguage(uint32_t language)
{
    g_deviceSettings.language = language;
}

/// @brief Wipe device.
void WipeDevice(void)
{
    // reset all account address index in receive page
    {
        void GuiResetAllUtxoAddressIndex(void);
        GuiResetAllUtxoAddressIndex();
#ifdef WEB3_VERSION
        void GuiResetAllEthAddressIndex(void);
        void GuiResetAllStandardAddressIndex(void);
        GuiResetAllEthAddressIndex();
        GuiResetAllStandardAddressIndex();
#endif
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
        printf("language=%d\n", GetLanguage());
        printf("enableBlindSigning=%d\n", GetEnableBlindSigning());
    } else if (strcmp(argv[0], "set") == 0) {
        SetSetupStep(0);
        SetBright(50);
        SetAutoLockScreen(15);
        SetAutoPowerOff(1);
        SetVibration(0);
        SetDarkMode(0);
        SetUSBSwitch(0);
        g_deviceSettings.lastVersion = 2;
        SetLanguage(DEFAULT_LANGUAGE);
        SetEnableBlindSigning(false);
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
    Gd25FlashSectorErase(SPI_FLASH_ADDR_NORMAL_PARAM); // Only one sector for device settings.
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
        g_deviceSettings.permitSign = GetIntValue(rootJson, KEY_PERMIT_SIGN, DEFAULT_PERMIT_SIGN);
        g_deviceSettings.darkMode = GetIntValue(rootJson, KEY_DARK_MODE, DEFAULT_DARK_MODE);
        g_deviceSettings.usbSwitch = GetIntValue(rootJson, KEY_USB_SWITCH, DEFAULT_USB_SWITCH);
        g_deviceSettings.lastVersion = GetIntValue(rootJson, KEY_LAST_VERSION, DEFAULT_LAST_VERSION);
        g_deviceSettings.language = GetIntValue(rootJson, KEY_LANGUAGE, DEFAULT_LANGUAGE);
        g_deviceSettings.nftEnable = GetBoolValue(rootJson, KEY_NFT_SCREEN, false);
        g_deviceSettings.nftValid = GetBoolValue(rootJson, KEY_NFT_VALID, false);
        g_deviceSettings.enableBlindSigning = GetBoolValue(rootJson, KEY_ENABLE_BLIND_SIGNING, false);
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
    cJSON_AddItemToObject(rootJson, KEY_PERMIT_SIGN, cJSON_CreateNumber(g_deviceSettings.permitSign));
    cJSON_AddItemToObject(rootJson, KEY_DARK_MODE, cJSON_CreateNumber(g_deviceSettings.darkMode));
    cJSON_AddItemToObject(rootJson, KEY_USB_SWITCH, cJSON_CreateNumber(g_deviceSettings.usbSwitch));
    cJSON_AddItemToObject(rootJson, KEY_LAST_VERSION, cJSON_CreateNumber(g_deviceSettings.lastVersion));
    cJSON_AddItemToObject(rootJson, KEY_LANGUAGE, cJSON_CreateNumber(g_deviceSettings.language));
    cJSON_AddItemToObject(rootJson, KEY_NFT_SCREEN, cJSON_CreateBool(g_deviceSettings.nftEnable));
    cJSON_AddItemToObject(rootJson, KEY_NFT_VALID, cJSON_CreateBool(g_deviceSettings.nftValid));
    cJSON_AddItemToObject(rootJson, KEY_ENABLE_BLIND_SIGNING, cJSON_CreateBool(g_deviceSettings.enableBlindSigning));
    retStr = cJSON_PrintBuffered(rootJson, SPI_FLASH_SIZE_NORMAL_PARAM - 4, false);
    RemoveFormatChar(retStr);
    cJSON_Delete(rootJson);

    return retStr;
}
