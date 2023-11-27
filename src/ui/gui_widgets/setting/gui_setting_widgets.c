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
#include "firmware_update.h"
#include "gui_page.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "keystore.h"
#else
#include "simulator_model.h"
#endif

typedef void (*setting_update_cb)(void *obj, void *param);

typedef struct DeviceSettingWidget {
    uint8_t currentTile;                        // current x tile
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

static lv_obj_t *g_delWalletHintbox = NULL;    // del wallet hintbox
static lv_obj_t *g_selectAmountHintbox = NULL; // select amount hintbox
static lv_obj_t *g_noticeHintBox = NULL;       // notice hintbox
static GuiEnterPasscodeItem_t *g_verifyCode = NULL;
static lv_obj_t *g_passphraseLearnMoreCont = NULL;
static PageWidget_t *g_pageWidget;

static void OpenPassphraseLearnMoreHandler(lv_event_t *e);
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

void DelCurrCloseToSubtopViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(obj));
        if (GuiSettingGetDeleteFlag()) {
            for (int i = g_deviceSetTileView.currentTile; i > 3; i--) {
                GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
            }
            g_noticeHintBox = NULL;
        } else {
            CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING_WALLET_SETTING);
            g_noticeHintBox = NULL;
        }
    }
}

void GuiWalletResetPassWordHintBox(void)
{
    lv_obj_t *label;
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

static void CloseCurrentPage(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_passphraseLearnMoreCont)
        lv_obj_clear_flag(g_deviceSettingArray[g_deviceSetTileView.currentTile].tile, LV_OBJ_FLAG_HIDDEN);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, OpenPassphraseLearnMoreHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_setting_passphrase"));
    }
}

static void GuiPassphraseOpenQRCodeHintBox()
{
    GuiQRCodeHintBoxOpen("https://keyst.one/t/3rd/passphrase", _("passphrase_learn_more_title"), "https://keyst.one/t/3rd/passphrase");
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

    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseCurrentPage, NULL);

    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "");

}

static void OpenPassphraseLearnMoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiOpenPassphraseLearnMore();
    }
}

static

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
        sprintf(showString, "#8E8E8E v%s#  /  #F5870A v%s  Available#", version, fileVersion);
        // sprintf(showString, "#8E8E8E %s#", version);
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

        label = GuiCreateTextLabel(g_selectAmountHintbox, _("import_wallet_ssb_20words"));
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

        label = GuiCreateTextLabel(g_selectAmountHintbox, _("import_wallet_ssb_33words"));
        img = GuiCreateImg(g_selectAmountHintbox, &imgArrowRight);
        table[0].obj = label;
        table[1].obj = img;
        btn = GuiCreateButton(g_selectAmountHintbox, 456, 84, table, NUMBER_OF_ARRAYS(table), SelectPhraseAmountHandler, &walletSetting[1]);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 703);
    }
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
        GuiWalletSeedCheckClearKb();

        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
        img = GuiCreateImg(g_noticeHintBox, &imgSuccess);
        lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 36, -236);
        label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("seed_check_verify_match_title"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -172);
        label = GuiCreateNoticeLabel(g_noticeHintBox, _("seed_check_verify_match_desc"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -110);

        btn = GuiCreateBtn(g_noticeHintBox, _("Done"));
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 332, 710);
        lv_obj_set_size(btn, 122, 66);
        lv_obj_add_event_cb(btn, DelCurrCloseToSubtopViewHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
    } else {
        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
        img = GuiCreateImg(g_noticeHintBox, &imgFailed);
        lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 36, -236);
        label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("seed_check_verify_not_match_title"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -172);
        label = GuiCreateIllustrateLabel(g_noticeHintBox, _("seed_check_verify_not_match_desc"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -110);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);

        btn = GuiCreateBtn(g_noticeHintBox, _("Done"));
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 332, 710);
        lv_obj_set_size(btn, 122, 66);
        lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
    }
}

void GuiDevSettingPassCode(bool result, uint16_t tileIndex)
{
    static uint16_t walletIndex = DEVICE_SETTING_RESET_PASSCODE_VERIFY;
    printf("tileIndex = %d\n", tileIndex);
    switch (tileIndex) {
    case SIG_FINGER_FINGER_SETTING:
        walletIndex = GuiGetFingerSettingIndex();
        break;
    case SIG_FINGER_SET_UNLOCK:
        GuiShowKeyboardDestruct();
        return GuiWalletFingerOpenUnlock();
    case SIG_FINGER_SET_SIGN_TRANSITIONS:
        GuiShowKeyboardDestruct();
        return GuiWalletFingerOpenSign();
    case SIG_FINGER_REGISTER_ADD_SUCCESS:
        FpSaveKeyInfo();
        walletIndex = DEVICE_SETTING_FINGER_ADD_SUCCESS;
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
    strcpy(g_deviceSettingArray[g_deviceSetTileView.currentTile].midLabel, _("device_setting_mid_btn"));
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
    GUI_DEL_OBJ(g_delWalletHintbox)
    GUI_DEL_OBJ(g_noticeHintBox)
    GUI_DEL_OBJ(g_selectAmountHintbox)
    GUI_DEL_OBJ(g_passphraseLearnMoreCont)
    GuiFpVerifyDestruct();
    // if (g_recoveryMkb->cont != NULL) {
    //     GUI_DEL_OBJ(g_recoveryMkb->cont)
    // }
    GuiWalletSeedCheckClearObject();
    CloseToTargetTileView(g_deviceSetTileView.currentTile, DEVICE_SETTING);
    lv_obj_del(g_deviceSetTileView.tileView);
    lv_obj_del(g_deviceSetTileView.cont);
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
        printf("wallet_setting_passphrase...\n");
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
        SetNavBarRightBtn(g_pageWidget->navBarWidget, rightBtn, rightCb, tile);
        break;
    case NVS_BAR_WORD_RESET:
        rightCb = ResetSeedCheckImportHandler;
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, USR_SYMBOL_RESET "Clear");
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