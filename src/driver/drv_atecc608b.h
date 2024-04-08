#ifndef _DRV_ATECC608B_H
#define _DRV_ATECC608B_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

//shared keys
#define SLOT_IO_PROTECT_KEY             0
#define SLOT_AUTH_KEY                   1
#define SLOT_ENCRYPT_KEY                2

//individual keys
#define SLOT_AUTH_KEY_1                 3
#define SLOT_ROLL_KDF_KEY_1             4
#define SLOT_HOST_KDF_KEY_1             5

#define SLOT_AUTH_KEY_2                 6
#define SLOT_ROLL_KDF_KEY_2             7
#define SLOT_HOST_KDF_KEY_2             9

#define SLOT_AUTH_KEY_3                 10
#define SLOT_ROLL_KDF_KEY_3             11
#define SLOT_HOST_KDF_KEY_3             12
#define SLOT_DEVICE_KEY                 14
#define SLOT_DEVICE_TAMPER_FLAG         15

#pragma pack(1)
typedef struct __Atecc608bConfig_t {
    uint8_t sn1[4];
    uint8_t revNum[4];
    uint8_t sn2[5];
    uint8_t aesEnable;
    uint8_t i2cEnable; // i2c swi
    uint8_t reserved0;
    uint8_t i2cAddress; // gpio control for swi
    uint8_t reserved1;
    uint8_t countMatch;
    uint8_t chipMode;
    uint16_t slotConfig[16];
    uint8_t counter0[8];
    uint8_t counter1[8];
    uint8_t useLock;
    uint8_t volatileKeyPermission;
    uint16_t secureBoot;
    uint8_t kdfivLoc;
    uint16_t kdfivStr;
    uint8_t reserved2[9];
    uint8_t userExtra;
    uint8_t userExtraAdd;
    uint8_t lockValue;
    uint8_t lockConfig;
    uint16_t slotLocked;
    uint16_t chipOptions;
    uint8_t x509Format[4];
    uint16_t keyConfig[16];
} Atecc608bConfig_t;
#pragma pack()

void Atecc608bInit(void);
int32_t Atecc608bGetRng(uint8_t *rngArray, uint32_t num);
int32_t Atecc608bEncryptWrite(uint8_t slot, uint8_t block, const uint8_t *data);
int32_t Atecc608bEncryptRead(uint8_t slot, uint8_t block, uint8_t *data);
int32_t Atecc608bKdf(uint8_t slot, const uint8_t *authKey, const uint8_t *inData, uint32_t inLen, uint8_t *outData);
int32_t Atecc608bDeriveKey(uint8_t slot, const uint8_t *authKey);
int32_t Atecc608bGenDevicePubkey(uint8_t* pubkey);
int32_t Atecc608bSignMessageWithDeviceKey(uint8_t *messageHash, uint8_t *signature);
void Atecc608bTest(int argc, char *argv[]);

#endif
