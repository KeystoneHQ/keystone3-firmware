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
#include "gui_menmonic_test.h"
#include "presetting.h"
#include "assert.h"
#include "gui_qr_hintbox.h"
#include "firmware_update.h"
#include "gui_mnemonic_input.h"
#include "motor_manager.h"
#include "fingerprint_process.h"
#include "gui_keyboard_hintbox.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "keystore.h"
#else
#include "simulator_model.h"
#endif

#define DEVICE_SETTING_PASS_MAX_LEN 16
#define DEVICE_SETTING_RIGHT_LABEL_MAX_LEN DEVICE_SETTING_PASS_MAX_LEN
#define DEVICE_SETTING_MID_LABEL_MAX_LEN 32
#define DEVICE_SETTING_LEVEL_MAX 8

typedef void (*setting_update_cb)(void *obj, void *param);

typedef struct DeviceSettingWidget {
    uint8_t currentTile;                        // current x tile
    char pass[GUI_DEFINE_MAX_PASSCODE_LEN + 1]; // passcode
    lv_obj_t *cont;                             // setting container
    lv_obj_t *tileView;                         // setting tile view
} DeviceSettingWidget_t;
static DeviceSettingWidget_t g_deviceSetTileView;

typedef struct DeviceSettingItem {
    lv_obj_t *tile;                                      // setting item tile
    lv_obj_t *obj;                                       // setting item object
    char rightLabel[DEVICE_SETTING_RIGHT_LABEL_MAX_LEN]; // right label
    char midLabel[DEVICE_SETTING_MID_LABEL_MAX_LEN];     // middle label
    NVS_RIGHT_BUTTON_ENUM rightBtn;
    NVS_LEFT_BUTTON_ENUM leftBtn; // right button
    void *rightParam;             // right button param
    lv_event_cb_t rightCb;        // right button callback
    lv_event_cb_t leftCb;
    setting_update_cb destructCb;  // destruct callback
    setting_update_cb structureCb; // structure callback
} DeviceSettingItem_t;
static DeviceSettingItem_t g_deviceSettingArray[DEVICE_SETTING_LEVEL_MAX];

typedef struct PassphraseWidget {
    lv_obj_t *inputTa;
    lv_obj_t *repeatTA;
    lv_obj_t *errLabel;
    lv_obj_t *lenOverLabel;
} PassphraseWidget_t;
static PassphraseWidget_t g_passphraseWidget;

typedef struct ContLabelWidget_t {
    lv_obj_t *label;
    lv_obj_t *cont;
} ContLabelWidget_t;
static ContLabelWidget_t g_waitAnimWidget;

static KeyBoard_t *g_setPassPhraseKb = NULL;         // setting keyboard
static lv_obj_t *g_delWalletHintbox = NULL;    // del wallet hintbox
static lv_obj_t *g_selectAmountHintbox = NULL; // select amount hintbox
static lv_obj_t *g_noticeHintBox = NULL;       // notice hintbox
static bool g_delWalletStatus;                 // delete wallet status
static lv_timer_t *g_countDownTimer;           // count down timer
static lv_obj_t *g_resetingCont;               // resetting container
static lv_obj_t *g_walletSetIcon;              // wallet setting icon
static lv_obj_t *g_walletSetLabel;             // wallet setting label
static lv_obj_t *g_mfpLabel;                   // wallet setting label
static KeyBoard_t *g_recoveryPhraseKb;         // recovery keyboard
static MnemonicKeyBoard_t *g_recoveryMkb;      // recovery mnemonic keyboard
static uint8_t g_inputWordsCnt = 0;
static GuiEnterPasscodeItem_t *g_verifyCode;
static GuiEnterPasscodeItem_t *g_setPassCode;
static GuiEnterPasscodeItem_t *g_repeatPassCode;
static uint8_t g_walletAmount;
static lv_obj_t *g_recoveryTitle; // recovery title label
static lv_obj_t *g_buttonCont;    // next buton cont

static lv_obj_t *g_passphraseLearnMoreCont = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;

static void ImportPhraseWordsHandler(lv_event_t *e);
static lv_obj_t *g_passphraseQuickAccessSwitch;

static void PassphraseQuickAccessHandler(lv_event_t *e);
static bool ModelGetPassphraseQuickAccess(void);
static void ModelSetPassphraseQuickAccess(bool enable);
static void OpenPassphraseLearnMoreHandler(lv_event_t *e);
void RebootHandler(lv_event_t *e);
void GuiShowKeyboardHandler(lv_event_t *e);

void GuiSettingCloseToTargetTileView(uint8_t targetIndex)
{
    for (int i = g_deviceSetTileView.currentTile; i > targetIndex; i--) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

void WalletSettingHandler(lv_event_t *e)
{
    static uint8_t walletIndex = 0;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        uint8_t *walletSetIndex = lv_event_get_user_data(e);
        walletIndex = *walletSetIndex;
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    }
}

static void ResetClearImportHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ClearMnemonicKeyboard(g_recoveryMkb, &g_recoveryMkb->currentId);
        GuiClearKeyBoard(g_recoveryPhraseKb);
    }
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
            g_keyboardWidget = GuiCreateKeyboardWidget(lv_obj_get_parent(g_deviceSetTileView.cont));
            SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
            SetKeyboardWidgetSig(g_keyboardWidget, &recoveryMethod);
        }
    }
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
static void GuiWalletSetWidget(lv_obj_t *parent)
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

void GuiShowKeyboardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

     if (code == LV_EVENT_CLICKED) {
        uint16_t *walletSetIndex = lv_event_get_user_data(e);
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        g_keyboardWidget = GuiCreateKeyboardWidget(lv_obj_get_parent(g_deviceSetTileView.cont));
        SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
        SetKeyboardWidgetSig(g_keyboardWidget, walletSetIndex);
    }
}

static void GuiSetPinDestruct(void *obj, void *param)
{
    if (g_setPassCode != NULL) {
        SRAM_FREE(g_setPassCode);
        g_setPassCode = NULL;
    }
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

static void CloseToFingerAndPassView(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeHintBox)
        for (int i = g_deviceSetTileView.currentTile; i > 2; i--) {
            GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        }
    }
}

void CloseToSubtopViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        void **param = lv_event_get_user_data(e);
        if (param != NULL) {
            *param = NULL;
        }
        CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING_WALLET_SETTING);
    }
}

static void AddCloseToSubtopViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING_WALLET_SETTING);
    }
}

static void DelCurrCloseToSubtopViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(obj));
        if (g_delWalletStatus) {
            for (int i = g_deviceSetTileView.currentTile; i > 3; i--) {
                GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
            }
        } else {
            CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING_WALLET_SETTING);
            g_noticeHintBox = NULL;
        }
    }
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

static void SetKeyboardTaHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *ta = lv_event_get_user_data(e);
        if (ta == g_passphraseWidget.repeatTA) {
            GuiSetFullKeyBoardConfirm(g_setPassPhraseKb, true);
        } else {
            GuiSetFullKeyBoardConfirm(g_setPassPhraseKb, false);
        }
        GuiSetFullKeyBoardTa(g_setPassPhraseKb, ta);

        lv_obj_add_flag(g_passphraseWidget.lenOverLabel, LV_OBJ_FLAG_HIDDEN);
    } else if (code == KEY_STONE_KEYBOARD_CHANGE) {
        lv_keyboard_user_mode_t *keyMode = lv_event_get_param(e);
        g_setPassPhraseKb->mode = *keyMode;
        GuiKeyBoardSetMode(g_setPassPhraseKb);
    }
}

