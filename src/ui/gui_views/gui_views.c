#include "gui_views.h"
#include "gui_setup_widgets.h"
#include "gui_qr_code.h"
#include "gui_model.h"
#include "gui_hintbox.h"
#include "gui_lock_widgets.h"
#include "gui_keyboard.h"
#include "gui_create_wallet_widgets.h"
#include "gui_status_bar.h"
#include "gui_lock_device_widgets.h"

static lv_obj_t *g_hintBox = NULL;
static lv_obj_t **g_hintParam = NULL;
void UnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    }
}

void OpenImportWalletHandler(lv_event_t *e)
{
    static uint8_t walletMethod = WALLET_METHOD_IMPORT;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (CHECK_BATTERY_LOW_POWER()) {
            g_hintBox = GuiCreateErrorCodeHintbox(ERR_KEYSTORE_SAVE_LOW_POWER, &g_hintBox);
        } else {
            GuiFrameOpenViewWithParam(&g_createWalletView, &walletMethod, sizeof(walletMethod));
        }
    }
}

void OpenCreateWalletHandler(lv_event_t *e)
{
    static uint8_t walletMethod = WALLET_METHOD_CREATE;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (CHECK_BATTERY_LOW_POWER()) {
            g_hintBox = GuiCreateErrorCodeHintbox(ERR_KEYSTORE_SAVE_LOW_POWER, &g_hintBox);
        } else {
            GuiFrameOpenViewWithParam(&g_createWalletView, &walletMethod, sizeof(walletMethod));
        }
    }
}

void OpenViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenView(lv_event_get_user_data(e));
    }
}

void CloseTimerCurrentViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        CloseQRTimer();
        GuiCLoseCurrentWorkingView();
    }
}

void GoToHomeViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        CloseQRTimer();
        GuiCloseToTargetView(&g_homeView);
    }
}

void CloseCurrentViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiCLoseCurrentWorkingView();
    }
}

void ReturnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

void NextTileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    }
}

void CloseToTargetTileView(uint8_t currentIndex, uint8_t targetIndex)
{
    for (int i = currentIndex; i > targetIndex; i--) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

void CloseCurrentParentHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    }
}

void CloseParentAndNextHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        void **param = lv_event_get_user_data(e);
        if (param != NULL) {
            *param = NULL;
        }
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    }
}

void CloseCurrentUserDataHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiViewHintBoxClear();
        GuiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
    }
}

void CloseCurrentParentAndCloseViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        GuiCLoseCurrentWorkingView();
        GuiLockScreenFpRecognize();
        GuiLockScreenTurnOn(&single);
        ResetSuccess();
        GuiModelWriteLastLockDeviceTime(0);
    }
}

void CloseWaringPageHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_event_get_user_data(e));
        if (g_hintParam != NULL) {
            *g_hintParam = NULL;
        }
    }
}

void ToggleSwitchBoxHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *switchBox = lv_event_get_user_data(e);
        bool en = lv_obj_has_state(switchBox, LV_STATE_CHECKED);
        if (en) {
            lv_obj_clear_state(switchBox, LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(switchBox, LV_STATE_CHECKED);
        }
        lv_event_send(switchBox, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

void GuiWriteSeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTextLabel(parent, _("create_wallet_generating_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 403 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("create_wallet_generating_desc"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 457 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
}

void DuplicateShareHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiCLoseCurrentWorkingView();
        GuiCLoseCurrentWorkingView();
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        GuiViewHintBoxClear();
    }
}

void GuiViewHintBoxClear(void)
{
    if (g_hintBox != NULL) {
        GUI_DEL_OBJ(g_hintBox)
    }
}

void GuiDoNothingHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    }
}

