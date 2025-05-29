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
#include "librust_c.h"
#include "assert.h"
#include "gui.h"
#include "gui_views.h"
#include "gui_api.h"
#include "gui_home_widgets.h"
#include "user_fatfs.h"
#include "multi_sig_wallet_manager.h"
#include "log_print.h"
#include "rsa.h"
#include "gui_model.h"

#define PUB_KEY_MAX_LENGTH                  1024 + 1
#define VERSION_MAX_LENGTH                  64
#define INVALID_ACCOUNT_INDEX               255

typedef struct {
    char *value;
    int32_t current;
} AccountPublicKeyItem_t;

typedef enum {
    ED25519,
    SECP256K1,
    BIP32_ED25519,
    RSA_KEY,
    TON_NATIVE,
    TON_CHECKSUM,
    LEDGER_BITBOX02,
    ZCASH_UFVK_ENCRYPTED,
    EDWARDS_25519,
    MONERO_PVK,
} PublicInfoType_t;

typedef struct {
    ChainType chain;
    PublicInfoType_t cryptoKey;
    char *name;
    char *path;
} ChainItem_t;

static bool GetPublicKeyFromJsonString(const char *string);
static char *GetJsonStringFromPublicKey(void);
static cJSON* ReadAndParseAccountJson(uint32_t *outAddr, uint32_t *outSize);
static void WriteJsonToFlash(uint32_t addr, cJSON *rootJson);
static uint32_t GetTemplateWalletValue(const char* walletName, const char* key);
static void SetTemplateWalletValue(const char* walletName, const char* key, uint32_t value);
static void CleanupJson(cJSON* json);
static void FreePublicKeyRam(void);
static void PrintInfo(void);
static void SetIsTempAccount(bool isTemp);

#ifdef BTC_ONLY
static void LoadCurrentAccountMultiReceiveIndex(void);
typedef struct {
    char verifyCode[BUFFER_SIZE_16];
    uint32_t index;
} MultiSigReceiveIndex_t;

static MultiSigReceiveIndex_t g_multiSigReceiveIndex[4];

static void ConvertXPub(char *dest, ChainType chainType);
uint32_t GetAccountMultiReceiveIndexFromFlash(char *verifyCode);

static void LoadCurrentAccountMultiReceiveIndex(void)
{
    for (int i = 0; i < 4; i++) {
        memset_s(&g_multiSigReceiveIndex[i], sizeof(MultiSigReceiveIndex_t), 0, sizeof(MultiSigReceiveIndex_t));
        if (GetCurrenMultisigWalletByIndex(i) == NULL) {
            continue;
        }
        strcpy(g_multiSigReceiveIndex[i].verifyCode, GetCurrenMultisigWalletByIndex(i)->verifyCode);
    }
}

static void replace(char *str, const char *old_str, const char *new_str)
{
    char *pos = strstr(str, old_str);
    if (pos != NULL) {
        size_t old_len = strlen(old_str);
        size_t new_len = strlen(new_str);
        size_t tail_len = strlen(pos + old_len);

        memmove(pos + new_len, pos + old_len, tail_len + 1);
        memcpy(pos, new_str, new_len);

        replace(pos + new_len, old_str, new_str);
    }
}

void ExportMultiSigXpub(ChainType chainType)
{
    ASSERT(chainType >= XPUB_TYPE_BTC_MULTI_SIG_P2SH);
    ASSERT(chainType <= XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST);

    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char mfpHexStr[9] = {0};
    ByteArrayToHexStr(mfp, sizeof(mfp), mfpHexStr);

    char path[64] = {0};
    strcpy(path, GetXPubPath(chainType));
    replace(path, "M", "m");

    char xpub[128] = {0};
    ConvertXPub(xpub, chainType);

    char *jsonString = NULL;
    cJSON *rootJson;
    rootJson = cJSON_CreateObject();
    cJSON_AddItemToObject(rootJson, "xfp", cJSON_CreateString(mfpHexStr));
    cJSON_AddItemToObject(rootJson, "xpub", cJSON_CreateString(xpub));
    cJSON_AddItemToObject(rootJson, "path", cJSON_CreateString(path));
    jsonString = cJSON_PrintBuffered(rootJson, 1024, false);
    RemoveFormatChar(jsonString);

    char exportFileName[32] = {0};

    switch (chainType) {
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH:
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST:
        sprintf(exportFileName, "0:%s_%s.json", mfpHexStr, "P2SH");
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH:
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST:
        sprintf(exportFileName, "0:%s_%s.json", mfpHexStr, "P2SH-P2WSH");
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH:
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST:
        sprintf(exportFileName, "0:%s_%s.json", mfpHexStr, "P2WSH");
        break;
    default:
        break;
    }

    int res = FatfsFileWrite(exportFileName, (uint8_t *)jsonString, strlen(jsonString));

    printf("export data is %s\r\n", jsonString);

    if (res == RES_OK) {
        printf("multi sig write to sdcard success\r\n");
    } else {
        printf("multi sig write to sdcard fail\r\n");
    }

    cJSON_Delete(rootJson);
    EXT_FREE(jsonString);
}

static void ConvertXPub(char *dest, ChainType chainType)
{
    SimpleResponse_c_char *result;

    char *xpub = GetCurrentAccountPublicKey(chainType);
    char head[] = "xpub";
    switch (chainType) {
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH:
        sprintf(dest, "%s", xpub);
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH:
        head[0] = 'Y';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH:
        head[0] = 'Z';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST:
        head[0] = 't';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST:
        head[0] = 'U';
        break;
    case XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST:
        head[0] = 'V';
        break;
    default:
        break;
    }
    result = xpub_convert_version(xpub, head);
    ASSERT(result);
    sprintf(dest, "%s", result->data);
    free_simple_response_c_char(result);
}

void ExportMultiSigWallet(char *verifyCode, uint8_t accountIndex)
{
    ASSERT(accountIndex >= 0);
    ASSERT(accountIndex <= 2);

    MultiSigWalletItem_t *multiSigWalletItem = GetMultisigWalletByVerifyCode(verifyCode);
    if (multiSigWalletItem == NULL) {
        printf("multiSigWalletItem == NULL\r\n");
        return;
    }

    char exportFileName[32] = {0};
    sprintf(exportFileName, "0:exprot-%s.txt", multiSigWalletItem->name);
    int res =  FatfsFileWrite(exportFileName, (uint8_t *)multiSigWalletItem->walletConfig, strlen(multiSigWalletItem->walletConfig));
    printf("export file name  is %s\r\n", exportFileName);
    printf("export data is %s\r\n", multiSigWalletItem->walletConfig);
    if (res == RES_OK) {
        printf("multi sig write to sdcard success\r\n");
    } else {
        printf("multi sig write to sdcard fail\r\n");
    }
}

uint32_t GetAccountMultiReceiveIndex(char *verifyCode)
{
    for (int i = 0; i < 4; i++) {
        if (strcmp(g_multiSigReceiveIndex[i].verifyCode, verifyCode) == 0) {
            return g_multiSigReceiveIndex[i].index;
        }
    }
    return 0;
}

uint32_t GetAccountMultiReceiveIndexFromFlash(char *verifyCode)
{
    char key[BUFFER_SIZE_64] = {0};
    sprintf(key, "multiRecvIndex_%s", verifyCode);
    printf("key = %s.\n", key);
    return GetTemplateWalletValue("BTC", key);
}

void SetAccountMultiReceiveIndex(uint32_t index, char *verifyCode)
{
    char key[BUFFER_SIZE_64] = {0};
    sprintf(key, "multiRecvIndex_%s", verifyCode);
    printf("key = %s.\n", key);
    for (int i = 0; i < 4; i++) {
        if (strlen(g_multiSigReceiveIndex[i].verifyCode) == 0) {
            g_multiSigReceiveIndex[i].index = index;
            strcpy(g_multiSigReceiveIndex[i].verifyCode, verifyCode);
            break;
        } else if (strcmp(g_multiSigReceiveIndex[i].verifyCode, verifyCode) == 0) {
            g_multiSigReceiveIndex[i].index = index;
            break;
        }
    }
    SetTemplateWalletValue("BTC", key, index);
}

void DeleteAccountMultiReceiveIndex(const char* chainName, char *verifyCode)
{
    uint32_t addr;
    char key[BUFFER_SIZE_64] = {0};
    sprintf(key, "multiRecvIndex_%s", verifyCode);
    printf("key = %s.\n", key);
    cJSON* rootJson = ReadAndParseAccountJson(&addr, NULL);

    cJSON* item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        item = cJSON_CreateObject();
        cJSON_AddItemToObject(rootJson, chainName, item);
    }

    cJSON* valueItem = cJSON_GetObjectItem(item, key);
    if (valueItem != NULL) {
        cJSON_DeleteItemFromObject(item, key);
    }

    WriteJsonToFlash(addr, rootJson);
    CleanupJson(rootJson);
    LoadCurrentAccountMultiReceiveIndex();
}

uint32_t GetAccountTestReceiveIndex(const char* chainName)
{
    return GetTemplateWalletValue(chainName, "testRecvIndex");
}

void SetAccountTestReceiveIndex(const char* chainName, uint32_t index)
{
    SetTemplateWalletValue(chainName, "testRecvIndex", index);
}

uint32_t GetAccountTestReceivePath(const char* chainName)
{
    return GetTemplateWalletValue(chainName, "testRecvPath");
}

void SetAccountTestReceivePath(const char* chainName, uint32_t index)
{
    SetTemplateWalletValue(chainName, "testRecvPath", index);
}
#endif

static AccountPublicKeyItem_t g_accountPublicInfo[XPUB_TYPE_NUM] = {0};

static cJSON *g_tempParsePhraseJson = NULL;

static uint8_t g_tempPublicKeyAccountIndex = INVALID_ACCOUNT_INDEX;
static bool g_isTempAccount = false;

static const char g_xpubInfoVersion[] = "1.0.0";
static const char g_multiSigInfoVersion[] = "1.0.0";

