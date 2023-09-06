/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Account public info.
 * Author: leon sun
 * Create: 2023-5-4
 ************************************************************************************************/


#ifndef _ACCOUNT_PUBLIC_INFO_H
#define _ACCOUNT_PUBLIC_INFO_H

#include "stdint.h"
#include "stdbool.h"
typedef struct {
    int32_t addressType;
    int32_t addressIndex;
} AccountSettingsItem_t;

typedef enum {
    XPUB_TYPE_BTC,
    XPUB_TYPE_BTC_LEGACY,
    XPUB_TYPE_BTC_NATIVE_SEGWIT,
    XPUB_TYPE_LTC,
    XPUB_TYPE_DASH,
    XPUB_TYPE_BCH,
    XPUB_TYPE_ETH_BIP44_STANDARD,
    XPUB_TYPE_ETH_LEDGER_LEGACY,
    XPUB_TYPE_ETH_LEDGER_LIVE_0,
    XPUB_TYPE_ETH_LEDGER_LIVE_1,
    XPUB_TYPE_ETH_LEDGER_LIVE_2,
    XPUB_TYPE_ETH_LEDGER_LIVE_3,
    XPUB_TYPE_ETH_LEDGER_LIVE_4,
    XPUB_TYPE_ETH_LEDGER_LIVE_5,
    XPUB_TYPE_ETH_LEDGER_LIVE_6,
    XPUB_TYPE_ETH_LEDGER_LIVE_7,
    XPUB_TYPE_ETH_LEDGER_LIVE_8,
    XPUB_TYPE_ETH_LEDGER_LIVE_9,
    XPUB_TYPE_TRX,
    XPUB_TYPE_COSMOS,
    XPUB_TYPE_SCRT,
    XPUB_TYPE_CRO,
    XPUB_TYPE_IOV,
    XPUB_TYPE_BLD,
    XPUB_TYPE_KAVA,
    XPUB_TYPE_TERRA,

    XPUB_TYPE_NUM,
} ChainType;

int32_t AccountPublicInfoSwitch(uint8_t accountIndex, const char *password, bool newKey);
int32_t TempAccountPublicInfo(uint8_t accountIndex, const char *password, bool set);
void DeleteAccountPublicInfo(uint8_t accountIndex);
char *GetCurrentAccountPublicKey(ChainType chain);
uint8_t SpecifiedXPubExist(const char *xPub);
void AccountPublicInfoTest(int argc, char *argv[]);
bool GetFirstReceive(const char* chainName);
void SetFirstReceive(const char* chainName, bool isFirst);

#endif

