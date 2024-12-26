#ifndef _GUI_HOME_WIDGETS_H
#define _GUI_HOME_WIDGETS_H

#include "gui_model.h"
#include "lvgl.h"
#include "gui_keyboard_hintbox.h"
#include "gui_attention_hintbox.h"
#include "general/gui_general_home_widgets.h"
#include "cyberpunk/gui_cyberpunk_home_widgets.h"
#ifdef COMPILE_SIMULATOR
#include "gui_pending_hintbox.h"
#endif

typedef enum {
    HOME_WALLET_CARD_BTC,
    HOME_WIDGETS_SURPLUS_CARD_ENUM, 
    HOME_WALLET_CARD_BUTT,      // This represents the end of the array (the number of arrays) and needs to be placed at the end.
} HOME_WALLET_CARD_ENUM;

typedef struct {
    HOME_WALLET_CARD_ENUM index;
    bool state;
    const char *name;
    bool enable;
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
void GuiHomePasswordErrorCount(void *param);
void GuiRemoveKeyboardWidget(void);
void RecalculateManageWalletState(void);
const ChainCoinCard_t* GetCoinCardByIndex(HOME_WALLET_CARD_ENUM index);
void GuiHomeDeInit(void);
void GuiShowRsaSetupasswordHintbox(void);
void GuiShowRsaInitializatioCompleteHintbox(void);
void ClearHomePageCurrentIndex(void);
void ReturnManageWalletHandler(lv_event_t *e);

#endif /* _GUI_HOME_WIDGETS_H */

