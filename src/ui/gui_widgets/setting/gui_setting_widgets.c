#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_enter_passcode.h"
#include "gui_model.h"
#include "gui_lock_widgets.h"
#include "gui_setting_widgets.h"
#include "gui_qr_hintbox.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "keystore.h"
#include "version.h"
#include "presetting.h"
#include "assert.h"
#include "gui_page.h"
#include "fingerprint_process.h"
#include "screen_manager.h"
#include <stdlib.h>
#include "user_fatfs.h"

typedef void (*setting_update_cb)(void *obj, void *param);

typedef struct DeviceSettingWidget {
    uint8_t currentTile;                        // current x tile
    lv_obj_t *cont;                             // setting container
    lv_obj_t *tileView;                         // setting tile view
} DeviceSettingWidget_t;
static DeviceSettingWidget_t g_deviceSetTileView;

typedef struct DeviceSettingItem {
    lv_obj_t *tile;                                     // setting item tile
    lv_obj_t *obj;                                      // setting item object
    char rightLabel[BUFFER_SIZE_64];                    // right label
    char midLabel[BUFFER_SIZE_64];                      // middle label
    NVS_RIGHT_BUTTON_ENUM rightBtn;
    NVS_LEFT_BUTTON_ENUM leftBtn;                       // right button
    void *rightParam;                                   // right button param
    lv_event_cb_t rightCb;                              // right button callback
    lv_event_cb_t leftCb;
    setting_update_cb destructCb;                       // destruct callback
    setting_update_cb structureCb;                      // structure callback
} DeviceSettingItem_t;
static DeviceSettingItem_t g_deviceSettingArray[DEVICE_SETTING_LEVEL_MAX];

static lv_obj_t *g_selectAmountHintbox = NULL; // select amount hintbox
static lv_obj_t *g_noticeWindow = NULL;       // notice hintbox
static GuiEnterPasscodeItem_t *g_verifyCode = NULL;
static lv_obj_t *g_passphraseLearnMoreCont = NULL;
static PageWidget_t *g_pageWidget;

static void OpenPassphraseLearnMoreHandler(lv_event_t *e);
static void *CreateSettingWidgetsButton(lv_obj_t *parent, const char *title, const char *desc,
                                        const void *src, lv_event_cb_t buttonCb, void *param);
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
    uint8_t *walletSetIndex = lv_event_get_user_data(e);
    walletIndex = *walletSetIndex;
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
}

static void CloseToFingerAndPassView(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    for (int i = g_deviceSetTileView.currentTile; i > 2; i--) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

void CloseToSubtopViewHandler(lv_event_t *e)
{

    lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    void **param = lv_event_get_user_data(e);
    if (param != NULL) {
        *param = NULL;
    }
    CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING_WALLET_SETTING);
}

static void AddCloseToSubtopViewHandler(lv_event_t *e)
{
    CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING_WALLET_SETTING);
}

void DelCurrCloseToSubtopViewHandler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);

    lv_obj_del(lv_obj_get_parent(obj));
    if (GuiSettingGetDeleteFlag()) {
        for (int i = g_deviceSetTileView.currentTile; i > 3; i--) {
            GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        }
        g_noticeWindow = NULL;
    } else {
        CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING_WALLET_SETTING);
        g_noticeWindow = NULL;
    }
}

void GuiWalletResetPassWordHintBox(void)
{
    g_noticeWindow = GuiCreateConfirmHintBox(&imgWarn, _("change_passcode_warning_title"), _("change_passcode_warning_desc"), NULL, _("got_it"), ORANGE_COLOR);
    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
}

static void CloseCurrentPage(lv_event_t *e)
{
    GUI_DEL_OBJ(g_passphraseLearnMoreCont)
    lv_obj_clear_flag(g_deviceSettingArray[g_deviceSetTileView.currentTile].tile, LV_OBJ_FLAG_HIDDEN);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, OpenPassphraseLearnMoreHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Passphrase"));
}

static void GuiPassphraseOpenQRCodeHintBox()
{
    GuiQRCodeHintBoxOpen("https://keyst.one/t/3rd/passphrase", _("passphrase_learn_more_title"), "https://keyst.one/t/3rd/passphrase");
}

static void OpenPassphraseQrCodeHandler(lv_event_t *e)
{
    GuiPassphraseOpenQRCodeHintBox();
}

