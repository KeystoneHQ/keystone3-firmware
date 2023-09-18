/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2025. All rights reserved.
 * Description: pillar error code.
 * Author: leon sun
 * Create: 2023-1-5
 ************************************************************************************************/

//由于整数经常需要表达返回长度等结果，所以ERR CODE均定义为负数，每个模块预留100个ERR CODE

#ifndef _ERR_CODE_H
#define _ERR_CODE_H

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    SUCCESS_CODE = 0,
    ERR_GENERAL_FAIL,

    ERR_DS28S60_OVERTIME,
    ERR_DS28S60_UNEXPECTLEN,
    ERR_DS28S60_INFO,
    ERR_DS28S60_AUTH,
    ERR_DS28S60_BIND,

    ERR_BYTEWORDS_LEN,
    ERR_BYTEWORDS_WORD,
    ERR_BYTEWORDS_CRC,

    ERR_URDECODE_HEAD,
    ERR_URDECODE_END,
    ERR_URDECODE_LEN,

    ERR_TOUCHPAD_NODATA,
    ERR_TOUCHPAD_NOREG,

    ERR_I2CIO_NOACK,

    ERR_ATECC608B_UNEXPECT_LOCK,
    ERR_ATECC608B_UNEXPECT_UNLOCK,
    ERR_ATECC608B_SLOT_NUM_ERR,
    ERR_ATECC608B_BIND,

    ERR_KEYSTORE_AUTH,
    ERR_KEYSTORE_PASSWORD_ERR,
    ERR_KEYSTORE_REPEAT_PASSWORD,
    ERR_KEYSTORE_NOT_LOGIN,
    ERR_KEYSTORE_MNEMONIC_REPEAT,
    ERR_KEYSTORE_SAVE_LOW_POWER,
    ERR_KEYSTORE_MNEMONIC_INVALID,
    ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET,
    ERR_KEYSTORE_EXTEND_PUBLIC_KEY_NOT_MATCH,

    ERR_GD25_BAD_PARAM,
    ERR_GD25_WEL_FAILED,

    ERR_PSRAM_OPERATE_FAILED,

    ERR_GUI_ERROR,
    ERR_GUI_UNHANDLED,

    ERR_SQLITE3_FAILED,

    ERR_SLIP39_DECODE_FAILED,

    ERR_SERIAL_NUMBER_NOT_EXIST,
    ERR_SERIAL_NUMBER_INVALID,
    ERR_SERIAL_NUMBER_ALREADY_EXIST,

    ERR_WEB_AUTH_KEY_NOT_EXIST,
    ERR_WEB_AUTH_KEY_ALREADY_EXIST,

    ERR_UPDATE_PUB_KEY_NOT_EXIST,
    ERR_UPDATE_PUB_KEY_NO_SPACE,

    ERR_UPDATE_FIRMWARE_NOT_DETECTED,
    ERR_END,
} Error_Code;

typedef struct  {
    int32_t errCode;
    const char *errDesc;
} ErrCodeDesc_t;

const char *GetErrorMessage(Error_Code errCode);

#define CHECK_ERRCODE_BREAK(content, ret)       {if (ret != SUCCESS_CODE) {printf("%s err,0x%X,line=%d\r\n", content, ret, __LINE__); break; }}
#define CHECK_ERRCODE_RETURN(ret)               {if (ret != SUCCESS_CODE) {printf("%s err,%s,line=%d\r\n", __func__, GetErrorMessage(ret), __LINE__); return; }}
#define CHECK_ERRCODE_RETURN_INT(ret)           {if (ret != SUCCESS_CODE) {printf("%s err,%s,line=%d\r\n", __func__, GetErrorMessage(ret), __LINE__); return ret; }}
#define PRINT_ERRCODE(ret)                      {if (ret != SUCCESS_CODE) {printf("%s err,%s,line=%d\r\n", __func__, GetErrorMessage(ret), __LINE__); }}

#endif


