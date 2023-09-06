/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Presetting infomation.
 * Author: leon sun
 * Create: 2023-6-30
 ************************************************************************************************/


#ifndef _PRESETTING_H
#define _PRESETTING_H

#include "stdint.h"
#include "stdbool.h"

#define SERIAL_NUMBER_MAX_LEN               64
#define WEB_AUTH_RSA_KEY_LEN                1024
#define UPDATE_PUB_KEY_LEN                  64

int32_t GetSerialNumber(char *serialNumber);
int32_t SetSerialNumber(const char *serialNumber);

int32_t GetWebAuthRsaKey(uint8_t *key);
int32_t SetWebAuthRsaKey(const uint8_t *key);

int32_t GetUpdatePubKey(uint8_t *pubKey);
int32_t SetUpdatePubKey(const uint8_t *pubKey);

bool GetFactoryResult(void);

void PresettingTest(int argc, char *argv[]);


#endif
