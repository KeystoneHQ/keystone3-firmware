#ifndef _SE_INTERFACE_H_
#define _SE_INTERFACE_H_

#include "stdint.h"
#include "stdlib.h"

int32_t SE_EncryptWrite(uint8_t slot, uint8_t block, const uint8_t *data);
int32_t SE_Kdf(uint8_t slot, const uint8_t *authKey, const uint8_t *inData, uint32_t inLen, uint8_t *outData);
int32_t SE_DeriveKey(uint8_t slot, const uint8_t *authKey);
int32_t SE_HmacEncryptRead(uint8_t *data, uint8_t page);
int32_t SE_HmacEncryptWrite(const uint8_t *data, uint8_t page);
int32_t SE_GetDS28S60Rng(uint8_t *rngArray, uint32_t num);
void SE_GetTRng(void *buf, uint32_t len);
int32_t SE_GetAtecc608bRng(uint8_t *rngArray, uint32_t num);


#endif