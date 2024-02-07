#include "gui.h"
#include "cjson/cJSON.h"
#include "flash_address.h"
#include "simulator_model.h"

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
int32_t StorageAccountPublicHomeCoinGet(uint32_t addr, uint8_t *buffer, uint32_t size);
int32_t StorageAccountPublicHomeCoinSet(uint32_t addr, uint8_t *buffer, uint32_t size);

SimulatorFlashPath g_simulatorPathMap[] ={
    {SPI_FLASH_ADDR_NORMAL_PARAM, "C:/assets/device_setting.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_NORMAL_PARAM + 4, "C:/assets/device_setting.json", StorageGetData, StorageSetData},
    {SPI_FLASH_ADDR_USER1_MUTABLE_DATA + 1 * SPI_FLASH_ADDR_EACH_SIZE, "C:/assets/coin.json", StorageGetDataSize, StorageSetDataSize},
    {SPI_FLASH_ADDR_USER1_MUTABLE_DATA + 1 * SPI_FLASH_ADDR_EACH_SIZE + 4, "C:/assets/coin.json", StorageGetData, StorageSetData},
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

// void StorageAccountPublicHomeCoinGet(WalletState_t *walletList, uint8_t count)
// {
//     lv_fs_file_t fd;
//     uint32_t size = 0;
//     lv_fs_res_t ret = LV_FS_RES_OK;
//     char buf[JSON_MAX_LEN] = {0};

//     ret = lv_fs_open(&fd, ACCOUNT_PUBLIC_HOME_COIN_PATH, LV_FS_MODE_RD);
//     if (ret != LV_FS_RES_OK) {
//         printf("lv_fs_open failed %s ret = %d line = %d\n", ACCOUNT_PUBLIC_HOME_COIN_PATH, ret, __LINE__);
//         return;
//     }
//     ret = lv_fs_read(&fd, buf, JSON_MAX_LEN, &size);
//     if (ret == LV_FS_RES_OK && size == 0) {
//         lv_fs_close(&fd);
//         ret = lv_fs_open(&fd, ACCOUNT_PUBLIC_HOME_COIN_PATH, LV_FS_MODE_WR);
//         if (ret != LV_FS_RES_OK) {
//             printf("lv_fs_open failed %s ret = %d line = %d\n", ACCOUNT_PUBLIC_HOME_COIN_PATH, ret, __LINE__);
//             return;
//         }
//         cJSON *rootJson, *jsonItem;
//         char *retStr;
//         rootJson = cJSON_CreateObject();
//         for (int i = 0; i < count; i++) {
//             jsonItem = cJSON_CreateObject();
//             cJSON_AddItemToObject(jsonItem, "firstRecv", cJSON_CreateBool(true));
//             if (!strcmp(walletList[i].name, "BTC") || !strcmp(walletList[i].name, "ETH")) {
//                 cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(true));
//             } else {
//                 cJSON_AddItemToObject(jsonItem, "manage", cJSON_CreateBool(false));
//             }
//             cJSON_AddItemToObject(rootJson, walletList[i].name, jsonItem);
//         }
//         retStr = cJSON_Print(rootJson);
//         printf("retStr = %s\n", retStr);
//         // RemoveFormatChar(retStr);
//         cJSON_Delete(rootJson);
//         ret = lv_fs_write(&fd, retStr, strlen(retStr), &size);
//         if (ret != LV_FS_RES_OK) {
//             printf("lv_fs_write failed %s ret = %d\n", ACCOUNT_PUBLIC_HOME_COIN_PATH, ret);
//             return;
//         }
//         lv_fs_close(&fd);
//     }
//     buf[size] = '\0';
//     cJSON *rootJson = cJSON_Parse(buf);
//     for (int i = 0; i < count; i++) {
//         cJSON *item = cJSON_GetObjectItem(rootJson, walletList[i].name);
//         if (item != NULL) {
//             cJSON *manage = cJSON_GetObjectItem(item, "manage");
//             bool state = manage->valueint;
//             walletList[i].state = state;
//         } else {
//             walletList[i].state = false;
//         }
//     }
//     lv_fs_close(&fd);
// }

// void StorageAccountPublicHomeCoinSet(WalletState_t *walletList, uint8_t count)
// {
//     lv_fs_file_t fd;
//     uint32_t size = 0;
//     char buf[JSON_MAX_LEN] = {0};
//     cJSON *rootJson, *jsonItem;
//     bool needUpdate = false;

//     if (LV_FS_RES_OK != lv_fs_open(&fd, ACCOUNT_PUBLIC_HOME_COIN_PATH, LV_FS_MODE_RD)) {
//         printf("lv_fs_open failed %s\n", ACCOUNT_PUBLIC_HOME_COIN_PATH);
//         return;
//     }

//     lv_fs_read(&fd, buf, JSON_MAX_LEN, &size);
//     buf[size] = '\0';
//     lv_fs_close(&fd);

//     rootJson = cJSON_Parse(buf);
//     for (int i = 0; i < count; i++) {
//         cJSON *item = cJSON_GetObjectItem(rootJson, walletList[i].name);
//         if (item != NULL) {
//             cJSON *manage = cJSON_GetObjectItem(item, "manage");
//             bool state = manage->valueint;
//             if (state != walletList[i].state) {
//                 needUpdate = true;
//                 cJSON_ReplaceItemInObject(item, "manage", cJSON_CreateBool(walletList[i].state));
//             }
//         }
//     }
//     if (needUpdate) {
//         char *retStr = cJSON_Print(rootJson);
//         if (LV_FS_RES_OK != lv_fs_open(&fd, ACCOUNT_PUBLIC_HOME_COIN_PATH, LV_FS_MODE_RD | LV_FS_MODE_WR)) {
//             printf("lv_fs_open failed %s\n", ACCOUNT_PUBLIC_HOME_COIN_PATH);
//             return;
//         }
//         if (LV_FS_RES_OK != lv_fs_write(&fd, retStr, strlen(retStr), &size)) {
//             lv_fs_close(&fd);
//             printf("lv_fs_write failed %s\n", ACCOUNT_PUBLIC_HOME_COIN_PATH);
//             return;
//         }
//         lv_fs_close(&fd);
//     }
// }

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

    ret = lv_fs_open(&fd, path, LV_FS_MODE_RD);
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
    return 0;
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

    ret = lv_fs_open(&fd, path, LV_FS_MODE_RD);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_open failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return;
    }

    ret = lv_fs_read(&fd, buffer, size, &readBytes);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_read failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return;
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
        return;
    }

    ret = lv_fs_write(&fd, buffer, size, &readBytes);
    if (ret != LV_FS_RES_OK) {
        printf("lv_fs_read failed %s ret = %d line = %d\n", path, ret, __LINE__);
        return;
    }
    lv_fs_close(&fd);
    
    return readBytes;
}

int32_t Gd25FlashReadBuffer(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    OperateStorageDataFunc func = FindSimulatorStorageFunc(addr, true);
    if (func) {
        return func(addr, buffer, size);
    } else {
        return -1;
    }
}

int32_t Gd25FlashWriteBuffer(uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    OperateStorageDataFunc func = FindSimulatorStorageFunc(addr, false);
    if (func) {
        return func(addr, buffer, size);
    } else {
        return -1;
    }
}

int32_t Gd25FlashBlockErase(uint32_t addr)
{

}

int32_t Gd25FlashSectorErase(uint32_t addr)
{

}