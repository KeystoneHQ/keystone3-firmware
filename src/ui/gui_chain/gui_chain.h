#ifndef _GUI_CHAIN_H
#define _GUI_CHAIN_H

#include "gui_btc.h"
#include "gui_eth.h"
#include "gui_trx.h"
#include "gui_cosmos.h"
#include "gui_sui.h"
#include "gui_sol.h"
#include "gui_aptos.h"
#include "gui_ada.h"
#include "gui_xrp.h"

#define LABEL_MAX_BUFF_LEN                                      (512)

typedef void (*SetChainDataFunc)(void *indata, bool multi);

// Enumeration for displaying in the middle of the status bar
typedef enum {
    CHAIN_BTC,
    CHAIN_ETH,
    CHAIN_SOL,
    CHAIN_APT,
    CHAIN_SUI,
    CHAIN_ADA,
    CHAIN_XRP,
    CHAIN_TRX,
    CHAIN_BCH,
    CHAIN_DASH,
    CHAIN_LTC,
    CHAIN_ATOM,
    CHAIN_OSMO,
    CHAIN_SCRT,
    CHAIN_AKT,
    CHAIN_CRO,
    CHAIN_IOV,
    CHAIN_ROWAN,
    CHAIN_CTK,
    CHAIN_IRIS,
    CHAIN_REGEN,
    CHAIN_XPRT,
    CHAIN_DVPN,
    CHAIN_IXO,
    CHAIN_NGM,
    CHAIN_BLD,
    CHAIN_BOOT,
    CHAIN_JUNO,
    CHAIN_STARS,
    CHAIN_AXL,
    CHAIN_SOMM,
    CHAIN_UMEE,
    CHAIN_GRAV,
    CHAIN_TGD,
    CHAIN_STRD,
    CHAIN_EVMOS,
    CHAIN_INJ,
    CHAIN_KAVA,
    CHAIN_QCK,
    CHAIN_LUNA,
    CHAIN_LUNC,
    CHAIN_NEAR,
    CHAIN_BNB,
    CHAIN_DOT,

    CHAIN_BUTT,
} GuiChainCoinType;

// Enumeration of pages used for transaction resolution
typedef enum {
    REMAPVIEW_BTC,
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
            free_ur_parse_multi_result(result);                         \
        } else {                                                        \
            free_ur_parse_result(result);                               \
        }                                                               \
        result = NULL;                                                  \
    }

GuiRemapViewType ViewTypeReMap(uint8_t viewType);
GuiChainCoinType ViewTypeToChainTypeSwitch(uint8_t viewType);
PtrT_TransactionCheckResult CheckUrResult(uint8_t viewType);
GenerateUR GetUrGenerator(GuiChainCoinType viewType);
GenerateUR GetSingleUrGenerator(GuiChainCoinType viewType);
bool IsMessageType(uint8_t type);
#endif
