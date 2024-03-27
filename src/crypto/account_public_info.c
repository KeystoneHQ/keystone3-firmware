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
};

void AccountPublicHomeCoinGet(WalletState_t *walletList, uint8_t count)
{

}

void AccountPublicHomeCoinSet(WalletState_t *walletList, uint8_t count)
{

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

}

void SetFirstReceive(const char* chainName, bool isFirst)
{

}
