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
    AVAX_C_CHAIN,
    AVAX_X_P_CHAIN,
} AVAX_CHAIN_TYPE;

typedef enum {
    SOL_SOLFLARE,
    SOL_SOLLET,
    SOL_PHANTOM,
} SOL_PATH_TYPE;

typedef enum {
    ADA_STANDARD,
    ADA_LEDGER,
} ADA_PATH_TYPE;

typedef enum {
    BTC_NATIVE_SEGWIT,
    BTC_TAPROOT,
    BTC_NESTED_SEGWIT,
    BTC_LEGACY,
} BTC_PATH_TYPE;

typedef enum {
    LTC_NESTED_SEGWIT,
    LTC_NATIVE_SEGWIT,
} LTC_PATH_TYPE;

typedef enum {
    ETH_DERIVATION_PATH_DESC,
    SOL_DERIVATION_PATH_DESC,
    BTC_DERIVATION_PATH_DESC,
    ADA_DERIVATION_PATH_DESC,
    AVAX_DERIVATION_PATH_DESC,
    LTC_DERIVATION_PATH_DESC,
#ifdef BTC_ONLY
    BTC_TEST_NET_DERIVATION_PATH_DESC,
#endif
} DERIVATION_PATH_DESC_INDEX;

char **GetDerivationPathDescs(uint8_t index);
void GlobalResourcesInit(void);

#endif