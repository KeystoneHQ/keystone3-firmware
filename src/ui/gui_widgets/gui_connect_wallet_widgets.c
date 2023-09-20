/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description:
 * author     : ginger liu
 * data       : 2023-03-02
 **********************************************************************/
#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "user_memory.h"
#include "gui_wallet.h"
#include "gui_connect_wallet_widgets.h"
#include "account_public_info.h"
#include "rust.h"
#include "gui_ethereum_receive_widgets.h"
#include "gui_animating_qrcode.h"
#include "gui_page.h"
#include "keystore.h"

#define DERIVATION_PATH_EG_LEN 2

typedef enum
{
    CONNECT_WALLET_SELECT_WALLET = 0,
    CONNECT_WALLET_QRCODE,

    CONNECT_WALLET_BUTT,
} CONNECT_WALLET_ENUM;

WalletListItem_t g_walletListArray[] =
{
    // {WALLET_LIST_KEYSTONE, &walletListKeyStone},
    {WALLET_LIST_OKX, &walletListOkx},
    {WALLET_LIST_METAMASK, &walletListMetaMask},
    {WALLET_LIST_BLUE, &walletListBlue},
    // { WALLET_LIST_SUB, &walletListSub},
    // { WALLET_LIST_SOLFARE, &walletListSolfare},
    {WALLET_LIST_RABBY, &walletListRabby},
    {WALLET_LIST_SAFE, &walletListSafe},
    {WALLET_LIST_BLOCK_WALLET, &walletListBlockWallet},
    {WALLET_LIST_ZAPPER, &walletListZapper},
    {WALLET_LIST_YEARN_FINANCE, &walletListYearn},
    {WALLET_LIST_SUSHISWAP, &walletListSushi},
    {WALLET_LIST_KEPLR, &walletListKeplr},
};

typedef struct ConnectWalletWidget
{
    uint8_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    WALLET_LIST_INDEX_ENUM walletIndex;
    lv_obj_t *qrCode;
} ConnectWalletWidget_t;

typedef struct
{
    COMPANION_APP_COINS_ENUM index;
    const char *coin;
    const lv_img_dsc_t *icon;
} ComapanionCoinCard_t;

static const ComapanionCoinCard_t g_companionCoinCardArray[COMPANION_APP_COINS_BUTT] =
{
    {
        .index = BTC,
        .coin = "Bitcoin",
        .icon = &coinBtc,
    },
    {
        .index = ETH,
        .coin = "Ethereum / BSC",
        .icon = &coinEth,
    },
    {
        .index = BCH,
        .coin = "Bitcoin Cash",
        .icon = &coinBch,
    },
    {
        .index = DASH,
        .coin = "Dash",
        .icon = &coinDash,
    },
    {
        .index = LTC,
        .coin = "LiteCoin",
        .icon = &coinLtc,
    },
    {
        .index = TRON,
        .coin = "TRON",
        .icon = &coinTrx,
    },
    {
        .index = XRP,
        .coin = "XRP",
        .icon = &coinXrp,
    },
    {
        .index = DOT,
        .coin = "Polkadot",
        .icon = &coinDot,
    },
};

static const lv_img_dsc_t *g_metaMaskCoinArray[4] =
{
    &coinEth,
    &coinBnb,
    &coinAva,
    &coinMatic,
};

static const lv_img_dsc_t *g_ethWalletCoinArray[4] =
{
    &coinEth,
    &coinBnb,
    &coinAva,
    &coinMatic,
};

static const lv_img_dsc_t *g_okxWalletCoinArray[5] =
{
    &coinBtc,
    &coinEth,
    &coinBnb,
    &coinMatic,
    &coinOkb,
};

static const lv_img_dsc_t *g_blueWalletCoinArray[4] =
{
    &coinBtc,
};

static const lv_img_dsc_t *g_keplrCoinArray[8] =
{
    &coinAtom,
    &coinOsmo,
    &coinBld,
    &coinAkt,
    &coinXprt,
    &coinAxl,
    &coinBoot,
    &coinCro,
};

static ConnectWalletWidget_t g_connectWalletTileView;
static CoinState_t g_defaultCompanionAppState[COMPANION_APP_COINS_BUTT] =
{
    {BTC, true},
    {ETH, true},
    {BCH, true},
    {DASH, true},
    {LTC, true},
    {TRON, true},
    {XRP, true},
    {DOT, true},
};

