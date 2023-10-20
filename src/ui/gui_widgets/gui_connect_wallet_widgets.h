/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description:
 * author     : ginger liu
 * data       : 2023-03-02
**********************************************************************/

#ifndef _GUI_CONNECT_WALLET_WIDGETS_H
#define _GUI_CONNECT_WALLET_WIDGETS_H

#include "gui_views.h"

typedef enum {
    WALLET_LIST_KEYSTONE,
    WALLET_LIST_METAMASK,
    WALLET_LIST_OKX,
    WALLET_LIST_BLUE,
    WALLET_LIST_SUB,
    WALLET_LIST_SOLFARE,
    WALLET_LIST_RABBY,
    WALLET_LIST_SAFE,
    WALLET_LIST_BLOCK_WALLET,
    WALLET_LIST_ZAPPER,
    WALLET_LIST_YEARN_FINANCE,
    WALLET_LIST_SUSHISWAP,
    WALLET_LIST_KEPLR,
    WALLET_LIST_FEWCHA,
    WALLET_LIST_PETRA,

    WALLET_LIST_BUTT,
} WALLET_LIST_INDEX_ENUM;

typedef struct {
    WALLET_LIST_INDEX_ENUM index;
    const lv_img_dsc_t *img;
} WalletListItem_t;

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
} CoinState_t;

void GuiConnectWalletInit(void);
int8_t GuiConnectWalletNextTile(void);
int8_t GuiConnectWalletPrevTile(void);
void GuiConnectWalletRefresh(void);
void GuiConnectWalletDeInit(void);
void GuiConnectWalletSetQrdata(WALLET_LIST_INDEX_ENUM index);
ETHAccountType GetMetamaskAccountType(void);
void GuiConnectWalletHandleURGenerate(char *data, uint16_t len);
void GuiConnectWalletHandleURUpdate(char *data, uint16_t len);
SOLAccountType GetSolflareAccountType(void);
#endif /* _GUI_CONNECT_WALLET_WIDGETS_H */

