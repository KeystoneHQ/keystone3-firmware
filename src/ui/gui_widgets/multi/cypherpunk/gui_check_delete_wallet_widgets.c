#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_status_bar.h"
#include "gui_views.h"
#include "gui_hintbox.h"
#include "gui_setup_widgets.h"
#include "presetting.h"
#include "version.h"
#include "gui_hintbox.h"
#include "gui_lock_widgets.h"

static lv_obj_t *g_deleteWalletCont = NULL;

static void DeleteWalletNextStepHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_deleteWalletCont)
    g_deleteWalletCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_t *label = GuiCreateTextLabel(g_deleteWalletCont, _("wallet_deleting"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 427);
    GuiCreateCircleAroundAnimation(lv_scr_act(), -35);
    GuiModelSettingDelWalletDesc();
}

static void DeleteWalletNotNowHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_deleteWalletCont)
    GuiCloseCurrentWorkingView();
    static uint16_t signal = SIG_LOCK_VIEW_VERIFY_PIN;
    LogoutCurrentAccount();
    GuiLockScreenSetFirstUnlock();
    GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &signal, sizeof(signal));
}

void GuiCheckDeleteWalletDeInit(void)
{
    GUI_DEL_OBJ(g_deleteWalletCont)
    GuiStopCircleAroundAnimation();
}

void GuiDelWalletToSetup(void)
{
    GuiCloseCurrentWorkingView();
    GuiLockScreenHidden();

    GuiFrameOpenView(&g_setupView);
}

void GuiCheckDeleteWalletInit(void)
{
    lv_obj_t *tempObj;
    if (g_deleteWalletCont == NULL) {
        g_deleteWalletCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - 48);
        lv_obj_add_flag(g_deleteWalletCont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align(g_deleteWalletCont, LV_ALIGN_DEFAULT, 0, 48);

        tempObj = GuiCreateImg(g_deleteWalletCont, &imgWarn);
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 132);
        tempObj = GuiCreateLittleTitleLabel(g_deleteWalletCont, _("Warning"));
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 238);
        tempObj = GuiCreateNoticeLabel(g_deleteWalletCont, _("check_ton_wallet_warning"));
        lv_obj_align(tempObj, LV_ALIGN_TOP_MID, 0, 288);
        lv_obj_set_style_text_align(tempObj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

        lv_obj_t *btn = GuiCreateTextBtn(g_deleteWalletCont, _("not_now"));
        lv_obj_set_style_bg_color(btn, DARK_GRAY_COLOR, LV_PART_MAIN);
        lv_obj_set_size(btn, 192, 66);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        lv_obj_add_event_cb(btn, DeleteWalletNotNowHandler, LV_EVENT_CLICKED, NULL);

        btn = GuiCreateTextBtn(g_deleteWalletCont, _("initialization_complete_hintbox_ok"));
        lv_obj_set_size(btn, 192, 66);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(btn, DeleteWalletNextStepHandler, LV_EVENT_CLICKED, NULL);
    }
}