typedef struct
{
    const char *accountType;
    const char *path;
} ChangeDerivationItem_t;

const static ChangeDerivationItem_t g_changeDerivationList[] =
{
    {"BIP 44 Standard", "#8E8E8E m/44'/60'/0'/0/##F5870A X#"},
    {"Ledger Live", "#8E8E8E m/44'/60'/##F5870A X##8E8E8E '/0/0#"},
    {"Ledger Legacy", "#8E8E8E m/44'/60'/0'/##F5870A X#"},
};

static lv_obj_t *g_coinListCont = NULL;
static PageWidget_t *g_pageWidget;

static void GetEthEgAddress(void);
static void initCompanionAppCoinsConfig(void);
static void OpenQRCodeHandler(lv_event_t *e);
static void ReturnShowQRHandler(lv_event_t *e);
static void UpdateCompanionAppCoinStateHandler(lv_event_t *e);
static void JumpSelectCoinPageHandler(lv_event_t *e);
static void ConfirmSelectCompanionAppCoinsHandler(lv_event_t *e);
static void GuiCreateSelectCompanionAppCoinWidget();
void ConnectWalletReturnHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);
static void AddMetaMaskCoins(void);
static void AddBlueWalletCoins(void);
static void AddCompanionAppCoins(void);
static void AddKeplrCoins(void);

CoinState_t g_companionAppcoinState[COMPANION_APP_COINS_BUTT];
static char g_derivationPathAddr[LedgerLegacy + 1][DERIVATION_PATH_EG_LEN][64];
static lv_obj_t *g_derivationCheck[LedgerLegacy + 1];
static ETHAccountType g_currentPathIndex[3] = {Bip44Standard, Bip44Standard, Bip44Standard};
static ETHAccountType g_currentBakPathIndex = Bip44Standard;
static lv_obj_t *g_egAddress[DERIVATION_PATH_EG_LEN];
static CoinState_t g_tempCompanionAppcoinState[COMPANION_APP_COINS_BUTT];
static lv_obj_t *g_coinCont = NULL;
static lv_obj_t *g_openMoreHintBox = NULL;
static lv_obj_t *g_bottomCont = NULL;
static lv_obj_t *g_manageImg = NULL;
static bool g_isCoinReselected = false;
static lv_obj_t *g_derivationPathDescLabel = NULL;
static char* *g_derivationPathDescs = NULL;

static void QRCodePause(bool);

static void InitDerivationPathDescs()
{
    if(g_derivationPathDescs == NULL)
    {
        g_derivationPathDescs = SRAM_MALLOC(3*128);
        g_derivationPathDescs[Bip44Standard] = _("Recommended. Widely adopted across numerous software wallets.");
        g_derivationPathDescs[LedgerLive] = _("Choose this path if you intend to import a seed phrase from Ledger Live.");
        g_derivationPathDescs[LedgerLegacy] = _("Choose this path if you are managing your digital assets with Ledger Legacy.");
    }
}

static void OpenQRCodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        WalletListItem_t *wallet = lv_event_get_user_data(e);
        g_connectWalletTileView.walletIndex = wallet->index;
        g_isCoinReselected = false;
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    }
}

static void ReturnShowQRHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ConnectWalletReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, NULL);
        GUI_DEL_OBJ(g_coinListCont)
    }
}

static void UpdateCompanionAppCoinStateHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        CoinState_t *coinState = lv_event_get_user_data(e);
        lv_obj_t *parent = lv_obj_get_parent(lv_event_get_target(e));
        bool state = lv_obj_has_state(lv_obj_get_child(parent, 2), LV_STATE_CHECKED);
        g_tempCompanionAppcoinState[coinState->index].state = state;

        for (int i = 0; i < COMPANION_APP_COINS_BUTT; i++)
        {
            if (g_tempCompanionAppcoinState[i].state)
            {
                lv_obj_add_state(g_defaultCompanionAppState[i].checkBox, LV_STATE_CHECKED);
            }
            else
            {
                lv_obj_clear_state(g_defaultCompanionAppState[i].checkBox, LV_STATE_CHECKED);
            }
        }
    }
}

static void ConfirmSelectCompanionAppCoinsHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        g_isCoinReselected = true;
        memcpy(g_companionAppcoinState, g_tempCompanionAppcoinState, sizeof(g_tempCompanionAppcoinState));
        GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ConnectWalletReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, NULL);
        GUI_DEL_OBJ(g_coinListCont)
    }
}