static void GuiWalletSetPinWidget(lv_obj_t *parent, uint8_t tile)
{
    static uint8_t currentTile = 0;
    currentTile = tile;
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label;

    g_setPassCode = GuiCreateEnterPasscode(parent, NULL, &currentTile, ENTER_PASSCODE_SET_PIN);
    printf("%s %d passcode = %p\n", __func__, __LINE__, g_setPassCode);

    g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 386, false);
    lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgWarn);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 36, 462);
    label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("change_passcode_warning_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 558);
    label = GuiCreateIllustrateLabel(g_noticeHintBox, _("change_passcode_warning_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 610);
    lv_obj_t *btn = GuiCreateBtn(g_noticeHintBox, _("Got it"));
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 332, 710);
    lv_obj_set_size(btn, 122, 66);
    lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
}

static void GuiWalletRepeatPinWidget(lv_obj_t *parent)
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

static void GuiWalletAddWalletNotice(lv_obj_t *parent)
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

static void GuiWalletSelectAddWallet(lv_obj_t *parent)
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

static void GuiWalletDelWalletConfirm(lv_obj_t *parent)
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

static void UpdatePassPhraseHandler(lv_event_t *e)
{
    static bool delayFlag = false;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        const char *input = lv_textarea_get_text(g_passphraseWidget.inputTa);
        const char *repeat = lv_textarea_get_text(g_passphraseWidget.repeatTA);
        if (!strcmp(input, repeat)) {
            SecretCacheSetPassphrase((char *)repeat);
            g_waitAnimWidget.cont = GuiCreateAnimHintBox(lv_scr_act(), 480, 278, 82);
            g_waitAnimWidget.label = GuiCreateTextLabel(g_waitAnimWidget.cont, _("seed_check_wait_verify"));
            lv_obj_align(g_waitAnimWidget.label, LV_ALIGN_BOTTOM_MID, 0, -76);
            lv_obj_add_flag(g_waitAnimWidget.cont, LV_OBJ_FLAG_CLICKABLE);
            GuiModelSettingWritePassphrase();
        } else {
            delayFlag = true;
            lv_obj_clear_flag(g_passphraseWidget.errLabel, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        Vibrate(SLIGHT);
        const char *intputText = lv_textarea_get_text(g_passphraseWidget.inputTa);
        if (!lv_obj_has_flag(g_passphraseWidget.errLabel, LV_OBJ_FLAG_HIDDEN)) {
            if (delayFlag == true) {
                delayFlag = false;
            } else {
                lv_obj_add_flag(g_passphraseWidget.errLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (lv_keyboard_get_textarea(lv_event_get_target(e)) == g_passphraseWidget.inputTa) {
            if (strlen(intputText) >= GUI_DEFINE_MAX_PASSCODE_LEN) {
                lv_obj_clear_flag(g_passphraseWidget.lenOverLabel, LV_OBJ_FLAG_HIDDEN);
            } else if (strlen(intputText) < GUI_DEFINE_MAX_PASSCODE_LEN) {
                lv_obj_add_flag(g_passphraseWidget.lenOverLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void GuiWalletPassphraseEnter(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_size(btn, 352, 60);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 26, 162 - GUI_MAIN_AREA_OFFSET);
    lv_obj_t *ta = lv_textarea_create(btn);
    lv_obj_set_align(ta, LV_ALIGN_CENTER);
    lv_obj_set_size(ta, 352, 60);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_placeholder_text(ta, _("Input passphrase"));
    lv_obj_set_style_bg_color(ta, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_color(ta, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(ta, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_text_font(ta, &openSans_24, LV_PART_MAIN);
    lv_obj_add_event_cb(ta, SetKeyboardTaHandler, LV_EVENT_ALL, ta);
    lv_obj_t *img = GuiCreateImg(parent, &imgEyeOff);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 411, 168 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, ta);
    lv_textarea_set_max_length(ta, GUI_DEFINE_MAX_PASSCODE_LEN);
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    g_passphraseWidget.inputTa = ta;

    btn = lv_btn_create(parent);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_size(btn, 352, 60);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 26, 246 - GUI_MAIN_AREA_OFFSET);
    lv_obj_t *repeatTa = lv_textarea_create(btn);
    lv_obj_set_align(repeatTa, LV_ALIGN_CENTER);
    lv_obj_set_size(repeatTa, 352, 60);
    lv_textarea_set_password_mode(repeatTa, true);
    lv_textarea_set_placeholder_text(repeatTa, _("Repeat passphrase"));
    lv_obj_set_style_bg_color(repeatTa, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_color(repeatTa, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(repeatTa, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_text_font(repeatTa, &openSans_24, LV_PART_MAIN);
    lv_textarea_set_max_length(repeatTa, GUI_DEFINE_MAX_PASSCODE_LEN);
    lv_textarea_set_one_line(repeatTa, true);
    lv_obj_set_scrollbar_mode(repeatTa, LV_SCROLLBAR_MODE_OFF);

    lv_obj_add_event_cb(repeatTa, SetKeyboardTaHandler, LV_EVENT_ALL, repeatTa);
    img = GuiCreateImg(parent, &imgEyeOff);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 411, 252 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, repeatTa);
    g_passphraseWidget.repeatTA = repeatTa;

    g_setPassPhraseKb = GuiCreateFullKeyBoard(parent, UpdatePassPhraseHandler, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_setPassPhraseKb, 0);
    GuiSetFullKeyBoardTa(g_setPassPhraseKb, ta);
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("Passphrase does not match"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 304 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_passphraseWidget.errLabel = label;

    label = GuiCreateIllustrateLabel(parent, _("input length cannot exceed 128 characters"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 415 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_passphraseWidget.lenOverLabel = label;
}

static void CloseCurrentPage(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_passphraseLearnMoreCont)
        lv_obj_clear_flag(g_deviceSettingArray[g_deviceSetTileView.currentTile].tile, LV_OBJ_FLAG_HIDDEN);
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
        GuiNvsBarSetRightCb(NVS_BAR_QUESTION_MARK, OpenPassphraseLearnMoreHandler, NULL);
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, _("wallet_setting_passphrase"));
    }
}

static void GuiPassphraseOpenQRCodeHintBox()
{
    GuiQRCodeHintBoxOpen("https://keyst.one/t/3rd/passphrase", "What is Passphrase", "https://keyst.one/t/3rd/passphrase");
}

static void OpenPassphraseQrCodeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiPassphraseOpenQRCodeHintBox();
    }
}

static void GuiOpenPassphraseLearnMore()
{
    lv_obj_add_flag(g_deviceSettingArray[g_deviceSetTileView.currentTile].tile, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);
    lv_obj_t *label = GuiCreateTextLabel(cont, _("passphrase_learn_more_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *led = lv_led_create(cont);
    lv_led_set_brightness(led, 150);
    lv_obj_align(led, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_size(led, 12, 12);
    lv_led_set_color(led, ORANGE_COLOR);
    label = GuiCreateNoticeLabel(cont, _("passphrase_learn_more_desc1"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 204 - GUI_MAIN_AREA_OFFSET);

    led = lv_led_create(cont);
    lv_led_set_brightness(led, 150);
    lv_obj_align(led, LV_ALIGN_DEFAULT, 36, 288 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_size(led, 12, 12);
    lv_led_set_color(led, ORANGE_COLOR);
    label = GuiCreateNoticeLabel(cont, _("passphrase_learn_more_desc2"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 276 - GUI_MAIN_AREA_OFFSET);

    led = lv_led_create(cont);
    lv_led_set_brightness(led, 150);
    lv_obj_align(led, LV_ALIGN_DEFAULT, 36, 360 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_size(led, 12, 12);
    lv_led_set_color(led, ORANGE_COLOR);
    label = GuiCreateNoticeLabel(cont, _("passphrase_learn_more_desc3"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 348 - GUI_MAIN_AREA_OFFSET);

    led = lv_led_create(cont);
    lv_led_set_brightness(led, 150);
    lv_obj_align(led, LV_ALIGN_DEFAULT, 36, 432 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_size(led, 12, 12);
    lv_led_set_color(led, ORANGE_COLOR);
    label = GuiCreateNoticeLabel(cont, _("passphrase_learn_more_desc4"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 52, 420 - GUI_MAIN_AREA_OFFSET);

    g_passphraseLearnMoreCont = cont;

    cont = GuiCreateContainerWithParent(g_passphraseLearnMoreCont, 144, 30);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 492 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, OpenPassphraseQrCodeHandler, LV_EVENT_CLICKED, NULL);

    label = GuiCreateIllustrateLabel(cont, _("passphrase_learn_more_link"));
    lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 0, 0);

    lv_obj_t *img = GuiCreateImg(cont, &imgQrcodeTurquoise);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 120, 3);

    GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, CloseCurrentPage, NULL);
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "");

}

static void OpenPassphraseLearnMoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiOpenPassphraseLearnMore();
    }
}

static void GuiWalletPassphrase(lv_obj_t *parent)
{
    static uint16_t walletSetting = DEVICE_SETTING_PASSPHRASE_VERIFY;

    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *label = GuiCreateTextLabel(parent, _("passphrase_enter_passcode"));
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[3] = {
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_LEFT_MID,
            .position = {376, 0},
        },
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, 2, GuiShowKeyboardHandler, &walletSetting);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 144 - GUI_MAIN_AREA_OFFSET);

    g_passphraseQuickAccessSwitch = lv_switch_create(parent);
    lv_obj_set_style_bg_color(g_passphraseQuickAccessSwitch, ORANGE_COLOR, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_passphraseQuickAccessSwitch, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_passphraseQuickAccessSwitch, LV_OPA_30, LV_PART_MAIN);
    if (ModelGetPassphraseQuickAccess()) {
        lv_obj_add_state(g_passphraseQuickAccessSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_passphraseQuickAccessSwitch, LV_STATE_CHECKED);
    }
    lv_obj_clear_flag(g_passphraseQuickAccessSwitch, LV_OBJ_FLAG_CLICKABLE);
    label = GuiCreateTextLabel(parent, _("passphrase_access_switch_title"));
    lv_obj_t *descLabel = GuiCreateIllustrateLabel(parent, _("passphrase_access_switch_desc"));
    lv_obj_set_style_text_opa(descLabel, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_width(descLabel, 336);
    table[0].obj = label;
    table[0].align = LV_ALIGN_DEFAULT;
    table[0].position.x = 24;
    table[0].position.y = 24;
    table[1].obj = g_passphraseQuickAccessSwitch;
    table[1].align = LV_ALIGN_DEFAULT;
    table[1].position.x = 376;
    table[1].position.y = 24;
    table[2].obj = descLabel;
    table[2].align = LV_ALIGN_DEFAULT;
    table[2].position.x = 24;
    table[2].position.y = 64;
    button = GuiCreateButton(parent, 456, 148, table, NUMBER_OF_ARRAYS(table), PassphraseQuickAccessHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 254 - GUI_MAIN_AREA_OFFSET);
}

void GuiSettingFullKeyBoardDestruct(void *obj, void *param)
{
    GuiWalletNameWalletDestruct();
}

static void GuiWalletAddLimit(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("Add Limited"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("You can only add up to a maximum of 3 wallets. Please delete other wallets before adding a new wallet."));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *btn = GuiCreateBtn(parent, _("Got it"));
    lv_obj_set_size(btn, 348, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 710 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, AddCloseToSubtopViewHandler, LV_EVENT_CLICKED, NULL);
}

static void UnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    }
}

static void AboutHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenView(&g_aboutView);
    }
}

static void GuiSettingEntranceWidget(lv_obj_t *parent)
{
    static uint32_t walletSetting[4] = {
        DEVICE_SETTING_WALLET_SETTING,
        DEVICE_SETTING_SYSTEM_SETTING,
        DEVICE_SETTING_CONNECT,
        DEVICE_SETTING_ABOUT
    };

    lv_obj_t *label = GuiCreateTextLabel(parent, _("device_setting_wallet_setting_title"));
    lv_obj_t *labelDesc = GuiCreateNoticeLabel(parent, _("device_setting_wallet_setting_desc"));
    lv_obj_t *img = GuiCreateImg(parent, &imgWalletSetting);
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[4] = {
        {
            .obj = img,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 24},
        },
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 24},
        },
        {
            .obj = labelDesc,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 64},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_DEFAULT,
            .position = {396, 24},
        },
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table),
                                       WalletSettingHandler, &walletSetting[0]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 144 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 274 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateTextLabel(parent, _("device_setting_system_setting_title"));
    labelDesc = GuiCreateNoticeLabel(parent, _("device_setting_system_setting_desc"));
    img = GuiCreateImg(parent, &imgSystemSetting);
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = img;
    table[1].obj = label;
    table[2].obj = labelDesc;
    table[3].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table), UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 287 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_event_cb(button, OpenViewHandler, LV_EVENT_CLICKED, &g_systemSettingView);

    label = GuiCreateTextLabel(parent, _("device_setting_connection_title"));
    labelDesc = GuiCreateNoticeLabel(parent, _("device_setting_connection_desc"));
    img = GuiCreateImg(parent, &imgConnection);
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = img;
    table[1].obj = label;
    table[2].obj = labelDesc;
    table[3].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table), UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 413 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_event_cb(button, OpenViewHandler, LV_EVENT_CLICKED, &g_connectionView);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 543 - GUI_MAIN_AREA_OFFSET);

    char showString[64] = {0};
    char version[16] = {0};
    char fileVersion[16] = {0};
    GetSoftWareVersionNumber(version);
    if (CheckOtaBinVersion(fileVersion)) {
        // sprintf(showString, "#8E8E8E v%s# / #F5870A v%sAvailable#", version, fileVersion);
        sprintf(showString, "#8E8E8E %s#", version);
    } else {
        sprintf(showString, "#8E8E8E %s#", version);
    }

    label = GuiCreateTextLabel(parent, _("device_setting_about_title"));
    labelDesc = GuiCreateIllustrateLabel(parent, showString);
    lv_label_set_recolor(labelDesc, true);
    img = GuiCreateImg(parent, &imgAbout);
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = img;
    table[1].obj = label;
    table[2].obj = labelDesc;
    table[3].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table), AboutHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 556 - GUI_MAIN_AREA_OFFSET);
}

