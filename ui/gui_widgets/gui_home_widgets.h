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
    HOME_WALLET_CARD_BNB,
    HOME_WALLET_CARD_SOL,
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
