/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Seeds are stored in ATECC608B and DS28S60.
 * Author: leon sun
 * Create: 2023-3-7
 ************************************************************************************************/


#ifndef _KEYSTORE_H
#define _KEYSTORE_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

#define ACCOUNT_INDEX_LOGOUT                    255

int32_t GenerateEntropy(uint8_t *entropy, uint8_t entropyLen, const char *password);
int32_t SaveNewEntropy(uint8_t accountIndex, const uint8_t *entropy, uint8_t entropyLen, const char *password);
int32_t SaveNewSlip39Entropy(uint8_t accountIndex, const uint8_t *ems, const uint8_t *entropy, uint8_t entropyLen, const char *password, uint16_t id, uint8_t ie);
int32_t GetAccountEntropy(uint8_t accountIndex, uint8_t *entropy, uint8_t *entropyLen, const char *password);
int32_t GetAccountSeed(uint8_t accountIndex, uint8_t *seed, const char *password);
int32_t GetAccountSlip39Ems(uint8_t accountIndex, uint8_t *slip39Ems, const char *password);
int32_t ChangePassword(uint8_t accountIndex, const char *newPassword, const char *password);
int32_t VerifyPassword(uint8_t *accountIndex, const char *password);

bool CheckPassphraseSame(uint8_t accountIndex, const char *passphrase);
char* GetPassphrase(uint8_t accountIndex);

int32_t CheckPasswordExisted(const char *password, uint8_t excludeIndex);


int32_t SetPassphrase(uint8_t accountIndex, const char *passphrase, const char *password);
void ClearAccountPassphrase(uint8_t accountIndex);
void GetMasterFingerPrint(uint8_t *mfp);

bool PassphraseExist(uint8_t accountIndex);

#ifndef BUILD_PRODUCTION
void KeyStoreTest(int argc, char *argv[]);
#endif

#endif
