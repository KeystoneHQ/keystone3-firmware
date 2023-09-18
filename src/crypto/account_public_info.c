/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Account public  info.
 * Author: leon sun
 * Create: 2023-5-4
 ************************************************************************************************/

#include "account_public_info.h"
#include "string.h"
#include "stdio.h"
#include "user_memory.h"
#include "keystore.h"
#include "flash_address.h"
#include "drv_gd25qxx.h"
#include "define.h"
#include "user_utils.h"
#include "librust_c.h"
#include "cjson/cJSON.h"
#include "assert.h"
#include "gui.h"
#include "gui_home_widgets.h"
#include "sha256.h"
#include "se_manager.h"

#define PUB_KEY_MAX_LENGTH                  256
#define VERSION_MAX_LENGTH                  64

#define INVALID_ACCOUNT_INDEX               255

typedef struct {
    char *pubKey;
    int32_t current;
} AccountPublicKeyItem_t;


typedef struct {
    ChainType chain;
    char *name;
    char *path;
} ChainItem_t;


static bool GetPublicKeyFromJsonString(const char *string);
static char *GetJsonStringFromPublicKey(void);

static void FreePublicKeyRam(void);
static void PrintInfo(void);
static void GetStringValue(cJSON *obj, const char *key, char *value, uint32_t maxLen);

static AccountPublicKeyItem_t g_accountPublicKey[XPUB_TYPE_NUM];
static uint8_t g_tempPublicKeyAccountIndex = INVALID_ACCOUNT_INDEX;

static const char g_xpubInfoVersion[] = "1.0.0";

static const ChainItem_t g_chainTable[] = {
    {XPUB_TYPE_BTC,                        "btc",                      "M/49'/0'/0'"       },
    {XPUB_TYPE_BTC_LEGACY,                 "btc_legacy",               "M/44'/0'/0'"       },
    {XPUB_TYPE_BTC_NATIVE_SEGWIT,          "btc_nested_segwit",        "M/84'/0'/0'"       },
    {XPUB_TYPE_LTC,                        "ltc",                      "M/49'/2'/0'"       },
    {XPUB_TYPE_DASH,                       "dash",                     "M/44'/5'/0'"       },
    {XPUB_TYPE_BCH,                        "bch",                      "M/44'/145'/0'"     },
    {XPUB_TYPE_ETH_BIP44_STANDARD,         "eth_bip44_standard",       "M/44'/60'/0'"      },
    {XPUB_TYPE_ETH_LEDGER_LEGACY,          "eth_ledger_legacy",        "M/44'/60'/0'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_0,          "eth_ledger_live_0",        "M/44'/60'/0'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_1,          "eth_ledger_live_1",        "M/44'/60'/1'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_2,          "eth_ledger_live_2",        "M/44'/60'/2'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_3,          "eth_ledger_live_3",        "M/44'/60'/3'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_4,          "eth_ledger_live_4",        "M/44'/60'/4'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_5,          "eth_ledger_live_5",        "M/44'/60'/5'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_6,          "eth_ledger_live_6",        "M/44'/60'/6'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_7,          "eth_ledger_live_7",        "M/44'/60'/7'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_8,          "eth_ledger_live_8",        "M/44'/60'/8'"      },
    {XPUB_TYPE_ETH_LEDGER_LIVE_9,          "eth_ledger_live_9",        "M/44'/60'/9'"      },
    {XPUB_TYPE_TRX,                        "trx",                      "M/44'/195'/0'"     },
    {XPUB_TYPE_COSMOS,                     "cosmos",                   "M/44'/118'/0'"     },
    {XPUB_TYPE_SCRT,                       "scrt",                     "M/44'/529'/0'"     },
    {XPUB_TYPE_CRO,                        "cro",                      "M/44'/394'/0'"     },
    {XPUB_TYPE_IOV,                        "iov",                      "M/44'/234'/0'"     },
    {XPUB_TYPE_BLD,                        "bld",                      "M/44'/564'/0'"     },
    {XPUB_TYPE_KAVA,                       "kava",                     "M/44'/459'/0'"     },
    {XPUB_TYPE_TERRA,                      "terra",                    "M/44'/330'/0'"     },
};

void AccountPublicHomeCoinGet(WalletState_t *walletList, uint8_t count)
{
    int32_t ret = SUCCESS_CODE;
    uint32_t addr, size, eraseAddr;
    bool needSet = false;
    char *jsonString = NULL;

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
            cJSON_AddItemToObject(jsonItem, "firstRecv", cJSON_CreateBool(false));
            if (!strcmp(walletList[i].name, "BTC") || !strcmp(walletList[i].name, "ETH")) {
                cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
            } else {
                cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(false));
            }
            cJSON_AddItemToObject(rootJson, walletList[i].name, jsonItem);
        }
        retStr = cJSON_Print(rootJson);
        cJSON_Delete(rootJson);
        RemoveFormatChar(retStr);
        size = strlen(retStr);
        Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
        Gd25FlashWriteBuffer(addr + 4, (uint8_t *)retStr, size);
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
            cJSON *manage = cJSON_GetObjectItem(item, "manage");
            bool state = manage->valueint;
            walletList[i].state = state;
        } else {
            walletList[i].state = false;
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
            cJSON_AddItemToObject(item, "firstRecv", cJSON_CreateBool(false));
            cJSON_AddItemToObject(item, "manage", cJSON_CreateBool(walletList[i].state));
            cJSON_AddItemToObject(rootJson, walletList[i].name, item);
            needUpdate = true;
        } else {
            cJSON *manage = cJSON_GetObjectItem(item, "manage");
            bool state = manage->valueint;
            if (state != walletList[i].state) {
                needUpdate = true;
                cJSON_ReplaceItemInObject(item, "manage", cJSON_CreateBool(walletList[i].state));
            }
        }
    }

    if (needUpdate) {
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
}