static void GuiOpenPassphraseLearnMore()
{
    uint16_t height;
    lv_obj_add_flag(g_deviceSettingArray[g_deviceSetTileView.currentTile].tile, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);
    lv_obj_t *label = GuiCreateTextLabel(cont, _("passphrase_learn_more_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *led = lv_led_create(cont);
    lv_led_set_brightness(led, 150);
    lv_obj_align_to(led, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12 + 15);
    lv_obj_set_size(led, 12, 12);
    lv_led_set_color(led, ORANGE_COLOR);
    label = GuiCreateNoticeLabel(cont, _("passphrase_learn_more_desc1"));
    lv_obj_align_to(label, led, LV_ALIGN_OUT_RIGHT_TOP, 12, -15);
    height = lv_obj_get_self_height(label) + 12;

    lv_obj_t *newLed = lv_led_create(cont);
    lv_led_set_brightness(newLed, 150);
    lv_obj_set_size(newLed, 12, 12);
    lv_led_set_color(newLed, ORANGE_COLOR);
    lv_obj_align_to(newLed, led, LV_ALIGN_TOP_LEFT, 0, height + 15);
    led = newLed;
    label = GuiCreateNoticeLabel(cont, _("passphrase_learn_more_desc2"));
    lv_obj_align_to(label, led, LV_ALIGN_OUT_RIGHT_TOP, 12, -15);
    height = lv_obj_get_self_height(label) + 12;

    newLed = lv_led_create(cont);
    lv_led_set_brightness(newLed, 150);
    lv_obj_set_size(newLed, 12, 12);
    lv_led_set_color(newLed, ORANGE_COLOR);
    lv_obj_align_to(newLed, led, LV_ALIGN_TOP_LEFT, 0, height + 15);
    led = newLed;
    label = GuiCreateNoticeLabel(cont, _("passphrase_learn_more_desc3"));
    lv_obj_align_to(label, led, LV_ALIGN_OUT_RIGHT_TOP, 12, -15);
    height = lv_obj_get_self_height(label) + 12;

    newLed = lv_led_create(cont);
    lv_led_set_brightness(newLed, 150);
    lv_obj_align(newLed, LV_ALIGN_DEFAULT, 36, 432 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_size(newLed, 12, 12);
    lv_led_set_color(newLed, ORANGE_COLOR);
    lv_obj_align_to(newLed, led, LV_ALIGN_TOP_LEFT, 0, height + 15);
    led = newLed;
    label = GuiCreateNoticeLabel(cont, _("passphrase_learn_more_desc4"));
    lv_obj_align_to(label, led, LV_ALIGN_OUT_RIGHT_TOP, 12, -15);
    height = lv_obj_get_self_height(label) + 12;

    g_passphraseLearnMoreCont = cont;

    cont = GuiCreateContainerWithParent(g_passphraseLearnMoreCont, 144, 30);
    lv_obj_align_to(cont, newLed, LV_ALIGN_TOP_LEFT, 0, height);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, OpenPassphraseQrCodeHandler, LV_EVENT_CLICKED, NULL);

    label = GuiCreateIllustrateLabel(cont, _("learn_more"));
    lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);

    lv_obj_t *img = GuiCreateImg(cont, &imgQrcodeTurquoise);
    lv_obj_align_to(img, label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_set_width(cont, lv_obj_get_self_width(label) + lv_obj_get_self_width(img) + 10);

    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseCurrentPage, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "");
}

static void OpenPassphraseLearnMoreHandler(lv_event_t *e)
{
    GuiOpenPassphraseLearnMore();
}

void GuiSettingFullKeyBoardDestruct(void *obj, void *param)
{
    GuiWalletNameWalletDestruct();
}

static void GuiWalletAddLimit(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("wallet_setting_add_wallet_limit"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("wallet_setting_add_wallet_limit_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    lv_obj_t *btn = GuiCreateTextBtn(parent, _("got_it"));
    lv_obj_set_size(btn, 348, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 710 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, AddCloseToSubtopViewHandler, LV_EVENT_CLICKED, NULL);
}

static void AboutHandler(lv_event_t *e)
{
    GuiFrameOpenView(&g_aboutView);
}

static void GuiSettingEntranceWidget(lv_obj_t *parent)
{
#define DEFAULT_ENGLISH_SETTING_DESC_LEN 31
    static uint32_t walletSetting[4] = {
        DEVICE_SETTING_WALLET_SETTING,
        DEVICE_SETTING_SYSTEM_SETTING,
        DEVICE_SETTING_CONNECT,
        DEVICE_SETTING_ABOUT
    };

    char descBuff[BUFFER_SIZE_128] = {0};
    const char *desc = _("device_setting_wallet_setting_desc");
    int descLen = strnlen_s(desc, sizeof(descBuff));
    if (descLen > DEFAULT_ENGLISH_SETTING_DESC_LEN) {
        strncpy_s(descBuff, sizeof(descBuff) - 3, desc, FindStringCharPosition(desc, '/', 2) - 1);
        strcat_s(descBuff, sizeof(descBuff), "...");
    } else {
        strcpy_s(descBuff, sizeof(descBuff), desc);
    }

    lv_obj_t *button = CreateSettingWidgetsButton(parent, _("device_setting_wallet_setting_title"),
                       descBuff, &imgWalletSetting, WalletSettingHandler, &walletSetting[0]);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 144 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 274 - GUI_MAIN_AREA_OFFSET);

    button = CreateSettingWidgetsButton(parent, _("device_setting_system_setting_title"),
                                        _("device_setting_system_setting_desc"), &imgSystemSetting, OpenViewHandler, &g_systemSettingView);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 287 - GUI_MAIN_AREA_OFFSET);

    button = CreateSettingWidgetsButton(parent, _("usb_connection_title"),
                                        _("device_setting_connection_desc"), &imgConnection, OpenViewHandler, &g_connectionView);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 413 - GUI_MAIN_AREA_OFFSET);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 543 - GUI_MAIN_AREA_OFFSET);

    char showString[BUFFER_SIZE_64] = {0};
    char version[SOFTWARE_VERSION_MAX_LEN] = {0};
    GetSoftWareVersionNumber(version);
    if (FatfsFileExist(SD_CARD_OTA_BIN_PATH)) {
        snprintf_s(showString, BUFFER_SIZE_64, "#8E8E8E v%s#  /  #F5870A %s#", version, _("firmware_update_title"));
    } else {
        snprintf_s(showString, BUFFER_SIZE_64, "#8E8E8E %s#", version);
    }

    button = CreateSettingWidgetsButton(parent, _("device_setting_about_title"),
                                        showString, &imgAbout, AboutHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 556 - GUI_MAIN_AREA_OFFSET);
}

