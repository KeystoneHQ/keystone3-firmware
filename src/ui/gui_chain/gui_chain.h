#ifndef _GUI_CHAIN_H
#define _GUI_CHAIN_H

#include "gui_animating_qrcode.h"
#include "gui_btc.h"
#ifndef BTC_ONLY
#include "gui_eth.h"
#include "gui_trx.h"
#include "gui_cosmos.h"
#include "gui_sui.h"
#include "gui_sol.h"
#include "gui_aptos.h"
#include "gui_ada.h"
#include "gui_xrp.h"
#include "gui_ar.h"
#endif

typedef void (*SetChainDataFunc)(void *resultData, void *multiResultData, bool multi);

// Enumeration for displaying in the middle of the status bar
typedef enum {
    CHAIN_BTC,
#ifndef BTC_ONLY
    CHAIN_ETH,
    CHAIN_SOL,
    CHAIN_BNB,
    CHAIN_XRP,
    CHAIN_ADA,
    CHAIN_DOT,
    CHAIN_TRX,
    CHAIN_LTC,
    CHAIN_BCH,
    CHAIN_APT,
    CHAIN_SUI,
    CHAIN_DASH,
    CHAIN_ARWEAVE,
    CHAIN_COSMOS,
    CHAIN_TIA,
    CHAIN_DYM,
    CHAIN_OSMO,
    CHAIN_INJ,
    CHAIN_ATOM,
    CHAIN_CRO,
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
    CHAIN_TON,
#endif
    CHAIN_BUTT,
} GuiChainCoinType;

// Enumeration of pages used for transaction resolution
typedef enum {
    REMAPVIEW_BTC,
    REMAPVIEW_BTC_MESSAGE,
#ifndef BTC_ONLY
    REMAPVIEW_ETH,
    REMAPVIEW_ETH_PERSONAL_MESSAGE,
    REMAPVIEW_ETH_TYPEDDATA,
    REMAPVIEW_TRX,
    REMAPVIEW_COSMOS,
    REMAPVIEW_SUI,
    REMAPVIEW_SOL,
    REMAPVIEW_SOL_MESSAGE,
    REMAPVIEW_APT,
    REMAPVIEW_ADA,
    REMAPVIEW_XRP,
    REMAPVIEW_AR,
    REMAPVIEW_AR_MESSAGE,
    REMAPVIEW_WEB_AUTH,
#endif
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
#ifndef BTC_ONLY
bool IsMessageType(uint8_t type);
#endif
#endif