static void SelectPhraseAmountHandler(lv_event_t *e)
{
    static uint8_t walletIndex = 0;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_selectAmountHintbox)
        uint8_t *walletSetIndex = lv_event_get_user_data(e);
        walletIndex = *walletSetIndex;
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    }
}
void GuiSettingCloseSelectAmountHintBox()
{
    GUI_DEL_OBJ(g_selectAmountHintbox)
}
// open select cont
void OpenSinglePhraseHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        static uint8_t walletSetting[3] = {
            DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_12WORDS,
            DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_18WORDS,
            DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_24WORDS,
        };
        g_selectAmountHintbox = GuiCreateHintBox(lv_scr_act(), 480, 378, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_selectAmountHintbox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_selectAmountHintbox);
        lv_obj_t *label = GuiCreateTextLabel(g_selectAmountHintbox, _("Seed Phrase Word Count"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 30, 451);
        lv_obj_t *img = GuiCreateImg(g_selectAmountHintbox, &imgClose);
        GuiButton_t table[2] = {
            {
                .obj = img,
                .align = LV_ALIGN_CENTER,
                .position = {0, 0},
            },
        };
        lv_obj_t *btn = GuiCreateButton(g_selectAmountHintbox, 36, 36, table, 1, CloseHintBoxHandler, &g_selectAmountHintbox);
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 408, 449);

        label = GuiCreateTextLabel(g_selectAmountHintbox, _("import_wallet_phrase_12words"));
        img = GuiCreateImg(g_selectAmountHintbox, &imgArrowRight);
        table[0].obj = label;
        table[0].position.x = 24;
        table[0].position.y = 0;
        table[0].align = LV_ALIGN_LEFT_MID;
        table[1].obj = img;
        table[1].position.x = 411;
        table[1].position.y = 0;
        table[1].align = LV_ALIGN_LEFT_MID;

        btn = GuiCreateButton(g_selectAmountHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table), SelectPhraseAmountHandler, &walletSetting[0]);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 510);

        label = GuiCreateTextLabel(g_selectAmountHintbox, _("import_wallet_phrase_18words"));
        img = GuiCreateImg(g_selectAmountHintbox, &imgArrowRight);
        table[0].obj = label;
        table[1].obj = img;
        btn = GuiCreateButton(g_selectAmountHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table), SelectPhraseAmountHandler, &walletSetting[1]);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 608);

        label = GuiCreateTextLabel(g_selectAmountHintbox, _("import_wallet_phrase_24words"));
        img = GuiCreateImg(g_selectAmountHintbox, &imgArrowRight);
        table[0].obj = label;
        table[1].obj = img;
        btn = GuiCreateButton(g_selectAmountHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table), SelectPhraseAmountHandler, &walletSetting[2]);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 704);
    }
}

