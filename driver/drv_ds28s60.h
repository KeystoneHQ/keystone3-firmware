/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: MAXIM DS28S60
 * Author: leon sun
 * Create: 2022-12-1
 ************************************************************************************************/


#ifndef _DRV_DS28S60_H
#define _DRV_DS28S60_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

//DS28S60 RETURN CODE, err code in err_code.h
#define DS28S60_SUCCESS                             SUCCESS_CODE


//DS28S60 COMMAND
#define DS28S60_CMD_READ_MEM                        0x44
#define DS28S60_CMD_WRITE_MEM                       0x96
#define DS28S60_CMD_READ_PROTECTION                 0xAA
#define DS28S60_CMD_SET_PROTECTION                  0xC3
#define DS28S60_CMD_ENC_READ                        0x4B
#define DS28S60_CMD_AUTH_WRITE                      0xB4
#define DS28S60_CMD_CRPA                            0xA5
#define DS28S60_CMD_CPT_SECRET                      0x3C
#define DS28S60_CMD_GENERATE_KEY                    0xCB
#define DS28S60_CMD_CPT_HASH                        0x33
#define DS28S60_CMD_GENERATE_SIG                    0x34
#define DS28S60_CMD_VERIFY_SIG                      0x59
#define DS28S60_CMD_GEN_KEY_EXCHANGE                0x73
#define DS28S60_CMD_AES_BULK_ENC                    0x91
#define DS28S60_CMD_AES_BULK_DEC                    0x6E
#define DS28S60_CMD_AUTH_ECDSA_CERT                 0xA8
#define DS28S60_CMD_COUNTER                         0xC9
#define DS28S60_CMD_AES_ENC_DEC                     0x22    //PENDING DIFFFERENT VALUE
#define DS28S60_CMD_CMPT_SESSION                    0x37
#define DS28S60_CMD_HMAC                            0xB3
#define DS28S60_CMD_READ_RNG                        0xD2


//Specialty pages
#define PUB_KEY_AX                                  92
#define PUB_KEY_AY                                  93
#define PUB_KEY_BX                                  94
#define PUB_KEY_BY                                  95
#define PUB_KEY_CX                                  96
#define PUB_KEY_CY                                  97
#define PUB_KEY_DX                                  98
#define PUB_KEY_DY                                  99

#define AUTH_KEY_AX                                 100
#define AUTH_KEY_AY                                 101
#define AUTH_KEY_BX                                 102
#define AUTH_KEY_BY                                 103
#define AUTH_KEY_CX                                 104
#define AUTH_KEY_CY                                 105
#define AUTH_KEY_DX                                 106
#define AUTH_KEY_DY                                 107

#define PRIV_KEY_A                                  108
#define PRIV_KEY_B                                  109
#define PRIV_KEY_C                                  110
#define PRIV_KEY_D                                  111

#define SECRET_A_PG                                 112
#define SECRET_B_PG                                 113
#define SECRET_C_PG                                 114
#define SECRET_D_PG                                 115

#define ROM_OPTION_PG                               116

#define PUB_KEY_SAX_PG                              117
#define PUB_KEY_SAY_PG                              118
#define PUB_KEY_SBX_PG                              119
#define PUB_KEY_SBY_PG                              120
#define PUB_KEY_SCX_PG                              121
#define PUB_KEY_SCY_PG                              122
#define PUB_KEY_SDX_PG                              123
#define PUB_KEY_SDY_PG                              124

#define MAX_USER_PAGE                               91

//block
#define MAX_USER_BLOCK                              22

#define SECRET_A_BLOCK                              31
#define SECRET_B_BLOCK                              32
#define SECRET_C_BLOCK                              33
#define SECRET_D_BLOCK                              34


//DS28S60 result byte
#define DS28S60_RESULT_BYTE_CMD_UNEXPECTEDLY        0x22
#define DS28S60_RESULT_BYTE_RNG_FAILURE             0x22
#define DS28S60_RESULT_BYTE_TARGET_PROTECTED        0x55
#define DS28S60_RESULT_BYTE_INVALID_PARA            0x77
#define DS28S60_RESULT_BYTE_SUCCESS                 0xAA

#define DS28S60_INFO_VALID                          0xA5


#define DS28S60_SECRET_KEY_A                        0x04
#define DS28S60_SECRET_KEY_B                        0x05
#define DS28S60_SECRET_KEY_C                        0x06
#define DS28S60_SECRET_KEY_D                        0x07

typedef struct {
    uint8_t ROMID[8];
    uint8_t MANID[2];
    uint8_t valid;
} DS28S60_Info_t;


typedef struct {
    union {
        uint8_t byte;
        struct {
            uint8_t ID : 3;
            uint8_t RFU : 4;
            uint8_t LOCK : 1;
        } b;
    } Secret;
    union {
        uint8_t byte;
        struct {
            uint8_t RP : 1;
            uint8_t WP : 1;
            uint8_t APH : 1;
            uint8_t EPH : 1;
            uint8_t RFU : 2;
            uint8_t ECH : 1;
            uint8_t ECW : 1;
        } b;
    } Prot;
} DS28S60_BlockProtection_t;


void DS28S60_Init(void);
void DS28S60_Open(void);

int32_t DS28S60_ReadPage(uint8_t *data, uint8_t page);
int32_t DS28S60_WritePage(uint8_t *data, uint8_t page);
int32_t DS28S60_GetRng(uint8_t *rngArray, uint32_t num);

int32_t DS28S60_HmacAuthentication(uint8_t page);

int32_t DS28S60_HmacEncryptRead(uint8_t *data, uint8_t page);
int32_t DS28S60_HmacEncryptWrite(const uint8_t *data, uint8_t page);

void DS28S60_Test(int argc, char *argv[]);

#endif
