#ifndef _GUI_CHAIN_H
#define _GUI_CHAIN_H

#include "gui_animating_qrcode.h"
#include "gui_btc.h"
#ifndef BTC_ONLY
#ifdef WEB3_VERSION
#include "gui_eth.h"
#include "gui_trx.h"
#include "gui_cosmos.h"
#include "gui_sui.h"
#include "gui_sol.h"
#include "gui_aptos.h"
#include "gui_ada.h"
#include "gui_xrp.h"
#include "gui_ar.h"
#include "gui_stellar.h"
#include "gui_ton.h"
#include "gui_avax.h"
#else
#include "gui_zcash.h"
#include "gui_monero.h"
#include "gui_ergo.h"
#endif
#endif

typedef void (*SetChainDataFunc)(void *resultData, void *multiResultData, bool multi);

// Enumeration for displaying in the middle of the status bar
typedef enum {
    CHAIN_BTC,
#ifdef WEB3_VERSION
    CHAIN_ETH,
    CHAIN_SOL,
    CHAIN_BNB,
    CHAIN_HNT,
    CHAIN_XRP,
    CHAIN_ADA,
    CHAIN_TON,
    CHAIN_DOT,
    CHAIN_TRX,
    CHAIN_LTC,
    CHAIN_DOGE,
    CHAIN_AVAX,
    CHAIN_BCH,
    CHAIN_APT,
    CHAIN_SUI,
    CHAIN_DASH,
    CHAIN_ARWEAVE,
    CHAIN_STELLAR,
    // cosmos start
    CHAIN_BABYLON,
    CHAIN_NEUTARO,
    CHAIN_TIA,
    CHAIN_NTRN,
    CHAIN_DYM,
    CHAIN_OSMO,
    CHAIN_INJ,
    CHAIN_ATOM,
    CHAIN_CRO,
    CHAIN_RUNE,
    CHAIN_KAVA,
    CHAIN_LUNC,
    CHAIN_AXL,
    CHAIN_LUNA,
    CHAIN_AKT,
    CHAIN_STRD,
    CHAIN_SCRT,
    CHAIN_BLD,
    CHAIN_CTK,
    CHAIN_EVMOS,
    CHAIN_STARS,
    CHAIN_XPRT,
    CHAIN_SOMM,
    CHAIN_JUNO,
    CHAIN_IRIS,
    CHAIN_DVPN,
    CHAIN_ROWAN,
    CHAIN_REGEN,
    CHAIN_BOOT,
    CHAIN_GRAV,
    CHAIN_IXO,
    CHAIN_NGM,
    CHAIN_IOV,
    CHAIN_UMEE,
    CHAIN_QCK,
    CHAIN_TGD,
    // cosmos end
#endif

#ifdef CYPHERPUNK_VERSION
    CHAIN_ZCASH,
    CHAIN_XMR,
    CHAIN_ERG,
#endif
    CHAIN_BUTT,
} GuiChainCoinType;

// Enumeration of pages used for transaction resolution
typedef enum {
    REMAPVIEW_BTC,
    REMAPVIEW_BTC_MESSAGE,
#ifdef WEB3_VERSION
    REMAPVIEW_ETH,
    REMAPVIEW_ETH_PERSONAL_MESSAGE,
    REMAPVIEW_ETH_TYPEDDATA,
    REMAPVIEW_TRX,
    REMAPVIEW_COSMOS,
    REMAPVIEW_SUI,
    REMAPVIEW_SUI_SIGN_MESSAGE_HASH,
    REMAPVIEW_SOL,
    REMAPVIEW_SOL_MESSAGE,
    REMAPVIEW_APT,
    REMAPVIEW_ADA,
    REMAPVIEW_ADA_SIGN_TX_HASH,
    REMAPVIEW_ADA_SIGN_DATA,
    REMAPVIEW_ADA_CATALYST,
    REMAPVIEW_XRP,
    REMAPVIEW_AR,
    REMAPVIEW_AR_MESSAGE,
    REMAPVIEW_AR_DATAITEM,
    REMAPVIEW_STELLAR,
    REMAPVIEW_STELLAR_HASH,
    REMAPVIEW_TON,
    REMAPVIEW_TON_SIGNPROOF,
    REMAPVIEW_AVAX,
#endif

#ifdef CYPHERPUNK_VERSION
    REMAPVIEW_ZCASH,
    REMAPVIEW_XMR_OUTPUT,
    REMAPVIEW_XMR_UNSIGNED,
    REMAPVIEW_ERGO,
#endif
    REMAPVIEW_WEB_AUTH,
    REMAPVIEW_BUTT,
} GuiRemapViewType;

typedef struct {
    uint16_t chain;
    SetChainDataFunc func;
} SetChainData_t;

#define CHECK_CHAIN_BREAK(result)                                       \
    if (result->error_code != 0) {                                      \
        printf("result->code = %d\n", result->error_code);              \
        printf("result->error message = %s\n", result->error_message);  \
        break;  \
    }

#define CHECK_CHAIN_RETURN(result)                                      \
    if (result->error_code != 0) {                                      \
        printf("result->code = %d\n", result->error_code);              \
        printf("result->error message = %s\n", result->error_message);  \
        return NULL;  \
    }

#define CHECK_CHAIN_PRINT(result)                                       \
    if (result->error_code != 0) {                                      \
        printf("result->code = %d\n", result->error_code);              \
        printf("result->error message = %s\n", result->error_message);  \
    }

#define CHECK_FREE_UR_RESULT(result, multi)                             \
    if (result != NULL) {                                               \
        if (multi) {                                                    \
            free_ur_parse_multi_result((PtrT_URParseMultiResult)result);\
        } else {                                                        \
            free_ur_parse_result((PtrT_URParseResult)result);           \
        }                                                               \
        result = NULL;                                                  \
    }

GuiRemapViewType ViewTypeReMap(uint8_t viewType);
GuiChainCoinType ViewTypeToChainTypeSwitch(uint8_t viewType);
PtrT_TransactionCheckResult CheckUrResult(uint8_t viewType);
GenerateUR GetUrGenerator(ViewType viewType);
GenerateUR GetSingleUrGenerator(ViewType viewType);
bool CheckViewTypeIsAllow(uint8_t viewType);
#ifndef BTC_ONLY
bool IsMessageType(uint8_t type);
bool isTonSignProof(uint8_t type);
bool isCatalystVotingRegistration(uint8_t type);
bool CheckViewTypeIsAllow(uint8_t viewType);
#endif
#endif
