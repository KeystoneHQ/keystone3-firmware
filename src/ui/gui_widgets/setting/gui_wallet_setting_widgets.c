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
#include "gui_keyboard_hintbox.h"
#include "version.h"
#include "presetting.h"
#include "assert.h"
#include "gui_qr_hintbox.h"
#include "firmware_update.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "keystore.h"
#else
#include "simulator_model.h"
#endif

typedef struct ContLabelWidget_t {
    lv_obj_t *label;
    lv_obj_t *cont;
} ContLabelWidget_t;
static ContLabelWidget_t g_waitAnimWidget;

static void RecoveryPassphraseHandler(lv_event_t *e);

static lv_obj_t *g_walletSetIcon = NULL;                // wallet setting icon
static lv_obj_t *g_walletSetLabel = NULL;               // wallet setting label
static lv_obj_t *g_mfpLabel = NULL;                     // wallet setting label
static KeyboardWidget_t *g_keyboardWidget = NULL;
static lv_obj_t *g_resetingCont = NULL;                 // resetting container
static char g_passCode[GUI_DEFINE_MAX_PASSCODE_LEN + 1];    // passcode
static GuiEnterPasscodeItem_t *g_repeatPassCode = NULL;
static GuiEnterPasscodeItem_t *g_setPassCode = NULL;
static bool g_delWalletStatus = false;                 // delete wallet status
static lv_timer_t *g_countDownTimer = NULL;           // count down timer
static uint8_t g_walletAmount;

void GuiAddWalletGetWalletAmount(uint8_t walletAmount)
{
    g_walletAmount = walletAmount + 1;
}

static void DelWalletConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_waitAnimWidget.cont = GuiCreateAnimHintBox(lv_scr_act(), 480, 278, 82);
        g_waitAnimWidget.label = GuiCreateTextLabel(g_waitAnimWidget.cont, _("Deleting"));
        lv_obj_align(g_waitAnimWidget.label, LV_ALIGN_BOTTOM_MID, 0, -76);
        GuiModelSettingDelWalletDesc();
    }
}

bool GuiSettingGetDeleteFlag(void)
{
    return g_delWalletStatus;
}

void GuiSettingAnimSetLabel(const char *text)
{
    g_waitAnimWidget.cont = GuiCreateAnimHintBox(lv_scr_act(), 480, 278, 82);
    g_waitAnimWidget.label = GuiCreateTextLabel(g_waitAnimWidget.cont, text);
    lv_obj_align(g_waitAnimWidget.label, LV_ALIGN_BOTTOM_MID, 0, -76);
    lv_obj_add_flag(g_waitAnimWidget.cont, LV_OBJ_FLAG_CLICKABLE);
}

void GuiSetPinDestruct(void *obj, void *param)
{
    if (g_setPassCode != NULL) {
        SRAM_FREE(g_setPassCode);
        g_setPassCode = NULL;
    }
}


void GuiSettingRecoveryCheck(void)
{
    g_waitAnimWidget.cont = GuiCreateAnimHintBox(lv_scr_act(), 480, 278, 82);
    g_waitAnimWidget.label = GuiCreateTextLabel(g_waitAnimWidget.cont, _("seed_check_wait_verify"));
    lv_obj_align(g_waitAnimWidget.label, LV_ALIGN_BOTTOM_MID, 0, -76);
}

void GuiWritePassphrase(bool result)
{
    // goto home
    // GuiStopCircleAryououndAnimation();
    GuiDeleteAnimHintBox();
    g_waitAnimWidget.cont = NULL;
    GuiSettingCloseToTargetTileView(DEVICE_SETTING_WALLET_SETTING);
}

void CountDownTimerHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    static int8_t countDown = 5;
    char buf[16] = {0};
    --countDown;
    if (countDown > 0) {
        sprintf(buf, "Got it (%d)", countDown);
    } else {
        strcpy(buf, "Got it");
    }
    lv_label_set_text(lv_obj_get_child(obj, 0), buf);
    if (countDown <= 0) {
        lv_obj_set_style_bg_opa(obj, LV_OPA_100, LV_PART_MAIN);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_timer_del(timer);
        countDown = 5;
        g_countDownTimer = NULL;
        UNUSED(g_countDownTimer);
    }
}

