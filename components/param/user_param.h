/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : user_param.h
 * Description:
 * author     : stone wang
 * data       : 2022-12-23 15:01
**********************************************************************/

#ifndef _USER_PARAM_H
#define _USER_PARAM_H

#include <stddef.h>
#include "stdint.h"
#include "stdbool.h"

#define PARAM_WRITE_OK                  (0)
#define PARAM_WRITE_MALLOC_ERR          (-1)    /* Memory application failed when writing configuration */
#define PARAM_WRITE_QUEUE_FULL_ERR      (-2)    /* Failed when the queue was full when writing the configuration */
#define PARAM_WRITE_ID_INVALID_ERR      (-3)    /* Invalid parameter when writing configuration */
#define PARAM_WRITE_NO_SPACE_ERR        (-4)    /* Insufficient space when writing configuration */
#define PARAM_WRITE_FILE_ERR            (-5)    /* Failed to write configuration file */
#define PARAM_OPEN_FILE_ERR             (-6)    /* fail to open the file */

#define PARAM_READ_OK                   (PARAM_WRITE_OK)
#define PARAM_READ_ID_INVALID_ERR       (-1)    /* Invalid parameter when reading configuration */
#define PARAM_READ_EMPTY_ERR            (-2)    /* There is no record in flash when reading configuration */
#define PARAM_READ_FLASH_ERR            (-3)    /* flash error while reading configuration */
#define PARAM_READ_OPEN_ERR             (PARAM_OPEN_FILE_ERR)
#define PARAM_READ_FILE_ERR             (PARAM_WRITE_FILE_ERR)

#define PARAM_NAME_MAX_LEN             (16)
typedef char PARAM_NAME[PARAM_NAME_MAX_LEN + 1];

typedef enum {
    PARAM_TYPE_INT      = 0,
    PARAM_TYPE_DOUBLE,
    PARAM_TYPE_STRING,
    PARAM_TYPE_STRING_ARRAY,

    PARAM_TYPE_BUTT,
} Param_Type;

typedef enum PARAM_PROTECT_TYPE_ {
    PARAM_USER_TYPE = 0,
    PARAM_PROTECT_TYPE,

    PARAM_PROTECT_TYPE_BUTT,
} PARAM_PROTECT_TYPE_ENUM;

typedef struct USER_PARAM_ {
    uint8_t     paramId;
    Param_Type  paramType;
    PARAM_NAME  paramName;
    union {
        char valString[16];
        double valDouble;
        int valInt;
    } val;
} UserParam_t;

int ParamInit(const UserParam_t paramTbl[], uint16_t idStart, uint16_t idMax);
void ParamPrintf(uint16_t id);
void ParamWriteParamBuf(uint16_t id, void *pBuf);
int ParamWriteParamById(uint16_t id, void *pBuf);
int ParamReadParamById(uint16_t id, void *pBuf);

#define PARAM_NORMAL_TYPE_INIT()        ParamInit(g_normalParamTb, NORMAL_PARAM_ID_START, NORMAL_PARAM_ID_MAX)
#define PARAM_PROTECT_TYPE_INIT()       ParamInit(g_protectedParamTb, PROTECTED_PARAM_ID_START, PROTECTED_PARAM_ID_MAX)
#endif /* _USER_PARAM_H */

