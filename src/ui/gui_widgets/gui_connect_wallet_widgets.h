#ifndef _GUI_CONNECT_WALLET_WIDGETS_H
#define _GUI_CONNECT_WALLET_WIDGETS_H

#include "gui_views.h"
#include "librust_c.h"
#include "gui_attention_hintbox.h"
#ifdef WEB3_VERSION
#include "gui_ar.h"
#endif

typedef enum {
#ifndef BTC_ONLY
    WALLET_LIST_KEYSTONE,
    WALLET_LIST_METAMASK,
    WALLET_LIST_OKX,
    WALLET_LIST_XAMAN,
    WALLET_LIST_ETERNL,
    WALLET_LIST_MEDUSA,
    // WALLET_LIST_YOROI,
    WALLET_LIST_TYPHON,
    WALLET_LIST_BLUE,
    WALLET_LIST_ZEUS,
    WALLET_LIST_BABYLON,
    WALLET_LIST_SUB,
    WALLET_LIST_ZASHI,
    WALLET_LIST_SOLFARE,
    WALLET_LIST_NUFI,
    WALLET_LIST_BACKPACK,
    WALLET_LIST_RABBY,
    WALLET_LIST_BITGET,
    WALLET_LIST_SAFE,
    WALLET_LIST_SPARROW,
    WALLET_LIST_UNISAT,
    WALLET_LIST_IMTOKEN,
    WALLET_LIST_CORE,
    WALLET_LIST_BLOCK_WALLET,
    WALLET_LIST_ZAPPER,
    WALLET_LIST_HELIUM,
    WALLET_LIST_IOTA,
    WALLET_LIST_YEARN_FINANCE,
    WALLET_LIST_SUSHISWAP,
    WALLET_LIST_KEPLR,
    WALLET_LIST_MINT_SCAN,
    WALLET_LIST_WANDER,
    WALLET_LIST_BEACON,
    WALLET_LIST_VESPR,
    WALLET_LIST_XBULL,
    WALLET_LIST_FEWCHA,
    WALLET_LIST_PETRA,
    WALLET_LIST_XRP_TOOLKIT,
    WALLET_LIST_THORWALLET,
    WALLET_LIST_TONKEEPER,
    WALLET_LIST_BEGIN,
    WALLET_LIST_LEAP,
    WALLET_LIST_NIGHTLY,
    WALLET_LIST_SUIET,
    WALLET_LIST_CAKE,
    WALLET_LIST_FEATHER,
#else
    WALLET_LIST_BLUE,
    WALLET_LIST_SPECTER,
    WALLET_LIST_SPARROW,
    WALLET_LIST_NUNCHUK,
    WALLET_LIST_ZEUS,
    WALLET_LIST_BABYLON,
    WALLET_LIST_BITCOIN_SAFE,
    WALLET_LIST_UNISAT,
#endif
    WALLET_LIST_BUTT,
} WALLET_LIST_INDEX_ENUM;

typedef enum {
    WALLET_FILTER_BTC = 0b1,
    WALLET_FILTER_ETH = 0b10,
    WALLET_FILTER_SOL = 0b100,
    WALLET_FILTER_ADA = 0b1000,
    WALLET_FILTER_OTHER = 0b10000,

    WALLET_FILTER_ALL = 0b11111111,

    WALLET_FILTER_BUTT = 0,
} WalletFilter_t;

typedef struct {
    WALLET_LIST_INDEX_ENUM index;
    const lv_img_dsc_t *img;
    bool enable;
#ifdef BTC_ONLY
    bool alpha;
#else
    uint8_t filter;
#endif
} WalletListItem_t;

void GuiConnectWalletInit(void);
int8_t GuiConnectWalletNextTile(void);
int8_t GuiConnectWalletPrevTile(void);
void GuiConnectWalletRefresh(void);
void GuiConnectWalletDeInit(void);
void GuiConnectWalletSetQrdata(WALLET_LIST_INDEX_ENUM index);
void GuiConnectWalletHandleURGenerate(char *data, uint16_t len);
void GuiConnectWalletHandleURUpdate(char *data, uint16_t len);
uint8_t GuiConnectWalletGetWalletIndex(void);
#ifdef WEB3_VERSION
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

typedef enum {
    AVAX,
    AVAX_ETH,
    AVAX_BTC,
    CORE_COINS_BUTT,
} CORE_COINS_ENUM;

typedef enum SOLAccountType {
    SOLBip44,
    SOLBip44ROOT,
    SOLBip44Change,
} SOLAccountType;

ETHAccountType GetMetamaskAccountType(void);
SOLAccountType GetSolflareAccountType(void);
SOLAccountType GetHeliumAccountType(void);
void GuiPrepareArConnectWalletView(void);
void GuiSetupArConnectWallet(void);
void GuiConnectWalletPasswordErrorCount(void *param);
void GuiConnectShowRsaSetupasswordHintbox(void);
#endif
#endif /* _GUI_CONNECT_WALLET_WIDGETS_H */