int32_t AccountPublicInfoSwitch(uint8_t accountIndex, const char *password, bool newKey)
{
    uint32_t addr, size, i, eraseAddr;
    char *jsonString = NULL;
    SimpleResponse_c_char *xPubResult;
    int32_t ret = SUCCESS_CODE;
    bool regeneratePubKey = newKey;
    uint8_t seed[64];
    uint8_t hash[32];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();

    ASSERT(accountIndex < 3);
    FreePublicKeyRam();
    addr = SPI_FLASH_ADDR_USER1_DATA + accountIndex * SPI_FLASH_ADDR_EACH_SIZE;

    do {
        if (regeneratePubKey) {
            break;
        }
        ret = Gd25FlashReadBuffer(addr, (uint8_t *)&size, sizeof(size));
        ASSERT(ret == 4);
        if (size > SPI_FLASH_ADDR_EACH_SIZE - 4) {
            regeneratePubKey = true;
            printf("pubkey size err,%d\r\n", size);
            break;
        }
        jsonString = SRAM_MALLOC(size + 1);
        ret = Gd25FlashReadBuffer(addr + 4, (uint8_t *)jsonString, size);
        ASSERT(ret == size);
        jsonString[size] = 0;
        sha256((struct sha256 *)hash, jsonString, strlen(jsonString));
        if (!VerifyWalletDataHash(accountIndex, hash)) {
            CLEAR_ARRAY(hash);
            return ERR_KEYSTORE_EXTEND_PUBLIC_KEY_NOT_MATCH;
        }
        CLEAR_ARRAY(hash);
        if (GetPublicKeyFromJsonString(jsonString) == false) {
            printf("GetPublicKeyFromJsonString false, need regenerate\r\n");
            printf("err jsonString=%s\r\n", jsonString);
            regeneratePubKey = true;
        }
    } while (0);

    if (jsonString) {
        SRAM_FREE(jsonString);
    }

    do {
        if (regeneratePubKey == false) {
            break;
        }
        printf("regenerate pub key!\r\n");
        FreePublicKeyRam();
        ret = GetAccountSeed(accountIndex, seed, password);
        CHECK_ERRCODE_BREAK("get seed", ret);
        for (i = 0; i < NUMBER_OF_ARRAYS(g_chainTable); i++) {
            xPubResult = get_extended_pubkey_by_seed(seed, len, g_chainTable[i].path);
            if (xPubResult->error_code != 0) {
                printf("get_extended_pubkey error\r\n");
                if (xPubResult->error_message != NULL) {
                    printf("error code = %d\r\nerror msg is: %s\r\n", xPubResult->error_code, xPubResult->error_message);
                }
                free_simple_response_c_char(xPubResult);
                ret = xPubResult->error_code;
                break;
            }
            printf("index=%d,path=%s,pub=%s\r\n", accountIndex, g_chainTable[i].path, xPubResult->data);
            ASSERT(xPubResult->data);
            g_accountPublicKey[i].pubKey = SRAM_MALLOC(strlen(xPubResult->data) + 1);
            strcpy(g_accountPublicKey[i].pubKey, xPubResult->data);
            printf("xPubResult=%s\r\n", xPubResult->data);
            free_simple_response_c_char(xPubResult);
        }
        printf("erase user data:0x%X\n", addr);
        for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_ADDR_EACH_SIZE; eraseAddr += GD25QXX_SECTOR_SIZE) {
            Gd25FlashSectorErase(eraseAddr);
        }
        printf("erase done\n");
        jsonString = GetJsonStringFromPublicKey();
        sha256((struct sha256 *)hash, jsonString, strlen(jsonString));
        SetWalletDataHash(accountIndex, hash);
        CLEAR_ARRAY(hash);
        size = strlen(jsonString);
        Gd25FlashWriteBuffer(addr, (uint8_t *)&size, 4);
        Gd25FlashWriteBuffer(addr + 4, (uint8_t *)jsonString, size);
        printf("regenerate jsonString=%s\r\n", jsonString);
        EXT_FREE(jsonString);
    } while (0);
    printf("AccountPublicInfoSwitch over\r\n");
    //PrintInfo();
    CLEAR_ARRAY(seed);
    return ret;
}