// share phrase
void OpenSharePhraseHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        static uint8_t walletSetting[2] = {
            DEVICE_SETTING_RECOVERY_SHARE_PHRASE_20WORDS,
            DEVICE_SETTING_RECOVERY_SHARE_PHRASE_33WORDS
        };
        g_selectAmountHintbox = GuiCreateHintBox(lv_scr_act(), 480, 282, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_selectAmountHintbox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_selectAmountHintbox);
        lv_obj_t *label = GuiCreateTextLabel(g_selectAmountHintbox, _("Seed Phrase Word Count"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 30, 546);
        lv_obj_t *img = GuiCreateImg(g_selectAmountHintbox, &imgClose);
        GuiButton_t table[2] = {
            {
                .obj = img,
                .align = LV_ALIGN_CENTER,
                .position = {0, 0},
            },
        };
        lv_obj_t *btn = GuiCreateButton(g_selectAmountHintbox, 36, 36, table, 1, CloseHintBoxHandler, &g_selectAmountHintbox);
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 408, 545);

        label = GuiCreateTextLabel(g_selectAmountHintbox, _("import_wallet_phrase_20words"));
        img = GuiCreateImg(g_selectAmountHintbox, &imgArrowRight);
        table[0].obj = label;
        table[0].position.x = 24;
        table[0].position.y = 0;
        table[0].align = LV_ALIGN_LEFT_MID;
        table[1].obj = img;
        table[1].position.x = 411;
        table[1].position.y = 0;
        table[1].align = LV_ALIGN_LEFT_MID;

        btn = GuiCreateButton(g_selectAmountHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table), SelectPhraseAmountHandler, &walletSetting[0]);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 607);

        label = GuiCreateTextLabel(g_selectAmountHintbox, _("import_wallet_phrase_33words"));
        img = GuiCreateImg(g_selectAmountHintbox, &imgArrowRight);
        table[0].obj = label;
        table[1].obj = img;
        btn = GuiCreateButton(g_selectAmountHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table), SelectPhraseAmountHandler, &walletSetting[1]);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 703);
    }
}

// recovery method
static void GuiWalletRecoveryMethodCheck(lv_obj_t *parent)
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

static void *GuiWalletRecoverySinglePhrase(lv_obj_t *parent, uint8_t wordAmount)
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
    g_recoveryMkb->letterKb = g_recoveryPhraseKb;

    return cont;
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

// share phrase
static void *GuiWalletRecoverySharePhrase(lv_obj_t *parent, uint8_t wordAmount)
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
    lv_label_set_text_fmt(label, "Write down the #F5870A %d#-words seed phrase of your first share in the blanks below", g_inputWordsCnt);

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
    g_recoveryMkb->letterKb = g_recoveryPhraseKb;

    return cont;
}

static void DelWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        g_delWalletHintbox = NULL;
        GuiShowKeyboardHandler(e);
    }
}

static void OpenDelWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    static uint8_t walletIndex = DEVICE_SETTING_DEL_WALLET;

    if (code == LV_EVENT_CLICKED) {
        g_delWalletHintbox = GuiCreateHintBox(lv_event_get_user_data(e), 480, 132, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_delWalletHintbox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_delWalletHintbox);
        lv_obj_t *label = GuiCreateTextLabel(g_delWalletHintbox, _("Delete Wallet"));
        lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
        lv_obj_t *img = GuiCreateImg(g_delWalletHintbox, &imgDel);
        GuiButton_t table[2] = {
            {
                .obj = img,
                .align = LV_ALIGN_LEFT_MID,
                .position = {24, 0},
            },
            {
                .obj = label,
                .align = LV_ALIGN_LEFT_MID,
                .position = {88, 0},
            },
        };
        lv_obj_t *btn = GuiCreateButton(g_delWalletHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                        DelWalletHandler, &walletIndex);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 692);
    }
}

void GuiWalletRecoveryWriteSe(bool result)
{
    GuiDeleteAnimHintBox();
    lv_obj_t *label;
    lv_obj_t *btn;
    lv_obj_t *img;
    if (result) {
        ClearMnemonicKeyboard(g_recoveryMkb, &g_recoveryMkb->currentId);
        GuiClearMnemonicKeyBoard(g_recoveryMkb);
        GuiDelMnemonicKeyBoard(g_recoveryMkb);

        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
        img = GuiCreateImg(g_noticeHintBox, &imgSuccess);
        lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 36, -236);
        label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("shamir_phrase_verify_success_title"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -172);
        label = GuiCreateNoticeLabel(g_noticeHintBox, _("Your seed has been validated and successfully verified."));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -110);

        btn = GuiCreateBtn(g_noticeHintBox, _("Done"));
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 332, 710);
        lv_obj_set_size(btn, 122, 66);
        lv_obj_add_event_cb(btn, DelCurrCloseToSubtopViewHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
    } else {
        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
        img = GuiCreateImg(g_noticeHintBox, &imgFailed);
        lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 36, -236);
        label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("Verification Unsuccessful"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -172);
        label = GuiCreateIllustrateLabel(g_noticeHintBox, _("seed_check_verify_not_match_desc"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -110);
        lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);

        btn = GuiCreateBtn(g_noticeHintBox, _("Done"));
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 332, 710);
        lv_obj_set_size(btn, 122, 66);
        lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
    }
}

void GuiDevSettingPassCode(bool result, uint8_t tileIndex)
{
    static uint8_t walletIndex = DEVICE_SETTING_RESET_PASSCODE_VERIFY;
    switch (tileIndex) {
    case DEVICE_SETTING_FINGER_SETTING:
        walletIndex = GuiGetFingerSettingIndex();
        break;
    case DEVICE_SETTING_RESET_PASSCODE:
        walletIndex = DEVICE_SETTING_RESET_PASSCODE_VERIFY;
        break;
    case DEVICE_SETTING_ADD_WALLET:
        walletIndex = DEVICE_SETTING_ADD_WALLET_NOTICE;
        break;
    case DEVICE_SETTING_DEL_WALLET:
        walletIndex = DEVICE_SETTING_DEL_WALLET_VERIFY;
        break;
    case DEVICE_SETTING_PASSPHRASE_VERIFY:
        walletIndex = DEVICE_SETTING_PASSPHRASE_ENTER;
        break;
    case DEVICE_SETTING_RECOVERY_PHRASE_VERIFY:
        walletIndex = DEVICE_SETTING_RECOVERY_METHOD_CHECK;
        break;
    default:
        if (result)
            return;
    }

    if (result) {
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    } 
}

