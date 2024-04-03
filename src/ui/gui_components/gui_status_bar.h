
#ifndef _GUI_STATUS_BAR_H
#define _GUI_STATUS_BAR_H

#include "gui.h"
#include "gui_chain.h"
#include "gui_connect_wallet_widgets.h"

#ifdef BTC_ONLY
#define WALLPAPER_ENABLE            0
#else
#define WALLPAPER_ENABLE            0
#endif
typedef enum {
    NVS_BAR_RETURN = 0,
    NVS_BAR_CLOSE,
    NVS_BAR_MANAGE,

    NVS_LEFT_BUTTON_BUTT,
} NVS_LEFT_BUTTON_ENUM;

typedef enum {
    NVS_BAR_MID_WORD_SELECT = NVS_LEFT_BUTTON_BUTT + 1,
    NVS_BAR_MID_LABEL,
    NVS_BAR_MID_COIN,

    NVS_MID_BUTTON_BUTT,
} NVS_MID_BUTTON_ENUM;

typedef enum {
    NVS_BAR_WORD_SELECT = NVS_MID_BUTTON_BUTT + 1,
    NVS_BAR_WORD_RESET,
    NVS_BAR_QUESTION_MARK,
    NVS_BAR_MORE_INFO,
    NVS_BAR_SKIP,
    NVS_BAR_SEARCH,
    NVS_BAR_NEW_SKIP,
    NVS_BAR_UNDO,
    NVS_BAR_SDCARD,

    NVS_RIGHT_BUTTON_BUTT,
} NVS_RIGHT_BUTTON_ENUM;

typedef struct NavBarWidget {
    lv_obj_t *navBar;

    lv_obj_t *leftBtn;
    lv_obj_t *midBtn;
    lv_obj_t *rightBtn;
} NavBarWidget_t;

void GuiNvsBarSetLeftCb(NVS_LEFT_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param);
void GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param);
void GuiNvsBarSetRightBtnLabel(NVS_RIGHT_BUTTON_ENUM button, const char *text);
void GuiNvsBarSetMidCb(NVS_MID_BUTTON_ENUM button, lv_event_cb_t eventCb, void* param);
void GuiNvsBarSetMidBtnLabel(NVS_MID_BUTTON_ENUM button, const char* text);
void ShowWallPaper(bool enable);
void GuiStatusBarInit(void);
void GuiStatusBarSetBattery(uint8_t percent, bool charging);
void GuiNvsBarSetWalletIcon(const void *src);
void GuiNvsBarSetWalletName(const char *name);
const char *GuiNvsBarGetWalletName(void);
void GuiNvsSetCoinWallet(GuiChainCoinType index, const char *name);
void GuiNvsSetWallet(WALLET_LIST_INDEX_ENUM index, const char *name);
void GuiNvsBarClear(void);
void GuiStatusBarSetSdCard(bool connected);
void GuiStatusBarSetUsb(void);
#ifdef BTC_ONLY
void GuiStatusBarSetTestNet(void);
#endif

NavBarWidget_t *CreateNavBarWidget(lv_obj_t *navBar);
void DestoryNavBarWidget(NavBarWidget_t *navBarWidget);
void SetNavBarLeftBtn(NavBarWidget_t *navBarWidget, NVS_LEFT_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param);
void SetNavBarMidBtn(NavBarWidget_t *navBarWidget, NVS_MID_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param);
void SetCoinWallet(NavBarWidget_t *navBarWidget, GuiChainCoinType index, const char *name);
void SetWallet(NavBarWidget_t *navBarWidget, WALLET_LIST_INDEX_ENUM index, const char *name);
void SetMidBtnLabel(NavBarWidget_t *navBarWidget, NVS_MID_BUTTON_ENUM button, const char *text);
void SetNavBarRightBtn(NavBarWidget_t *navBarWidget, NVS_RIGHT_BUTTON_ENUM button, lv_event_cb_t eventCb, void *param);
void SetRightBtnLabel(NavBarWidget_t *navBarWidget, NVS_RIGHT_BUTTON_ENUM button, const char *text);
void SetRightBtnCb(NavBarWidget_t *navBarWidget, lv_event_cb_t eventCb, void *param);

#endif /* _GUI_STATUS_BAR_H */

