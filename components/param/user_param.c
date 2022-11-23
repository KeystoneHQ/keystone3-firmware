/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : user_param.c
 * Description:
 * author     : stone wang
 * data       : 2022-12-23 15:01
**********************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mhscpu.h"
#include "drv_gd25qxx.h"
#include "user_param.h"
#include "cjson/cJSON.h"

#include "user_param_enum.h"
#include "user_utils.h"


static UserParam_t *g_paramTbl = NULL;
static uint16_t g_paramIdStart = 0;
static uint16_t g_paramIdMax = 0;

static UserParam_t *g_protectedParamTbl = NULL;
static uint16_t g_protectedIdStart = 0;
static uint16_t g_protectedIdMax = 0;

static cJSON *g_userParamJson = NULL;
static cJSON *g_protectParamJson = NULL;

#define PARAM_NORMAL_ADDR                   0xFFE000
#define PARAM_PROTECT_ADDR                  0xFFF000

static PARAM_PROTECT_TYPE_ENUM GetParamProtectType(uint16_t id);
static int ParamWriteParam(PARAM_PROTECT_TYPE_ENUM protectType, const UserParam_t *paramTbl, cJSON *paramJson,
                           uint16_t id, void *pBuf);
static int ParamReadParam(const UserParam_t *paramTbl, cJSON *paramJson, uint16_t id, void *pBuf);

void ParamSave(PARAM_PROTECT_TYPE_ENUM protectType)
{
    uint8_t paramBuf[2048] = {0};
    uint32_t paramSize = 0;
    uint32_t paramAddr = 0;
    char* str = NULL;

    if (PARAM_USER_TYPE == protectType) {
        paramAddr = PARAM_NORMAL_ADDR;
        str = cJSON_Print(g_userParamJson);
    } else {
        paramAddr = PARAM_PROTECT_ADDR;
        str = cJSON_Print(g_protectParamJson);
    }
    RemoveFormatChar(str);
    paramSize = strlen(str);
    sprintf((char *)paramBuf, "%4d%s", paramSize, str);

    Gd25FlashSectorErase(paramAddr);
    Gd25FlashWriteBuffer(paramAddr, paramBuf, paramSize + sizeof(paramSize));
}

void ParamPrintf(uint16_t id)
{
    UserParam_t *paramTbl;
    uint32_t tempIntVal = 0;
    double tempDoubleVal = 0;
    char tempBuf[32] = {0};
    cJSON *paramJson;
    PARAM_PROTECT_TYPE_ENUM protectType = GetParamProtectType(id);

    if (PARAM_USER_TYPE == protectType) {
        paramTbl = g_paramTbl;
        paramJson = g_userParamJson;
    } else if (PARAM_PROTECT_TYPE == protectType) {
        paramJson = g_protectParamJson;
        paramTbl = g_protectedParamTbl;
        id -= 100;
    } else {
        return;
    }

    switch (paramTbl[id].paramType) {
    case PARAM_TYPE_INT:
        ParamReadParam(paramTbl, paramJson, id, &tempIntVal);
        printf("%s = %d\n", paramTbl[id].paramName, tempIntVal);
        break;
    case PARAM_TYPE_DOUBLE:
        ParamReadParam(paramTbl, paramJson, id, &tempDoubleVal);
        printf("%s = %f\n", paramTbl[id].paramName, tempDoubleVal);
        break;
    case PARAM_TYPE_STRING:
        ParamReadParam(paramTbl, paramJson, id, tempBuf);
        printf("%s = %s\n", paramTbl[id].paramName, tempBuf);
        break;
    default:
        break;
    }
}

void ParamWriteParamBuf(uint16_t id, void *pBuf)
{
    if (NULL == g_paramTbl || NULL == g_protectedParamTbl) {
        printf("param tb is NULL");
        return ;
    }

    int res = 0;
    UserParam_t *paramTbl;
    int tempIntVal;
    double tempDoubleVal;
    cJSON *paramJson;
    PARAM_PROTECT_TYPE_ENUM protectType = GetParamProtectType(id);

    if (PARAM_USER_TYPE == protectType) {
        paramTbl = g_paramTbl;
        paramJson = g_userParamJson;
    } else if (PARAM_PROTECT_TYPE == protectType) {
        paramTbl = g_protectedParamTbl;
        paramJson = g_protectParamJson;
        id -= 100;
    } else {
        return;
    }

    switch (paramTbl[id].paramType) {
    case PARAM_TYPE_INT:
        sscanf(pBuf, "%d", &tempIntVal);
        res = ParamWriteParam(protectType, (const UserParam_t *)paramTbl, paramJson, id, &tempIntVal);
        break;
    case PARAM_TYPE_DOUBLE:
        sscanf(pBuf, "%lf", &tempDoubleVal);
        res = ParamWriteParam(protectType, (const UserParam_t *)paramTbl, paramJson, id, &tempDoubleVal);
        break;
    case PARAM_TYPE_STRING:
        res = ParamWriteParam(protectType, (const UserParam_t *)paramTbl, paramJson, id, pBuf);
        break;
    default:
        break;
    }

    if (PARAM_WRITE_OK == res) {
        printf("write %s success\n", paramTbl[id].paramName);
    } else {
        printf("write %s failed\n", paramTbl[id].paramName);
    }
}

