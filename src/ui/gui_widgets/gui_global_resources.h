#ifndef _GUI_GLOBAL_RESROUCES_H_
#define _GUI_GLOBAL_RESROUCES_H_

#include "stdint.h"
#include "stdlib.h"

typedef enum {
    ETH_STANDARD,
    ETH_LEDGER_LIVE,
    ETH_LEDGER_LEGACY,
} ETH_PATH_TYPE;

typedef enum {
    SOL_SOLFLARE,
    SOL_SOLLET,
    SOL_PHANTOM,
} SOL_PATH_TYPE;

typedef enum {
    BTC_NATIVE_SEGWIT,
    BTC_NESTED_SEGWIT,
    BTC_LEGACY,
} BTC_PATH_TYPE;

typedef enum {
    NEAR_STANDARD,
    NEAR_LEDGER_LIVE,
} NEAR_PATH_TYPE;

typedef enum {
    ETH_DERIVATION_PATH_DESC,
    SOL_DERIVATION_PATH_DESC,
    BTC_DERIVATION_PATH_DESC,
    NEAR_DERIVATION_PATH_DESC,
} DERIVATION_PATH_DESC_INDEX;

char **GetDerivationPathDescs(uint8_t index);
void GlobalResourcesInit(void);

#endif