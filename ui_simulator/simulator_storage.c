#include "gui.h"
#include "cjson/cJSON.h"
#include "flash_address.h"
#include "simulator_model.h"
#include "se_manager.h"
#include "log_print.h"
#include "keystore.h"
#include "account_manager.h"

#define DS28S60_DATA_ADDR                           0x1000
#define ATECC608B_DATA_ADDR                         0x2000
#define SIMULATOR_USER1_SECRET_ADDR                 0x3000
#define SIMULATOR_USER2_SECRET_ADDR                 0x4000
#define SIMULATOR_USER3_SECRET_ADDR                 0x5000

typedef int32_t (*OperateStorageDataFunc)(uint32_t addr, uint8_t *buffer, uint32_t size);

typedef struct {
    uint32_t addr;
    const char *path;
    OperateStorageDataFunc getFunc;
    OperateStorageDataFunc setFunc;
} SimulatorFlashPath;

int32_t StorageGetDataSize(uint32_t addr, uint8_t *buffer, uint32_t size);
int32_t StorageSetDataSize(uint32_t addr, uint8_t *buffer, uint32_t size);
int32_t StorageGetData(uint32_t addr, uint8_t *buffer, uint32_t size);
int32_t StorageSetData(uint32_t addr, uint8_t *buffer, uint32_t size);

#define PC_SIMULATOR_PATH "C:/assets"

