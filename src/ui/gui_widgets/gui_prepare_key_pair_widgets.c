#include "define.h"
#include "gui.h"
#include "lvgl.h"
#include "gui_framework.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_qr_hintbox.h"
#include "gui_page.h"
#include "gui_button.h"
#include "service_trans_usb_pubkey.h"
#include "presetting.h"
#include "log_print.h"

typedef enum {
    PREPARE_KEY_PAIR_NOTICE,
    PREPARE_KEY_PAIR_PLUG_TO_PC,
    PREPARE_KEY_PAIR_CONFIRM_PUBKEY,

    PREPARE_KEY_BUTT,
} PREPARE_KEY_PAIR_ENUM;

#define CONFIRM_SET_PUBKEY_PROCESS 66

typedef struct {
    uint8_t currentTile;
    lv_obj_t *tileView;
    lv_obj_t *pubkeyLabel;
    lv_obj_t *confirmSlider;
} PrepareKeyPairWidget_t;

static PrepareKeyPairWidget_t g_prepareKeyPairWidget;
static PageWidget_t *g_pageWidget = NULL;
static PubkeySignatureVerifyParam_t g_verifyParam = {0};

static void GuiPrepareKeyPairCreateNoticeTile(lv_obj_t *parent);
static void GuiCreatePlugToPcTile(lv_obj_t *parent);
static void GuiCreateConfirmPubkeyTile(lv_obj_t *parent);
static void CheckSliderProcessHandler(lv_event_t *e);
void GuiPrepareKeyPairNextTile(void);
void GuiPrepareKeyPairRefresh(void);
int32_t FormatPublicKeyHexStr(uint8_t *pubkey, uint32_t maxLen, char *pubKeyStr, uint32_t pubKeyStrLen);

void GuiPrepareKeyPairInit(void)
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    g_prepareKeyPairWidget.tileView = tileView;
    lv_obj_t *tile = lv_tileview_add_tile(tileView, PREPARE_KEY_PAIR_NOTICE, 0, LV_DIR_RIGHT);
    GuiPrepareKeyPairCreateNoticeTile(tile);

    tile = lv_tileview_add_tile(tileView, PREPARE_KEY_PAIR_PLUG_TO_PC, 0, LV_DIR_RIGHT);
    GuiCreatePlugToPcTile(tile);

    tile = lv_tileview_add_tile(tileView, PREPARE_KEY_PAIR_CONFIRM_PUBKEY, 0, LV_DIR_RIGHT);
    GuiCreateConfirmPubkeyTile(tile);

    g_prepareKeyPairWidget.currentTile = PREPARE_KEY_PAIR_NOTICE;
    g_prepareKeyPairWidget.tileView = tileView;

    lv_obj_set_tile_id(g_prepareKeyPairWidget.tileView, g_prepareKeyPairWidget.currentTile, 0, LV_ANIM_OFF);
}

static void GuiPrepareKeyPairCreateNoticeTile(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateOrangeIllustrateLabel(parent, "Secure your firmware signing private key!");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t *number = GuiCreateOrangeIllustrateLabel(parent, "1");
    lv_obj_align(number, LV_ALIGN_TOP_LEFT, 36, 82);

    label = GuiCreateIllustrateLabel(parent, "Visit here for CLI configuration:");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 82);

    GuiButton_t table[] = {
        {.obj = GuiCreateColorIllustrateLabel(parent, "https://github.com/keystonehq/devkit", 0x1BE0C6), .align = LV_ALIGN_TOP_LEFT, .position = {0, 0},},
        {.obj = GuiCreateImg(parent, &imgQrcodeTurquoise), .align = LV_ALIGN_RIGHT_MID, .position = {0, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 389, 30, table, NUMBER_OF_ARRAYS(table), OpenCliConfigurationHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 60, 116);

    number = GuiCreateOrangeIllustrateLabel(parent, "2");
    lv_obj_align(number, LV_ALIGN_TOP_LEFT, 36, 158);
    label = GuiCreateIllustrateLabel(parent, "Generate your key pair via the CLI or generate it yourself.");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 158);

    number = GuiCreateOrangeIllustrateLabel(parent, "3");
    lv_obj_align(number, LV_ALIGN_TOP_LEFT, 36, 230);
    label = GuiCreateIllustrateLabel(parent, "#F5870A Secure your private key!# After registering the public key to the device, it is the only way to sign and update your custom firmware.");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 230);

    lv_obj_t *btn = GuiCreateBtn(parent, USR_SYMBOL_ARROW_NEXT);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_CLICKED, NULL);
}