static void JumpSelectCoinPageHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
#ifndef COMPILE_SIMULATOR
        GuiAnimatingQRCodeDestroyTimer();
#endif
        GuiCreateSelectCompanionAppCoinWidget();
    }
}

static void GuiCreateSelectCompanionAppCoinWidget()
{
    // root container
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    g_coinListCont = cont;
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);

    // coin list container
    lv_obj_t *coinListCont = GuiCreateContainerWithParent(cont, 408, 542);
    lv_obj_set_align(coinListCont, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(coinListCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(coinListCont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *labelHint = GuiCreateIllustrateLabel(coinListCont, _("Select networks you’d like to manage in the software wallet"));
    lv_obj_set_style_text_opa(labelHint, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *coinLabel;
    lv_obj_t *icon;
    lv_obj_t *checkbox;

    for (int i = 0; i < COMPANION_APP_COINS_BUTT; i++)
    {
        coinLabel = GuiCreateTextLabel(coinListCont, g_companionCoinCardArray[i].coin);
        icon = GuiCreateScaleImg(coinListCont, g_companionCoinCardArray[i].icon, 118);
        checkbox = GuiCreateMultiCheckBox(coinListCont, _(""));
        g_defaultCompanionAppState[i].checkBox = checkbox;

        if (g_tempCompanionAppcoinState[i].state)
        {
            lv_obj_add_state(g_defaultCompanionAppState[i].checkBox, LV_STATE_CHECKED);
        }
        else
        {
            lv_obj_clear_state(g_defaultCompanionAppState[i].checkBox, LV_STATE_CHECKED);
        }

        GuiButton_t table[4] =
        {
            {
                .obj = coinLabel,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 16},
            },
            {
                .obj = icon,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 60},
            },
            {
                .obj = checkbox,
                .align = LV_ALIGN_TOP_RIGHT,
                .position = {-28, 36},
            },
        };
        int buttonNum = 3;
        if (i == ETH)
        {
            buttonNum = 4;
            lv_obj_t *bnb = GuiCreateScaleImg(coinListCont, &coinBnb, 118);
            table[3].obj = bnb;
            table[3].align = LV_ALIGN_DEFAULT;
            table[3].position.x = 56;
            table[3].position.y = 60;
        }
        lv_obj_t *button = GuiCreateButton(coinListCont, lv_obj_get_width(coinListCont), 100, table, buttonNum,
                                           UpdateCompanionAppCoinStateHandler, &g_companionAppcoinState[i]);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 0, 100 * i + 16 * i + lv_obj_get_height(labelHint) + 24);
    }

    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_CONFIRM);
    lv_obj_add_event_cb(btn, ConfirmSelectCompanionAppCoinsHandler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);

    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Select Network"));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnShowQRHandler, cont);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
}

