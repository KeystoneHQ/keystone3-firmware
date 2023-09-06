/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Secret data ram cache.
 * Author: leon sun
 * Create: 2023-3-22
 ************************************************************************************************/

#ifndef _SECRET_CACHE_H
#define _SECRET_CACHE_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"


void SecretCacheSetPassword(char *password);
char *SecretCacheGetPassword(void);

void SecretCacheSetNewPassword(char *password);
char *SecretCacheGetNewPassword(void);

void SecretCacheSetEntropy(uint8_t *entropy, uint32_t len);
uint8_t *SecretCacheGetEntropy(uint32_t *len);

void SecretCacheSetMnemonic(char *mnemonic);
char *SecretCacheGetMnemonic(void);

char *SecretCacheGetSlip39Mnemonic(int index);
void SecretCacheSetSlip39Mnemonic(char *mnemonic, int index);

void SecretCacheSetPassphrase(char *passPhrase);
char *SecretCacheGetPassphrase(void);

void SecretCacheSetIteration(uint8_t ie);
void SecretCacheSetIdentifier(uint16_t id);
uint16_t SecretCacheGetIdentifier(void);
uint8_t SecretCacheGetIteration(void);

uint8_t *SecretCacheGetEms(uint32_t *len);
void SecretCacheSetEms(uint8_t *ems, uint32_t len);


void ClearSecretCache(void);

#endif