static const ChainItem_t g_chainTable[] = {
    {XPUB_TYPE_BTC,                   SECP256K1,    "btc",                      "M/49'/0'/0'"       },
    {XPUB_TYPE_BTC_LEGACY,            SECP256K1,    "btc_legacy",               "M/44'/0'/0'"       },
    {XPUB_TYPE_BTC_NATIVE_SEGWIT,     SECP256K1,    "btc_nested_segwit",        "M/84'/0'/0'"       },
    {XPUB_TYPE_BTC_TAPROOT,           SECP256K1,    "btc_taproot",              "M/86'/0'/0'"       },
#ifdef WEB3_VERSION
    {XPUB_TYPE_LTC,                   SECP256K1,    "ltc",                      "M/49'/2'/0'"       },
    {XPUB_TYPE_DOGE,                  SECP256K1,    "doge",                     "M/44'/3'/0'"       },
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
    {XPUB_TYPE_THOR,                  SECP256K1,    "thor",                     "M/44'/931'/0'"     },
    {XPUB_TYPE_AVAX_BIP44_STANDARD,   SECP256K1,    "avax_c",                   "M/44'/60'/0'"      },
    {XPUB_TYPE_AVAX_X_P,              SECP256K1,    "avax_x_p",                 "M/44'/9000'/0'"    },
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
    {XPUB_TYPE_SOL_BIP44_10,          ED25519,      "sol_bip44_10",             "M/44'/501'/10'"    },
    {XPUB_TYPE_SOL_BIP44_11,          ED25519,      "sol_bip44_11",             "M/44'/501'/11'"    },
    {XPUB_TYPE_SOL_BIP44_12,          ED25519,      "sol_bip44_12",             "M/44'/501'/12'"    },
    {XPUB_TYPE_SOL_BIP44_13,          ED25519,      "sol_bip44_13",             "M/44'/501'/13'"    },
    {XPUB_TYPE_SOL_BIP44_14,          ED25519,      "sol_bip44_14",             "M/44'/501'/14'"    },
    {XPUB_TYPE_SOL_BIP44_15,          ED25519,      "sol_bip44_15",             "M/44'/501'/15'"    },
    {XPUB_TYPE_SOL_BIP44_16,          ED25519,      "sol_bip44_16",             "M/44'/501'/16'"    },
    {XPUB_TYPE_SOL_BIP44_17,          ED25519,      "sol_bip44_17",             "M/44'/501'/17'"    },
    {XPUB_TYPE_SOL_BIP44_18,          ED25519,      "sol_bip44_18",             "M/44'/501'/18'"    },
    {XPUB_TYPE_SOL_BIP44_19,          ED25519,      "sol_bip44_19",             "M/44'/501'/19'"    },
    {XPUB_TYPE_SOL_BIP44_20,          ED25519,      "sol_bip44_20",             "M/44'/501'/20'"    },
    {XPUB_TYPE_SOL_BIP44_21,          ED25519,      "sol_bip44_21",             "M/44'/501'/21'"    },
    {XPUB_TYPE_SOL_BIP44_22,          ED25519,      "sol_bip44_22",             "M/44'/501'/22'"    },
    {XPUB_TYPE_SOL_BIP44_23,          ED25519,      "sol_bip44_23",             "M/44'/501'/23'"    },
    {XPUB_TYPE_SOL_BIP44_24,          ED25519,      "sol_bip44_24",             "M/44'/501'/24'"    },
    {XPUB_TYPE_SOL_BIP44_25,          ED25519,      "sol_bip44_25",             "M/44'/501'/25'"    },
    {XPUB_TYPE_SOL_BIP44_26,          ED25519,      "sol_bip44_26",             "M/44'/501'/26'"    },
    {XPUB_TYPE_SOL_BIP44_27,          ED25519,      "sol_bip44_27",             "M/44'/501'/27'"    },
    {XPUB_TYPE_SOL_BIP44_28,          ED25519,      "sol_bip44_28",             "M/44'/501'/28'"    },
    {XPUB_TYPE_SOL_BIP44_29,          ED25519,      "sol_bip44_29",             "M/44'/501'/29'"    },
    {XPUB_TYPE_SOL_BIP44_30,          ED25519,      "sol_bip44_30",             "M/44'/501'/30'"    },
    {XPUB_TYPE_SOL_BIP44_31,          ED25519,      "sol_bip44_31",             "M/44'/501'/31'"    },
    {XPUB_TYPE_SOL_BIP44_32,          ED25519,      "sol_bip44_32",             "M/44'/501'/32'"    },
    {XPUB_TYPE_SOL_BIP44_33,          ED25519,      "sol_bip44_33",             "M/44'/501'/33'"    },
    {XPUB_TYPE_SOL_BIP44_34,          ED25519,      "sol_bip44_34",             "M/44'/501'/34'"    },
    {XPUB_TYPE_SOL_BIP44_35,          ED25519,      "sol_bip44_35",             "M/44'/501'/35'"    },
    {XPUB_TYPE_SOL_BIP44_36,          ED25519,      "sol_bip44_36",             "M/44'/501'/36'"    },
    {XPUB_TYPE_SOL_BIP44_37,          ED25519,      "sol_bip44_37",             "M/44'/501'/37'"    },
    {XPUB_TYPE_SOL_BIP44_38,          ED25519,      "sol_bip44_38",             "M/44'/501'/38'"    },
    {XPUB_TYPE_SOL_BIP44_39,          ED25519,      "sol_bip44_39",             "M/44'/501'/39'"    },
    {XPUB_TYPE_SOL_BIP44_40,          ED25519,      "sol_bip44_40",             "M/44'/501'/40'"    },
    {XPUB_TYPE_SOL_BIP44_41,          ED25519,      "sol_bip44_41",             "M/44'/501'/41'"    },
    {XPUB_TYPE_SOL_BIP44_42,          ED25519,      "sol_bip44_42",             "M/44'/501'/42'"    },
    {XPUB_TYPE_SOL_BIP44_43,          ED25519,      "sol_bip44_43",             "M/44'/501'/43'"    },
    {XPUB_TYPE_SOL_BIP44_44,          ED25519,      "sol_bip44_44",             "M/44'/501'/44'"    },
    {XPUB_TYPE_SOL_BIP44_45,          ED25519,      "sol_bip44_45",             "M/44'/501'/45'"    },
    {XPUB_TYPE_SOL_BIP44_46,          ED25519,      "sol_bip44_46",             "M/44'/501'/46'"    },
    {XPUB_TYPE_SOL_BIP44_47,          ED25519,      "sol_bip44_47",             "M/44'/501'/47'"    },
    {XPUB_TYPE_SOL_BIP44_48,          ED25519,      "sol_bip44_48",             "M/44'/501'/48'"    },
    {XPUB_TYPE_SOL_BIP44_49,          ED25519,      "sol_bip44_49",             "M/44'/501'/49'"    },
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
    {XPUB_TYPE_SOL_BIP44_CHANGE_10,   ED25519,      "sol_bip44_change_10",      "M/44'/501'/10'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_11,   ED25519,      "sol_bip44_change_11",      "M/44'/501'/11'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_12,   ED25519,      "sol_bip44_change_12",      "M/44'/501'/12'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_13,   ED25519,      "sol_bip44_change_13",      "M/44'/501'/13'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_14,   ED25519,      "sol_bip44_change_14",      "M/44'/501'/14'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_15,   ED25519,      "sol_bip44_change_15",      "M/44'/501'/15'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_16,   ED25519,      "sol_bip44_change_16",      "M/44'/501'/16'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_17,   ED25519,      "sol_bip44_change_17",      "M/44'/501'/17'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_18,   ED25519,      "sol_bip44_change_18",      "M/44'/501'/18'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_19,   ED25519,      "sol_bip44_change_19",      "M/44'/501'/19'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_20,   ED25519,      "sol_bip44_change_20",      "M/44'/501'/20'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_21,   ED25519,      "sol_bip44_change_21",      "M/44'/501'/21'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_22,   ED25519,      "sol_bip44_change_22",      "M/44'/501'/22'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_23,   ED25519,      "sol_bip44_change_23",      "M/44'/501'/23'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_24,   ED25519,      "sol_bip44_change_24",      "M/44'/501'/24'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_25,   ED25519,      "sol_bip44_change_25",      "M/44'/501'/25'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_26,   ED25519,      "sol_bip44_change_26",      "M/44'/501'/26'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_27,   ED25519,      "sol_bip44_change_27",      "M/44'/501'/27'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_28,   ED25519,      "sol_bip44_change_28",      "M/44'/501'/28'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_29,   ED25519,      "sol_bip44_change_29",      "M/44'/501'/29'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_30,   ED25519,      "sol_bip44_change_30",      "M/44'/501'/30'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_31,   ED25519,      "sol_bip44_change_31",      "M/44'/501'/31'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_32,   ED25519,      "sol_bip44_change_32",      "M/44'/501'/32'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_33,   ED25519,      "sol_bip44_change_33",      "M/44'/501'/33'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_34,   ED25519,      "sol_bip44_change_34",      "M/44'/501'/34'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_35,   ED25519,      "sol_bip44_change_35",      "M/44'/501'/35'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_36,   ED25519,      "sol_bip44_change_36",      "M/44'/501'/36'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_37,   ED25519,      "sol_bip44_change_37",      "M/44'/501'/37'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_38,   ED25519,      "sol_bip44_change_38",      "M/44'/501'/38'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_39,   ED25519,      "sol_bip44_change_39",      "M/44'/501'/39'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_40,   ED25519,      "sol_bip44_change_40",      "M/44'/501'/40'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_41,   ED25519,      "sol_bip44_change_41",      "M/44'/501'/41'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_42,   ED25519,      "sol_bip44_change_42",      "M/44'/501'/42'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_43,   ED25519,      "sol_bip44_change_43",      "M/44'/501'/43'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_44,   ED25519,      "sol_bip44_change_44",      "M/44'/501'/44'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_45,   ED25519,      "sol_bip44_change_45",      "M/44'/501'/45'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_46,   ED25519,      "sol_bip44_change_46",      "M/44'/501'/46'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_47,   ED25519,      "sol_bip44_change_47",      "M/44'/501'/47'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_48,   ED25519,      "sol_bip44_change_48",      "M/44'/501'/48'/0'"  },
    {XPUB_TYPE_SOL_BIP44_CHANGE_49,   ED25519,      "sol_bip44_change_49",      "M/44'/501'/49'/0'"  },
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
    {XPUB_TYPE_LEDGER_ADA_0,          LEDGER_BITBOX02, "ada_ledger_0",           "M/1852'/1815'/0'"},
    {XPUB_TYPE_LEDGER_ADA_1,          LEDGER_BITBOX02, "ada_ledger_1",           "M/1852'/1815'/1'"},
    {XPUB_TYPE_LEDGER_ADA_2,          LEDGER_BITBOX02, "ada_ledger_2",           "M/1852'/1815'/2'"},
    {XPUB_TYPE_LEDGER_ADA_3,          LEDGER_BITBOX02, "ada_ledger_3",           "M/1852'/1815'/3'"},
    {XPUB_TYPE_LEDGER_ADA_4,          LEDGER_BITBOX02, "ada_ledger_4",           "M/1852'/1815'/4'"},
    {XPUB_TYPE_LEDGER_ADA_5,          LEDGER_BITBOX02, "ada_ledger_5",           "M/1852'/1815'/5'"},
    {XPUB_TYPE_LEDGER_ADA_6,          LEDGER_BITBOX02, "ada_ledger_6",           "M/1852'/1815'/6'"},
    {XPUB_TYPE_LEDGER_ADA_7,          LEDGER_BITBOX02, "ada_ledger_7",           "M/1852'/1815'/7'"},
    {XPUB_TYPE_LEDGER_ADA_8,          LEDGER_BITBOX02, "ada_ledger_8",           "M/1852'/1815'/8'"},
    {XPUB_TYPE_LEDGER_ADA_9,          LEDGER_BITBOX02, "ada_ledger_9",           "M/1852'/1815'/9'"},
    {XPUB_TYPE_LEDGER_ADA_10,         LEDGER_BITBOX02, "ada_ledger_10",          "M/1852'/1815'/10'"},
    {XPUB_TYPE_LEDGER_ADA_11,         LEDGER_BITBOX02, "ada_ledger_11",          "M/1852'/1815'/11'"},
    {XPUB_TYPE_LEDGER_ADA_12,         LEDGER_BITBOX02, "ada_ledger_12",          "M/1852'/1815'/12'"},
    {XPUB_TYPE_LEDGER_ADA_13,         LEDGER_BITBOX02, "ada_ledger_13",          "M/1852'/1815'/13'"},
    {XPUB_TYPE_LEDGER_ADA_14,         LEDGER_BITBOX02, "ada_ledger_14",          "M/1852'/1815'/14'"},
    {XPUB_TYPE_LEDGER_ADA_15,         LEDGER_BITBOX02, "ada_ledger_15",          "M/1852'/1815'/15'"},
    {XPUB_TYPE_LEDGER_ADA_16,         LEDGER_BITBOX02, "ada_ledger_16",          "M/1852'/1815'/16'"},
    {XPUB_TYPE_LEDGER_ADA_17,         LEDGER_BITBOX02, "ada_ledger_17",          "M/1852'/1815'/17'"},
    {XPUB_TYPE_LEDGER_ADA_18,         LEDGER_BITBOX02, "ada_ledger_18",          "M/1852'/1815'/18'"},
    {XPUB_TYPE_LEDGER_ADA_19,         LEDGER_BITBOX02, "ada_ledger_19",          "M/1852'/1815'/19'"},
    {XPUB_TYPE_LEDGER_ADA_20,         LEDGER_BITBOX02, "ada_ledger_20",          "M/1852'/1815'/20'"},
    {XPUB_TYPE_LEDGER_ADA_21,         LEDGER_BITBOX02, "ada_ledger_21",          "M/1852'/1815'/21'"},
    {XPUB_TYPE_LEDGER_ADA_22,         LEDGER_BITBOX02, "ada_ledger_22",          "M/1852'/1815'/22'"},
    {XPUB_TYPE_LEDGER_ADA_23,         LEDGER_BITBOX02, "ada_ledger_23",          "M/1852'/1815'/23'"},
    {XPUB_TYPE_ARWEAVE,               RSA_KEY,       "ar",                       ""                 },
    {XPUB_TYPE_STELLAR_0,             ED25519,       "stellar_0",                "M/44'/148'/0'"    },
    {XPUB_TYPE_STELLAR_1,             ED25519,       "stellar_1",                "M/44'/148'/1'"    },
    {XPUB_TYPE_STELLAR_2,             ED25519,       "stellar_2",                "M/44'/148'/2'"    },
    {XPUB_TYPE_STELLAR_3,             ED25519,       "stellar_3",                "M/44'/148'/3'"    },
    {XPUB_TYPE_STELLAR_4,             ED25519,       "stellar_4",                "M/44'/148'/4'"    },
    {XPUB_TYPE_TON_BIP39,             ED25519,       "ton_bip39",                "M/44'/607'/0'"    },
    {XPUB_TYPE_TON_NATIVE,            TON_NATIVE,    "ton",                      ""                 },
    {PUBLIC_INFO_TON_CHECKSUM,        TON_CHECKSUM,  "ton_checksum",             ""                 },
#endif

#ifdef CYPHERPUNK_VERSION
    //this path is not 100% correct because UFVK is a combination of
    //a k1 key with path M/44'/133'/x'
    //and
    //a redpallas key with path M/32'/133'/x'
    //but we use 32 to identify it for now
    {ZCASH_UFVK_ENCRYPTED_0,          ZCASH_UFVK_ENCRYPTED, "zcash_ufvk_0",      "M/32'/133'/0'"    },
    {XPUB_TYPE_MONERO_0,              EDWARDS_25519,  "monero_0",                "M/44'/128'/0'"    },
    {XPUB_TYPE_MONERO_PVK_0,          MONERO_PVK,     "monero_pvk_0",            ""                 },
    {XPUB_TYPE_ERG,                   SECP256K1,  "erg",                      "M/44'/429'/0'"     },
#endif

#ifdef BTC_ONLY
    {XPUB_TYPE_BTC_TEST,                SECP256K1,      "btc_nested_segwit_test",   "M/49'/1'/0'"   },
    {XPUB_TYPE_BTC_LEGACY_TEST,         SECP256K1,      "btc_legacy_test",          "M/44'/1'/0'"   },
    {XPUB_TYPE_BTC_NATIVE_SEGWIT_TEST,  SECP256K1,      "btc_native_segwit_test",   "M/84'/1'/0'"   },
    {XPUB_TYPE_BTC_TAPROOT_TEST,        SECP256K1,      "btc_taproot_test",         "M/86'/1'/0'"   },

    {XPUB_TYPE_BTC_MULTI_SIG_P2SH,              SECP256K1,      "btc_multi_sig_p2sh",               "M/45'"   },
    {XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH,        SECP256K1,      "btc_multi_sig_p2wsh_p2sh",         "M/48'/0'/0'/1'"   },
    {XPUB_TYPE_BTC_MULTI_SIG_P2WSH,             SECP256K1,      "btc_multi_sig_p2wsh",              "M/48'/0'/0'/2'"   },

    {XPUB_TYPE_BTC_MULTI_SIG_P2SH_TEST,              SECP256K1,      "btc_multi_sig_p2sh_test",               "M/45'"   },
    {XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH_TEST,        SECP256K1,      "btc_multi_sig_p2wsh_p2sh_test",         "M/48'/1'/0'/1'"   },
    {XPUB_TYPE_BTC_MULTI_SIG_P2WSH_TEST,             SECP256K1,      "btc_multi_sig_p2wsh_test",              "M/48'/1'/0'/2'"   },
#endif
};

#ifdef WEB3_VERSION
ChainType CheckSolPathSupport(char *path)
{
    int startIndex = -1;
    int endIndex = -1;
    for (int i = 0; i < XPUB_TYPE_NUM; i++) {
        if (XPUB_TYPE_SOL_BIP44_0 == g_chainTable[i].chain) {
            startIndex = i;
        }
        if (XPUB_TYPE_SOL_BIP44_CHANGE_49 == g_chainTable[i].chain) {
            endIndex = i;
            break;
        }
    }

    if (startIndex == -1 || endIndex == -1) {
        return XPUB_TYPE_NUM;
    }

    for (int i = startIndex; i <= endIndex; i++) {
        // skip the first two characters
        if (!strcmp(path, g_chainTable[i].path + 2)) {
            return g_chainTable[i].chain;
        }
    }
    return XPUB_TYPE_NUM;
}
#endif

static SimpleResponse_c_char *ProcessKeyType(uint8_t *seed, int len, int cryptoKey, const char *path, void *icarusMasterKey, void *ledgerBitbox02MasterKey)
{
    switch (cryptoKey) {
    case SECP256K1:
        return get_extended_pubkey_by_seed(seed, len, (char *)path);
    case ED25519:
        return get_ed25519_pubkey_by_seed(seed, len, (char *)path);
    case BIP32_ED25519:
        ASSERT(icarusMasterKey);
        return derive_bip32_ed25519_extended_pubkey(icarusMasterKey, (char *)path);
    case LEDGER_BITBOX02:
        ASSERT(ledgerBitbox02MasterKey);
        return derive_bip32_ed25519_extended_pubkey(ledgerBitbox02MasterKey, (char *)path);
#ifdef CYPHERPUNK_VERSION
    case EDWARDS_25519:
        return get_extended_monero_pubkeys_by_seed(seed, len, (char *)path);
    case MONERO_PVK:
        return get_monero_pvk_by_seed(seed, len);
#endif

#ifdef WEB3_VERSION
    case RSA_KEY: {
        Rsa_primes_t *primes = FlashReadRsaPrimes();
        if (primes == NULL)
            return NULL;
        SimpleResponse_c_char *result = generate_rsa_public_key(primes->p, 256, primes->q, 256);
        SRAM_FREE(primes);
        return result;
    }
    case TON_NATIVE:
        return ton_seed_to_publickey(seed, len);
    case TON_CHECKSUM:
        // should not be here.
        ASSERT(0);
#endif
    default:
        return NULL;
    }
}

void CalculateTonChecksum(uint8_t *entropy, char* output)
{
    uint8_t checksum[32];
    sha256((struct sha256 *)checksum, entropy, 64);
    memcpy_s(output, 32, checksum, 32);
}

char *GetXPubPath(uint8_t index)
{
    ASSERT(index < XPUB_TYPE_NUM);
    return g_chainTable[index].path;
}

void AccountPublicHomeCoinGet(WalletState_t *walletList, uint8_t count)
{
    int32_t ret = SUCCESS_CODE;
    uint32_t addr, size, eraseAddr;
    bool needSet = false;
    char *jsonString = NULL;

    bool isTon = GetMnemonicType() == MNEMONIC_TYPE_TON;

    uint8_t account = GetCurrentAccountIndex();
    ASSERT(account < 3);
    addr = SPI_FLASH_ADDR_USER1_MUTABLE_DATA + account * SPI_FLASH_ADDR_EACH_SIZE;
    ret = Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
    ASSERT(ret == 4);

    if (size == 0xffffffff || size == 0) {
        needSet = true;
    }

    if (needSet) {
        for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MUTABLE_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
            Gd25FlashSectorErase(eraseAddr);
        }
        cJSON *rootJson, *jsonItem;
        char *retStr;
        rootJson = cJSON_CreateObject();
        for (int i = 0; i < count; i++) {
            jsonItem = cJSON_CreateObject();
            cJSON_AddItemToObject(jsonItem, "recvIndex", cJSON_CreateNumber(0));
            cJSON_AddItemToObject(jsonItem, "recvPath", cJSON_CreateNumber(0));
            cJSON_AddItemToObject(jsonItem, "firstRecv", cJSON_CreateBool(false));
            if (!strcmp(walletList[i].name, "TON") && isTon) {
                cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
            } else if ((!strcmp(walletList[i].name, "BTC") || !strcmp(walletList[i].name, "ETH")) && !isTon) {
                cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
#ifdef CYPHERPUNK_VERSION
            } else if (!strcmp(walletList[i].name, "ZEC")) {
                if (GetMnemonicType() == MNEMONIC_TYPE_BIP39 && !PassphraseExist(GetCurrentAccountIndex())) {
                    cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
                }
            } else if (!strcmp(walletList[i].name, "XMR")) {
                if (GetMnemonicType() == MNEMONIC_TYPE_BIP39) {
                    cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
                }
#endif
            } else {
                cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(false));
            }
#ifdef BTC_ONLY
            cJSON_AddItemToObject(jsonItem, "testNet", cJSON_CreateBool(false));
            cJSON_AddItemToObject(jsonItem, "defaultWallet", cJSON_CreateNumber(SINGLE_WALLET));
#endif
            cJSON_AddItemToObject(rootJson, walletList[i].name, jsonItem);
        }
        retStr = cJSON_PrintBuffered(rootJson, SPI_FLASH_SIZE_USER1_MUTABLE_DATA - 4, false);
        cJSON_Delete(rootJson);
        RemoveFormatChar(retStr);
        size = strlen(retStr);
        Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
        Gd25FlashWriteBuffer(addr + 4, (uint8_t *)retStr, size);
        EXT_FREE(retStr);
    }

    jsonString = SRAM_MALLOC(size + 1);
    ret = Gd25FlashReadBuffer(addr + 4, (uint8_t *)jsonString, size);
    ASSERT(ret == size);
    jsonString[size] = 0;

    cJSON *rootJson = cJSON_Parse(jsonString);
    SRAM_FREE(jsonString);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetObjectItem(rootJson, walletList[i].name);
        if (item != NULL) {
            walletList[i].state = GetBoolValue(item, "manage", false);
#ifdef BTC_ONLY
            walletList[i].testNet = GetBoolValue(item, "testNet", false);
            walletList[i].defaultWallet = GetIntValue(item, "defaultWallet", SINGLE_WALLET);
#endif
        }
    }
    cJSON_Delete(rootJson);
}