static void GuiCreateSelectWalletWidget(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    for (int i = 0; i < NUMBER_OF_ARRAYS(g_walletListArray); i++)
    {
        lv_obj_t *img = GuiCreateImg(parent, g_walletListArray[i].img);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, i * 107);
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, OpenQRCodeHandler, LV_EVENT_CLICKED, &g_walletListArray[i]);
        lv_obj_add_style(img, &g_generalBtnPressStyle, LV_STATE_PRESSED);
    }

    // ** temporary add.
    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, NUMBER_OF_ARRAYS(g_walletListArray) * 107 + 8);

    lv_obj_t *label = GuiCreateIllustrateLabel(parent, "Please upgrade to the latest version for access to expanded software wallet compatibility.");
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 32, NUMBER_OF_ARRAYS(g_walletListArray) * 107 + 8 + 8);
    // **
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("Scan the QR code with your software wallet"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 152 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *qrCont = GuiCreateContainerWithParent(parent, 408, 482);
    lv_obj_add_flag(qrCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(qrCont, LV_ALIGN_TOP_MID, 0, 62);
    lv_obj_set_style_bg_color(qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(qrCont, 24, LV_PART_MAIN);

    lv_obj_t *qrBgCont = GuiCreateContainerWithParent(qrCont, 336, 336);
    lv_obj_align(qrBgCont, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_bg_color(qrBgCont, WHITE_COLOR, LV_PART_MAIN);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(qrBgCont, 294, 294);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);

    // lv_obj_t *qrcode = lv_qrcode_create(qrBgCont, 294, BLACK_COLOR, WHITE_COLOR);
    // lv_qrcode_update(qrcode, "", 0);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 21);
    g_connectWalletTileView.qrCode = qrcode;

    g_bottomCont = GuiCreateContainerWithParent(qrCont, 408, 104);
    lv_obj_align(g_bottomCont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(g_bottomCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_bottomCont, LV_OPA_0, LV_STATE_DEFAULT | LV_PART_MAIN);

    label = GuiCreateNoticeLabel(g_bottomCont, _("Supported Networks"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);
    lv_obj_add_event_cb(g_bottomCont, JumpSelectCoinPageHandler, LV_EVENT_CLICKED, NULL);

    g_coinCont = GuiCreateContainerWithParent(g_bottomCont, 280, 30);
    lv_obj_align(g_coinCont, LV_ALIGN_TOP_LEFT, 36, 50);
    lv_obj_set_style_bg_color(g_coinCont, DARK_BG_COLOR, LV_PART_MAIN);
    g_manageImg = GuiCreateImg(g_bottomCont, &imgManage);
    lv_obj_set_style_img_opa(g_manageImg, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(g_manageImg, LV_ALIGN_BOTTOM_RIGHT, -45, -41);
    lv_obj_add_flag(g_manageImg, LV_OBJ_FLAG_HIDDEN);
}

static void AddCompanionAppCoins()
{
    lv_obj_add_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_manageImg, LV_OBJ_FLAG_HIDDEN);
    if (lv_obj_get_child_cnt(g_coinCont) > 0)
    {
        lv_obj_clean(g_coinCont);
    }
    int count = 0;
    for (int i = 0; i < COMPANION_APP_COINS_BUTT; i++)
    {
        if (g_companionAppcoinState[i].state)
        {
            if (count < 8)
            {
                lv_obj_t *img = GuiCreateImg(g_coinCont, g_companionCoinCardArray[g_companionAppcoinState[i].index].icon);
                lv_img_set_zoom(img, 110);
                lv_img_set_pivot(img, 0, 0);
                lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * count, 0);
            }
            count++;
        }
    }
    // add more icon
    if (count > 8)
    {
        lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
        lv_img_set_zoom(img, 150);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * count, 2);
    }
}

static void AddMetaMaskCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0)
    {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_metaMaskCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }

    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 132, 2);
}

static void AddEthWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0)
    {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_ethWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 132, 2);
}

static void AddOkxWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0)
    {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 5; i++)
    {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_okxWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 132, 2);
}

static void AddBlueWalletCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0)
    {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 1; i++)
    {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_blueWalletCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }
}

static void AddKeplrCoins(void)
{
    if (lv_obj_get_child_cnt(g_coinCont) > 0)
    {
        lv_obj_clean(g_coinCont);
    }
    for (int i = 0; i < 8; i++)
    {
        lv_obj_t *img = GuiCreateImg(g_coinCont, g_keplrCoinArray[i]);
        lv_img_set_zoom(img, 110);
        lv_img_set_pivot(img, 0, 0);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 32 * i, 0);
    }

    lv_obj_t *img = GuiCreateImg(g_coinCont, &imgMore);
    lv_img_set_zoom(img, 150);
    lv_img_set_pivot(img, 0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 256, 2);
}

