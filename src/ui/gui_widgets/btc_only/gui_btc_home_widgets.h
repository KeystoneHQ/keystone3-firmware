#ifdef BTC_ONLY
#ifndef _GUI_BTC_HOME_WIDGETS_H
#define _GUI_BTC_HOME_WIDGETS_H

#include "gui_model.h"
#include "lvgl.h"

typedef enum {
    HOME_WALLET_CARD_BTC,
    HOME_WALLET_CARD_BTC_MULTISIG,

    HOME_WALLET_CARD_BUTT,      // This represents the end of the array (the number of arrays) and needs to be placed at the end.
} HOME_WALLET_CARD_ENUM;

typedef enum {
    MULTI_SIG_WALLET_FIRST,
    MULTI_SIG_WALLET_SECOND,
    MULTI_SIG_WALLET_THIRD,
    MULTI_SIG_WALLET_FOURTH,
    SINGLE_WALLET,

    CURRENT_WALLET_BUTT,
} CURRENT_WALLET_INDEX_ENUM;

typedef struct {
    HOME_WALLET_CARD_ENUM index;
    bool state;
    const char *name;
    bool enable;
    bool testNet;
    CURRENT_WALLET_INDEX_ENUM defaultWallet;
    lv_obj_t *checkBox;
} WalletState_t;

typedef struct {
    HOME_WALLET_CARD_ENUM index;
    const char *coin;
    const char *chain;
    const lv_img_dsc_t *icon;
} ChainCoinCard_t;

void GuiHomeRefresh(void);
void GuiHomeAreaInit(void);
void GuiHomeDisActive(void);
void GuiHomeSetWalletDesc(WalletDesc_t *wallet);
void GuiHomeRestart(void);
bool GuiHomePageIsTop(void);
bool GetIsTestNet(void);
void SetIsTestNet(bool testNet);
void SetCurrentWalletIndex(CURRENT_WALLET_INDEX_ENUM walletIndex);
CURRENT_WALLET_INDEX_ENUM GetCurrentWalletIndex(void);
const ChainCoinCard_t* GetCoinCardByIndex(HOME_WALLET_CARD_ENUM index);
void GuiHomeDeInit(void);
#endif /* _GUI_BTC_HOME_WIDGETS_H */
#endif