void GuiWalletSelectAddWallet(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("purpose_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    char tempBuf[16];
    sprintf(tempBuf, "#F5870A %d/3#", g_walletAmount);
    lv_obj_t *numLabel = GuiCreateTitleLabel(parent, tempBuf);
    lv_obj_align_to(numLabel, label, LV_ALIGN_OUT_RIGHT_MID, 36, 0);
    lv_label_set_recolor(numLabel, true);

    label = GuiCreateIllustrateLabel(parent, _("purpose_desc"));
    lv_obj_set_style_text_line_space(label, 12, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 324 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *img = GuiCreateImg(parent, &imgWallet);
    label = GuiCreateLittleTitleLabel(parent, _("purpose_new_wallet"));
    lv_obj_t *right = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[3] = {
        {
            .obj = img,
            .align = LV_ALIGN_DEFAULT,
            .position = {28, 40},
        },
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 40},
        },
        {
            .obj = right,
            .align = LV_ALIGN_DEFAULT,
            .position = {400, 42},
        },
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 120, table, NUMBER_OF_ARRAYS(table), OpenCreateWalletHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 324 - GUI_MAIN_AREA_OFFSET);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 443 - GUI_MAIN_AREA_OFFSET);

    img = GuiCreateImg(parent, &imgImport);
    label = GuiCreateTextLabel(parent, _("purpose_import_wallet"));
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);
    table[0].obj = img;
    table[0].position.x = -lv_obj_get_self_width(label) / 2 - 10;
    table[0].position.y = 0;
    table[0].align = LV_ALIGN_CENTER;

    table[1].obj = label;
    table[1].position.x = lv_obj_get_self_width(img) / 2 + 10;
    table[1].position.y = 0;
    table[1].align = LV_ALIGN_CENTER;
    button = GuiCreateButton(parent, 228, 50, table, 2, OpenImportWalletHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 684 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
}