static void SelectPhraseAmountHandler(lv_event_t *e)
{
    static uint8_t walletIndex = 0;
    GUI_DEL_OBJ(g_selectAmountHintbox)
    uint8_t *walletSetIndex = lv_event_get_user_data(e);
    walletIndex = *walletSetIndex;
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
}
void GuiSettingCloseSelectAmountHintBox()
{
    GUI_DEL_OBJ(g_selectAmountHintbox)
}
// open select cont
void OpenSinglePhraseHandler(lv_event_t *e)
{
    static uint8_t walletAmounts[] = {DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_12WORDS, DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_18WORDS, DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_24WORDS};
    MoreInfoTable_t moreInfoTable[] = {
        {.name = _("wallet_phrase_12words"), .src = &imgArrowRight, .callBack = SelectPhraseAmountHandler, &walletAmounts[0]},
        {.name = _("wallet_phrase_18words"), .src = &imgArrowRight, .callBack = SelectPhraseAmountHandler, &walletAmounts[1]},
        {.name = _("wallet_phrase_24words"), .src = &imgArrowRight, .callBack = SelectPhraseAmountHandler, &walletAmounts[2]},
    };
    g_selectAmountHintbox = GuiCreateMoreInfoHintBox(&imgClose, _("seed_check_word_select"), moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), false, &g_selectAmountHintbox);
}

// share phrase
void OpenSharePhraseHandler(lv_event_t *e)
{
    static uint8_t walletAmounts[] = {DEVICE_SETTING_RECOVERY_SHARE_PHRASE_20WORDS, DEVICE_SETTING_RECOVERY_SHARE_PHRASE_33WORDS};
    MoreInfoTable_t moreInfoTable[] = {
        {.name = _("wallet_phrase_20words"), .src = &imgArrowRight, .callBack = SelectPhraseAmountHandler, &walletAmounts[0]},
        {.name = _("wallet_phrase_33words"), .src = &imgArrowRight, .callBack = SelectPhraseAmountHandler, &walletAmounts[1]},
    };
    g_selectAmountHintbox = GuiCreateMoreInfoHintBox(&imgClose, _("seed_check_word_select"), moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), false, &g_selectAmountHintbox);
}