int32_t TempAccountPublicInfo(uint8_t accountIndex, const char *password, bool set)
{
    uint32_t i;
    SimpleResponse_c_char *xPubResult;
    int32_t ret = SUCCESS_CODE;
    uint8_t seed[64];
    int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();

    if (g_tempPublicKeyAccountIndex == accountIndex && set == false) {
        // g_accountPublicKey stores the current temp public key.
        printf("g_accountPublicKey stores the current temp public key.\r\n");
    } else {
        FreePublicKeyRam();
        ret = GetAccountSeed(accountIndex, seed, password);
        CHECK_ERRCODE_RETURN_INT(ret);
        for (i = 0; i < NUMBER_OF_ARRAYS(g_chainTable); i++) {
            xPubResult = get_extended_pubkey_by_seed(seed, len, g_chainTable[i].path);
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
            g_accountPublicKey[i].pubKey = SRAM_MALLOC(strlen(xPubResult->data) + 1);
            strcpy(g_accountPublicKey[i].pubKey, xPubResult->data);
            printf("xPubResult=%s\r\n", xPubResult->data);
            free_simple_response_c_char(xPubResult);
        }
        g_tempPublicKeyAccountIndex = accountIndex;
    }
    CLEAR_ARRAY(seed);
    return ret;
}


void DeleteAccountPublicInfo(uint8_t accountIndex)
{
    uint32_t addr, eraseAddr;

    ASSERT(accountIndex < 3);
    addr = SPI_FLASH_ADDR_USER1_DATA + accountIndex * SPI_FLASH_ADDR_EACH_SIZE;
    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_ADDR_EACH_SIZE; eraseAddr += GD25QXX_SECTOR_SIZE) {
        Gd25FlashSectorErase(eraseAddr);
    }

    addr = SPI_FLASH_ADDR_USER1_MUTABLE_DATA + accountIndex * SPI_FLASH_ADDR_EACH_SIZE;
    for (eraseAddr = addr; eraseAddr < addr + SPI_FLASH_SIZE_USER1_MUTABLE_DATA; eraseAddr += GD25QXX_SECTOR_SIZE) {
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
    return g_accountPublicKey[chain].pubKey;
}


/// @brief Get if the xPub already Exists.
/// @param[in] xPub
/// @return accountIndex, if not exists, return 255.
uint8_t SpecifiedXPubExist(const char *xPub)
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
        if (size > SPI_FLASH_ADDR_EACH_SIZE - 4) {
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
            chainJson = cJSON_GetObjectItem(keyJson, g_chainTable[0].name);
            if (chainJson == NULL) {
                break;
            }
            GetStringValue(chainJson, "value", pubKeyString, PUB_KEY_MAX_LENGTH);
            if (strcmp(pubKeyString, xPub) == 0) {
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
        accountIndex = SpecifiedXPubExist(argv[1]);
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
        for (i = 0; i < NUMBER_OF_ARRAYS(g_chainTable); i++) {
            chainJson = cJSON_GetObjectItem(keyJson, g_chainTable[i].name);
            if (chainJson == NULL) {
                ret = false;
                break;
            } else {
                GetStringValue(chainJson, "value", pubKeyString, PUB_KEY_MAX_LENGTH);
                //printf("%s pub key=%s\r\n", g_chainTable[i].name, pubKeyString);
                g_accountPublicKey[i].pubKey = SRAM_MALLOC(strlen(pubKeyString) + 1);
                strcpy(g_accountPublicKey[i].pubKey, pubKeyString);
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
        cJSON_AddItemToObject(jsonItem, "value", cJSON_CreateString(g_accountPublicKey[i].pubKey));
        //printf("g_accountPublicKey[%d].pubKey=%s\r\n", i, g_accountPublicKey[i].pubKey);
        cJSON_AddItemToObject(jsonItem, "current", cJSON_CreateNumber(g_accountPublicKey[i].current));
        cJSON_AddItemToObject(chainsJson, g_chainTable[i].name, jsonItem);
    }
    cJSON_AddItemToObject(rootJson, "version", cJSON_CreateString(g_xpubInfoVersion));
    cJSON_AddItemToObject(rootJson, "key", chainsJson);
    retStr = cJSON_Print(rootJson);
    RemoveFormatChar(retStr);
    cJSON_Delete(rootJson);
    return retStr;
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
    char *pubKey;
    for (uint32_t i = 0; i < XPUB_TYPE_NUM; i++) {
        pubKey = GetCurrentAccountPublicKey(i);
        if (pubKey != NULL) {
            printf("%s pub key=%s\r\n", g_chainTable[i].name, pubKey);
        }
    }
}


static void GetStringValue(cJSON *obj, const char *key, char *value, uint32_t maxLen)
{
    cJSON *json;
    uint32_t len;
    char *strTemp;

    json = cJSON_GetObjectItem(obj, key);
    if (json != NULL) {
        strTemp = json->valuestring;
        len = strlen(strTemp);
        if (len < maxLen) {
            strcpy(value, strTemp);
        } else {
            strcpy(value, "");
        }
    } else {
        strcpy(value, "");
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
