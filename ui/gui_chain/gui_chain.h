#ifndef _GUI_CHAIN_H
#define _GUI_CHAIN_H

#include "gui_btc.h"
#include "gui_eth.h"
#include "gui_trx.h"

#define LABEL_MAX_BUFF_LEN                                      (512)

typedef void (*SetChainDataFunc)(void *indata, bool multi);

// Enumeration for displaying in the middle of the status bar
typedef enum {
    CHAIN_BTC,
    CHAIN_ETH,
    CHAIN_BNB,
    CHAIN_SOL,
    CHAIN_DOT,
    CHAIN_XRP,
    CHAIN_LTC,
    CHAIN_DASH,
    CHAIN_BCH,
    CHAIN_TRX,

    CHAIN_BUTT,
} GuiChainCoinType;

// Enumeration of pages used for transaction resolution
typedef enum {
    REMAPVIEW_BTC,
    REMAPVIEW_ETH,
    REMAPVIEW_ETH_PERSONAL_MESSAGE,
    REMAPVIEW_ETH_TYPEDDATA,
    REMAPVIEW_TRX,
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

#endif