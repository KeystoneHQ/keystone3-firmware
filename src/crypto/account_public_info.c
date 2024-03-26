#include <stdio.h>
#include <string.h>
#include "define.h"
#include "cjson/cJSON.h"
#include "sha256.h"
#include "flash_address.h"
#include "drv_gd25qxx.h"
#include "keystore.h"
#include "user_memory.h"
#include "account_public_info.h"
#include "account_manager.h"
#include "se_manager.h"
#include "user_utils.h"
#include "assert.h"
#include "gui.h"
#include "gui_views.h"
#include "gui_api.h"


#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

#define PUB_KEY_MAX_LENGTH                  256
#define VERSION_MAX_LENGTH                  64
#define INVALID_ACCOUNT_INDEX               255

typedef struct {
    char *pubKey;
    int32_t current;
} AccountPublicKeyItem_t;

typedef enum {
    ED25519,
    SECP256K1,
    BIP32_ED25519,
} Curve_t;

typedef struct {
    ChainType chain;
    Curve_t curve;
    char *name;
    char *path;
} ChainItem_t;


static bool GetPublicKeyFromJsonString(const char *string);
static char *GetJsonStringFromPublicKey(void);

static void FreePublicKeyRam(void);
static void PrintInfo(void);
static void GetStringValue(cJSON *obj, const char *key, char *value, uint32_t maxLen);
static bool GetBoolValue(const cJSON *obj, const char *key, bool defaultValue);

static AccountPublicKeyItem_t g_accountPublicKey[XPUB_TYPE_NUM];
static uint8_t g_tempPublicKeyAccountIndex = INVALID_ACCOUNT_INDEX;

static const char g_xpubInfoVersion[] = "1.0.0";