void GuiSettingSetPinPass(const char *buf)
{
    static uint8_t walletIndex = DEVICE_SETTING_RESET_PASSCODE_SETPIN;
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    strcpy(g_deviceSetTileView.pass, buf);
}

static void GuiResettingWriteSe(void)
{
    g_resetingCont = GuiCreateHintBox(lv_scr_act(), 480, 326, false);
    lv_obj_t *label = GuiCreateLittleTitleLabel(g_resetingCont, _("Resetting, Keep Screen ON"));
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -124);
    label = GuiCreateIllustrateLabel(g_resetingCont, _("Writing Secure Element..."));
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -76);
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);
    GuiCreateCircleAroundAnimation(lv_scr_act(), 165);
}

void GuiSettingRepeatPinPass(const char *buf)
{
    uint8_t accountIndex = 0;
    if (!strcmp(buf, g_deviceSetTileView.pass)) {
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
    g_waitAnimWidget.cont = NULL;
    GuiCLoseCurrentWorkingView();
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    LogoutCurrentAccount();
    GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_VERIFY);
    GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
}

void GuiDelWalletSetup(void)
{
    // CloseToSubtopView();
    // GuiStopCircleAroundAnimation();
    GuiDeleteAnimHintBox();
    g_waitAnimWidget.cont = NULL;
    GuiCLoseCurrentWorkingView();
    // close to g_setupView
    GuiCloseToTargetView(&g_setupView);
    // close g_setupView
    GuiFrameCLoseView(&g_setupView);
    // open new g_setupView
    GuiFrameOpenView(&g_setupView);
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetLeftCb(NVS_LEFT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "");
}

void GuiAddWalletCreateOrImport(uint8_t walletAmount)
{
    g_walletAmount = walletAmount;
    static uint8_t walletIndex = DEVICE_SETTING_ADD_WALLET_CREATE_OR_IMPORT;
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
}

void GuiAddWalletGetWalletAmount(uint8_t walletAmount)
{
    g_walletAmount = walletAmount + 1;
}

void GuiChangePassWord(bool result)
{
    GuiStopCircleAroundAnimation();
    GUI_DEL_OBJ(g_resetingCont)
    if (result) {
        GuiEmitSignal(SIG_SETTING_PASSWORD_RESET_PASS, NULL, 0);
    }
}

void GuiResettingPassWordSuccess(void)
{
    g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgSuccess);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 36, 492);
    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("change_passcode_reset_success_title"));
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -170);
    label = GuiCreateIllustrateLabel(g_noticeHintBox, _("change_passcode_reset_success_desc"));
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -130);
    lv_obj_t *btn = GuiCreateBtn(g_noticeHintBox, _("Done"));
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 332, 710);
    lv_obj_set_size(btn, 122, 66);
    lv_obj_add_event_cb(btn, CloseToFingerAndPassView, LV_EVENT_CLICKED, g_noticeHintBox);
}

void GuiAddWalletAmountLimit(void)
{
    GuiLockScreenTurnOff();
    static uint8_t walletIndex = DEVICE_SETTING_ADD_WALLET_LIMIT;
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
}

void GuiWritePassphrase(bool result)
{
    // goto home
    // GuiStopCircleAryououndAnimation();
    GuiDeleteAnimHintBox();
    g_waitAnimWidget.cont = NULL;
    CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING_WALLET_SETTING);
}

void GuiSettingInit(void)
{
    CLEAR_OBJECT(g_deviceSetTileView);
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);

    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, DEVICE_SETTING, 0, LV_DIR_HOR);
    GuiSettingEntranceWidget(tile);

    g_deviceSetTileView.currentTile = DEVICE_SETTING;
    g_deviceSetTileView.tileView = tileView;
    g_deviceSetTileView.cont = cont;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].tile = tile;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].obj = NULL;
    strcpy(g_deviceSettingArray[g_deviceSetTileView.currentTile].midLabel, _("device_setting_mid_btn"));
    g_deviceSettingArray[g_deviceSetTileView.currentTile].destructCb = NULL;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].structureCb = NULL;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].rightBtn = NVS_RIGHT_BUTTON_BUTT;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].leftBtn = NVS_BAR_RETURN;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].leftCb = ReturnHandler;

    lv_obj_set_tile_id(g_deviceSetTileView.tileView, g_deviceSetTileView.currentTile, 0, LV_ANIM_OFF);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, g_deviceSettingArray[g_deviceSetTileView.currentTile].midLabel);
}

void GuiSettingDeInit(void)
{
    g_delWalletStatus = false;
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    GUI_DEL_OBJ(g_delWalletHintbox)
    GUI_DEL_OBJ(g_noticeHintBox)
    GUI_DEL_OBJ(g_selectAmountHintbox)
    GUI_DEL_OBJ(g_passphraseLearnMoreCont)
    GuiFpVerifyDestruct();
    // if (g_recoveryMkb->cont != NULL) {
    //     GUI_DEL_OBJ(g_recoveryMkb->cont)
    // }
    CLEAR_OBJECT(g_recoveryMkb);
    GuiMnemonicHintboxClear();
    CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING);
    lv_obj_del(g_deviceSetTileView.tileView);
    lv_obj_del(g_deviceSetTileView.cont);
    CLEAR_OBJECT(g_deviceSetTileView);
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }
}

