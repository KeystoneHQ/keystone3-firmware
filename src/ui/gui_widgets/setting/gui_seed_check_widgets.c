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
#include "presetting.h"
#include "assert.h"
#include "gui_qr_hintbox.h"
#include "motor_manager.h"
#include "fingerprint_process.h"
#include "gui_qrcode_widgets.h"
#include "gui_mnemonic_input.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "keystore.h"
#else
#include "simulator_model.h"
#endif

void RebootHandler(lv_event_t *e);
static void ImportPhraseWordsHandler(lv_event_t *e);
static void ImportShareNextSliceHandler(lv_event_t *e);

static lv_obj_t *g_buttonCont = NULL;           // next buton cont
static uint8_t g_inputWordsCnt = 0;
static MnemonicKeyBoard_t *g_recoveryMkb;       // recovery mnemonic keyboard
static KeyBoard_t *g_recoveryPhraseKb;          // recovery keyboard
static lv_obj_t *g_recoveryTitle; // recovery title label

void ResetSeedCheckImportHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ClearMnemonicKeyboard(g_recoveryMkb, &g_recoveryMkb->currentId);
        GuiClearKeyBoard(g_recoveryPhraseKb);
    }
}

void GuiWalletSeedCheckClearKb(void)
{
    ClearMnemonicKeyboard(g_recoveryMkb, &g_recoveryMkb->currentId);
    GuiClearMnemonicKeyBoard(g_recoveryMkb);
    GuiDelMnemonicKeyBoard(g_recoveryMkb);
}

void GuiWalletSeedCheckClearObject(void)
{
    CLEAR_OBJECT(g_recoveryMkb);
}

void GuiWalletRecoveryDestruct(void *obj, void *param)
{
    // todo
    lv_obj_add_flag(g_recoveryPhraseKb->cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag((lv_obj_t *)obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_buttonCont, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_del((lv_obj_t *)obj);
    g_recoveryPhraseKb = NULL;
    lv_obj_del(g_recoveryMkb->nextButton);
    g_buttonCont = NULL;
    // lv_obj_del(obj);
}

void GuiWalletRecoveryMethodCheck(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *button;

    if (PassphraseExist(GetCurrentAccountIndex()) == true) {
        lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 514);
        lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
        lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
        lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 156 - GUI_MAIN_AREA_OFFSET);

        lv_obj_t *imgLock = GuiCreateImg(cont, &imgWalletLock);
        lv_obj_align(imgLock, LV_ALIGN_TOP_MID, 0, 36);

        lv_obj_t *disableTitle = GuiCreateTextLabel(cont, _("Disable Passphrase Wallet"));
        lv_obj_align(disableTitle, LV_ALIGN_TOP_MID, 0, 140);
        lv_obj_set_style_text_align(disableTitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

        lv_obj_t *disableDesc = GuiCreateNoticeLabel(cont, "You are presently using a passphrase-protected wallet. Prior to proceeding with the seed phrase verification process, please restart your device without entering the passphrase.");
        lv_obj_align(disableDesc, LV_ALIGN_TOP_MID, 0, 188);
        lv_obj_set_style_text_align(disableDesc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_long_mode(disableDesc, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(disableDesc, 336);

        lv_obj_t *restartLabel = GuiCreateTextLabel(cont, _("Restart Now"));
        lv_obj_set_style_text_color(restartLabel, ORANGE_COLOR, LV_PART_MAIN);
        GuiButton_t restartTable[] = {
            {
                .obj = restartLabel,
                .align = LV_ALIGN_CENTER,
                .position = {0, 0},
            },
        };
        button = GuiCreateButton(cont, 198, 66, restartTable, NUMBER_OF_ARRAYS(restartTable), RebootHandler, NULL);
        lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -26);
        return;
    }
    static uint8_t walletSetting[2] = {
        DEVICE_SETTING_RECOVERY_SINGLE_PHRASE,
        DEVICE_SETTING_RECOVERY_SHARE_PHRASE
    };

    lv_obj_t *label = GuiCreateTextLabel(parent, _("seed_check_single_phrase"));
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[2] = {
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_LEFT_MID,
            .position = {411, 0},
        },
    };
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), OpenSinglePhraseHandler, &walletSetting[0]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 144 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateTextLabel(parent, _("seed_check_share_phrase"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[1].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), OpenSharePhraseHandler, &walletSetting[1]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 240 - GUI_MAIN_AREA_OFFSET);
}