void GuiWalletAddWalletNotice(lv_obj_t *parent)
{
    // static uint32_t walletSetting = DEVICE_SETTING_ADD_WALLET_SETPIN;
    static uint32_t walletSetting = DEVICE_SETTING_ADD_WALLET_CREATE_OR_IMPORT;
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("Notice"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *led = lv_led_create(parent);
    lv_led_set_brightness(led, 150);
    lv_obj_align(led, LV_ALIGN_DEFAULT, 36, 240 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_size(led, 6, 6);
    lv_led_set_color(led, ORANGE_COLOR);

    label = GuiCreateNoticeLabel(parent, _("Keystone support at most 3 wallets"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 228 - GUI_MAIN_AREA_OFFSET);

    led = lv_led_create(parent);
    lv_led_set_brightness(led, 150);
    lv_obj_align(led, LV_ALIGN_DEFAULT, 36, 282 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_size(led, 6, 6);
    lv_led_set_color(led, ORANGE_COLOR);

    label = GuiCreateNoticeLabel(parent, _("You should set a different passcode for each wallet"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 270 - GUI_MAIN_AREA_OFFSET);

    led = lv_led_create(parent);
    lv_led_set_brightness(led, 150);
    lv_obj_align(led, LV_ALIGN_DEFAULT, 36, 354 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_size(led, 6, 6);
    lv_led_set_color(led, ORANGE_COLOR);

    label = GuiCreateNoticeLabel(parent, _("For your security, we will not display the wallet list anywhere. The only way to switch wallets is to unlock the device with the corresponding passcode."));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 342 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *btn = GuiCreateBtn(parent, _("Got it (5)"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 710 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_bg_opa(btn, LV_OPA_60, LV_STATE_DEFAULT);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, WalletSettingHandler, LV_EVENT_CLICKED, &walletSetting);
    g_countDownTimer = lv_timer_create(CountDownTimerHandler, 1000, btn);
}

void GuiSettingCountDownDestruct(void *obj, void *param)
{
    if (g_countDownTimer != NULL) {
        lv_timer_del(g_countDownTimer);
        g_countDownTimer = NULL;
    }
}

void GuiWalletSetPinWidget(lv_obj_t *parent, uint8_t tile)
{
    static uint8_t currentTile = 0;
    currentTile = tile;
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    g_setPassCode = GuiCreateEnterPasscode(parent, NULL, &currentTile, ENTER_PASSCODE_SET_PIN);
    printf("%s %d passcode = %p\n", __func__, __LINE__, g_setPassCode);

    GuiWalletResetPassWordHintBox();
}

void GuiWalletSettingDeinit(void)
{
    g_delWalletStatus = false;
}

void GuiWalletSettingRefresh(void)
{
    if (g_setPassCode != NULL) {
        GuiUpdateEnterPasscodeParam(g_setPassCode, NULL);
    }
    if (g_repeatPassCode != NULL) {
        GuiUpdateEnterPasscodeParam(g_repeatPassCode, NULL);
    }
}

void GuiWalletRepeatPinWidget(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("repeat_passcode_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("repeat_passcode_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);

    g_repeatPassCode = GuiCreateEnterPasscode(parent, NULL, NULL, g_setPassCode->mode + 2);
    UNUSED(g_repeatPassCode);
}

void GuiRepeatDestruct(void *obj, void *param)
{
    static uint16_t currentTile = DEVICE_SETTING_RESET_PASSCODE_VERIFY;
    GuiDelEnterPasscode(g_repeatPassCode, NULL);
    g_repeatPassCode = NULL;
    if (g_setPassCode != NULL) {
        GuiUpdateEnterPasscodeParam(g_setPassCode, &currentTile);
    }
}

void GuiSettingSetPinPass(const char *buf)
{
    static uint8_t walletIndex = DEVICE_SETTING_RESET_PASSCODE_SETPIN;
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    strcpy(g_passCode, buf);
}

void GuiSettingRepeatPinPass(const char *buf)
{
    uint8_t accountIndex = 0;
    if (!strcmp(buf, g_passCode)) {
        GuiResettingWriteSe();
        SecretCacheSetNewPassword((char *)buf);
        GuiModelChangeAmountPassWord(accountIndex);
    } else {
        GuiEnterPassCodeStatus(g_repeatPassCode, false);
    }
}

void GuiDelWallet(bool result)
{
    GuiDeleteAnimHintBox();
    // g_waitAnimWidget.cont = NULL;
    GuiCLoseCurrentWorkingView();
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    LogoutCurrentAccount();
    GuiLockScreenSetFirstUnlock();
    GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_VERIFY);
    GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
}

void GuiChangePassWord(bool result)
{
    GuiStopCircleAroundAnimation();
    GUI_DEL_OBJ(g_resetingCont)
    if (result) {
        GuiEmitSignal(SIG_SETTING_PASSWORD_RESET_PASS, NULL, 0);
    }
}

void GuiResettingWriteSe(void)
{
    g_resetingCont = GuiCreateHintBox(lv_scr_act(), 480, 326, false);
    lv_obj_t *label = GuiCreateLittleTitleLabel(g_resetingCont, _("Resetting, Keep Screen ON"));
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -124);
    label = GuiCreateIllustrateLabel(g_resetingCont, _("Writing Secure Element..."));
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -76);
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);
    GuiCreateCircleAroundAnimation(lv_scr_act(), 165);
}

void GuiAddWalletAmountLimit(void)
{
    GuiShowKeyboardDestruct();
    GuiLockScreenTurnOff();
    static uint8_t walletIndex = DEVICE_SETTING_ADD_WALLET_LIMIT;
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
}

void GuiDelWalletSetup(void)
{
    // CloseToSubtopView();
    // GuiStopCircleAroundAnimation();
    GuiDeleteAnimHintBox();
    // g_waitAnimWidget.cont = NULL; //todo
    GuiCLoseCurrentWorkingView();
    // close to g_setupView
    GuiCloseToTargetView(&g_setupView);
    // close g_setupView
    GuiFrameCLoseView(&g_setupView);
    // open new g_setupView
    GuiFrameOpenView(&g_setupView);
}

void GuiShowKeyboardDestruct(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
}

void GuiShowKeyboardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        uint16_t *walletSetIndex = lv_event_get_user_data(e);
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        g_keyboardWidget = GuiCreateKeyboardWidget(GuiSettingGetCurrentCont());
        SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
        SetKeyboardWidgetSig(g_keyboardWidget, walletSetIndex);
    }
}

void GuiVerifyCurrentPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

void GuiSettingStructureCb(void *obj, void *param)
{
    char tempBuf[16] = "MFP: ";
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    for (int i = 0; i < sizeof(mfp); i++) {
        sprintf(&tempBuf[5 + i * 2], "%02X", mfp[i]);
    }
    memset(mfp, 0, sizeof(mfp));
    lv_label_set_text(g_mfpLabel, tempBuf);
}

void GuiWalletSettingSetIconLabel(const lv_img_dsc_t *src, const char *name)
{
    if (g_walletSetIcon != NULL) {
        lv_img_set_src(g_walletSetIcon, GuiGetEmojiIconImg());
    }

    if (g_walletSetLabel != NULL) {
        lv_label_set_text(g_walletSetLabel, name);
    }
}

// wallet setting
void GuiWalletSetWidget(lv_obj_t *parent)
{
    static uint32_t walletSetting[5] = {
        DEVICE_SETTING_CHANGE_WALLET_DESC,
        DEVICE_SETTING_FINGERPRINT_PASSCODE,
        DEVICE_SETTING_PASSPHRASE,
        DEVICE_SETTING_RECOVERY_PHRASE_VERIFY,
        DEVICE_SETTING_ADD_WALLET
    };
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    char tempBuf[16] = "MFP: ";
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    for (int i = 0; i < sizeof(mfp); i++) {
        sprintf(&tempBuf[5 + i * 2], "%02X", mfp[i]);
    }
    memset(mfp, 0, sizeof(mfp));
    lv_obj_t *label = GuiCreateTextLabel(parent, GuiNvsBarGetWalletName());
    lv_obj_set_style_text_font(label, &openSansButton, LV_PART_MAIN);
    lv_obj_t *mfpLabel = GuiCreateNoticeLabel(parent, tempBuf);
    g_mfpLabel = mfpLabel;
    g_walletSetLabel = label;
    lv_obj_t *img = GuiCreateImg(parent, GuiGetEmojiIconImg());
    g_walletSetIcon = img;
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgEdit);

    GuiButton_t table[4] = {
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 28},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_DEFAULT,
            .position = {411, 32},
        },
        {
            .obj = img,
            .align = LV_ALIGN_DEFAULT,
            .position = {27, 27},
        },
        {
            .obj = mfpLabel,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 64},
        },
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 145, table, NUMBER_OF_ARRAYS(table),
                                       WalletSettingHandler, &walletSetting[0]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 144 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 275 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateTextLabel(parent, _("wallet_setting_passcode"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[0].position.x = 24;
    table[0].position.y = 24;
    table[1].obj = imgArrow;
    table[1].position.x = 411;
    table[1].position.y = 32;
    button = GuiCreateButton(parent, 456, 84, table, 2, WalletSettingHandler, &walletSetting[1]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 287 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateTextLabel(parent, _("wallet_setting_passphrase"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[1].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 84, table, 2, WalletSettingHandler, &walletSetting[2]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 383 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateTextLabel(parent, _("wallet_setting_seed_phrase"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[1].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 84, table, 2, RecoveryPassphraseHandler, &walletSetting[3]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 479 - GUI_MAIN_AREA_OFFSET);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 577 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateTextLabel(parent, _("+ Add Wallet"));
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    table[0].obj = label;
    button = GuiCreateButton(parent, 456, 84, table, 1, GuiShowKeyboardHandler, &walletSetting[4]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 588 - GUI_MAIN_AREA_OFFSET);
}

void GuiSettingDestruct(void *obj, void *param)
{
    g_walletSetIcon = NULL;
    g_walletSetLabel = NULL;
    g_mfpLabel = NULL;
}

static void RecoveryPassphraseHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (PassphraseExist(GetCurrentAccountIndex()) == true) {
            static uint16_t recoveryMethod = DEVICE_SETTING_RECOVERY_METHOD_CHECK;
            GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &recoveryMethod, sizeof(recoveryMethod));
        } else {
            static uint16_t recoveryMethod = DEVICE_SETTING_RECOVERY_PHRASE_VERIFY;
            GuiDeleteKeyboardWidget(g_keyboardWidget);
            g_keyboardWidget = GuiCreateKeyboardWidget(GuiSettingGetCurrentCont());
            SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
            SetKeyboardWidgetSig(g_keyboardWidget, &recoveryMethod);
        }
    }
}

void GuiWalletDelWalletConfirm(lv_obj_t *parent)
{
    g_delWalletStatus = true;
    static uint16_t walletSetting = DEVICE_SETTING_RECOVERY_METHOD_CHECK;
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *img = GuiCreateImg(parent, &imgDelWallet);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 180 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *label = GuiCreateLittleTitleLabel(parent, _("Delete Wallet?"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 284 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateNoticeLabel(parent, _("To prevent you from losing your assets, it is recommended that you check the recovery backup before deleting it."));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 336 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *btn = GuiCreateBtn(parent, _("Seed Phrase Check"));
    lv_obj_set_size(btn, 319, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 580 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_font(lv_obj_get_child(btn, 0), g_defTextFont, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn, WalletSettingHandler, LV_EVENT_CLICKED, &walletSetting);

    btn = GuiCreateBtn(parent, _("Confirm Delete"));
    lv_obj_set_size(btn, 230, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 670 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_obj_get_child(btn, 0), RED_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(lv_obj_get_child(btn, 0), g_defTextFont, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, DelWalletConfirmHandler, LV_EVENT_CLICKED, NULL);
}