void GuiConnectWalletInit(void)
{
    InitDerivationPathDescs();
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = lv_tileview_create(cont);
    lv_obj_clear_flag(tileView, LV_OBJ_FLAG_SCROLLABLE);
    if (GuiDarkMode())
    {
        lv_obj_set_style_bg_color(tileView, BLACK_COLOR, LV_PART_MAIN);
    }
    else
    {
        lv_obj_set_style_bg_color(tileView, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *tile = lv_tileview_add_tile(tileView, CONNECT_WALLET_SELECT_WALLET, 0, LV_DIR_HOR);
    GuiCreateSelectWalletWidget(tile);

    tile = lv_tileview_add_tile(tileView, CONNECT_WALLET_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(tile);

    g_connectWalletTileView.currentTile = CONNECT_WALLET_SELECT_WALLET;
    g_connectWalletTileView.tileView = tileView;
    g_connectWalletTileView.cont = cont;

    lv_obj_set_tile_id(g_connectWalletTileView.tileView, g_connectWalletTileView.currentTile, 0, LV_ANIM_OFF);
}

void GuiConnectWalletSetQrdata(WALLET_LIST_INDEX_ENUM index)
{
#ifndef COMPILE_SIMULATOR
    SetWallet(g_pageWidget->navBarWidget, index, NULL);
    GenerateUR func = NULL;
    lv_obj_clear_flag(g_bottomCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(g_manageImg, LV_OBJ_FLAG_HIDDEN);
    switch (index)
    {
    case WALLET_LIST_KEYSTONE:
        if (!g_isCoinReselected)
        {
            initCompanionAppCoinsConfig();
        }
        func = GuiGetCompanionAppData;
        AddCompanionAppCoins();
        break;
    case WALLET_LIST_METAMASK:
        func = GuiGetMetamaskData;
        AddMetaMaskCoins();
        break;
    case WALLET_LIST_RABBY:
    case WALLET_LIST_SAFE:
    case WALLET_LIST_BLOCK_WALLET:
    case WALLET_LIST_ZAPPER:
    case WALLET_LIST_YEARN_FINANCE:
    case WALLET_LIST_SUSHISWAP:
        func = GuiGetMetamaskData;
        AddEthWalletCoins();
        break;
    case WALLET_LIST_OKX:
        func = GuiGetOkxWalletData;
        AddOkxWalletCoins();
        break;
    case WALLET_LIST_BLUE:
        func = GuiGetBlueWalletBtcData;
        AddBlueWalletCoins();
        break;
    case WALLET_LIST_SUB:
        break;
    case WALLET_LIST_SOLFARE:
        break;
    case WALLET_LIST_KEPLR:
        func = GuiGetKeplrData;
        AddKeplrCoins();
        break;
    default:
        return;
    }
    if (func)
    {
        GuiAnimatingQRCodeInit(g_connectWalletTileView.qrCode, func, true);
    }
#endif
}

static void QRCodePause(bool pause)
{
    GuiAnimatingQRCodeControl(pause);
}

void GuiConnectWalletHandleURGenerate(char *data, uint16_t len)
{
    GuiAnimantingQRCodeFirstUpdate(data, len);
}

void GuiConnectWalletHandleURUpdate(char *data, uint16_t len)
{
    GuiAnimatingQRCodeUpdate(data, len);
}

void ConnectWalletReturnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
#ifndef COMPILE_SIMULATOR
        // CloseQRTimer();
#endif
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

static void initCompanionAppCoinsConfig(void)
{
    memcpy(g_tempCompanionAppcoinState, g_defaultCompanionAppState, sizeof(g_defaultCompanionAppState));
    memcpy(g_companionAppcoinState, g_defaultCompanionAppState, sizeof(g_defaultCompanionAppState));
}

ETHAccountType GetMetamaskAccountType(void)
{
    return g_currentPathIndex[GetCurrentAccountIndex()];
}

static lv_obj_t *g_derivationPathCont = NULL;

static void CloseDerivationHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        if (g_currentBakPathIndex != GetMetamaskAccountType())
        {
            GuiAnimatingQRCodeDestroyTimer();
            GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
            g_currentBakPathIndex = GetMetamaskAccountType();
        }
        else
        {
            QRCodePause(false);
        }
        GUI_DEL_OBJ(g_derivationPathCont);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetWallet(g_pageWidget->navBarWidget, g_connectWalletTileView.walletIndex, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, &g_connectWalletTileView.walletIndex);
    }
}

static void GetEthEgAddress(void)
{
#ifndef COMPILE_SIMULATOR
    SimpleResponse_c_char *result;
    result = eth_get_address("44'/60'/0'/0/0", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD), "44'/60'/0'");
    AddressLongModeCut(g_derivationPathAddr[Bip44Standard][0], result->data);
    free_simple_response_c_char(result);

    result = eth_get_address("44'/60'/0'/0/1", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_BIP44_STANDARD), "44'/60'/0'");
    AddressLongModeCut(g_derivationPathAddr[Bip44Standard][1], result->data);
    free_simple_response_c_char(result);

    result = eth_get_address("44'/60'/0'/0/0", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LIVE_0), "44'/60'/0'");
    AddressLongModeCut(g_derivationPathAddr[LedgerLive][0], result->data);
    free_simple_response_c_char(result);

    result = eth_get_address("44'/60'/1'/0/0", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LIVE_1), "44'/60'/1'");
    AddressLongModeCut(g_derivationPathAddr[LedgerLive][1], result->data);
    free_simple_response_c_char(result);

    result = eth_get_address("44'/60'/0'/0", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY), "44'/60'/0'");
    AddressLongModeCut(g_derivationPathAddr[LedgerLegacy][0], result->data);
    free_simple_response_c_char(result);

    result = eth_get_address("44'/60'/0'/1", GetCurrentAccountPublicKey(XPUB_TYPE_ETH_LEDGER_LEGACY), "44'/60'/0'");
    AddressLongModeCut(g_derivationPathAddr[LedgerLegacy][1], result->data);
    free_simple_response_c_char(result);