static void DelWalletHandler(lv_event_t *e)
{
    lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    g_noticeWindow = NULL;
    GuiShowKeyboardHandler(e);
}

static void OpenDelWalletHandler(lv_event_t *e)
{
    static uint16_t walletIndex = DEVICE_SETTING_DEL_WALLET;
    g_noticeWindow = GuiCreateHintBox(132);
    lv_obj_add_event_cb(lv_obj_get_child(g_noticeWindow, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    lv_obj_t *btn = GuiCreateSelectButton(g_noticeWindow, _("wallet_settings_delete_button"), &imgDel, DelWalletHandler, &walletIndex, true);
    lv_obj_set_style_text_color(lv_obj_get_child(btn, 0), RED_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 692);
}

void GuiWalletRecoveryWriteSe(bool result)
{
    GuiDeleteAnimHintBox();
    if (result) {
        GuiWalletSeedCheckClearKb();
        g_noticeWindow = GuiCreateConfirmHintBox(&imgSuccess, _("seed_check_verify_match_title"), _("seed_check_verify_match_desc"), NULL, _("Done"), ORANGE_COLOR);
        lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), DelCurrCloseToSubtopViewHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    } else {
        g_noticeWindow = GuiCreateConfirmHintBox(&imgFailed, _("seed_check_verify_not_match_title"), _("error_box_mnemonic_not_match_wallet_desc"), NULL, _("Done"), ORANGE_COLOR);
        lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    }
}

void GuiDevSettingPassCode(bool result, uint16_t tileIndex)
{
    static uint16_t walletIndex = DEVICE_SETTING_RESET_PASSCODE_VERIFY;
    printf("tileIndex = %d\n", tileIndex);
    if (!result)
        return;
    switch (tileIndex) {
    case SIG_FINGER_FINGER_SETTING:
        walletIndex = GuiGetFingerSettingIndex();
        break;
    case SIG_FINGER_SET_UNLOCK:
        if (result) {
            GuiShowKeyboardDestruct();
            GuiWalletFingerOpenUnlock();
            return;
        }
        break;
    case SIG_FINGER_SET_SIGN_TRANSITIONS:
        if (result) {
            GuiShowKeyboardDestruct();
            GuiWalletFingerOpenSign();
            return;
        }
        break;
    case SIG_FINGER_REGISTER_ADD_SUCCESS:
        if (result) {
            FpSaveKeyInfo(true);
            SetPageLockScreen(true);
            walletIndex = DEVICE_SETTING_FINGER_ADD_SUCCESS;
        }
        break;
    case SIG_SETTING_CHANGE_PASSWORD:
        walletIndex = DEVICE_SETTING_RESET_PASSCODE_VERIFY;
        break;
    case DEVICE_SETTING_PASSPHRASE_VERIFY:
        walletIndex = DEVICE_SETTING_PASSPHRASE_ENTER;
        break;
    case DEVICE_SETTING_ADD_WALLET:
        ClearSecretCache();
        walletIndex = DEVICE_SETTING_ADD_WALLET_NOTICE;
        break;
    case DEVICE_SETTING_DEL_WALLET:
        ClearSecretCache();
        walletIndex = DEVICE_SETTING_DEL_WALLET_VERIFY;
        break;
    default:
        if (result)
            return;
    }

    if (result) {
        GuiShowKeyboardDestruct();
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, &walletIndex, sizeof(walletIndex));
    }
}

void GuiResettingPassWordSuccess(void)
{
    g_noticeWindow = GuiCreateConfirmHintBox(&imgSuccess, _("change_passcode_reset_success_title"), _("change_passcode_reset_success_desc"), NULL, _("Done"), ORANGE_COLOR);
    lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseToFingerAndPassView, LV_EVENT_CLICKED, NULL);
}