void AccountPublicHomeCoinSet(WalletState_t *walletList, uint8_t count)
{
    cJSON *rootJson;
    bool needUpdate = false;
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

    rootJson = cJSON_Parse(jsonString);
    SRAM_FREE(jsonString);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetObjectItem(rootJson, walletList[i].name);
        if (item == NULL) {
            item = cJSON_CreateObject();
            cJSON_AddItemToObject(item, "recvIndex", cJSON_CreateNumber(0));
            cJSON_AddItemToObject(item, "recvPath", cJSON_CreateNumber(0));
            cJSON_AddItemToObject(item, "firstRecv", cJSON_CreateBool(false));
            cJSON_AddItemToObject(item, "manage", cJSON_CreateBool(walletList[i].state));
#ifdef BTC_ONLY
            cJSON_AddItemToObject(item, "testNet", cJSON_CreateBool(walletList[i].testNet));
            cJSON_AddItemToObject(item, "defaultWallet", cJSON_CreateNumber(walletList[i].defaultWallet));
#endif
            cJSON_AddItemToObject(rootJson, walletList[i].name, item);
            needUpdate = true;
        } else {
            if (cJSON_GetObjectItem(item, "manage") == NULL) {
                cJSON_AddItemToObject(item, "manage", cJSON_CreateBool(walletList[i].state));
                needUpdate = true;
            } else if (GetBoolValue(item, "manage", false) != walletList[i].state) {
                cJSON_ReplaceItemInObject(item, "manage", cJSON_CreateBool(walletList[i].state));
                needUpdate = true;
            }
#ifdef BTC_ONLY
            if (cJSON_GetObjectItem(item, "testNet") == NULL) {
                cJSON_AddItemToObject(item, "testNet", cJSON_CreateBool(walletList[i].testNet));
                needUpdate = true;
            } else if (GetBoolValue(item, "testNet", false) != walletList[i].testNet) {
                cJSON_ReplaceItemInObject(item, "testNet", cJSON_CreateBool(walletList[i].testNet));
                needUpdate = true;
            }
            if (cJSON_GetObjectItem(item, "defaultWallet") == NULL) {
                cJSON_AddItemToObject(item, "defaultWallet", cJSON_CreateNumber(walletList[i].defaultWallet));
                needUpdate = true;
            } else if (GetIntValue(item, "defaultWallet", SINGLE_WALLET) != walletList[i].defaultWallet) {
                cJSON_ReplaceItemInObject(item, "defaultWallet", cJSON_CreateNumber(walletList[i].defaultWallet));
                needUpdate = true;
            }
#endif
        }
    }

    if (needUpdate) {
        for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MUTABLE_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
            Gd25FlashSectorErase(eraseAddr);
        }
        jsonString = cJSON_PrintBuffered(rootJson, SPI_FLASH_SIZE_USER1_MUTABLE_DATA - 4, false);
        RemoveFormatChar(jsonString);
        size = strlen(jsonString);
        Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
        Gd25FlashWriteBuffer(addr + 4, (uint8_t *)jsonString, size);
        EXT_FREE(jsonString);
    }
    cJSON_Delete(rootJson);
}