int8_t GuiDevSettingNextTile(uint8_t tileIndex)
{
    lv_obj_t *tile = NULL;
    static uint8_t currTileIndex = DEVICE_SETTING;
    char rightLabel[16] = {0};
    char midLabel[32] = {0};
    lv_event_cb_t rightCb = NULL;
    lv_event_cb_t leftCb = ReturnHandler;
    NVS_RIGHT_BUTTON_ENUM rightBtn = NVS_RIGHT_BUTTON_BUTT;
    NVS_LEFT_BUTTON_ENUM leftBtn = NVS_BAR_RETURN;
    void *obj = NULL;
    setting_update_cb destructCb = NULL;
    setting_update_cb structureCb = NULL;
    uint8_t currentTile = g_deviceSetTileView.currentTile;
    currentTile++;
    switch (tileIndex) {
    case DEVICE_SETTING_WALLET_SETTING:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletSetWidget(tile);
        strcpy(midLabel, _("wallet_settings_mid_btn"));
        rightBtn = NVS_BAR_MORE_INFO;
        rightCb = OpenDelWalletHandler;
        destructCb = GuiSettingDestruct;
        structureCb = GuiSettingStructureCb;
        break;
    case DEVICE_SETTING_CHANGE_WALLET_DESC:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        currTileIndex = DEVICE_SETTING_CHANGE_WALLET_DESC;
        obj = GuiWalletNameWallet(tile, currTileIndex);
        destructCb = GuiSettingFullKeyBoardDestruct;
        break;

    case DEVICE_SETTING_FINGERPRINT_PASSCODE:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletSetFingerPassCodeWidget(tile);
        strcpy(midLabel, _("fingerprint_passcode_mid_btn"));
        break;

    // finger module
    case DEVICE_SETTING_FINGER_MANAGER:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletFingerManagerWidget(tile);
        strcpy(midLabel, "Fingerprint Settings");
        structureCb = GuiFingerMangerStructureCb;
        destructCb = GuiFingerManagerDestruct;
        break;
    case DEVICE_SETTING_FINGER_ADD_ENTER:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        leftBtn = NVS_BAR_CLOSE;
        leftCb = CancelCurFingerHandler;
        GuiWalletFingerAddWidget(tile);
        destructCb = GuiFingerAddDestruct;
        break;
    case DEVICE_SETTING_FINGER_ADD_SUCCESS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletFingerAddFpWidget(tile, true);
        leftBtn = NVS_LEFT_BUTTON_BUTT;
        break;
    case DEVICE_SETTING_FINGER_ADD_OUT_LIMIT:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletFingerAddFpWidget(tile, false);
        leftBtn = NVS_LEFT_BUTTON_BUTT;
        break;
    case DEVICE_SETTING_FINGER_DELETE: // todo
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletFingerDeleteWidget(tile);
        break;
    case DEVICE_SETTING_FINGER_SET_PATTERN: // todo
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletFingerManagerWidget(tile);
        strcpy(midLabel, _("fingerprint_passcode_mid_btn"));
        break;
    // reset passcode
    case DEVICE_SETTING_RESET_PASSCODE_VERIFY:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletSetPinWidget(tile, DEVICE_SETTING_RESET_PASSCODE_VERIFY);
        obj = g_setPassCode;
        destructCb = GuiSetPinDestruct;
        break;
    case DEVICE_SETTING_RESET_PASSCODE_SETPIN:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletRepeatPinWidget(tile);
        destructCb = GuiRepeatDestruct;
        break;

    // add wallet
    case DEVICE_SETTING_ADD_WALLET_NOTICE:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        destructCb = GuiSettingCountDownDestruct;
        GuiWalletAddWalletNotice(tile);
        break;
    case DEVICE_SETTING_ADD_WALLET_CREATE_OR_IMPORT:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletSelectAddWallet(tile);
        break;
    case DEVICE_SETTING_ADD_WALLET_SETPIN:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletSetPinWidget(tile, DEVICE_SETTING_ADD_WALLET_SETPIN);
        break;
    case DEVICE_SETTING_ADD_WALLET_REPEATPASS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        currTileIndex = DEVICE_SETTING_ADD_WALLET_NAME_WALLET;
        GuiWalletNameWallet(tile, currTileIndex);
        break;
    case DEVICE_SETTING_ADD_WALLET_NAME_WALLET:
        break;
    case DEVICE_SETTING_ADD_WALLET_LIMIT:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletAddLimit(tile);
        break;

    // del wallet
    case DEVICE_SETTING_DEL_WALLET:
        GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        currTileIndex = DEVICE_SETTING_DEL_WALLET;
        strcpy(midLabel, _("change_passcode_mid_btn"));
        g_verifyCode = GuiCreateEnterPasscode(tile, NULL, &currTileIndex, ENTER_PASSCODE_VERIFY_PIN);
        break;
    case DEVICE_SETTING_DEL_WALLET_VERIFY:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletDelWalletConfirm(tile);
        break;

    // passphrase
    case DEVICE_SETTING_PASSPHRASE:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletPassphrase(tile);
        rightBtn = NVS_BAR_QUESTION_MARK;
        rightCb = OpenPassphraseLearnMoreHandler;
        strcpy(midLabel, _("wallet_setting_passphrase"));
        break;
    case DEVICE_SETTING_PASSPHRASE_VERIFY:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        currTileIndex = DEVICE_SETTING_PASSPHRASE_VERIFY;
        destructCb = GuiDelEnterPasscode;
        g_verifyCode = GuiCreateEnterPasscode(tile, NULL, &currTileIndex, ENTER_PASSCODE_VERIFY_PIN);
        obj = g_verifyCode;
        strcpy(midLabel, _("change_passcode_mid_btn"));
        break;
    case DEVICE_SETTING_PASSPHRASE_ENTER:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletPassphraseEnter(tile);
        strcpy(midLabel, _("wallet_setting_passphrase"));
        break;
    // RECOVERY PHRASE CHECK
    case DEVICE_SETTING_RECOVERY_METHOD_CHECK:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy(midLabel, _("seed_check_mid_btn"));
        GuiWalletRecoveryMethodCheck(tile);
        break;
    case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_12WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy(rightLabel, _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySinglePhrase(tile, 12);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_18WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy(rightLabel, _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySinglePhrase(tile, 18);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_24WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy(rightLabel, _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySinglePhrase(tile, 24);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    case DEVICE_SETTING_RECOVERY_SHARE_PHRASE_20WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy(rightLabel, _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySharePhrase(tile, 20);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    case DEVICE_SETTING_RECOVERY_SHARE_PHRASE_33WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy(rightLabel, _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySharePhrase(tile, 33);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    default:
        return SUCCESS_CODE;
    }

    switch (rightBtn) {
    case NVS_BAR_QUESTION_MARK:
    case NVS_BAR_MORE_INFO:
        GuiNvsBarSetRightCb(rightBtn, rightCb, tile);
        break;
    case NVS_BAR_WORD_RESET:
        rightCb = ResetClearImportHandler;
        GuiNvsBarSetRightBtnLabel(NVS_BAR_WORD_RESET, USR_SYMBOL_RESET "Clear");
        GuiNvsBarSetRightCb(NVS_BAR_WORD_RESET, rightCb, NULL);
        break;
    case NVS_RIGHT_BUTTON_BUTT:
        GuiNvsBarSetRightBtnLabel(NVS_BAR_WORD_RESET, "");
        GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    default:
        break;
    }
    GuiNvsBarSetLeftCb(leftBtn, leftCb, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, midLabel);

    g_deviceSetTileView.currentTile = currentTile;
    lv_obj_set_tile_id(g_deviceSetTileView.tileView, currentTile, 0, LV_ANIM_OFF);
    g_deviceSettingArray[currentTile].tile = tile;
    g_deviceSettingArray[currentTile].rightCb = rightCb;
    strcpy(g_deviceSettingArray[currentTile].rightLabel, rightLabel);
    strcpy(g_deviceSettingArray[currentTile].midLabel, midLabel);
    g_deviceSettingArray[currentTile].obj = obj;
    g_deviceSettingArray[currentTile].destructCb = destructCb;
    g_deviceSettingArray[currentTile].structureCb = structureCb;
    g_deviceSettingArray[currentTile].rightBtn = rightBtn;
    g_deviceSettingArray[currentTile].leftBtn = leftBtn;
    g_deviceSettingArray[currentTile].leftCb = leftCb;

    return SUCCESS_CODE;
}

int8_t GuiDevSettingPrevTile(uint8_t tileIndex)
{
    uint8_t currentTile = g_deviceSetTileView.currentTile;
    char rightLabel[16] = {0};
    char midLabel[32] = {0};
    lv_event_cb_t rightCb = NULL;
    NVS_RIGHT_BUTTON_ENUM rightBtn = NVS_RIGHT_BUTTON_BUTT;
    NVS_LEFT_BUTTON_ENUM leftBtn = NVS_BAR_RETURN;
    if (currentTile == 0) {
        return GuiCLoseCurrentWorkingView();
    }
    if (g_deviceSettingArray[currentTile].destructCb != NULL) {
        g_deviceSettingArray[currentTile].destructCb(g_deviceSettingArray[currentTile].obj, NULL);
        g_deviceSettingArray[currentTile].destructCb = NULL;
        g_deviceSettingArray[currentTile].obj = NULL;
    }

    lv_obj_del(g_deviceSettingArray[currentTile].tile);

    g_deviceSettingArray[currentTile].tile = NULL;
    currentTile--;
    g_deviceSetTileView.currentTile = currentTile;
    if (g_deviceSettingArray[currentTile].structureCb != NULL) {
        g_deviceSettingArray[currentTile].structureCb(g_deviceSettingArray[currentTile].obj, NULL);
    }

    rightCb = g_deviceSettingArray[currentTile].rightCb;
    strcpy(rightLabel, g_deviceSettingArray[currentTile].rightLabel);
    strcpy(midLabel, g_deviceSettingArray[currentTile].midLabel);
    rightBtn = g_deviceSettingArray[currentTile].rightBtn;
    leftBtn = g_deviceSettingArray[currentTile].leftBtn;

    switch (rightBtn) {
    case NVS_BAR_QUESTION_MARK:
    case NVS_BAR_MORE_INFO:
        GuiNvsBarSetRightCb(rightBtn, rightCb, g_deviceSettingArray[currentTile].tile);
        break;
    case NVS_BAR_WORD_RESET:
        rightCb = ResetClearImportHandler;
        GuiNvsBarSetRightBtnLabel(NVS_BAR_WORD_RESET, rightLabel);
        GuiNvsBarSetRightCb(NVS_BAR_WORD_RESET, ResetClearImportHandler, NULL);
        break;
    case NVS_RIGHT_BUTTON_BUTT:
        GuiNvsBarSetRightBtnLabel(NVS_BAR_WORD_RESET, "");
        GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    default:
        break;
    }
    GuiNvsBarSetLeftCb(leftBtn, ReturnHandler, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, midLabel);
    lv_obj_set_tile_id(g_deviceSetTileView.tileView, currentTile, 0, LV_ANIM_OFF);

    return SUCCESS_CODE;
}

void GuiSettingRefresh(void)
{
    DeviceSettingItem_t *item = &g_deviceSettingArray[g_deviceSetTileView.currentTile];
    GuiNvsBarSetLeftCb(item->leftBtn, item->leftCb, NULL);
    GuiNvsBarSetRightCb(item->rightBtn, item->rightCb, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, item->midLabel);
    if (g_passphraseLearnMoreCont != NULL) {
        GUI_DEL_OBJ(g_passphraseLearnMoreCont);
        GuiOpenPassphraseLearnMore();
    }
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
        GuiPassphraseOpenQRCodeHintBox();
    }
    if (item->leftCb == CancelCurFingerHandler) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
    if (g_setPassCode != NULL) {
        GuiUpdateEnterPasscodeParam(g_setPassCode, NULL);
    }
    if (g_repeatPassCode != NULL) {
        GuiUpdateEnterPasscodeParam(g_repeatPassCode, NULL);
    }
}

static void PassphraseQuickAccessHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *switchBox = g_passphraseQuickAccessSwitch;
        bool en = lv_obj_has_state(switchBox, LV_STATE_CHECKED);
        if (en) {
            lv_obj_clear_state(switchBox, LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(switchBox, LV_STATE_CHECKED);
        }
        SetPassphraseQuickAccess(!en);
        lv_event_send(switchBox, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static bool ModelGetPassphraseQuickAccess(void)
{
#ifdef COMPILE_SIMULATOR
    return false;
#else
    return GetPassphraseQuickAccess();
#endif
}

void GuiSettingRecoveryCheck(void)
{
    g_waitAnimWidget.cont = GuiCreateAnimHintBox(lv_scr_act(), 480, 278, 82);
    g_waitAnimWidget.label = GuiCreateTextLabel(g_waitAnimWidget.cont, _("seed_check_wait_verify"));
    lv_obj_align(g_waitAnimWidget.label, LV_ALIGN_BOTTOM_MID, 0, -76);
}

void GuiVerifyCurrentPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}