lv_obj_t *CreateNoticeCard(lv_obj_t *parent, const char *notice)
{
    uint16_t height = 24 + 36 + 8 + 24;
    lv_obj_t* card = GuiCreateContainerWithParent(parent, 408, 24);
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(card, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(card, 30, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* noticeIcon = GuiCreateImg(card, &imgNotice);
    lv_obj_align(noticeIcon, LV_ALIGN_TOP_LEFT, 24, 24);

    lv_obj_t* title_label = GuiCreateTextLabel(card, "Notice");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xF5870A), LV_PART_MAIN);
    lv_obj_align_to(title_label, noticeIcon, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

    lv_obj_t* content_label = GuiCreateIllustrateLabel(card, notice);
    lv_obj_set_width(content_label, 360);
    lv_obj_update_layout(content_label);
    height += lv_obj_get_self_height(content_label);
    lv_obj_set_height(card, height);
    lv_obj_align(content_label, LV_ALIGN_TOP_LEFT, 24, 68);

    return card;
}

static void GuiCreateConfirmPubkeyTile(lv_obj_t *parent)
{
    const char *notice = "You can register a public key only once. Ensure it matches your key pair. Only firmware signed with the matching private key can be installed.";
    lv_obj_t *noticeCont = CreateNoticeCard(parent, notice);
    lv_obj_align(noticeCont, LV_ALIGN_TOP_MID, 0, 24);
    lv_obj_set_style_bg_color(noticeCont, lv_color_hex(0xF55831), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *container = GuiCreateContainerWithParent(parent, 408, 224);
    lv_obj_set_style_bg_color(container, WHITE_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(container, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(container, LV_ALIGN_TOP_MID, 0, 260);

    lv_obj_t *label = GuiCreateNoticeLabel(container, "Public Key");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);

    char pubkeyStr[256] = {0};
    // char pubkey[65] = {
    //     0x04, 0x04, 0xAD, 0x1F, 0x92, 0x09, 0x38, 0xE9, 0x5C, 0x71, 0xEF, 0x62, 0xEE, 0xBD, 0x00, 0x2E,
    //     0xC4, 0xEC, 0x98, 0x4F, 0x9D, 0x2B, 0x3D, 0xB7, 0x68, 0x96, 0x28, 0x59, 0x1F, 0x7C, 0xE9, 0x92,
    //     0xA3, 0x64, 0x87, 0xC8, 0x62, 0x7D, 0x64, 0x3B, 0x78, 0xC1, 0xD7, 0x07, 0xB4, 0x39, 0x37, 0xD2,
    //     0x9D, 0x46, 0x79, 0xF1, 0x5F, 0x76, 0xE4, 0xFC, 0x9E, 0xC9, 0xA4, 0x78, 0x51, 0xC4, 0x4C, 0xF7,
    //     0x63
    // };
    // FormatPublicKeyHexStr(pubkey, sizeof(pubkey), pubkeyStr, sizeof(pubkeyStr));
    g_prepareKeyPairWidget.pubkeyLabel = GuiCreateIllustrateLabel(container, pubkeyStr);
    lv_obj_set_width(g_prepareKeyPairWidget.pubkeyLabel, 360);
    lv_label_set_recolor(g_prepareKeyPairWidget.pubkeyLabel, true);
    lv_obj_align(g_prepareKeyPairWidget.pubkeyLabel, LV_ALIGN_DEFAULT, 24, 54);

    g_prepareKeyPairWidget.confirmSlider = GuiCreateConfirmSlider(parent, CheckSliderProcessHandler);
}

static void GuiCreatePlugToPcTile(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateNoticeLabel(parent, "If the CLI is configured successfully, then connect your device to the PC to register the public key.");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t *number = GuiCreateOrangeIllustrateLabel(parent, "1");
    lv_obj_align(number, LV_ALIGN_TOP_LEFT, 36, 142);
    label = GuiCreateIllustrateLabel(parent, "Connect your Keystone to your computer using a USB-C cable. ");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 142);

    number = GuiCreateOrangeIllustrateLabel(parent, "2");
    lv_obj_align(number, LV_ALIGN_TOP_LEFT, 36, 214);
    label = GuiCreateIllustrateLabel(parent, "Strictly follow the CLI README on your PC to register your public key.");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 214);

    number = GuiCreateOrangeIllustrateLabel(parent, "3");
    lv_obj_align(number, LV_ALIGN_TOP_LEFT, 36, 286);
    label = GuiCreateIllustrateLabel(parent, "Do not unplug the device during setup until \"Setup Completed\" appears. ");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 286);
}

void GuiPrepareKeyPairDeInit()
{
    DestroyPageWidget(g_pageWidget);
    g_pageWidget = NULL;
}

void GuiPrepareKeyPairOnPubkeyVerifySuccess(void *param, uint16_t usLen)
{
    memcpy(&g_verifyParam, param, sizeof(PubkeySignatureVerifyParam_t));
    PubkeySignatureVerifyParam_t *verifyParam = &g_verifyParam;
    char pubkeyStr[256] = {0};

    if (verifyParam == NULL || (verifyParam->pubkeyLen != 64 && verifyParam->pubkeyLen != 65)) {
        printf("verify param invalid\n");
        return;
    }

    PrintArray("Pubkey from CLI", verifyParam->pubkey, verifyParam->pubkeyLen);
    PrintArray("Signature from CLI", verifyParam->signature, 64);

    FormatPublicKeyHexStr(verifyParam->pubkey, verifyParam->pubkeyLen, pubkeyStr, sizeof(pubkeyStr));
    printf("pubkey from CLI verified successfully, pubkeyStr=%s\n", pubkeyStr);
    lv_label_set_text(g_prepareKeyPairWidget.pubkeyLabel, pubkeyStr);
    GuiPrepareKeyPairNextTile();
}

