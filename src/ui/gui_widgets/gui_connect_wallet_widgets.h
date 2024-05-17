#ifndef _GUI_CONNECT_WALLET_WIDGETS_H
#define _GUI_CONNECT_WALLET_WIDGETS_H

#include "gui_views.h"
#include "librust_c.h"
#include "gui_attention_hintbox.h"
#include "gui_ar.h"

typedef enum {
#ifndef BTC_ONLY
    WALLET_LIST_METAMASK,
    WALLET_LIST_OKX,
    WALLET_LIST_ETERNL,
    // WALLET_LIST_YOROI,
    WALLET_LIST_TYPHON,
    WALLET_LIST_BLUE,
    WALLET_LIST_SUB,
    WALLET_LIST_SOLFARE,
    WALLET_LIST_RABBY,
    WALLET_LIST_SAFE,
    WALLET_LIST_SPARROW,
    WALLET_LIST_UNISAT,
    WALLET_LIST_IMTOKEN,
    WALLET_LIST_BLOCK_WALLET,
    WALLET_LIST_ZAPPER,
    WALLET_LIST_YEARN_FINANCE,
    WALLET_LIST_SUSHISWAP,
    WALLET_LIST_KEPLR,
    WALLET_LIST_ARCONNECT,
    WALLET_LIST_FEWCHA,
    WALLET_LIST_PETRA,
    WALLET_LIST_XRP_TOOLKIT,
    WALLET_LIST_TONKEEPER,
#else
    WALLET_LIST_BLUE,
    WALLET_LIST_SPECTER,
    WALLET_LIST_SPARROW,
    WALLET_LIST_NUNCHUK,
    WALLET_LIST_UNISAT,
#endif
    WALLET_LIST_BUTT,
} WALLET_LIST_INDEX_ENUM;

typedef struct {
    WALLET_LIST_INDEX_ENUM index;
    const lv_img_dsc_t *img;
    bool enable;
#ifdef BTC_ONLY
    bool alpha;
#endif
} WalletListItem_t;

#ifndef BTC_ONLY
typedef enum {
    BTC,
    ETH,
    BCH,
    DASH,
    LTC,
    TRON,
    XRP,
    DOT,
    COMPANION_APP_COINS_BUTT,
} COMPANION_APP_COINS_ENUM;

typedef enum {
    APT,
    SUI,
    FEWCHA_COINS_BUTT,
} FEWCHA_COINS_ENUM;

typedef enum SOLAccountType {
    SOLBip44,
    SOLBip44ROOT,
    SOLBip44Change,
} SOLAccountType;

typedef struct {
    int8_t                  index;
    bool                    state;
    lv_obj_t                *checkBox;
    lv_obj_t                *uncheckedImg;
    lv_obj_t                *checkedImg;
} CoinState_t;
#endif

void GuiConnectWalletInit(void);
int8_t GuiConnectWalletNextTile(void);
int8_t GuiConnectWalletPrevTile(void);
void GuiConnectWalletRefresh(void);
void GuiConnectWalletDeInit(void);
void GuiConnectWalletSetQrdata(WALLET_LIST_INDEX_ENUM index);
#ifndef BTC_ONLY
ETHAccountType GetMetamaskAccountType(void);
#endif
void GuiConnectWalletHandleURGenerate(char *data, uint16_t len);
void GuiConnectWalletHandleURUpdate(char *data, uint16_t len);
uint8_t GuiConnectWalletGetWalletIndex(void);
#ifndef BTC_ONLY
SOLAccountType GetSolflareAccountType(void);
void GuiPrepareArConnectWalletView(void);
void GuiSetupArConnectWallet(void);
void GuiConnectWalletPasswordErrorCount(void *param);
void GuiConnectShowRsaSetupasswordHintbox(void);
#endif
#endif /* _GUI_CONNECT_WALLET_WIDGETS_H */

