/*
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * @FilePath: \project-pillar-firmware\ui\gui_widgets\gui_home_widgets.h
 * @Description:
 * @Author: stone wang
 * @LastEditTime: 2023-04-12 18:43:17
 */
#ifndef _GUI_HOME_WIDGETS_H
#define _GUI_HOME_WIDGETS_H

#include "gui_model.h"

typedef enum {
    HOME_WALLET_CARD_BTC,
    HOME_WALLET_CARD_ETH,
    HOME_WALLET_CARD_SOL,
    HOME_WALLET_CARD_SUI,
    HOME_WALLET_CARD_ATOM,
    HOME_WALLET_CARD_OSMO,
    HOME_WALLET_CARD_SCRT,
    HOME_WALLET_CARD_AKT,
    HOME_WALLET_CARD_CRO,
    HOME_WALLET_CARD_IOV,
    HOME_WALLET_CARD_ROWAN,
    HOME_WALLET_CARD_CTK,
    HOME_WALLET_CARD_IRIS,
    HOME_WALLET_CARD_REGEN,
    HOME_WALLET_CARD_XPRT,
    HOME_WALLET_CARD_DVPN,
    HOME_WALLET_CARD_IXO,
    HOME_WALLET_CARD_NGM,
    HOME_WALLET_CARD_BLD,
    HOME_WALLET_CARD_BOOT,
    HOME_WALLET_CARD_JUNO,
    HOME_WALLET_CARD_STARS,
    HOME_WALLET_CARD_AXL,
    HOME_WALLET_CARD_SOMM,
    HOME_WALLET_CARD_UMEE,
    HOME_WALLET_CARD_GRAV,
    HOME_WALLET_CARD_TGD,
    HOME_WALLET_CARD_STRD,
    HOME_WALLET_CARD_EVMOS,
    HOME_WALLET_CARD_INJ,
    HOME_WALLET_CARD_KAVA,
    HOME_WALLET_CARD_QCK,
    HOME_WALLET_CARD_LUNA,
    HOME_WALLET_CARD_LUNC,
    HOME_WALLET_CARD_BNB,
    HOME_WALLET_CARD_DOT,
    HOME_WALLET_CARD_XRP,
    HOME_WALLET_CARD_LTC,
    HOME_WALLET_CARD_DASH,
    HOME_WALLET_CARD_BCH,
    HOME_WALLET_CARD_TRX,

    HOME_WALLET_CARD_BUTT,
} HOME_WALLET_CARD_ENUM;

typedef struct {
    HOME_WALLET_CARD_ENUM index;
    bool state;
    const char *name;
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
const ChainCoinCard_t* GetCoinCardByIndex(HOME_WALLET_CARD_ENUM index);

#endif /* _GUI_HOME_WIDGETS_H */