void GuiPrepareKeyPairNextTile(void)
{
    switch (g_prepareKeyPairWidget.currentTile) {
    case PREPARE_KEY_PAIR_NOTICE:
        break;
    case PREPARE_KEY_PAIR_PLUG_TO_PC:
        break;
    default:
        break;
    }
    g_prepareKeyPairWidget.currentTile++;
    GuiPrepareKeyPairRefresh();
    lv_obj_set_tile_id(g_prepareKeyPairWidget.tileView, g_prepareKeyPairWidget.currentTile, 0, LV_ANIM_OFF);
}

void GuiPrepareKeyPairRefresh(void)
{
    switch (g_prepareKeyPairWidget.currentTile) {
    case PREPARE_KEY_PAIR_NOTICE:
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Prepare Key Pair");
        break;
    case PREPARE_KEY_PAIR_PLUG_TO_PC:
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Plug to computer");
        break;
    case PREPARE_KEY_PAIR_CONFIRM_PUBKEY:
        SetConfigWalletPubkey(g_pageWidget->navBarWidget, "Confirm Pubkey");
        break;
    }
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
}

void GuiPrepareKeyPairPrevTile(void)
{
    switch (g_prepareKeyPairWidget.currentTile) {
    case PREPARE_KEY_PAIR_NOTICE:
        GuiCloseCurrentWorkingView();
        return;
    case PREPARE_KEY_PAIR_PLUG_TO_PC:
        break;
    case PREPARE_KEY_PAIR_CONFIRM_PUBKEY:
        return;
    default:
        break;
    }
    g_prepareKeyPairWidget.currentTile--;
    GuiPrepareKeyPairRefresh();
    lv_obj_set_tile_id(g_prepareKeyPairWidget.tileView, g_prepareKeyPairWidget.currentTile, 0, LV_ANIM_OFF);
}

static void ConfirmSetPubkey(void)
{
    int32_t ret = SUCCESS_CODE;
    uint8_t updatePubKey[65] = {0};
    static bool isSettingPubkey = false;

    printf("pubkey len = %d\n", g_verifyParam.pubkeyLen);
    PrintArray("Pubkey to set", g_verifyParam.pubkey, g_verifyParam.pubkeyLen);

    if (g_verifyParam.pubkeyLen == 64) {
        updatePubKey[0] = 0x04;
        memcpy_s(updatePubKey + 1, sizeof(updatePubKey) - 1, g_verifyParam.pubkey, 64);
    } else if (g_verifyParam.pubkeyLen == 65) {
        if ((uint8_t)g_verifyParam.pubkey[0] == 0x04) {
            memcpy_s(updatePubKey, sizeof(updatePubKey), g_verifyParam.pubkey, 65);
        } else {
            updatePubKey[0] = 0x04;
            memcpy_s(updatePubKey + 1, sizeof(updatePubKey) - 1, g_verifyParam.pubkey, 64);
        }
    } else {
        printf("confirm set pubkey failed: invalid pubkey len=%d\n", g_verifyParam.pubkeyLen);
        return;
    }

    printf("confirm set pubkey\n");
    // ret = SetUpdatePubKey(updatePubKey);
    ret = isSettingPubkey ? -1 : SUCCESS_CODE;
    isSettingPubkey = !isSettingPubkey;

    if (ret != SUCCESS_CODE) {
        const char *message = "setting pubkey failed";
        printf("SetUpdatePubKey failed, ret=%d\n", ret);
        SendUsbPubkeySetResult(PRS_SET_PUBKEY_ERROR, message);
    } else {
        const char *message = "setting pubkey success";
        printf("SetUpdatePubKey success, ret=%d\n", ret);
        SendUsbPubkeySetResult(PRS_SET_PUBKEY_SET_SUCCESS, message);
    }
    printf("SetUpdatePubKey ret=%d\n", ret);
    lv_slider_set_value(g_prepareKeyPairWidget.confirmSlider, 0, LV_ANIM_OFF);
    // if (ret != SUCCESS_CODE) {
    //     return;
    // }
    // GuiFrameOpenView(&g_setupDoneView);
}

static void CheckSliderProcessHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_RELEASED) {
        int32_t value = lv_slider_get_value(lv_event_get_target(e));
        printf("CheckSliderProcessHandler: released, value=%d\n", value);
        if (value >= CONFIRM_SET_PUBKEY_PROCESS) {
            lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_OFF);
            ConfirmSetPubkey();
        } else {
            lv_slider_set_value(lv_event_get_target(e), 0, LV_ANIM_ON);
        }
    }
}