int ParamReadParamById(uint16_t id, void *pBuf)
{
    if (NULL == g_paramTbl || NULL == g_protectedParamTbl) {
        printf("param tb is NULL");
        return PARAM_OPEN_FILE_ERR;
    }

    UserParam_t *paramTbl;
    cJSON *paramJson;
    PARAM_PROTECT_TYPE_ENUM protectType = GetParamProtectType(id);

    if (PARAM_USER_TYPE == protectType) {
        paramTbl = g_paramTbl;
        paramJson = g_userParamJson;
    } else if (PARAM_PROTECT_TYPE == protectType) {
        paramJson = g_protectParamJson;
        paramTbl = g_protectedParamTbl;
        id -= 100;
    } else {
        return PARAM_READ_ID_INVALID_ERR;
    }

    return ParamReadParam(paramTbl, paramJson, id, pBuf);
}

int ParamWriteParamById(uint16_t id, void *pBuf)
{
    if (NULL == g_paramTbl || NULL == g_protectedParamTbl) {
        printf("param tb is NULL");
        return PARAM_OPEN_FILE_ERR;
    }

    UserParam_t *paramTbl;
    cJSON *paramJson;
    PARAM_PROTECT_TYPE_ENUM protectType = GetParamProtectType(id);

    if (PARAM_USER_TYPE == protectType) {
        paramTbl = g_paramTbl;
        paramJson = g_userParamJson;
    } else if (PARAM_PROTECT_TYPE == protectType) {
        paramTbl = g_protectedParamTbl;
        paramJson = g_protectParamJson;
        id -= 100;
    } else {
        return PARAM_WRITE_ID_INVALID_ERR;
    }

    return ParamWriteParam(protectType, (const UserParam_t *)paramTbl, paramJson, id, pBuf);
}

static int ParamWriteParam(PARAM_PROTECT_TYPE_ENUM protectType, const UserParam_t *paramTbl, cJSON *paramJson,
                           uint16_t id, void *pBuf)
{
    cJSON *item = cJSON_GetObjectItem(paramJson, paramTbl[id].paramName);
    if (item == NULL) {
        printf("find param error\n");
        return PARAM_OPEN_FILE_ERR;
    }

    switch (paramTbl[id].paramType) {
    case PARAM_TYPE_INT:
        cJSON_ReplaceItemInObject(paramJson, paramTbl[id].paramName, cJSON_CreateNumber(*(uint32_t *)pBuf));
        break;
    case PARAM_TYPE_DOUBLE:
        cJSON_ReplaceItemInObject(paramJson, paramTbl[id].paramName, cJSON_CreateNumber(*(double *)pBuf));
        break;
    case PARAM_TYPE_STRING:
        cJSON_ReplaceItemInObject(paramJson, paramTbl[id].paramName, cJSON_CreateString(pBuf));
        break;
    default:
        return PARAM_WRITE_ID_INVALID_ERR;
    }
    ParamSave(protectType);

    return PARAM_WRITE_OK;
}

static int ParamReadParam(const UserParam_t *paramTbl, cJSON *paramJson, uint16_t id, void *pBuf)
{
    cJSON *item = cJSON_GetObjectItem(paramJson, paramTbl[id].paramName);
    if (item == NULL) {
        printf("find param error\n");
        return PARAM_OPEN_FILE_ERR;
    }
    switch (paramTbl[id].paramType) {
    case PARAM_TYPE_INT:
        memcpy(pBuf, &(item->valueint), sizeof(item->valueint));
        break;
    case PARAM_TYPE_DOUBLE:
        memcpy(pBuf, &(item->valuedouble), sizeof(item->valuedouble));
        break;
    case PARAM_TYPE_STRING:
        memcpy(pBuf, item->valuestring, strlen(item->valuestring));
        break;
    default:
        return PARAM_READ_EMPTY_ERR;
    }

    return PARAM_READ_OK;
}

static PARAM_PROTECT_TYPE_ENUM GetParamProtectType(uint16_t id)
{
    if (id > g_paramIdStart && id < g_paramIdMax) {
        return PARAM_USER_TYPE;
    } else if (id > g_protectedIdStart && id < g_protectedIdMax) {
        return PARAM_PROTECT_TYPE;
    } else {
        return PARAM_PROTECT_TYPE_BUTT;
    }
}