void *GuiWalletRecoverySinglePhrase(lv_obj_t *parent, uint8_t wordAmount)
{
    lv_keyboard_user_mode_t kbMode = GuiGetMnemonicKbType(wordAmount);
    g_inputWordsCnt = wordAmount;

    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *label = GuiCreateTitleLabel(parent, _("seed_check_single_phrase_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 12);
    label = GuiCreateIllustrateLabel(parent, _("seed_check_share_phrase_title"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 72);

    g_recoveryMkb = GuiCreateMnemonicKeyBoard(parent, GuiMnemonicInputHandler, kbMode, NULL);
    g_recoveryMkb->intputType = MNEMONIC_INPUT_SETTING_VIEW;
    lv_obj_set_size(g_recoveryMkb->cont, 408, 236);
    lv_obj_align_to(g_recoveryMkb->cont, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 36);
    lv_btnmatrix_set_selected_btn(g_recoveryMkb->btnm, g_recoveryMkb->currentId);

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_t *btn = GuiCreateBtn(cont, "");
    lv_obj_t *img = GuiCreateImg(btn, &imgArrowNext);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    g_recoveryMkb->nextButton = btn;
    lv_obj_add_event_cb(btn, ImportPhraseWordsHandler, LV_EVENT_CLICKED, NULL);
    // g_recoveryPhraseKb.buttonCont = cont;
    cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 242);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    g_recoveryPhraseKb = GuiCreateLetterKeyBoard(cont, NULL, true, g_recoveryMkb);
    g_recoveryPhraseKb->btnm = g_recoveryMkb->btnm;
    g_recoveryMkb->letterKb = g_recoveryPhraseKb;

    return cont;
}

// share phrase
void *GuiWalletRecoverySharePhrase(lv_obj_t *parent, uint8_t wordAmount)
{
    g_inputWordsCnt = wordAmount;
    lv_keyboard_user_mode_t kbMode = GuiGetMnemonicKbType(wordAmount);

    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    g_recoveryMkb = GuiCreateMnemonicKeyBoard(parent, GuiMnemonicInputHandler, kbMode, NULL);
    g_recoveryMkb->intputType = MNEMONIC_INPUT_SETTING_VIEW;
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("import_wallet_ssb_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 12);
    lv_label_set_recolor(label, true);
    g_recoveryMkb->titleLabel = label;

    label = GuiCreateIllustrateLabel(parent, _("import_wallet_ssb_desc"));
    lv_obj_set_style_text_color(label, DARK_GRAY_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 72);
    lv_label_set_recolor(label, true);
    // lv_label_set_text_fmt(label, "Write down the #F5870A %d#-words seed phrase of your first share in the blanks below", g_inputWordsCnt);

    lv_obj_set_size(g_recoveryMkb->cont, 408, 236);
    lv_obj_align_to(g_recoveryMkb->cont, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 36);
    lv_btnmatrix_set_selected_btn(g_recoveryMkb->btnm, g_recoveryMkb->currentId);
    lv_label_set_text_fmt(g_recoveryTitle, "%s #F5870A %d#", _("import_wallet_ssb_title"), g_recoveryMkb->currentSlice + 1);

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_t *btn = GuiCreateBtn(cont, "");
    lv_obj_t *img = GuiCreateImg(btn, &imgArrowNext);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    g_buttonCont = cont;
    g_recoveryMkb->nextButton = btn;
    lv_obj_set_style_bg_opa(g_recoveryMkb->nextButton, LV_OPA_30, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, ImportShareNextSliceHandler, LV_EVENT_CLICKED, NULL);

    cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 242);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);

    g_recoveryPhraseKb = GuiCreateLetterKeyBoard(cont, NULL, false, g_recoveryMkb);
    g_recoveryPhraseKb->btnm = g_recoveryMkb->btnm;
    g_recoveryMkb->letterKb = g_recoveryPhraseKb;

    return cont;
}

static void ImportPhraseWordsHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ImportSinglePhraseWords(g_recoveryMkb, g_recoveryPhraseKb);
    }
}

static void ImportShareNextSliceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ImportShareNextSlice(g_recoveryMkb, g_recoveryPhraseKb);
    }
}