#endif
}

static void UpdateEthEgAddress(uint8_t index)
{
    lv_label_set_text(g_egAddress[0], (const char *)g_derivationPathAddr[index][0]);
    lv_label_set_text(g_egAddress[1], (const char *)g_derivationPathAddr[index][1]);
}

static void SelectDerivationHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_t *newCheckBox = lv_event_get_user_data(e);
        for (int i = 0; i < 3; i++)
        {
            if (newCheckBox == g_derivationCheck[i])
            {
                lv_obj_add_state(newCheckBox, LV_STATE_CHECKED);
                g_currentPathIndex[GetCurrentAccountIndex()] = i;
                lv_label_set_text(g_derivationPathDescLabel, g_derivationPathDescs[g_currentPathIndex[GetCurrentAccountIndex()]]);
                UpdateEthEgAddress(i);
            }
            else
            {
                lv_obj_clear_state(g_derivationCheck[i], LV_STATE_CHECKED);
            }
        }
    }
}
static void OpenTutorialHandler(lv_event_t *e)
{
    QRCodePause(true);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
        GuiFrameOpenViewWithParam(&g_walletTutorialView, wallet, sizeof(wallet));
        GUI_DEL_OBJ(g_openMoreHintBox);
    }
}

static void OpenDerivationPath()
{
    lv_obj_t *bgCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                          GUI_MAIN_AREA_OFFSET);
    lv_obj_align(bgCont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
    lv_obj_t *label = GuiCreateNoticeLabel(bgCont, "Select the derivation path you’d like to use for Ethereum");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *cont = GuiCreateContainerWithParent(bgCont, 408, 308);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    for (int i = 0; i < 3; i++)
    {
        lv_obj_t *accountType = GuiCreateTextLabel(cont, g_changeDerivationList[i].accountType);
        lv_obj_t *path = GuiCreateIllustrateLabel(cont, g_changeDerivationList[i].path);
        lv_label_set_recolor(path, true);
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(cont, _(""));
        lv_obj_set_size(checkBox, 36, 36);
        g_derivationCheck[i] = checkBox;
        if (i == GetMetamaskAccountType())
        {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        GuiButton_t table[] =
        {
            {
                .obj = accountType,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 16},
            },
            {
                .obj = path,
                .align = LV_ALIGN_DEFAULT,
                .position = {24, 56},
            },
            {
                .obj = checkBox,
                .align = LV_ALIGN_RIGHT_MID,
                .position = {-24, 0},
            },
        };
        lv_obj_t *button = GuiCreateButton(cont, 408, 102, table, NUMBER_OF_ARRAYS(table),
                                           SelectDerivationHandler, g_derivationCheck[i]);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, i * 102);
        if (i != 0)
        {
            static lv_point_t points[2] = {{0, 0}, {360, 0}};
            lv_obj_t *line = (lv_obj_t *)GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, i * 102);
        }
    }

    lv_obj_t *egCont = GuiCreateContainerWithParent(bgCont, 408, 186);
    lv_obj_align(egCont, LV_ALIGN_BOTTOM_MID, 0, -54);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);

    label = GuiCreateNoticeLabel(egCont, g_derivationPathDescs[g_currentPathIndex[GetCurrentAccountIndex()]]);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    g_derivationPathDescLabel = label;
    label = GuiCreateNoticeLabel(egCont, _("Addresses eg."));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 76);
    lv_obj_t *index = GuiCreateNoticeLabel(egCont, _("0"));
    lv_obj_align(index, LV_ALIGN_TOP_LEFT, 24, 110);
    label = GuiCreateIllustrateLabel(egCont, "");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 48, 110);
    g_egAddress[0] = label;
    index = GuiCreateNoticeLabel(egCont, _("1"));
    lv_obj_align(index, LV_ALIGN_TOP_LEFT, 24, 144);
    label = GuiCreateIllustrateLabel(egCont, "");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 48, 144);
    g_egAddress[1] = label;
    GetEthEgAddress();
    UpdateEthEgAddress(GetMetamaskAccountType());
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Change Derivation Path"));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseDerivationHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    GUI_DEL_OBJ(g_openMoreHintBox);
    g_derivationPathCont = bgCont;
}