void GuiWriteSeResult(bool en, int32_t errCode)
{
    GuiStopCircleAroundAnimation();
    if (en) {
        WalletDesc_t wallet = {
            .iconIndex = GuiGetEmojiIconIndex(),
        };
        GuiSetupKeyboardWidgetMode();
        SetStatusBarEmojiIndex(wallet.iconIndex);
        strcpy(wallet.name, GetCurrentKbWalletName());
        GuiNvsBarSetWalletName(GetCurrentKbWalletName());
        GuiNvsBarSetWalletIcon(GuiGetEmojiIconImg());
        GuiModelSettingSaveWalletDesc(&wallet);
        GuiCloseToTargetView(&g_initView);
        GuiFrameOpenViewWithParam(&g_lockView, NULL, 0);
        GuiLockScreenHidden();
        GuiFrameOpenView(&g_homeView);
        GuiUpdateOldAccountIndex();
    } else {
        int height = 370;
        lv_obj_t *desc = NULL;
        lv_obj_t *title = NULL;
        lv_event_cb_t cb = CloseCurrentUserDataHandler;
        char *titleText = (char *)_("error_box_invalid_seed_phrase");
        char *descText = (char *)_("error_box_invalid_seed_phrase_desc");
        switch (errCode) {
        case ERR_KEYSTORE_MNEMONIC_REPEAT:
            height = 400;
            titleText = _("error_box_duplicated_seed_phrase");
            descText = _("error_box_duplicated_seed_phrase_desc");
            cb = DuplicateShareHandler;
            break;
        case ERR_KEYSTORE_MNEMONIC_INVALID:
            break;
        case ERR_KEYSTORE_SAVE_LOW_POWER:
            height = 400;
            titleText = _("error_box_low_power");
            descText = _("error_box_low_power_desc");
            break;
        }

        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        g_hintBox = GuiCreateHintBox(lv_scr_act(), 480, height, false);
        lv_obj_t *btn = GuiCreateBtn(g_hintBox, _("OK"));
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

        desc = GuiCreateIllustrateLabel(g_hintBox, descText);
        lv_obj_align_to(desc, btn, LV_ALIGN_OUT_TOP_LEFT, -320, -40);

        title = GuiCreateLittleTitleLabel(g_hintBox, titleText);
        lv_obj_align_to(title, desc, LV_ALIGN_OUT_TOP_LEFT, 0, -12);

        lv_obj_t *img = GuiCreateImg(g_hintBox, &imgFailed);
        lv_obj_align_to(img, title, LV_ALIGN_OUT_TOP_LEFT, 0, -24);
    }
}

void *GuiCreateErrorCodeHintbox(int32_t errCode, lv_obj_t **param)
{
    g_hintParam = param;
    int height = 370;
    lv_obj_t *desc = NULL;
    lv_obj_t *title = NULL;
    char *titleText = (char *)_("error_box_invalid_seed_phrase");
    char *descText = (char *)_("error_box_invalid_seed_phrase_desc");
    switch (errCode) {
    case ERR_KEYSTORE_MNEMONIC_REPEAT:
        height = 400;
        titleText = _("error_box_duplicated_seed_phrase");
        descText = _("error_box_duplicated_seed_phrase_desc");
        break;
    case ERR_KEYSTORE_MNEMONIC_INVALID:
        break;
    case ERR_KEYSTORE_SAVE_LOW_POWER:
        height = 400;
        titleText = _("error_box_low_power");
        descText = _("error_box_low_power_desc");
        break;
    case ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET:
        height = 400;
        titleText = (char *)_("error_box_mnemonic_not_match_wallet");
        descText = (char *)_("error_box_mnemonic_not_match_wallet_desc");
        break;
    case ERR_UPDATE_FIRMWARE_NOT_DETECTED:
        height = 400;
        titleText = _("error_box_firmware_not_detected");
        descText = _("error_box_firmware_not_detected_desc");
        break;
    }

    lv_obj_t *cont = GuiCreateHintBox(lv_scr_act(), 480, height, false);
    lv_obj_t *btn = GuiCreateBtn(cont, _("OK"));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, CloseWaringPageHandler, LV_EVENT_CLICKED, cont);

    desc = GuiCreateIllustrateLabel(cont, descText);
    lv_obj_align_to(desc, btn, LV_ALIGN_OUT_TOP_LEFT, -320, -40);

    title = GuiCreateLittleTitleLabel(cont, titleText);
    lv_obj_align_to(title, desc, LV_ALIGN_OUT_TOP_LEFT, 0, -12);

    lv_obj_t *img = GuiCreateImg(cont, &imgFailed);
    lv_obj_align_to(img, title, LV_ALIGN_OUT_TOP_LEFT, 0, -24);

    return cont;
}