SimulatorFlashPath g_simulatorPathMap[] = {
    {SPI_FLASH_ADDR_NORMAL_PARAM, PC_SIMULATOR_PATH "/device_setting.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_NORMAL_PARAM + 4, PC_SIMULATOR_PATH "/device_setting.json", StorageGetData, StorageSetData},

    {SPI_FLASH_ADDR_USER1_DATA, PC_SIMULATOR_PATH "/user1_data.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER1_DATA + 4, PC_SIMULATOR_PATH "/user1_data.json", StorageGetData, StorageSetData},
    {SPI_FLASH_ADDR_USER1_MULTI_SIG_DATA, PC_SIMULATOR_PATH "/user1_multisig.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER1_MULTI_SIG_DATA + 4, PC_SIMULATOR_PATH "/user1_multisig.json", StorageGetData, StorageSetData},
    {SPI_FLASH_ADDR_USER1_MUTABLE_DATA, PC_SIMULATOR_PATH "/coin1.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER1_MUTABLE_DATA + 4, PC_SIMULATOR_PATH "/coin1.json", StorageGetData, StorageSetData},
    {SIMULATOR_USER1_SECRET_ADDR, PC_SIMULATOR_PATH "/user1_secret.json", StorageGetData, StorageSetData},

    {SPI_FLASH_ADDR_USER2_DATA, PC_SIMULATOR_PATH "/user2_data.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER2_DATA + 4, PC_SIMULATOR_PATH "/user2_data.json", StorageGetData, StorageSetData},
    {SPI_FLASH_ADDR_USER2_MULTI_SIG_DATA, PC_SIMULATOR_PATH "/user2_multisig.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER2_MULTI_SIG_DATA + 4, PC_SIMULATOR_PATH "/user2_multisig.json", StorageGetData, StorageSetData},
    {SPI_FLASH_ADDR_USER2_MUTABLE_DATA, PC_SIMULATOR_PATH "/coin2.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER2_MUTABLE_DATA + 4, PC_SIMULATOR_PATH "/coin2.json", StorageGetData, StorageSetData},
    {SIMULATOR_USER2_SECRET_ADDR, PC_SIMULATOR_PATH "/user2_secret.json", StorageGetData, StorageSetData},

    {SPI_FLASH_ADDR_USER3_DATA, PC_SIMULATOR_PATH "/user3_data.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER3_DATA + 4, PC_SIMULATOR_PATH "/user3_data.json", StorageGetData, StorageSetData},
    {SPI_FLASH_ADDR_USER3_MULTI_SIG_DATA, PC_SIMULATOR_PATH "/user3_multisig.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER3_MULTI_SIG_DATA + 4, PC_SIMULATOR_PATH "/user3_multisig.json", StorageGetData, StorageSetData},
    {SPI_FLASH_ADDR_USER3_MUTABLE_DATA, PC_SIMULATOR_PATH "/coin3.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER3_MUTABLE_DATA + 4, PC_SIMULATOR_PATH "/coin3.json", StorageGetData, StorageSetData},
    {SIMULATOR_USER3_SECRET_ADDR, PC_SIMULATOR_PATH "/user3_secret.json", StorageGetData, StorageSetData},

    {DS28S60_DATA_ADDR, PC_SIMULATOR_PATH "/ds28s60.json", StorageGetData, StorageSetData},
    {ATECC608B_DATA_ADDR, PC_SIMULATOR_PATH "/atecc608b.json", StorageGetData, StorageSetData},
};

const char *FindSimulatorFlashPath(uint32_t addr)
{
    for (int i = 0; i < sizeof(g_simulatorPathMap) / sizeof(g_simulatorPathMap[0]); i++) {
        if (g_simulatorPathMap[i].addr == addr) {
            return g_simulatorPathMap[i].path;
        }
    }
    return NULL;
}

OperateStorageDataFunc FindSimulatorStorageFunc(uint32_t addr, bool get)
{
    for (int i = 0; i < sizeof(g_simulatorPathMap) / sizeof(g_simulatorPathMap[0]); i++) {
        if (g_simulatorPathMap[i].addr == addr) {
            return get ? g_simulatorPathMap[i].getFunc : g_simulatorPathMap[i].setFunc;
        }
    }
    return NULL;
}

int32_t StorageGetDataSize(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    int32_t readBytes = 0;
    lv_fs_file_t fd;
    lv_fs_res_t ret = LV_FS_RES_OK;
    char tmpBuff[JSON_MAX_LEN] = {0};
    const char *path = FindSimulatorFlashPath(addr);
    if (path == NULL) {
        return -1;
    }

    ret = lv_fs_open(&fd, path, LV_FS_MODE_ALWAYS | LV_FS_MODE_RD);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_open failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }

    ret = lv_fs_read(&fd, tmpBuff, JSON_MAX_LEN, &readBytes);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_read failed %s ret = %d line = %d\n", path, ret, __LINE__);
        lv_fs_close(&fd);
        return -1;
    }
    *(uint32_t *)buffer = readBytes;
    lv_fs_close(&fd);

    return size;
}

int32_t StorageSetDataSize(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    return size;
}

int32_t StorageGetData(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    int32_t readBytes = 0;
    lv_fs_file_t fd;
    lv_fs_res_t ret = LV_FS_RES_OK;
    uint32_t offset = addr % 4;
    const char *path = FindSimulatorFlashPath(addr);
    if (path == NULL) {
        return -1;
    }

    ret = lv_fs_open(&fd, path, LV_FS_MODE_ALWAYS | LV_FS_MODE_RD);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_open failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }

    ret = lv_fs_read(&fd, buffer, size, &readBytes);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_read failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }
    lv_fs_close(&fd);

    return readBytes;
}

int32_t StorageSetData(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    int32_t readBytes = 0;
    lv_fs_file_t fd;
    lv_fs_res_t ret = LV_FS_RES_OK;
    uint32_t offset = addr % 4;
    const char *path = FindSimulatorFlashPath(addr);
    if (path == NULL) {
        return -1;
    }

    ret = lv_fs_open(&fd, path, LV_FS_MODE_WR);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_open failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }

    ret = lv_fs_write(&fd, buffer, size, &readBytes);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_write failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return -1;
    }
    lv_fs_close(&fd);

    return readBytes;
}

int32_t Gd25FlashReadBuffer(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    OperateStorageDataFunc func = FindSimulatorStorageFunc(addr, true);
    if (func == NULL) {
        printf("%s failed, addr = 0x%x\n", __func__, addr);
        return -1;
    } else {
        return func(addr, buffer, size);
    }
    return func ? func(addr, buffer, size) : -1;
}

int32_t Gd25FlashWriteBuffer(uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    OperateStorageDataFunc func = FindSimulatorStorageFunc(addr, false);
    return func ? func(addr, buffer, size) : -1;
}

int32_t Gd25FlashBlockErase(uint32_t addr)
{

}

int32_t Gd25FlashSectorErase(uint32_t addr)
{

}

void InsertJsonU8Array(cJSON *root, const uint8_t *data, uint8_t len, char *key)
{
    cJSON *array = cJSON_CreateArray();
    for (int i = 0; i < len; i++) {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(data[i]));
    }
    cJSON_AddItemToObject(root, key, array);
}

void GetJsonArrayData(cJSON *root, uint8_t *data, uint8_t len, char *key)
{
    cJSON *item = cJSON_GetObjectItem(root, key);
    if (item == NULL) {
        return;
    }
    for (int i = 0; i < cJSON_GetArraySize(item); i++) {
        cJSON *item2 = cJSON_GetArrayItem(item, i);
        data[i] = item2->valueint;
    }
}

void ModifyJsonArrayData(cJSON *root, uint8_t *data, uint8_t len, char *key)
{
    cJSON *item = cJSON_GetObjectItem(root, key);
    if (item == NULL) {
        return InsertJsonU8Array(root, data, len, key);
    }
    printf("cJSON_GetArraySize(item) = %d\n", cJSON_GetArraySize(item));
    for (int i = 0; i < cJSON_GetArraySize(item); i++) {
        cJSON_ReplaceItemInArray(item, i, cJSON_CreateNumber(data[i]));
    }
}

uint8_t SimulatorGetAccountNum(void)
{
    uint8_t buffer[JSON_MAX_LEN] = {0};
    uint8_t accountNum = 0;
    for (int i = 0; i < 3; i++) {
        memset(buffer, 0, JSON_MAX_LEN);
        OperateStorageDataFunc func = FindSimulatorStorageFunc(SIMULATOR_USER1_SECRET_ADDR + i * 0x1000, true);
        func(SIMULATOR_USER1_SECRET_ADDR + i * 0x1000, buffer, JSON_MAX_LEN);
        cJSON *rootJson = cJSON_Parse(buffer);
        if (rootJson == NULL) {
            continue;
        }
        cJSON *item = cJSON_GetObjectItem(rootJson, "password");
        if (item != NULL) {
            accountNum++;
        }
        cJSON_Delete(rootJson);
    }

    return accountNum;
}

int32_t SimulatorSaveAccountSecret(uint8_t accountIndex, const AccountSecret_t *accountSecret, const char *password)
{
    uint8_t buffer[JSON_MAX_LEN] = {'\0'};
    uint32_t size = 0;

    cJSON *rootJson = cJSON_CreateObject();
    InsertJsonU8Array(rootJson, accountSecret->entropy, ENTROPY_MAX_LEN, "entropy");
    InsertJsonU8Array(rootJson, accountSecret->seed, SEED_LEN, "seed");
    InsertJsonU8Array(rootJson, accountSecret->slip39EmsOrTonEntropyL32, SLIP39_EMS_LEN, "slip39_ems");
    InsertJsonU8Array(rootJson, accountSecret->reservedData, SE_DATA_RESERVED_LEN, "reserved_data");
    cJSON *item = cJSON_CreateNumber(accountSecret->entropyLen);
    cJSON_AddItemToObject(rootJson, "entropy_len", item);
    item = cJSON_CreateString(password);
    cJSON_AddItemToObject(rootJson, "password", item);
    char *jsonBuf = cJSON_Print(rootJson);
    strncpy(buffer, jsonBuf, JSON_MAX_LEN);
    OperateStorageDataFunc func = FindSimulatorStorageFunc(SIMULATOR_USER1_SECRET_ADDR + accountIndex * 0x1000, false);
    func(SIMULATOR_USER1_SECRET_ADDR + accountIndex * 0x1000, buffer, strlen(jsonBuf));
    cJSON_Delete(rootJson);

    return SUCCESS_CODE;
}

int32_t SimulatorLoadAccountSecret(uint8_t accountIndex, AccountSecret_t *accountSecret, const char *password)
{
    int32_t ret = SUCCESS_CODE;
    uint8_t buffer[JSON_MAX_LEN] = {'\0'};
    uint32_t size = 0;

    OperateStorageDataFunc func = FindSimulatorStorageFunc(SIMULATOR_USER1_SECRET_ADDR + accountIndex * 0x1000, true);
    func(SIMULATOR_USER1_SECRET_ADDR + accountIndex * 0x1000, buffer, JSON_MAX_LEN);

    cJSON *rootJson = cJSON_Parse(buffer);
    if (rootJson == NULL) {
        return ERR_KEYSTORE_PASSWORD_ERR;
    }

    cJSON *passwordJson = cJSON_GetObjectItem(rootJson, "password");
    if (passwordJson == NULL || strcmp(passwordJson->valuestring, password) != 0) {
        cJSON_Delete(rootJson);
        return ret;
    }
    GetJsonArrayData(rootJson, accountSecret->entropy, ENTROPY_MAX_LEN, "entropy");
    GetJsonArrayData(rootJson, accountSecret->seed, SEED_LEN, "seed");
    GetJsonArrayData(rootJson, accountSecret->slip39EmsOrTonEntropyL32, SLIP39_EMS_LEN, "slip39_ems");
    GetJsonArrayData(rootJson, accountSecret->reservedData, SE_DATA_RESERVED_LEN, "reserved_data");

    cJSON *entropyLenJson = cJSON_GetObjectItem(rootJson, "entropy_len");
    if (entropyLenJson != NULL) {
        accountSecret->entropyLen = (uint8_t)entropyLenJson->valueint;
    }

    cJSON_Delete(rootJson);

    return ret;
}

int32_t SimulatorVerifyPassword(uint8_t *accountIndex, const char *password)
{
    uint8_t buffer[JSON_MAX_LEN] = {0};
    for (int i = 0; i < 3; i++) {
        // func must be found
        OperateStorageDataFunc func = FindSimulatorStorageFunc(SIMULATOR_USER1_SECRET_ADDR + i * 0x1000, true);
        func(SIMULATOR_USER1_SECRET_ADDR + i * 0x1000, buffer, JSON_MAX_LEN);
        cJSON *rootJson = cJSON_Parse(buffer);
        if (rootJson == NULL) {
            continue;
        }
        cJSON *item = cJSON_GetObjectItem(rootJson, "password");
        if (item != NULL && strncmp(item->valuestring, password, strlen(password)) == 0) {
            *accountIndex = i;
            cJSON_Delete(rootJson);
            return SUCCESS_CODE;
        }
        cJSON_Delete(rootJson);
    }

    return ERR_KEYSTORE_PASSWORD_ERR;
}

int32_t SimulatorVerifyCurrentPassword(uint8_t accountIndex, const char *password)
{
    uint8_t buffer[JSON_MAX_LEN] = {0};
    OperateStorageDataFunc func = FindSimulatorStorageFunc(SIMULATOR_USER1_SECRET_ADDR + accountIndex * 0x1000, true);
    func(SIMULATOR_USER1_SECRET_ADDR + accountIndex * 0x1000, buffer, JSON_MAX_LEN);
    cJSON *rootJson = cJSON_Parse(buffer);
    if (rootJson == NULL) {
        return ERR_KEYSTORE_PASSWORD_ERR;
    }
    cJSON *item = cJSON_GetObjectItem(rootJson, "password");
    if (item != NULL && strncmp(item->valuestring, password, strlen(password)) == 0) {
        cJSON_Delete(rootJson);
        return SUCCESS_CODE;
    }
    cJSON_Delete(rootJson);

    return ERR_KEYSTORE_PASSWORD_ERR;
}

// 28S60
#if 1
uint8_t SE_GetAccountIndexFromPage(uint8_t page)
{
    return page / PAGE_NUM_PER_ACCOUNT;
}

int32_t SE_HmacEncryptRead(uint8_t *data, uint8_t page)
{
    uint8_t buffer[JSON_MAX_LEN] = {0};
    if (page == PAGE_PF_ENCRYPTED_PASSWORD) {
    } else if (page == PAGE_WALLET1_PUB_KEY_HASH) {

    } else if (page == PAGE_WALLET2_PUB_KEY_HASH) {

    } else if (page == PAGE_WALLET3_PUB_KEY_HASH) {

    } else if (page == PAGE_PUBLIC_INFO) {

        return 0;
    }

    uint8_t account = SE_GetAccountIndexFromPage(page);
    OperateStorageDataFunc func = FindSimulatorStorageFunc(SIMULATOR_USER1_SECRET_ADDR + account * 0x1000, true);
    if (func) {
        func(SIMULATOR_USER1_SECRET_ADDR + account * 0x1000, buffer, JSON_MAX_LEN);
    }

    cJSON *rootJson = cJSON_Parse(buffer);
    if (page == account * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV) {
        GetJsonArrayData(rootJson, data, 32, "iv");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_ENTROPY_OR_TON_ENTROPY_H32) {
        GetJsonArrayData(rootJson, data, 32, "entropy");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SEED_H32) {
        GetJsonArrayData(rootJson, data, 32, "seed_h32");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SEED_L32) {
        GetJsonArrayData(rootJson, data, 32, "seed_l32");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SLIP39_EMS_OR_TON_ENTROPY_L32) {
        GetJsonArrayData(rootJson, data, 32, "slip39_ems");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_HMAC) {
        GetJsonArrayData(rootJson, data, 32, "hmac");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_KEY_PIECE) {
        GetJsonArrayData(rootJson, data, 32, "key_piece");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH) {
        GetJsonArrayData(rootJson, data, 32, "password_hash");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PARAM) {
        GetJsonArrayData(rootJson, data, sizeof(AccountInfo_t), "param");
        AccountInfo_t *pAccountInfo = (AccountInfo_t *)data;
        // printf("read.....\n");
        // printf("pAccountInfo->entropyLen = %d\n", pAccountInfo->entropyLen);
        // printf("pAccountInfo->passcodeType = %d\n", pAccountInfo->passcodeType);
        // printf("pAccountInfo->isSlip39 = %u\n", pAccountInfo->isSlip39);
        // printf("pAccountInfo->passphraseQuickAccess = %d\n", pAccountInfo->passphraseQuickAccess);
        // printf("pAccountInfo->passphraseMark = %d\n", pAccountInfo->passphraseMark);
        // printf("pAccountInfo->isTon = %u\n", pAccountInfo->isTon);
        // printf("pAccountInfo->slip39Id = %d\n", pAccountInfo->slip39Id[0] * 256 + pAccountInfo->slip39Id[1]);
        // PrintArray("mfp", pAccountInfo->mfp, 4);
        // printf("pAccountInfo->slip39Ie = %d\n", pAccountInfo->slip39Ie[0]);
        // printf("pAccountInfo->iconIndex = %d\n", pAccountInfo->iconIndex);
        // printf("pAccountInfo->walletName = %s\n", pAccountInfo->walletName);
    }

    return 0;
}

int32_t SE_HmacEncryptWrite(const uint8_t *data, uint8_t page)
{
    uint8_t buffer[JSON_MAX_LEN] = {0};
    if (page == PAGE_PF_ENCRYPTED_PASSWORD) {
    } else if (page == PAGE_WALLET1_PUB_KEY_HASH) {

    } else if (page == PAGE_WALLET2_PUB_KEY_HASH) {

    } else if (page == PAGE_WALLET3_PUB_KEY_HASH) {

    } else if (page == PAGE_PUBLIC_INFO) {
        return 0;
    }

    uint8_t account = SE_GetAccountIndexFromPage(page);
    OperateStorageDataFunc func = FindSimulatorStorageFunc(SIMULATOR_USER1_SECRET_ADDR + account * 0x1000, true);
    if (func) {
        func(SIMULATOR_USER1_SECRET_ADDR + account * 0x1000, buffer, JSON_MAX_LEN);
    }
    cJSON *rootJson = cJSON_Parse(buffer);
    if (rootJson == NULL) {
        rootJson = cJSON_CreateObject();
    }
    if (page == account * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV) {
        // ModifyJsonArrayData(rootJson, data, 32, "iv");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_ENTROPY_OR_TON_ENTROPY_H32) {
        // ModifyJsonArrayData(rootJson, data, 32, "entropy");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SEED_H32) {
        // ModifyJsonArrayData(rootJson, data, 32, "seed_h32");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SEED_L32) {
        // ModifyJsonArrayData(rootJson, data, 32, "seed_l32");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_SLIP39_EMS_OR_TON_ENTROPY_L32) {
        // ModifyJsonArrayData(rootJson, data, 32, "slip39_ems");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_HMAC) {
        // ModifyJsonArrayData(rootJson, data, 32, "hmac");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_KEY_PIECE) {
        // ModifyJsonArrayData(rootJson, data, 32, "key_piece");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PASSWORD_HASH) {
        // ModifyJsonArrayData(rootJson, data, 32, "password_hash");
    } else if (page == account *  PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_PARAM) {
        ModifyJsonArrayData(rootJson, data, sizeof(AccountInfo_t), "param");
        AccountInfo_t *pAccountInfo = (AccountInfo_t *)data;
        // printf("write.....\n");
        // printf("pAccountInfo->entropyLen = %d\n", pAccountInfo->entropyLen);
        // printf("pAccountInfo->passcodeType = %d\n", pAccountInfo->passcodeType);
        // printf("pAccountInfo->isSlip39 = %u\n", pAccountInfo->isSlip39);
        // printf("pAccountInfo->passphraseQuickAccess = %d\n", pAccountInfo->passphraseQuickAccess);
        // printf("pAccountInfo->passphraseMark = %d\n", pAccountInfo->passphraseMark);
        // printf("pAccountInfo->isTon = %u\n", pAccountInfo->isTon);
        // printf("pAccountInfo->slip39Id = %d\n", pAccountInfo->slip39Id[0] * 256 + pAccountInfo->slip39Id[1]);
        // PrintArray("mfp", pAccountInfo->mfp, 4);
        // printf("pAccountInfo->slip39Ie = %d\n", pAccountInfo->slip39Ie[0]);
        // printf("pAccountInfo->iconIndex = %d\n", pAccountInfo->iconIndex);
        // printf("pAccountInfo->walletName = %s\n", pAccountInfo->walletName);
    } else {
        return SUCCESS_CODE;
    }

    char *buff = cJSON_Print(rootJson);
    // printf("buff = %s\n", buff);
    func = FindSimulatorStorageFunc(SIMULATOR_USER1_SECRET_ADDR + account * 0x1000, false);
    if (func) {
        func(SIMULATOR_USER1_SECRET_ADDR + account * 0x1000, buff, strlen(buff));
    }

    return SUCCESS_CODE;
}
#endif

// 608B
int32_t Atecc608bEncryptWrite(uint8_t slot, uint8_t block, const uint8_t *data)
{

}

int32_t Atecc608bKdf(uint8_t slot, const uint8_t *authKey, const uint8_t *inData, uint32_t inLen, uint8_t *outData)
{

}

int32_t Atecc608bDeriveKey(uint8_t slot, const uint8_t *authKey)
{

}

int32_t Atecc608bSignMessageWithDeviceKey(uint8_t *messageHash, uint8_t *signature)
{

}

int32_t Atecc608bGenDevicePubkey(uint8_t *pubkey)
{

}

cJSON *Atecc608bEncryptReadJson(void)
{
    uint8_t buffer[JSON_MAX_LEN] = {0};
    OperateStorageDataFunc func = FindSimulatorStorageFunc(ATECC608B_DATA_ADDR, true);
    if (func) {
        func(ATECC608B_DATA_ADDR, buffer, JSON_MAX_LEN);
    }

    return cJSON_Parse(buffer);
}

int32_t SE_EncryptWrite(uint8_t slot, uint8_t block, const uint8_t *data)
{
    // block default 0
    cJSON *rootKey = Atecc608bEncryptReadJson();
    int i = 0;
    if (slot == 3) {

    }
    return 0;
}

int32_t SE_Kdf(uint8_t slot, const uint8_t *authKey, const uint8_t *inData, uint32_t inLen, uint8_t *outData)
{
    cJSON *rootKey = Atecc608bEncryptReadJson();
    int i = 0;
    if (slot == 4) {

    }
    return 0;
}

int32_t SE_DeriveKey(uint8_t slot, const uint8_t *authKey)
{
    cJSON *rootKey = Atecc608bEncryptReadJson();
    int i = 0;
    if (slot == 4) {

    }
    return 0;
}

void FatfsGetFileName(const char *path, char *fileName[], uint32_t maxLen, uint32_t *number, const char *contain)
{
    lv_fs_dir_t dir;
    char fname[256];
    uint32_t count = 0;
    lv_fs_res_t res = lv_fs_dir_open(&dir, path);
    if (res != LV_FS_RES_OK) {
        return;
    }

    while (lv_fs_dir_read(&dir, fname) == LV_FS_RES_OK) {
        if (strlen(fname) == 0) {
            break;
        }
        if (contain != NULL && !strstr(fname, contain)) {
            continue;
        }

        printf("fname = %s\n", fname);
        strcpy(fileName[count], fname);
        count++;
    }
    lv_fs_dir_close(&dir);
    *number = count;
}