int32_t AccountPublicInfoReadFromFlash(uint8_t accountIndex, uint32_t addr)
{
    uint32_t size;
    uint8_t hash[32];
    int32_t ret = SUCCESS_CODE;
    int len = 0;
    char *jsonString;
    len = Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
    ASSERT(len == 4);
    if (size > SPI_FLASH_SIZE_USER1_DATA - 4) {
        printf("pubkey size err,%d\r\n", size);
        return ERR_GENERAL_FAIL;
    }
    jsonString = SRAM_MALLOC(size + 1);
    len = Gd25FlashReadBuffer(addr + 4, (uint8_t *)jsonString, size);
    ASSERT(len == size);
    jsonString[size] = 0;
#ifndef COMPILE_SIMULATOR
    sha256((struct sha256 *)hash, jsonString, size);
    if (!VerifyWalletDataHash(accountIndex, hash)) {
        CLEAR_ARRAY(hash);
        return ERR_KEYSTORE_EXTEND_PUBLIC_KEY_NOT_MATCH;
    } else {
        ret = SUCCESS_CODE;
    }
#else
    ret = SUCCESS_CODE;
#endif
    CLEAR_ARRAY(hash);
    if (GetPublicKeyFromJsonString(jsonString) == false) {
        printf("GetPublicKeyFromJsonString false, need regenerate\r\n");
        printf("err jsonString=%s\r\n", jsonString);
        ret = ERR_GENERAL_FAIL;
    }

    SRAM_FREE(jsonString);
    return ret;
}

#define CHECK_AND_FREE_XPUB(x)              ASSERT(x);          \
            if (x->error_code != 0) {                           \
                printf("get_extended_pubkey error\r\n");        \
                if (x->error_message != NULL) {                 \
                    printf("error code = %d\r\nerror msg is: %s\r\n", x->error_code, x->error_message); \
                }                                               \
                free_simple_response_c_char(x);                 \
                ret = x->error_code;                            \
                break;                                          \
            }

