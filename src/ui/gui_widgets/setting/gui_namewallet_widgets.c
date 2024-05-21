#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_enter_passcode.h"
#include "gui_model.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "keystore.h"
#include "gui_setting_widgets.h"
#include "gui_lock_widgets.h"
#include "bip39_english.h"
#include "bip39.h"
#include "slip39.h"
#include "version.h"
#include "presetting.h"
#include "assert.h"
#include "gui_qr_hintbox.h"

#include "gui_mnemonic_input.h"
#include "motor_manager.h"
#include "fingerprint_process.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "keystore.h"
#else
#include "simulator_model.h"
#include "simulator_mock_define.h"
#endif

static KeyBoard_t *g_setNameKb = NULL;         // setting keyboard
static lv_obj_t *g_walletIcon;                 // wallet icon

static void UpdateWalletDescHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        WalletDesc_t wallet = {
            .iconIndex = GuiSearchIconIndex(g_walletIcon),
        };
        GuiSetEmojiIconIndex(wallet.iconIndex);
        strcpy_s(wallet.name, WALLET_NAME_MAX_LEN + 1, lv_textarea_get_text(g_setNameKb->ta));
        GuiModelSettingSaveWalletDesc(&wallet);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        if (strlen(lv_textarea_get_text(g_setNameKb->ta)) > 0) {
            lv_obj_set_style_text_font(g_setNameKb->ta, &buttonFont, 0);
        } else {
            lv_obj_set_style_text_font(g_setNameKb->ta, g_defTextFont, 0);
        }
    }
}

static void GotoAddWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        lv_obj_add_flag(g_setNameKb->cont, LV_OBJ_FLAG_HIDDEN);
        const char *name = lv_textarea_get_text(g_setNameKb->ta);
        GuiNvsBarSetWalletName(name);
        GuiNvsBarSetWalletIcon(GuiGetEmojiIconImg());
        GuiFrameOpenView(&g_singlePhraseView);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        if (strlen(lv_textarea_get_text(g_setNameKb->ta)) > 0) {
            lv_obj_set_style_text_font(g_setNameKb->ta, &buttonFont, 0);
        } else {
            lv_obj_set_style_text_font(g_setNameKb->ta, g_defTextFont, 0);
        }
    }
}

static void OpenEmojiKbHandler(lv_event_t *e)
{
    GuiCreateEmojiKeyBoard(lv_scr_act(), g_walletIcon);
}

void GuiChangeWalletDesc(bool result)
{
    if (g_setNameKb != NULL && g_setNameKb->ta != NULL) {
        const char *name = lv_textarea_get_text(g_setNameKb->ta);
        GuiNvsBarSetWalletName(name);
        SetStatusBarEmojiIndex(GuiGetEmojiIconIndex());
        if (g_walletIcon != NULL) {
            GuiNvsBarSetWalletIcon(GuiGetEmojiIconImg());
        }

        GuiWalletSettingSetIconLabel(GuiGetEmojiIconImg(), name);
    }
    return GuiSettingCloseToTargetTileView(DEVICE_SETTING_WALLET_SETTING);
}

void *GuiWalletNameWallet(lv_obj_t *parent, uint8_t tile)
{
    lv_event_cb_t cb = NULL;
    if (tile == DEVICE_SETTING_CHANGE_WALLET_DESC) {
        cb = UpdateWalletDescHandler;
    } else if (tile == DEVICE_SETTING_ADD_WALLET_NAME_WALLET) {
        cb = GotoAddWalletHandler;
    }
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("single_backup_namewallet_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("single_backup_namewallet_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    g_setNameKb = GuiCreateFullKeyBoard(parent, cb, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_setNameKb, 0);
    lv_obj_set_size(g_setNameKb->ta, 300, 60);
    lv_obj_set_style_text_opa(g_setNameKb->ta, LV_OPA_100, 0);
    lv_obj_align(g_setNameKb->ta, LV_ALIGN_DEFAULT, 126, 320 - GUI_MAIN_AREA_OFFSET);
    lv_textarea_set_max_length(g_setNameKb->ta, 16);
    lv_textarea_set_placeholder_text(g_setNameKb->ta, _("single_backup_namewallet_previnput"));
    lv_obj_set_style_text_opa(g_setNameKb->ta, LV_OPA_50, LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_set_style_border_color(g_setNameKb->ta, ORANGE_COLOR, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_textarea_set_text(g_setNameKb->ta, GuiNvsBarGetWalletName());

    char tempBuf[BUFFER_SIZE_16] = {0};
    int len = strnlen_s(GuiNvsBarGetWalletName(), 17);
    snprintf_s(tempBuf, BUFFER_SIZE_32, "%d/16", len);
    if (len > 0) {
        lv_obj_set_style_text_font(g_setNameKb->ta, &buttonFont, 0);
    }
    GuiSetEmojiIconIndex(GUI_KEYBOARD_EMOJI_CANCEL_NEW_INDEX);
    lv_obj_t *progresslabel = GuiCreateNoticeLabel(parent, tempBuf);
    lv_obj_align(progresslabel, LV_ALIGN_DEFAULT, 402, 384 - GUI_MAIN_AREA_OFFSET);
    GuiSetEnterProgressLabel(progresslabel);
    lv_obj_t *img = GuiCreateImg(parent, GuiGetEmojiIconImg());
    g_walletIcon = img;

    lv_obj_t *arrowDownImg = GuiCreateImg(parent, &imgArrowDownS);

    GuiButton_t table[2] = {
        {.obj = img, .align = LV_ALIGN_LEFT_MID, .position = {15, 0}},
        {.obj = arrowDownImg, .align = LV_ALIGN_LEFT_MID, .position = {59, 0}}
    };
    lv_obj_t *button = GuiCreateButton(parent, 100, 70, table, NUMBER_OF_ARRAYS(table), OpenEmojiKbHandler, parent);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 24, 312 - GUI_MAIN_AREA_OFFSET);

    return progresslabel;
}

void GuiWalletNameWalletDestruct(void)
{
    lv_obj_del(g_setNameKb->cont);
    g_setNameKb = NULL;
    GuiDelEnterProgressLabel();
}