static void ParamCheckNewConfig(cJSON *paramJson, PARAM_PROTECT_TYPE_ENUM type)
{
    UserParam_t *paramTbl;
    uint16_t idStart;
    uint16_t idMax;
    bool needSave = false;

    if (PARAM_USER_TYPE == type) {
        paramTbl = g_paramTbl;
        idStart = g_paramIdStart;
        idMax = g_paramIdMax;
    } else if (PARAM_PROTECT_TYPE == type) {
        paramTbl = g_protectedParamTbl;
        idStart = g_protectedIdStart;
        idMax = g_protectedIdMax;
    } else {
        return;
    }

    for (int i = 1; i < idMax - idStart; i++) {
        printf("paramTbl[%d].paramName=%s\r\n", i, paramTbl[i].paramName);
        cJSON *item = cJSON_GetObjectItem(paramJson, paramTbl[i].paramName);
        if (item == NULL) {
            needSave = true;
            if (PARAM_TYPE_INT == paramTbl[i].paramType) {
                cJSON_AddNumberToObject(paramJson, paramTbl[i].paramName, paramTbl[i].val.valInt);
            } else if (PARAM_TYPE_DOUBLE == paramTbl[i].paramType) {
                cJSON_AddNumberToObject(paramJson, paramTbl[i].paramName, paramTbl[i].val.valDouble);
            } else if (PARAM_TYPE_STRING == paramTbl[i].paramType) {
                cJSON_AddStringToObject(paramJson, paramTbl[i].paramName, paramTbl[i].val.valString);
            }
        }
    }

    if (needSave) {
        printf("save...\n");
        ParamSave(type);
    }
}

int ParamInit(const UserParam_t paramTbl[], uint16_t idStart, uint16_t idMax)
{
    uint8_t paramBuf[2048] = {0};
    uint32_t paramSize = 0;
    uint32_t paramAddr = 0;
    cJSON *paramJson = NULL;
    bool needInit = false;

    if (NULL == paramTbl || idStart > idMax) {
        printf("param illegal\n");
        return PARAM_WRITE_ID_INVALID_ERR;
    }

    if (idMax <= NORMAL_PARAM_ID_MAX) {
        g_paramTbl = (UserParam_t *)paramTbl;
        g_paramIdStart = idStart;
        g_paramIdMax = idMax;
        paramAddr = PARAM_NORMAL_ADDR;
    } else {
        g_protectedParamTbl = (UserParam_t *)paramTbl;
        g_protectedIdStart = idStart;
        g_protectedIdMax = idMax;
        paramAddr = PARAM_PROTECT_ADDR;
    }

    Gd25FlashReadBuffer(paramAddr, paramBuf, sizeof(paramSize));
    sscanf((char *)paramBuf, "%04d", &paramSize);

    do {
        if (paramSize == 0xffffffff || paramSize == 0) {
            needInit = true;
            break;
        }
        Gd25FlashReadBuffer(paramAddr + sizeof(paramSize), paramBuf, paramSize);
        if (idMax <= NORMAL_PARAM_ID_MAX) {
            g_userParamJson = cJSON_Parse((const char *)paramBuf);
            paramJson = g_userParamJson;
            if (paramJson == NULL) {
                printf("invalid json\r\n");
                needInit = true;
                break;
            }
            ParamCheckNewConfig(paramJson, PARAM_USER_TYPE);
        } else {
            g_protectParamJson = cJSON_Parse((const char *)paramBuf);
            paramJson = g_protectParamJson;
            if (paramJson == NULL) {
                printf("invalid json\r\n");
                needInit = true;
                break;
            }
            ParamCheckNewConfig(paramJson, PARAM_PROTECT_TYPE);
        }
        printf("param json parse success\n");
    } while (0);
    if (needInit) {
        char* str = NULL;
        paramJson = cJSON_CreateObject();

        for (int i = 1; i < idMax - idStart; i++) {
            if (PARAM_TYPE_INT == paramTbl[i].paramType) {
                cJSON_AddNumberToObject(paramJson, paramTbl[i].paramName, paramTbl[i].val.valInt);
            } else if (PARAM_TYPE_DOUBLE == paramTbl[i].paramType) {
                cJSON_AddNumberToObject(paramJson, paramTbl[i].paramName, paramTbl[i].val.valDouble);
            } else if (PARAM_TYPE_STRING == paramTbl[i].paramType) {
                cJSON_AddStringToObject(paramJson, paramTbl[i].paramName, paramTbl[i].val.valString);
            }
        }
        if (idMax <= NORMAL_PARAM_ID_MAX) {
            g_userParamJson = paramJson;
        } else {
            g_protectParamJson = paramJson;
        }

        str = cJSON_Print(paramJson);
        paramSize = strlen(str);
        RemoveFormatChar(str);
        sprintf((char *)paramBuf, "%4d%s", paramSize, str);
        printf("param = %s\n", paramBuf);

        Gd25FlashSectorErase(paramAddr);
        Gd25FlashWriteBuffer(paramAddr, paramBuf, paramSize + sizeof(paramSize));
    }
    return PARAM_READ_OK;
}