int32_t AccountPublicSavePublicInfo(uint8_t accountIndex, const char *password, uint32_t addr)
{
    uint8_t entropyLen = 0;
    uint8_t seed[64], entropy[64], hash[32];
    char *jsonString;
    int32_t ret = SUCCESS_CODE;
    SimpleResponse_c_char *xPubResult = NULL;
    MnemonicType mnemonicType = GetMnemonicType();
    bool isSlip39 = mnemonicType == MNEMONIC_TYPE_SLIP39;
    bool isBip39 = mnemonicType == MNEMONIC_TYPE_BIP39;
    int len = isSlip39 ? GetCurrentAccountEntropyLen() : sizeof(seed) ;
    do {
        GuiApiEmitSignal(SIG_START_GENERATE_XPUB, NULL, 0);
        char* icarusMasterKey = NULL;
        char* ledgerBitbox02Key = NULL;
        printf("regenerate pub key!\r\n");
        FreePublicKeyRam();
        ret = GetAccountSeed(accountIndex, seed, password);
        CHECK_ERRCODE_BREAK("get seed", ret);
        ret = GetAccountEntropy(accountIndex, entropy, &entropyLen, password);
        CHECK_ERRCODE_BREAK("get entropy", ret);
        SimpleResponse_c_char* cip3_response = NULL;
        SimpleResponse_c_char *ledger_bitbox02_response = NULL;
        // should setup ADA for bip39 wallet;
        if (isBip39) {
            char *mnemonic = NULL;
            bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
            cip3_response = get_icarus_master_key(entropy, entropyLen, GetPassphrase(accountIndex));
            ledger_bitbox02_response = get_ledger_bitbox02_master_key(mnemonic, GetPassphrase(accountIndex));
            CHECK_AND_FREE_XPUB(cip3_response);
            CHECK_AND_FREE_XPUB(ledger_bitbox02_response);
            icarusMasterKey = cip3_response->data;
            ledgerBitbox02Key = ledger_bitbox02_response->data;
        }

#ifdef WEB3_VERSION
        if (mnemonicType == MNEMONIC_TYPE_TON) {
            //store public key for ton wallet;
            xPubResult = ProcessKeyType(seed, len, g_chainTable[XPUB_TYPE_TON_NATIVE].cryptoKey, g_chainTable[XPUB_TYPE_TON_NATIVE].path, NULL, NULL);
            CHECK_AND_FREE_XPUB(xPubResult)
            ASSERT(xPubResult->data);
            g_accountPublicInfo[XPUB_TYPE_TON_NATIVE].value = SRAM_MALLOC(strnlen_s(xPubResult->data, SIMPLERESPONSE_C_CHAR_MAX_LEN) + 1);
            strcpy_s(g_accountPublicInfo[XPUB_TYPE_TON_NATIVE].value, strnlen_s(xPubResult->data, SIMPLERESPONSE_C_CHAR_MAX_LEN) + 1, xPubResult->data);
            free_simple_response_c_char(xPubResult);
            //store a checksum of entropy for quick compare;
            uint8_t checksum[32] = {'\0'};
            CalculateTonChecksum(entropy, (char *)checksum);
            printf("ton checksum: %s\r\n", checksum);
            g_accountPublicInfo[PUBLIC_INFO_TON_CHECKSUM].value = SRAM_MALLOC(65);
            char* ptr = g_accountPublicInfo[PUBLIC_INFO_TON_CHECKSUM].value;
            memset_s(ptr, 65, 0, 65);
            for (size_t i = 0; i < 32; i++) {
                snprintf_s(ptr, 65, "%s%02x", ptr, checksum[i]);
            }
        } else {
#endif
            for (int i = 0; i < NUMBER_OF_ARRAYS(g_chainTable); i++) {
                // slip39 wallet does not support:
                // ADA
                // Zcash
                if (isSlip39 && (g_chainTable[i].cryptoKey == BIP32_ED25519 || g_chainTable[i].cryptoKey == LEDGER_BITBOX02 || g_chainTable[i].cryptoKey == ZCASH_UFVK_ENCRYPTED)) {
                    continue;
                }
                // do not generate public keys for ton-only wallet;
                if (g_chainTable[i].cryptoKey == TON_CHECKSUM || g_chainTable[i].cryptoKey == TON_NATIVE) {
                    continue;
                }
#ifdef CYPHERPUNK_VERSION
                //encrypt zcash ufvk
                if (g_chainTable[i].cryptoKey == ZCASH_UFVK_ENCRYPTED) {
                    char* zcashUfvk = NULL;
                    SimpleResponse_c_char *zcash_ufvk_response = NULL;
                    zcash_ufvk_response = derive_zcash_ufvk(seed, len, g_chainTable[i].path);
                    CHECK_AND_FREE_XPUB(zcash_ufvk_response)
                    zcashUfvk = zcash_ufvk_response->data;
                    SimpleResponse_u8 *iv_response = rust_derive_iv_from_seed(seed, len);
                    //iv_response won't fail
                    uint8_t iv_bytes[16];
                    memcpy_s(iv_bytes, 16, iv_response->data, 16);
                    free_simple_response_u8(iv_response);
                    xPubResult = rust_aes256_cbc_encrypt(zcashUfvk, password, iv_bytes, 16);
                } else {
                    xPubResult = ProcessKeyType(seed, len, g_chainTable[i].cryptoKey, g_chainTable[i].path, icarusMasterKey, ledgerBitbox02Key);
                }
#else
                xPubResult = ProcessKeyType(seed, len, g_chainTable[i].cryptoKey, g_chainTable[i].path, icarusMasterKey, ledgerBitbox02Key);
#endif
                if (g_chainTable[i].cryptoKey == RSA_KEY && xPubResult == NULL) {
                    continue;
                }
                CHECK_AND_FREE_XPUB(xPubResult)
                // printf("index=%d,path=%s,pub=%s\r\n", accountIndex, g_chainTable[i].path, xPubResult->data);
                ASSERT(xPubResult->data);
                g_accountPublicInfo[i].value = SRAM_MALLOC(strnlen_s(xPubResult->data, SIMPLERESPONSE_C_CHAR_MAX_LEN) + 1);
                strcpy_s(g_accountPublicInfo[i].value, strnlen_s(xPubResult->data, SIMPLERESPONSE_C_CHAR_MAX_LEN) + 1, xPubResult->data);
                // printf("xPubResult=%s\r\n", xPubResult->data);
                free_simple_response_c_char(xPubResult);
            }
#ifdef WEB3_VERSION
        }
#endif
        printf("erase user data:0x%X\n", addr);
        for (uint32_t eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
            Gd25FlashSectorErase(eraseAddr);
        }
        printf("erase done\n");
        jsonString = GetJsonStringFromPublicKey();

        printf("save jsonString = \r\n%s\n", jsonString);
        sha256((struct sha256 *)hash, jsonString, strlen(jsonString));
        SetWalletDataHash(accountIndex, hash);
        CLEAR_ARRAY(hash);
        uint32_t size = strlen(jsonString);
        len = Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
        ASSERT(len == 4);
        len = Gd25FlashWriteBuffer(addr + 4, (uint8_t *)jsonString, size);
        ASSERT(len == size);
        printf("regenerate jsonString=%s\r\n", jsonString);
        if (!isSlip39) {
            free_simple_response_c_char(cip3_response);
            free_simple_response_c_char(ledger_bitbox02_response);
        }
        GuiApiEmitSignal(SIG_END_GENERATE_XPUB, NULL, 0);
        EXT_FREE(jsonString);
    } while (0);

    CLEAR_ARRAY(seed);
    return ret;
}

int32_t AccountPublicInfoSwitch(uint8_t accountIndex, const char *password, bool newKey)
{
    SetIsTempAccount(false);
    uint32_t addr;
    int32_t ret = SUCCESS_CODE;
    bool regeneratePubKey = newKey;

    ASSERT(accountIndex < 3);
    FreePublicKeyRam();
    //Load Multisig wallet Manager

    addr = SPI_FLASH_ADDR_USER1_DATA + accountIndex * SPI_FLASH_ADDR_EACH_SIZE;
    if (!regeneratePubKey) {
        ret = AccountPublicInfoReadFromFlash(accountIndex, addr);
        if (ret == ERR_KEYSTORE_EXTEND_PUBLIC_KEY_NOT_MATCH) {
            return ret;
        } else if (ret == ERR_GENERAL_FAIL) {
            regeneratePubKey = true;
        }
    }

    if (regeneratePubKey) {
        ret = AccountPublicSavePublicInfo(accountIndex, password, addr);
    }

#ifdef BTC_ONLY
    initMultiSigWalletManager();
    ret = LoadCurrentAccountMultisigWallet(password);
    CHECK_ERRCODE_RETURN_INT(ret);
    LoadCurrentAccountMultiReceiveIndex();
#endif
    printf("acount public key info sitch over\r\n");
    //PrintInfo();
    return ret;
}

static void SetIsTempAccount(bool isTemp)
{
    g_isTempAccount = isTemp;
}

bool GetIsTempAccount(void)
{
#ifndef COMPILE_SIMULATOR
    return g_isTempAccount;
#else
    return false;
#endif
}

int32_t TempAccountPublicInfo(uint8_t accountIndex, const char *password, bool set)
{
    uint32_t i;
    SimpleResponse_c_char *xPubResult;
    int32_t ret = SUCCESS_CODE;
    uint8_t seed[64];
    uint8_t entropy[64];
    uint8_t entropyLen;
    MnemonicType mnemonicType = GetMnemonicType();
    bool isSlip39 = mnemonicType == MNEMONIC_TYPE_SLIP39;
    bool isTon = mnemonicType == MNEMONIC_TYPE_TON;

    //TON Wallet doesn't support passphrase so we dont need to consider it.
    if (isTon) {
        ASSERT(false);
    }

    int len = isSlip39 ? GetCurrentAccountEntropyLen() : sizeof(seed);

    char *passphrase = GetPassphrase(accountIndex);
    SetIsTempAccount(passphrase != NULL && passphrase[0] != 0);
    if (g_tempPublicKeyAccountIndex == accountIndex && set == false) {
        // g_accountPublicInfo stores the current temp public key.
        printf("g_accountPublicInfo stores the current temp public key.\r\n");
    } else {
        GuiApiEmitSignal(SIG_START_GENERATE_XPUB, NULL, 0);
        char* icarusMasterKey = NULL;
        char* ledgerBitbox02Key = NULL;
        FreePublicKeyRam();
        ret = GetAccountSeed(accountIndex, seed, password);
        CHECK_ERRCODE_RETURN_INT(ret);
        ret = GetAccountEntropy(accountIndex, entropy, &entropyLen, password);
        CHECK_ERRCODE_RETURN_INT(ret);
        SimpleResponse_c_char *cip3_response = NULL;
        SimpleResponse_c_char *ledger_bitbox02_response = NULL;

        if (!isSlip39) {
            do {
                char *mnemonic = NULL;
                bip39_mnemonic_from_bytes(NULL, entropy, entropyLen, &mnemonic);
                cip3_response = get_icarus_master_key(entropy, entropyLen, GetPassphrase(accountIndex));
                ledger_bitbox02_response = get_ledger_bitbox02_master_key(mnemonic, GetPassphrase(accountIndex));
                CHECK_AND_FREE_XPUB(cip3_response);
                CHECK_AND_FREE_XPUB(ledger_bitbox02_response);
                icarusMasterKey = cip3_response->data;
                ledgerBitbox02Key = ledger_bitbox02_response->data;
            } while (0);
        }

        for (i = 0; i < NUMBER_OF_ARRAYS(g_chainTable); i++) {
            // SLIP32 wallet does not support ADA
            if (isSlip39 && (g_chainTable[i].cryptoKey == BIP32_ED25519 || g_chainTable[i].cryptoKey == LEDGER_BITBOX02)) {
                continue;
            }
            if (g_chainTable[i].cryptoKey == TON_CHECKSUM || g_chainTable[i].cryptoKey == TON_NATIVE) {
                continue;
            }
            if (g_chainTable[i].cryptoKey == ZCASH_UFVK_ENCRYPTED) {
                continue;
            }

            xPubResult = ProcessKeyType(seed, len, g_chainTable[i].cryptoKey, g_chainTable[i].path, icarusMasterKey, ledgerBitbox02Key);
            if (g_chainTable[i].cryptoKey == RSA_KEY && xPubResult == NULL) {
                continue;
            }
            ASSERT(xPubResult);
            if (xPubResult->error_code != 0) {
                printf("get_extended_pubkey error\r\n");
                if (xPubResult->error_message != NULL) {
                    printf("error code = %d\r\nerror msg is: %s\r\n", xPubResult->error_code, xPubResult->error_message);
                }
                free_simple_response_c_char(xPubResult);
                break;
            }
            printf("index=%d,path=%s,pub=%s\r\n", accountIndex, g_chainTable[i].path, xPubResult->data);
            ASSERT(xPubResult->data);
            g_accountPublicInfo[i].value = SRAM_MALLOC(strnlen_s(xPubResult->data, SIMPLERESPONSE_C_CHAR_MAX_LEN) + 1);
            strcpy(g_accountPublicInfo[i].value, xPubResult->data);
            printf("xPubResult=%s\r\n", xPubResult->data);
            free_simple_response_c_char(xPubResult);
        }
        if (!isSlip39) {
            free_simple_response_c_char(cip3_response);
            free_simple_response_c_char(ledger_bitbox02_response);
        }
        g_tempPublicKeyAccountIndex = accountIndex;
        GuiApiEmitSignal(SIG_END_GENERATE_XPUB, NULL, 0);
        if (g_tempParsePhraseJson != NULL) {
            cJSON_Delete(g_tempParsePhraseJson);
            g_tempParsePhraseJson = NULL;
        }
    }
    CLEAR_ARRAY(seed);

    return ret;
}