void GuiSettingInit(void)
{
    CLEAR_OBJECT(g_deviceSetTileView);
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, DEVICE_SETTING, 0, LV_DIR_HOR);
    GuiSettingEntranceWidget(tile);

    g_deviceSetTileView.currentTile = DEVICE_SETTING;
    g_deviceSetTileView.tileView = tileView;
    g_deviceSetTileView.cont = cont;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].tile = tile;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].obj = NULL;
    strcpy_s(g_deviceSettingArray[g_deviceSetTileView.currentTile].midLabel, BUFFER_SIZE_64, _("device_setting_mid_btn"));
    g_deviceSettingArray[g_deviceSetTileView.currentTile].destructCb = NULL;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].structureCb = NULL;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].rightBtn = NVS_RIGHT_BUTTON_BUTT;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].leftBtn = NVS_BAR_RETURN;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].leftCb = ReturnHandler;

    lv_obj_set_tile_id(g_deviceSetTileView.tileView, g_deviceSetTileView.currentTile, 0, LV_ANIM_OFF);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, g_deviceSettingArray[g_deviceSetTileView.currentTile].midLabel);
}

void GuiSettingDeInit(void)
{
    GuiShowKeyboardDestruct();
    GUI_DEL_OBJ(g_noticeWindow)
    GUI_DEL_OBJ(g_selectAmountHintbox)
    GUI_DEL_OBJ(g_passphraseLearnMoreCont)
    GuiFpVerifyDestruct();
    GuiWalletSeedCheckClearObject();
    GuiWalletSettingDeinit();
    CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING);
    CLEAR_OBJECT(g_deviceSetTileView);
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

int8_t GuiDevSettingNextTile(uint8_t tileIndex)
{
    lv_obj_t *tile = NULL;
    static uint8_t currTileIndex = DEVICE_SETTING;
    char rightLabel[BUFFER_SIZE_64] = {0};
    char midLabel[BUFFER_SIZE_64] = {0};
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
        strcpy_s(midLabel, sizeof(midLabel), _("wallet_settings_mid_btn"));
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
        strcpy_s(midLabel, sizeof(midLabel), _("wallet_setting_passcode"));
        break;

    // finger module
    case DEVICE_SETTING_FINGER_MANAGER:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletFingerManagerWidget(tile);
        strcpy_s(midLabel, sizeof(midLabel), _("fingerprint_passcode_fingerprint_setting"));
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
        strcpy_s(midLabel, sizeof(midLabel), _("wallet_setting_passcode"));
        break;
    // reset passcode
    case DEVICE_SETTING_RESET_PASSCODE_VERIFY:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletSetPinWidget(tile, DEVICE_SETTING_RESET_PASSCODE_VERIFY);
        // obj = g_setPassCode;
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
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        currTileIndex = DEVICE_SETTING_DEL_WALLET;
        strcpy_s(midLabel, sizeof(midLabel), _("change_passcode_mid_btn"));
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
        strcpy_s(midLabel, sizeof(midLabel), _("Passphrase"));
        break;
    case DEVICE_SETTING_PASSPHRASE_VERIFY:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        currTileIndex = DEVICE_SETTING_PASSPHRASE_VERIFY;
        destructCb = GuiDelEnterPasscode;
        g_verifyCode = GuiCreateEnterPasscode(tile, NULL, &currTileIndex, ENTER_PASSCODE_VERIFY_PIN);
        obj = g_verifyCode;
        strcpy_s(midLabel, sizeof(midLabel), _("change_passcode_mid_btn"));
        break;
    case DEVICE_SETTING_PASSPHRASE_ENTER:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        GuiWalletPassphraseEnter(tile);
        strcpy_s(midLabel, sizeof(midLabel), _("Passphrase"));
        break;

    // RECOVERY PHRASE CHECK
    case DEVICE_SETTING_RECOVERY_METHOD_CHECK:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy_s(midLabel, sizeof(midLabel), _("wallet_setting_seed_phrase"));
        GuiWalletRecoveryMethodCheck(tile);
        break;
    case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_12WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy_s(rightLabel, sizeof(rightLabel), _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySinglePhrase(tile, 12);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_18WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy_s(rightLabel, sizeof(rightLabel), _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySinglePhrase(tile, 18);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_24WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy_s(rightLabel, sizeof(rightLabel), _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySinglePhrase(tile, 24);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    case DEVICE_SETTING_RECOVERY_SHARE_PHRASE_20WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy_s(rightLabel, sizeof(rightLabel), _("import_wallet_phrase_clear_btn"));
        rightBtn = NVS_BAR_WORD_RESET;
        obj = GuiWalletRecoverySharePhrase(tile, 20);
        destructCb = GuiWalletRecoveryDestruct;
        break;
    case DEVICE_SETTING_RECOVERY_SHARE_PHRASE_33WORDS:
        tile = lv_tileview_add_tile(g_deviceSetTileView.tileView, currentTile, 0, LV_DIR_HOR);
        strcpy_s(rightLabel, sizeof(rightLabel), _("import_wallet_phrase_clear_btn"));
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
        SetNavBarRightBtn(g_pageWidget->navBarWidget, rightBtn, rightCb, tile);
        break;
    case NVS_BAR_WORD_RESET:
        rightCb = ResetSeedCheckImportHandler;
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, _("import_wallet_phrase_clear_btn"));
        SetRightBtnCb(g_pageWidget->navBarWidget, rightCb, NULL);
        break;
    case NVS_RIGHT_BUTTON_BUTT:
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    default:
        break;
    }
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, leftBtn, leftCb, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, midLabel);

    g_deviceSetTileView.currentTile = currentTile;
    lv_obj_set_tile_id(g_deviceSetTileView.tileView, currentTile, 0, LV_ANIM_OFF);
    g_deviceSettingArray[currentTile].tile = tile;
    g_deviceSettingArray[currentTile].rightCb = rightCb;
    strcpy_s(g_deviceSettingArray[currentTile].rightLabel, sizeof(rightLabel), rightLabel);
    strcpy_s(g_deviceSettingArray[currentTile].midLabel, sizeof(midLabel), midLabel);
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
    char rightLabel[BUFFER_SIZE_64] = {0};
    char midLabel[BUFFER_SIZE_64] = {0};
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
    strcpy_s(rightLabel, BUFFER_SIZE_64, g_deviceSettingArray[currentTile].rightLabel);
    strcpy_s(midLabel, BUFFER_SIZE_64, g_deviceSettingArray[currentTile].midLabel);
    rightBtn = g_deviceSettingArray[currentTile].rightBtn;
    leftBtn = g_deviceSettingArray[currentTile].leftBtn;

    switch (rightBtn) {
    case NVS_BAR_QUESTION_MARK:
    case NVS_BAR_MORE_INFO:
        SetNavBarRightBtn(g_pageWidget->navBarWidget, rightBtn, rightCb, g_deviceSettingArray[currentTile].tile);
        break;
    case NVS_BAR_WORD_RESET:
        rightCb = ResetSeedCheckImportHandler;
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, rightLabel);
        SetRightBtnCb(g_pageWidget->navBarWidget, ResetSeedCheckImportHandler, NULL);
        break;
    case NVS_RIGHT_BUTTON_BUTT:
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    default:
        break;
    }
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, leftBtn, ReturnHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, midLabel);
    lv_obj_set_tile_id(g_deviceSetTileView.tileView, currentTile, 0, LV_ANIM_OFF);

    return SUCCESS_CODE;
}