static void ChangeDerivationPathHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        OpenDerivationPath();
        QRCodePause(true);
    }
}

static bool IsEVMChain(int walletIndex)
{
    switch (walletIndex)
    {
    case WALLET_LIST_METAMASK:
    case WALLET_LIST_RABBY:
    case WALLET_LIST_SAFE:
    case WALLET_LIST_BLOCK_WALLET:
    case WALLET_LIST_ZAPPER:
    case WALLET_LIST_YEARN_FINANCE:
    case WALLET_LIST_SUSHISWAP:
        return true;
    default:
        return false;
    }
}

static void OpenMoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        int hintboxHeight = 132;
        WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
        if (IsEVMChain(*wallet))
        {
            hintboxHeight = 228;
        }
        g_openMoreHintBox = GuiCreateHintBox(lv_scr_act(), 480, hintboxHeight, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_openMoreHintBox);
        lv_obj_t *label = GuiCreateTextLabel(g_openMoreHintBox, "Tutorial");
        lv_obj_t *img = GuiCreateImg(g_openMoreHintBox, &imgTutorial);

        GuiButton_t table[] =
        {
            {
                .obj = img,
                .align = LV_ALIGN_LEFT_MID,
                .position = {24, 0},
            },
            {
                .obj = label,
                .align = LV_ALIGN_LEFT_MID,
                .position = {76, 0},
            },
        };
        lv_obj_t *btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                        OpenTutorialHandler, wallet);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);

        if (IsEVMChain(*wallet))
        {
            label = GuiCreateTextLabel(g_openMoreHintBox, "Change Derivation Path");
            img = GuiCreateImg(g_openMoreHintBox, &imgPath);
            table[0].obj = img;
            table[1].obj = label;
            btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                  ChangeDerivationPathHandler, NULL);
            lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -120);
        }
    }
}

int8_t GuiConnectWalletNextTile(void)
{
    switch (g_connectWalletTileView.currentTile)
    {
    case CONNECT_WALLET_SELECT_WALLET:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ConnectWalletReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, &g_connectWalletTileView.walletIndex);
        GuiConnectWalletSetQrdata(g_connectWalletTileView.walletIndex);
        break;
    case CONNECT_WALLET_QRCODE:
        return 0;
    }

    g_connectWalletTileView.currentTile++;
    lv_obj_set_tile_id(g_connectWalletTileView.tileView, g_connectWalletTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiConnectWalletPrevTile(void)
{
    switch (g_connectWalletTileView.currentTile)
    {
    case CONNECT_WALLET_SELECT_WALLET:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        break;
    case CONNECT_WALLET_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Choose Wallet"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        GuiAnimatingQRCodeDestroyTimer();
        break;
    }
    g_connectWalletTileView.currentTile--;
    lv_obj_set_tile_id(g_connectWalletTileView.tileView, g_connectWalletTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiConnectWalletRefresh(void)
{
    switch (g_connectWalletTileView.currentTile)
    {
    case CONNECT_WALLET_SELECT_WALLET:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Choose Wallet"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    case CONNECT_WALLET_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ConnectWalletReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, &g_connectWalletTileView.walletIndex);
        SetWallet(g_pageWidget->navBarWidget, g_connectWalletTileView.walletIndex, NULL);
        if (g_coinListCont != NULL)
        {
            GUI_DEL_OBJ(g_coinListCont)
            GuiCreateSelectCompanionAppCoinWidget();
        }
        QRCodePause(false);
    }
    if (g_derivationPathCont != NULL)
    {
        GUI_DEL_OBJ(g_derivationPathCont);
        OpenDerivationPath();
    }

}

void GuiConnectWalletDeInit(void)
{
    GUI_DEL_OBJ(g_openMoreHintBox)
    GUI_DEL_OBJ(g_coinCont)
    GUI_DEL_OBJ(g_derivationPathCont)

    CloseToTargetTileView(g_connectWalletTileView.currentTile, CONNECT_WALLET_SELECT_WALLET);
    GUI_DEL_OBJ(g_connectWalletTileView.cont)
    if (g_pageWidget != NULL)
    {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}