void DeleteAccountPublicInfo(uint8_t accountIndex)
{
    uint32_t addr, eraseAddr;

    ASSERT(accountIndex < 3);
    addr = SPI_FLASH_ADDR_USER1_DATA + accountIndex * SPI_FLASH_ADDR_EACH_SIZE;
    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }

    addr = SPI_FLASH_ADDR_USER1_MUTABLE_DATA + accountIndex * SPI_FLASH_ADDR_EACH_SIZE;
    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MUTABLE_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }

    addr = SPI_FLASH_ADDR_USER1_MULTI_SIG_DATA + accountIndex * SPI_FLASH_ADDR_EACH_SIZE;
    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MULTI_SIG_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }
    //remove current publickey info to avoid accident reading.
    FreePublicKeyRam();
}

char *GetCurrentAccountPublicKey(ChainType chain)
{
    uint8_t accountIndex;

    accountIndex = GetCurrentAccountIndex();
    if (accountIndex > 2) {
        return NULL;
    }
    return g_accountPublicInfo[chain].value;
}

/// @brief Get if the xPub already Exists.
/// @param[in] xPub
/// @return accountIndex, if not exists, return 255.
uint8_t SpecifiedXPubExist(const char *value, bool isTon)
{
    uint32_t addr, index, size;
    int32_t ret;
    cJSON *rootJson, *keyJson, *chainJson;
    char *jsonString = NULL, pubKeyString[PUB_KEY_MAX_LENGTH];
    uint8_t accountIndex = 255;

    for (index = 0; index < 3; index++) {
        addr = SPI_FLASH_ADDR_USER1_DATA + index * SPI_FLASH_ADDR_EACH_SIZE;
        ret = Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
        ASSERT(ret == 4);
        if (size > SPI_FLASH_SIZE_USER1_DATA - 4) {
            continue;
        }

        do {
            jsonString = SRAM_MALLOC(size + 1);
            ret = Gd25FlashReadBuffer(addr + 4, (uint8_t *)jsonString, size);
            ASSERT(ret == size);
            jsonString[size] = 0;
            rootJson = cJSON_Parse(jsonString);
            if (rootJson == NULL) {
                break;
            }
            keyJson = cJSON_GetObjectItem(rootJson, "key");
            if (keyJson == NULL) {
                break;
            }
#ifdef WEB3_VERSION
            chainJson = cJSON_GetObjectItem(keyJson, isTon ? g_chainTable[PUBLIC_INFO_TON_CHECKSUM].name : g_chainTable[0].name);
#else
            chainJson = cJSON_GetObjectItem(keyJson, g_chainTable[0].name);
#endif
            if (chainJson == NULL) {
                break;
            }
            GetStringValue(chainJson, "value", pubKeyString, PUB_KEY_MAX_LENGTH);
            if (strcmp(pubKeyString, value) == 0) {
                accountIndex = index;
                break;
            }
        } while (0);
        SRAM_FREE(jsonString);
        cJSON_Delete(rootJson);
        if (accountIndex != 255) {
            break;
        }
    }

    return accountIndex;
}

/// @brief
/// @param argc Test arg count.
/// @param argv Test arg values.
void AccountPublicInfoTest(int argc, char *argv[])
{
    char *result;
    uint8_t accountIndex;
    uint32_t addr, eraseAddr, size;

    if (strcmp(argv[0], "info") == 0) {
        PrintInfo();
    } else if (strcmp(argv[0], "json_fmt") == 0) {
        result = GetJsonStringFromPublicKey();
        printf("json string=%s\r\n", result);
        EXT_FREE(result);

        for (int i = 0; i < 3; i++) {
            addr = SPI_FLASH_ADDR_USER1_MUTABLE_DATA + i * SPI_FLASH_ADDR_EACH_SIZE;
            Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
            result = SRAM_MALLOC(size + 1);
            Gd25FlashReadBuffer(addr + 4, (uint8_t *)result, size);
            printf("%d json string=%s\r\n", i, result);
            result[size] = 0;
            SRAM_FREE(result);
        }
    } else if (strcmp(argv[0], "xpub_exist") == 0) {
        VALUE_CHECK(argc, 2);
        accountIndex = SpecifiedXPubExist(argv[1], false);
        printf("SpecifiedXPubExist=%d\r\n", accountIndex);
    } else if (strcmp(argv[0], "erase_coin") == 0) {
        addr = SPI_FLASH_ADDR_USER1_MUTABLE_DATA + GetCurrentAccountIndex() * SPI_FLASH_ADDR_EACH_SIZE;
        for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MUTABLE_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
            Gd25FlashSectorErase(eraseAddr);
        }
    } else {
        printf("account public cmd err\r\n");
    }
}

static int GetChainTableSizeFromJson(cJSON *keyJson)
{
#ifdef WEB3_VERSION
    return cJSON_GetObjectItem(keyJson, "ar") != NULL ? NUMBER_OF_ARRAYS(g_chainTable) : NUMBER_OF_ARRAYS(g_chainTable) - 1;
#else
    return NUMBER_OF_ARRAYS(g_chainTable);
#endif
}

static bool GetPublicKeyFromJsonString(const char *string)
{
    cJSON *rootJson, *keyJson, *chainJson;
    char pubKeyString[PUB_KEY_MAX_LENGTH], versionString[VERSION_MAX_LENGTH];
    bool ret = true;
    uint32_t i;

    do {
        rootJson = cJSON_Parse(string);
        if (rootJson == NULL) {
            ret = false;
            break;
        }
        GetStringValue(rootJson, "version", versionString, VERSION_MAX_LENGTH);
        printf("xpub info version:%s\r\n", versionString);
        printf("g_xpubInfoVersion:%s\r\n", g_xpubInfoVersion);
        if (strcmp(versionString, g_xpubInfoVersion) != 0) {
            ret = false;
            break;
        }
        keyJson = cJSON_GetObjectItem(rootJson, "key");
        if (keyJson == NULL) {
            ret = false;
            break;
        }

        int arraySize = GetChainTableSizeFromJson(keyJson);

        if (cJSON_GetArraySize(keyJson) != arraySize) {
            printf("chain number does not match:%d %d\n", cJSON_GetArraySize(keyJson), arraySize);
            ret = false;
            break;
        }
        for (i = 0; i < arraySize; i++) {
            chainJson = cJSON_GetObjectItem(keyJson, g_chainTable[i].name);
            if (g_chainTable[i].cryptoKey == RSA_KEY && chainJson == NULL) {
                continue;
            }
            if (chainJson == NULL) {
                ret = false;
                break;
            } else {
                GetStringValue(chainJson, "value", pubKeyString, PUB_KEY_MAX_LENGTH);
                //printf("%s pub key=%s\r\n", g_chainTable[i].name, pubKeyString);
                g_accountPublicInfo[i].value = SRAM_MALLOC(strnlen_s(pubKeyString, PUB_KEY_MAX_LENGTH) + 1);
                strcpy(g_accountPublicInfo[i].value, pubKeyString);
            }
        }
    } while (0);
    cJSON_Delete(rootJson);

    return ret;
}

static char *GetJsonStringFromPublicKey(void)
{
    cJSON *rootJson, *chainsJson, *jsonItem;
    uint32_t i;
    char *retStr;

    rootJson = cJSON_CreateObject();
    chainsJson = cJSON_CreateObject();
    for (i = 0; i < NUMBER_OF_ARRAYS(g_chainTable); i++) {
        jsonItem = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonItem, "value", cJSON_CreateString(g_accountPublicInfo[i].value));
        //printf("g_accountPublicInfo[%d].value=%s\r\n", i, g_accountPublicInfo[i].value);
        cJSON_AddItemToObject(jsonItem, "current", cJSON_CreateNumber(g_accountPublicInfo[i].current));
        cJSON_AddItemToObject(chainsJson, g_chainTable[i].name, jsonItem);
    }
    cJSON_AddItemToObject(rootJson, "version", cJSON_CreateString(g_xpubInfoVersion));
    cJSON_AddItemToObject(rootJson, "key", chainsJson);
    retStr = cJSON_PrintBuffered(rootJson, SPI_FLASH_SIZE_USER1_DATA - 4, 0);
    RemoveFormatChar(retStr);
    cJSON_Delete(rootJson);
    return retStr;
}