static const ChainItem_t g_chainTable[] = {
#ifndef BTC_ONLY
    {XPUB_TYPE_BTC,                   SECP256K1,    "btc",                      "M/49'/0'/0'"       },
    {XPUB_TYPE_BTC_LEGACY,            SECP256K1,    "btc_legacy",               "M/44'/0'/0'"       },
    {XPUB_TYPE_BTC_NATIVE_SEGWIT,     SECP256K1,    "btc_nested_segwit",        "M/84'/0'/0'"       },
    {XPUB_TYPE_BTC_TAPROOT,           SECP256K1,    "btc_taproot",              "M/86'/0'/0'"       },
    {XPUB_TYPE_LTC,                   SECP256K1,    "ltc",                      "M/49'/2'/0'"       },
    {XPUB_TYPE_DASH,                  SECP256K1,    "dash",                     "M/44'/5'/0'"       },
    {XPUB_TYPE_BCH,                   SECP256K1,    "bch",                      "M/44'/145'/0'"     },
    {XPUB_TYPE_ETH_BIP44_STANDARD,    SECP256K1,    "eth_bip44_standard",       "M/44'/60'/0'"      },
    {XPUB_TYPE_ETH_LEDGER_LEGACY,     SECP256K1,    "eth_ledger_legacy",        "M/44'/60'/0'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_0,     SECP256K1,    "eth_ledger_live_0",        "M/44'/60'/0'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_1,     SECP256K1,    "eth_ledger_live_1",        "M/44'/60'/1'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_2,     SECP256K1,    "eth_ledger_live_2",        "M/44'/60'/2'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_3,     SECP256K1,    "eth_ledger_live_3",        "M/44'/60'/3'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_4,     SECP256K1,    "eth_ledger_live_4",        "M/44'/60'/4'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_5,     SECP256K1,    "eth_ledger_live_5",        "M/44'/60'/5'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_6,     SECP256K1,    "eth_ledger_live_6",        "M/44'/60'/6'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_7,     SECP256K1,    "eth_ledger_live_7",        "M/44'/60'/7'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_8,     SECP256K1,    "eth_ledger_live_8",        "M/44'/60'/8'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_9,     SECP256K1,    "eth_ledger_live_9",        "M/44'/60'/9'"      },
    {XPUB_TYPE_TRX,                   SECP256K1,    "trx",                      "M/44'/195'/0'"     },
    {XPUB_TYPE_COSMOS,                SECP256K1,    "cosmos",                   "M/44'/118'/0'"     },
    {XPUB_TYPE_SCRT,                  SECP256K1,    "scrt",                     "M/44'/529'/0'"     },
    {XPUB_TYPE_CRO,                   SECP256K1,    "cro",                      "M/44'/394'/0'"     },
    {XPUB_TYPE_IOV,                   SECP256K1,    "iov",                      "M/44'/234'/0'"     },
    {XPUB_TYPE_BLD,                   SECP256K1,    "bld",                      "M/44'/564'/0'"     },
    {XPUB_TYPE_KAVA,                  SECP256K1,    "kava",                     "M/44'/459'/0'"     },
    {XPUB_TYPE_TERRA,                 SECP256K1,    "terra",                    "M/44'/330'/0'"     },
    {XPUB_TYPE_XRP,                   SECP256K1,    "xrp",                      "M/44'/144'/0'"     },
    {XPUB_TYPE_SOL_BIP44_0,           ED25519,      "sol_bip44_0",              "M/44'/501'/0'"     },
    {XPUB_TYPE_SOL_BIP44_1,           ED25519,      "sol_bip44_1",              "M/44'/501'/1'"     },
    {XPUB_TYPE_SOL_BIP44_2,           ED25519,      "sol_bip44_2",              "M/44'/501'/2'"     },
    {XPUB_TYPE_SOL_BIP44_3,           ED25519,      "sol_bip44_3",              "M/44'/501'/3'"     },
    {XPUB_TYPE_SOL_BIP44_4,           ED25519,      "sol_bip44_4",              "M/44'/501'/4'"     },
    {XPUB_TYPE_SOL_BIP44_5,           ED25519,      "sol_bip44_5",              "M/44'/501'/5'"     },
    {XPUB_TYPE_SOL_BIP44_6,           ED25519,      "sol_bip44_6",              "M/44'/501'/6'"     },
    {XPUB_TYPE_SOL_BIP44_7,           ED25519,      "sol_bip44_7",              "M/44'/501'/7'"     },
    {XPUB_TYPE_SOL_BIP44_8,           ED25519,      "sol_bip44_8",              "M/44'/501'/8'"     },
    {XPUB_TYPE_SOL_BIP44_9,           ED25519,      "sol_bip44_9",              "M/44'/501'/9'"     },
    {XPUB_TYPE_SOL_BIP44_ROOT,        ED25519,      "sol_bip44_root",           "M/44'/501'"        },
    {XPUB_TYPE_SOL_BIP44_CHANGE_0,    ED25519,      "sol_bip44_change_0",       "M/44'/501'/0'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_1,    ED25519,      "sol_bip44_change_1",       "M/44'/501'/1'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_2,    ED25519,      "sol_bip44_change_2",       "M/44'/501'/2'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_3,    ED25519,      "sol_bip44_change_3",       "M/44'/501'/3'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_4,    ED25519,      "sol_bip44_change_4",       "M/44'/501'/4'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_5,    ED25519,      "sol_bip44_change_5",       "M/44'/501'/5'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_6,    ED25519,      "sol_bip44_change_6",       "M/44'/501'/6'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_7,    ED25519,      "sol_bip44_change_7",       "M/44'/501'/7'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_8,    ED25519,      "sol_bip44_change_8",       "M/44'/501'/8'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_9,    ED25519,      "sol_bip44_change_9",       "M/44'/501'/9'/0'"  },
    {XPUB_TYPE_SUI_0,                 ED25519,      "sui_0",                    "M/44'/784'/0'/0'/0'"},
    {XPUB_TYPE_SUI_1,                 ED25519,      "sui_1",                    "M/44'/784'/1'/0'/0'"},
    {XPUB_TYPE_SUI_2,                 ED25519,      "sui_2",                    "M/44'/784'/2'/0'/0'"},
    {XPUB_TYPE_SUI_3,                 ED25519,      "sui_3",                    "M/44'/784'/3'/0'/0'"},
    {XPUB_TYPE_SUI_4,                 ED25519,      "sui_4",                    "M/44'/784'/4'/0'/0'"},
    {XPUB_TYPE_SUI_5,                 ED25519,      "sui_5",                    "M/44'/784'/5'/0'/0'"},
    {XPUB_TYPE_SUI_6,                 ED25519,      "sui_6",                    "M/44'/784'/6'/0'/0'"},
    {XPUB_TYPE_SUI_7,                 ED25519,      "sui_7",                    "M/44'/784'/7'/0'/0'"},
    {XPUB_TYPE_SUI_8,                 ED25519,      "sui_8",                    "M/44'/784'/8'/0'/0'"},
    {XPUB_TYPE_SUI_9,                 ED25519,      "sui_9",                    "M/44'/784'/9'/0'/0'"},
    {XPUB_TYPE_APT_0,                 ED25519,      "apt_0",                    "M/44'/637'/0'/0'/0'"},
    {XPUB_TYPE_APT_1,                 ED25519,      "apt_1",                    "M/44'/637'/1'/0'/0'"},
    {XPUB_TYPE_APT_2,                 ED25519,      "apt_2",                    "M/44'/637'/2'/0'/0'"},
    {XPUB_TYPE_APT_3,                 ED25519,      "apt_3",                    "M/44'/637'/3'/0'/0'"},
    {XPUB_TYPE_APT_4,                 ED25519,      "apt_4",                    "M/44'/637'/4'/0'/0'"},
    {XPUB_TYPE_APT_5,                 ED25519,      "apt_5",                    "M/44'/637'/5'/0'/0'"},
    {XPUB_TYPE_APT_6,                 ED25519,      "apt_6",                    "M/44'/637'/6'/0'/0'"},
    {XPUB_TYPE_APT_7,                 ED25519,      "apt_7",                    "M/44'/637'/7'/0'/0'"},
    {XPUB_TYPE_APT_8,                 ED25519,      "apt_8",                    "M/44'/637'/8'/0'/0'"},
    {XPUB_TYPE_APT_9,                 ED25519,      "apt_9",                    "M/44'/637'/9'/0'/0'"},
    {XPUB_TYPE_ADA_0,                 BIP32_ED25519, "ada_0",                    "M/1852'/1815'/0'"},
    {XPUB_TYPE_ADA_1,                 BIP32_ED25519, "ada_1",                    "M/1852'/1815'/1'"},
    {XPUB_TYPE_ADA_2,                 BIP32_ED25519, "ada_2",                    "M/1852'/1815'/2'"},
    {XPUB_TYPE_ADA_3,                 BIP32_ED25519, "ada_3",                    "M/1852'/1815'/3'"},
    {XPUB_TYPE_ADA_4,                 BIP32_ED25519, "ada_4",                    "M/1852'/1815'/4'"},
    {XPUB_TYPE_ADA_5,                 BIP32_ED25519, "ada_5",                    "M/1852'/1815'/5'"},
    {XPUB_TYPE_ADA_6,                 BIP32_ED25519, "ada_6",                    "M/1852'/1815'/6'"},
    {XPUB_TYPE_ADA_7,                 BIP32_ED25519, "ada_7",                    "M/1852'/1815'/7'"},
    {XPUB_TYPE_ADA_8,                 BIP32_ED25519, "ada_8",                    "M/1852'/1815'/8'"},
    {XPUB_TYPE_ADA_9,                 BIP32_ED25519, "ada_9",                    "M/1852'/1815'/9'"},
    {XPUB_TYPE_ADA_10,                BIP32_ED25519, "ada_10",                   "M/1852'/1815'/10'"},
    {XPUB_TYPE_ADA_11,                BIP32_ED25519, "ada_11",                   "M/1852'/1815'/11'"},
    {XPUB_TYPE_ADA_12,                BIP32_ED25519, "ada_12",                   "M/1852'/1815'/12'"},
    {XPUB_TYPE_ADA_13,                BIP32_ED25519, "ada_13",                   "M/1852'/1815'/13'"},
    {XPUB_TYPE_ADA_14,                BIP32_ED25519, "ada_14",                   "M/1852'/1815'/14'"},
    {XPUB_TYPE_ADA_15,                BIP32_ED25519, "ada_15",                   "M/1852'/1815'/15'"},
    {XPUB_TYPE_ADA_16,                BIP32_ED25519, "ada_16",                   "M/1852'/1815'/16'"},
    {XPUB_TYPE_ADA_17,                BIP32_ED25519, "ada_17",                   "M/1852'/1815'/17'"},
    {XPUB_TYPE_ADA_18,                BIP32_ED25519, "ada_18",                   "M/1852'/1815'/18'"},
    {XPUB_TYPE_ADA_19,                BIP32_ED25519, "ada_19",                   "M/1852'/1815'/19'"},
    {XPUB_TYPE_ADA_20,                BIP32_ED25519, "ada_20",                   "M/1852'/1815'/20'"},
    {XPUB_TYPE_ADA_21,                BIP32_ED25519, "ada_21",                   "M/1852'/1815'/21'"},
    {XPUB_TYPE_ADA_22,                BIP32_ED25519, "ada_22",                   "M/1852'/1815'/22'"},
    {XPUB_TYPE_ADA_23,                BIP32_ED25519, "ada_23",                   "M/1852'/1815'/23'"},
#else
    {XPUB_TYPE_BTC,                     SECP256K1,      "btc_nested_segwit",        "M/49'/0'/0'"   },
    {XPUB_TYPE_BTC_LEGACY,              SECP256K1,      "btc_legacy",               "M/44'/0'/0'"   },
    {XPUB_TYPE_BTC_NATIVE_SEGWIT,       SECP256K1,      "btc_native_segwit",        "M/84'/0'/0'"   },
    {XPUB_TYPE_BTC_TAPROOT,             SECP256K1,      "btc_taproot",              "M/86'/0'/0'"   },
    {XPUB_TYPE_BTC_TEST,                SECP256K1,      "btc_nested_segwit_test",   "M/49'/1'/0'"   },
    {XPUB_TYPE_BTC_LEGACY_TEST,         SECP256K1,      "btc_legacy_test",          "M/44'/1'/0'"   },
    {XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST,  SECP256K1,      "btc_native_segwit_test",   "M/84'/1'/0'"   },
    {XPUB_TYPE_BTC_TAPROOT_TEST,        SECP256K1,      "btc_taproot_test",         "M/86'/1'/0'"   },
#endif
};

char *GetXPubPath(uint8_t index)
{
    ASSERT(index < XPUB_TYPE_NUM);
    return g_chainTable[index].path;
}


int32_t AccountPublicInfoSwitch(uint8_t accountIndex, const char *password, bool newKey)
{
}

int32_t TempAccountPublicInfo(uint8_t accountIndex, const char *password, bool set)
{
}


void DeleteAccountPublicInfo(uint8_t accountIndex)
{
}

char *GetCurrentAccountPublicKey(ChainType chain)
{
}


/// @brief Get if the xPub already Exists.
/// @param[in] xPub
/// @return accountIndex, if not exists, return 255.
uint8_t SpecifiedXPubExist(const char *xPub)
{
}


/// @brief
/// @param argc Test arg count.
/// @param argv Test arg values.
void AccountPublicInfoTest(int argc, char *argv[])
{
}


static bool GetPublicKeyFromJsonString(const char *string)
{
}


static char *GetJsonStringFromPublicKey(void)
{
}


static void FreePublicKeyRam(void)
{
    g_tempPublicKeyAccountIndex = INVALID_ACCOUNT_INDEX;
    for (uint32_t i = 0; i < XPUB_TYPE_NUM; i++) {
        if (g_accountPublicKey[i].pubKey != NULL) {
            SRAM_FREE(g_accountPublicKey[i].pubKey);
            g_accountPublicKey[i].pubKey = NULL;
        }
    }
}


static void PrintInfo(void)
{
}


static void GetStringValue(cJSON *obj, const char *key, char *value, uint32_t maxLen)
{
    cJSON *json;
    uint32_t len;
    char *strTemp;

    json = cJSON_GetObjectItem(obj, key);
    if (json != NULL) {
        strTemp = json->valuestring;
        len = strnlen_s(strTemp, 256);
        if (len < maxLen) {
            strcpy(value, strTemp);
        } else {
            strcpy(value, "");
        }
    } else {
        strcpy(value, "");
    }
}

static bool GetBoolValue(const cJSON *obj, const char *key, bool defaultValue)
{
    cJSON *boolJson = cJSON_GetObjectItem((cJSON *)obj, key);
    if (boolJson != NULL) {
        return boolJson->valueint != 0;
    }
    printf("key:%s does not exist\r\n", key);
    return defaultValue;
}

bool GetFirstReceive(const char* chainName)
{
    int32_t ret = SUCCESS_CODE;
    uint32_t addr, size;
    char *jsonString = NULL;

    uint8_t account = GetCurrentAccountIndex();
    ASSERT(account < 3);
    addr = SPI_FLASH_ADDR_USER1_MUTABLE_DATA + account * SPI_FLASH_ADDR_EACH_SIZE;
    ret = Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
    ASSERT(ret == 4);

    jsonString = SRAM_MALLOC(size + 1);
    ret = Gd25FlashReadBuffer(addr + 4, (uint8_t *)jsonString, size);
    ASSERT(ret == size);
    jsonString[size] = 0;

    cJSON *rootJson = cJSON_Parse(jsonString);
    SRAM_FREE(jsonString);
    cJSON *item = cJSON_GetObjectItem(rootJson, chainName);
    bool state = false;
    if (item == NULL) {
        printf("GetFirstReceive cannot get %s\r\n", chainName);
    } else {
        cJSON *firstRecv = cJSON_GetObjectItem(item, "firstRecv");
        state = firstRecv->valueint;
    }
    cJSON_Delete(rootJson);
    return state;
}

void SetFirstReceive(const char* chainName, bool isFirst)
{
    int32_t ret = SUCCESS_CODE;
    uint32_t addr, size, eraseAddr;
    char *jsonString = NULL;

    uint8_t account = GetCurrentAccountIndex();
    ASSERT(account < 3);
    addr = SPI_FLASH_ADDR_USER1_MUTABLE_DATA + account * SPI_FLASH_ADDR_EACH_SIZE;
    ret = Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
    ASSERT(ret == 4);

    jsonString = SRAM_MALLOC(size + 1);
    ret = Gd25FlashReadBuffer(addr + 4, (uint8_t *)jsonString, size);
    ASSERT(ret == size);
    jsonString[size] = 0;

    cJSON *rootJson = cJSON_Parse(jsonString);
    SRAM_FREE(jsonString);
    cJSON *item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        printf("SetFirstReceive cannot get %s\r\n", chainName);
        cJSON_Delete(rootJson);
        return;
    }
    cJSON_ReplaceItemInObject(item, "firstRecv", cJSON_CreateBool(isFirst));

    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MUTABLE_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }
    jsonString = cJSON_Print(rootJson);
    cJSON_Delete(rootJson);
    RemoveFormatChar(jsonString);
    size = strlen(jsonString);
    Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
    Gd25FlashWriteBuffer(addr + 4, (uint8_t *)jsonString, size);
}