void GuiSettingRefresh(void)
{
    GuiWalletSettingRefresh();
    DeviceSettingItem_t *item = &g_deviceSettingArray[g_deviceSetTileView.currentTile];
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, item->leftBtn, item->leftCb, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, item->rightBtn, item->rightCb, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, item->midLabel);
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
}

int GuiSettingGetCurrentTileIndex(void)
{
    return g_deviceSetTileView.currentTile;
}

lv_obj_t *GuiSettingGetCurrentCont(void)
{
    return lv_obj_get_parent(g_deviceSetTileView.cont);
}

static void *CreateSettingWidgetsButton(lv_obj_t *parent, const char *title, const char *desc,
                                        const void *src, lv_event_cb_t buttonCb, void *param)
{
    lv_obj_t *label = GuiCreateTextLabel(parent, title);
    lv_obj_t *labelDesc = GuiCreateNoticeLabel(parent, desc);
    lv_label_set_recolor(labelDesc, true);
    lv_obj_t *img = GuiCreateImg(parent, src);
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[4] = {
        {
            .obj = img,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 24},
        }, {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 24},
        }, {
            .obj = labelDesc,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 64},
        }, {
            .obj = imgArrow,
            .align = LV_ALIGN_DEFAULT,
            .position = {396, 24},
        }
    };
    return GuiCreateButton(parent, 456, 118, table, NUMBER_OF_ARRAYS(table), buttonCb, param);
}