static void FreePublicKeyRam(void)
{
    g_tempPublicKeyAccountIndex = INVALID_ACCOUNT_INDEX;
    for (uint32_t i = 0; i < XPUB_TYPE_NUM; i++) {
        if (g_accountPublicInfo[i].value != NULL) {
            SRAM_FREE(g_accountPublicInfo[i].value);
            g_accountPublicInfo[i].value = NULL;
        }
    }
}

static void PrintInfo(void)
{
    char *value;
    for (uint32_t i = 0; i < XPUB_TYPE_NUM; i++) {
        value = GetCurrentAccountPublicKey(i);
        if (value != NULL) {
            printf("%s pub key=%s\r\n", g_chainTable[i].name, value);
        }
    }
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
        cJSON *jsonItem = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonItem, "recvIndex", cJSON_CreateNumber(0));
        cJSON_AddItemToObject(jsonItem, "recvPath", cJSON_CreateNumber(0));
        cJSON_AddItemToObject(jsonItem, "firstRecv", cJSON_CreateBool(isFirst));
        cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
        cJSON_AddItemToObject(rootJson, chainName, jsonItem);
    } else {
        cJSON_ReplaceItemInObject(item, "firstRecv", cJSON_CreateBool(isFirst));
    }

    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MUTABLE_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }
    jsonString = cJSON_PrintBuffered(rootJson, SPI_FLASH_SIZE_USER1_MUTABLE_DATA - 4, false);
    cJSON_Delete(rootJson);
    RemoveFormatChar(jsonString);
    size = strlen(jsonString);
    Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
    Gd25FlashWriteBuffer(addr + 4, (uint8_t *)jsonString, size);
    EXT_FREE(jsonString);
}

void appendWalletItemToJson(MultiSigWalletItem_t *item, void *root)
{
    cJSON *walletItem = cJSON_CreateObject();
    cJSON_AddNumberToObject(walletItem, "order", item->order);
    cJSON_AddStringToObject(walletItem, "name", item->name);
    cJSON_AddStringToObject(walletItem, "verify_code", item->verifyCode);
    cJSON_AddNumberToObject(walletItem, "network", item->network);
    cJSON_AddStringToObject(walletItem, "wallet_config", item->walletConfig);
    cJSON_AddStringToObject(walletItem, "format", item->format);
    cJSON_AddItemToArray((cJSON*)root, walletItem);
}

static uint32_t MultiSigWalletSaveDefault(uint32_t addr, uint8_t accountIndex)
{
    uint32_t eraseAddr, size;
    char *retStr;
    uint8_t hash[32];
    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MULTI_SIG_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }

    cJSON *rootJson = cJSON_CreateObject();
    assert(rootJson != NULL);
    cJSON_AddItemToObject(rootJson, "version", cJSON_CreateString(g_multiSigInfoVersion));
    cJSON_AddItemToObject(rootJson, "multi_sig_wallet_list", cJSON_CreateArray());
    retStr = cJSON_PrintBuffered(rootJson, SPI_FLASH_SIZE_USER1_MULTI_SIG_DATA - 4, false);

    cJSON_Delete(rootJson);
    size = strlen(retStr);
    Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
    int len = Gd25FlashWriteBuffer(addr + 4, (uint8_t *)retStr, size);
    assert(len == size);
    sha256((struct sha256 *)hash, retStr, size);
    if (SetMultisigDataHash(accountIndex, hash) != SUCCESS_CODE) {
        printf("set multi hash failed\r\n");
    }
    EXT_FREE(retStr);
    return size;
}

void MultiSigWalletSave(const char *password, MultiSigWalletManager_t *manager)
{
    uint8_t account = GetCurrentAccountIndex();
    ASSERT(account < 3);
    uint32_t addr, eraseAddr, size;
    uint8_t hash[32];
    int len = 0;

    addr = SPI_FLASH_ADDR_USER1_MULTI_SIG_DATA + account * SPI_FLASH_ADDR_EACH_SIZE;
    printf("MultiSigWalletsave save addr is %x\r\n", addr);
    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MULTI_SIG_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }
    cJSON *rootJson = cJSON_CreateObject();
    cJSON_AddItemToObject(rootJson, "version", cJSON_CreateString(g_multiSigInfoVersion));
    cJSON *walletList = cJSON_CreateArray();
    manager->traverseList(appendWalletItemToJson, (void*)walletList);
    cJSON_AddItemToObject(rootJson, "multi_sig_wallet_list", walletList);
    char *retStr;
    retStr = cJSON_PrintBuffered(rootJson, 1024 * 8, 0);
    size = strlen(retStr);
    assert(size < SPI_FLASH_SIZE_USER1_MULTI_SIG_DATA - 4);
    cJSON_Delete(rootJson);

    Gd25FlashWriteBuffer(addr, (uint8_t *)&size, sizeof(size));
    len = Gd25FlashWriteBuffer(addr + 4, (uint8_t *)retStr, size);
    assert(len == size);
    // write se
    sha256((struct sha256 *)hash, retStr, size);
    if (SetMultisigDataHash(account, hash) != SUCCESS_CODE) {
        printf("set multi hash failed\r\n");
    }
    CLEAR_ARRAY(hash);
    EXT_FREE(retStr);
}

int32_t MultiSigWalletGet(uint8_t accountIndex, const char *password, MultiSigWalletManager_t *manager)
{
    ASSERT(accountIndex < 3);

    uint32_t addr, size;
    int32_t ret = SUCCESS_CODE;
    char *jsonString = NULL;
    uint8_t hash[32];

    addr = SPI_FLASH_ADDR_USER1_MULTI_SIG_DATA + accountIndex * SPI_FLASH_ADDR_EACH_SIZE;
    printf("MultiSigWalletGet read addr is %x\r\n", addr);
    ret = Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
    ASSERT(ret == 4);
    if (size == 0xffffffff || size == 0) {
        size = MultiSigWalletSaveDefault(addr, accountIndex);
    }

    jsonString = SRAM_MALLOC(size + 1);
    ret = Gd25FlashReadBuffer(addr + 4, (uint8_t *)jsonString, size);
    ASSERT(ret == size);
    jsonString[size] = 0;
    printf("multi sig wallet get data is %s\r\n", jsonString);

#ifndef COMPILE_SIMULATOR
    sha256((struct sha256 *)hash, jsonString, strlen(jsonString));
    if (!VerifyMultisigWalletDataHash(accountIndex, hash)) {
        CLEAR_ARRAY(hash);
        return ERR_KEYSTORE_EXTEND_PUBLIC_KEY_NOT_MATCH;
    } else {
        ret = SUCCESS_CODE;
    }
    CLEAR_ARRAY(hash);
#else
    ret = SUCCESS_CODE;
#endif

    cJSON *rootJson = cJSON_Parse(jsonString);
    SRAM_FREE(jsonString);

    cJSON *multiSigWalletList = cJSON_GetObjectItem(rootJson, "multi_sig_wallet_list");
    int walletListSize = cJSON_GetArraySize(multiSigWalletList);

    if (multiSigWalletList != NULL) {

        char *strCache = (char *) MULTI_SIG_MALLOC(MULTI_SIG_STR_CACHE_LENGTH);

        for (int i = 0; i < walletListSize; i++) {

            MultiSigWalletItem_t *multiSigWalletItem = (MultiSigWalletItem_t*) MULTI_SIG_MALLOC(sizeof(MultiSigWalletItem_t));
            cJSON *wallet = cJSON_GetArrayItem(multiSigWalletList, i);
            cJSON *order = cJSON_GetObjectItem(wallet, "order");
            multiSigWalletItem->order = order->valueint;
            GetStringValue(wallet, "name", strCache, MULTI_SIG_STR_CACHE_LENGTH);
            multiSigWalletItem->name = MULTI_SIG_MALLOC(strlen(strCache) + 1);
            strcpy(multiSigWalletItem->name, strCache);

            GetStringValue(wallet, "verify_code", strCache, MULTI_SIG_STR_CACHE_LENGTH);
            multiSigWalletItem->verifyCode = MULTI_SIG_MALLOC(strlen(strCache) + 1);
            strcpy(multiSigWalletItem->verifyCode, strCache);

            cJSON *network = cJSON_GetObjectItem(wallet, "network");

            multiSigWalletItem->network = network->valueint;

            GetStringValue(wallet, "wallet_config", strCache, MULTI_SIG_STR_CACHE_LENGTH);
            multiSigWalletItem->walletConfig = MULTI_SIG_MALLOC(strlen(strCache) + 1);
            strcpy(multiSigWalletItem->walletConfig, strCache);

            GetStringValue(wallet, "format", strCache, MULTI_SIG_STR_CACHE_LENGTH);
            multiSigWalletItem->format = MULTI_SIG_MALLOC(strlen(strCache) + 1);
            strcpy(multiSigWalletItem->format, strCache);

            manager->insertNode(multiSigWalletItem);
        }
        MULTI_SIG_FREE(strCache);
    }
    cJSON_Delete(rootJson);
    return ret;
}

uint32_t GetAccountReceiveIndex(const char* chainName)
{
    uint32_t index = 0;
    cJSON *rootJson = ReadAndParseAccountJson(NULL, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        printf("receive index cannot get %s\r\n", chainName);
        cJSON *jsonItem = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonItem, "recvIndex", cJSON_CreateNumber(0)); // recvIndex is the address index
        cJSON_AddItemToObject(jsonItem, "recvPath", cJSON_CreateNumber(0)); // recvPath is the derivation path type
        cJSON_AddItemToObject(jsonItem, "firstRecv", cJSON_CreateBool(true)); // firstRecv is the first receive address
        if (!strcmp(chainName, "TON")) {
            cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
        } else if ((!strcmp(chainName, "BTC") || !strcmp(chainName, "ETH"))) {
            cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
        } else {
            cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(false));
        }
        cJSON_AddItemToObject(rootJson, chainName, jsonItem);
    } else {
        cJSON *recvIndex = cJSON_GetObjectItem(item, "recvIndex");
        index = recvIndex ? recvIndex->valueint : 0;
    }

    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
    return index;
}

void SetAccountReceiveIndex(const char* chainName, uint32_t index)
{
    uint32_t addr;
    cJSON *rootJson = ReadAndParseAccountJson(&addr, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        printf("SetAccountReceiveIndex cannot get %s\r\n", chainName);
    }

    cJSON *recvIndex = cJSON_GetObjectItem(item, "recvIndex");
    if (recvIndex != NULL) {
        cJSON_ReplaceItemInObject(item, "recvIndex", cJSON_CreateNumber(index));
    } else {
        cJSON_AddItemToObject(item, "recvIndex", cJSON_CreateNumber(index));
    }

    WriteJsonToFlash(addr, rootJson);
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
}

uint32_t GetAccountReceivePath(const char* chainName)
{
    uint32_t index = 0;
    cJSON *rootJson = ReadAndParseAccountJson(NULL, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        printf("GetAccountReceivePath index cannot get %s\r\n", chainName);
    } else {
        cJSON *recvPath = cJSON_GetObjectItem(item, "recvPath");
        index = recvPath ? recvPath->valueint : 0;
    }
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
    return index;
}

void SetAccountReceivePath(const char* chainName, uint32_t index)
{
    uint32_t addr;
    cJSON *rootJson = ReadAndParseAccountJson(&addr, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        printf("SetAccountReceivePath cannot get %s\r\n", chainName);
        cJSON_Delete(rootJson);
        return;
    }
    cJSON *recvPath = cJSON_GetObjectItem(item, "recvPath");
    if (recvPath != NULL) {
        cJSON_ReplaceItemInObject(item, "recvPath", cJSON_CreateNumber(index));
    } else {
        cJSON_AddItemToObject(item, "recvPath", cJSON_CreateNumber(index));
    }

    WriteJsonToFlash(addr, rootJson);
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
}

uint32_t GetAccountIndex(const char* chainName)
{
    uint32_t index = 0;
    cJSON *rootJson = ReadAndParseAccountJson(NULL, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        printf("account index cannot get %s\r\n", chainName);
    } else {
        cJSON *recvAccount = cJSON_GetObjectItem(item, "recvAccount");
        index = recvAccount ? recvAccount->valueint : 0;
    }
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
    return index;
}

void SetAccountIndex(const char* chainName, uint32_t index)
{
    uint32_t addr;
    cJSON *rootJson = ReadAndParseAccountJson(&addr, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, chainName);
    if (item == NULL) {
        printf("SetAccountIndex cannot get %s\r\n", chainName);
        cJSON_Delete(rootJson);
        return;
    }
    cJSON *recvAccount = cJSON_GetObjectItem(item, "recvAccount");
    if (recvAccount != NULL) {
        cJSON_ReplaceItemInObject(item, "recvAccount", cJSON_CreateNumber(index));
    } else {
        cJSON_AddItemToObject(item, "recvAccount", cJSON_CreateNumber(index));
    }

    WriteJsonToFlash(addr, rootJson);
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
}

uint32_t GetConnectWalletPathIndex(const char* walletName)
{
    char name[BUFFER_SIZE_32];
    strncpy_s(name, BUFFER_SIZE_32, walletName, BUFFER_SIZE_32);
    RemoveFormatChar(name);
    return GetTemplateWalletValue(name, "derivePath");
}

void SetConnectWalletPathIndex(const char* walletName, uint32_t index)
{
    char name[BUFFER_SIZE_32];
    strncpy_s(name, BUFFER_SIZE_32, walletName, BUFFER_SIZE_32);
    RemoveFormatChar(name);
    SetTemplateWalletValue(name, "derivePath", index);
}

uint32_t GetConnectWalletAccountIndex(const char* walletName)
{
    uint32_t index = 0;
    char name[BUFFER_SIZE_32];
    strncpy_s(name, BUFFER_SIZE_32, walletName, BUFFER_SIZE_32);
    RemoveFormatChar(name);
    cJSON *rootJson = ReadAndParseAccountJson(NULL, NULL);
    cJSON *item = cJSON_GetObjectItem(rootJson, name);
    if (item == NULL) {
        printf("GetConnectWalletAccountIndex get %s not exist\r\n", name);
    } else {
        cJSON *walletAccount = cJSON_GetObjectItem(item, "walletAccount");
        index = walletAccount ? walletAccount->valueint : 0;
    }
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
    return index;
}

void SetConnectWalletAccountIndex(const char* walletName, uint32_t index)
{
    uint32_t addr;
    char name[BUFFER_SIZE_32];
    strncpy_s(name, BUFFER_SIZE_32, walletName, BUFFER_SIZE_32);
    RemoveFormatChar(name);
    cJSON *rootJson = ReadAndParseAccountJson(&addr, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, name);
    if (item == NULL) {
        cJSON *jsonItem = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonItem, "walletAccount", cJSON_CreateNumber(index));
        cJSON_AddItemToObject(rootJson, name, jsonItem);
    } else {
        cJSON *walletAccount = cJSON_GetObjectItem(item, "walletAccount");
        if (walletAccount != NULL) {
            cJSON_ReplaceItemInObject(item, "walletAccount", cJSON_CreateNumber(index));
        } else {
            cJSON_AddItemToObject(item, "walletAccount", cJSON_CreateNumber(index));
        }
    }

    WriteJsonToFlash(addr, rootJson);
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
}

uint32_t GetConnectWalletNetwork(const char* walletName)
{
    uint32_t index = 0;
    cJSON *rootJson = ReadAndParseAccountJson(NULL, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, walletName);
    if (item == NULL) {
        printf("GetConnectWalletNetwork get %s\r\n", walletName);
    } else {
        cJSON *walletNetwork = cJSON_GetObjectItem(item, "walletNetwork");
        index = walletNetwork ? walletNetwork->valueint : 0;
    }
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
    return index;
}

void SetConnectWalletNetwork(const char* walletName, uint32_t index)
{
    uint32_t addr;
    cJSON *rootJson = ReadAndParseAccountJson(&addr, NULL);

    cJSON *item = cJSON_GetObjectItem(rootJson, walletName);
    if (item == NULL) {
        cJSON *jsonItem = cJSON_CreateObject();
        cJSON_AddItemToObject(jsonItem, "walletNetwork", cJSON_CreateNumber(index));
        cJSON_AddItemToObject(rootJson, walletName, jsonItem);
    } else {
        cJSON *walletNetwork = cJSON_GetObjectItem(item, "walletNetwork");
        if (walletNetwork != NULL) {
            cJSON_ReplaceItemInObject(item, "walletNetwork", cJSON_CreateNumber(index));
        } else {
            cJSON_AddItemToObject(item, "walletNetwork", cJSON_CreateNumber(index));
        }
    }

    WriteJsonToFlash(addr, rootJson);
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(rootJson);
    }
}

static uint32_t GetTemplateWalletValue(const char* walletName, const char* key)
{
    uint32_t value = 0;
    cJSON* rootJson = ReadAndParseAccountJson(NULL, NULL);
    cJSON* item = cJSON_GetObjectItem(rootJson, walletName);
    if (item == NULL) {
        printf("GetTemplateWalletValue get %s not exist\r\n", walletName);
    } else {
        cJSON* valueItem = cJSON_GetObjectItem(item, key);
        value = valueItem ? valueItem->valueint : 0;
    }
    CleanupJson(rootJson);
    return value;
}

static void SetTemplateWalletValue(const char* walletName, const char* key, uint32_t value)
{
    uint32_t addr;
    cJSON* rootJson = ReadAndParseAccountJson(&addr, NULL);

    cJSON* item = cJSON_GetObjectItem(rootJson, walletName);
    if (item == NULL) {
        item = cJSON_CreateObject();
        cJSON_AddItemToObject(rootJson, walletName, item);
    }

    cJSON* valueItem = cJSON_GetObjectItem(item, key);
    if (valueItem != NULL) {
        cJSON_SetNumberValue(valueItem, value);
    } else {
        cJSON_AddNumberToObject(item, key, value);
    }

    WriteJsonToFlash(addr, rootJson);
    CleanupJson(rootJson);
}

static void CleanupJson(cJSON* json)
{
    if (!PassphraseExist(GetCurrentAccountIndex())) {
        cJSON_Delete(json);
    }
}

static cJSON* ReadAndParseAccountJson(uint32_t *outAddr, uint32_t *outSize)
{
    int32_t ret = SUCCESS_CODE;
    uint32_t addr, size;
    char *jsonString = NULL;
    cJSON *rootJson = NULL;

    uint8_t account = GetCurrentAccountIndex();
    ASSERT(account < 3);
    if (PassphraseExist(account)) {
        if (g_tempParsePhraseJson == NULL) {
            g_tempParsePhraseJson = cJSON_CreateObject();
        }
        return g_tempParsePhraseJson;
    } else {
        if (g_tempParsePhraseJson != NULL) {
            cJSON_Delete(g_tempParsePhraseJson);
            g_tempParsePhraseJson = NULL;
        }
    }
    addr = SPI_FLASH_ADDR_USER1_MUTABLE_DATA + account * SPI_FLASH_ADDR_EACH_SIZE;
    ret = Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
    ASSERT(ret == 4);

    jsonString = SRAM_MALLOC(size + 1);
    ret = Gd25FlashReadBuffer(addr + 4, (uint8_t *)jsonString, size);
    ASSERT(ret == size);
    jsonString[size] = 0;

    rootJson = cJSON_Parse(jsonString);
    SRAM_FREE(jsonString);

    if (outAddr) *outAddr = addr;
    if (outSize) *outSize = size;

    return rootJson;
}

static void WriteJsonToFlash(uint32_t addr, cJSON *rootJson)
{
    uint32_t eraseAddr, size;
    char *jsonString = NULL;

    uint8_t account = GetCurrentAccountIndex();
    ASSERT(account < 3);
    if (PassphraseExist(account)) {
        return;
    }

    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MUTABLE_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }
    jsonString = cJSON_PrintBuffered(rootJson, SPI_FLASH_SIZE_USER1_MUTABLE_DATA - 4, false);
    printf("save jsonString=%s\r\n", jsonString);
    RemoveFormatChar(jsonString);
    size = strlen(jsonString);
    Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
    Gd25FlashWriteBuffer(addr + 4, (uint8_t *)jsonString, size);
    EXT_FREE(jsonString);
}
