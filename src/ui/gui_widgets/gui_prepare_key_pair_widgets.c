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
static PubkeySignatureVerifyParam_t *g_verifyParam = NULL;

static void GuiPrepareKeyPairCreateNoticeTile(lv_obj_t *parent);
static void GuiCreatePlugToPcTile(lv_obj_t *parent);
static void GuiCreateConfirmPubkeyTile(lv_obj_t *parent);
static void CheckSliderProcessHandler(lv_event_t *e);
int32_t FormatPublicKeyHexStr(char *pubkey, uint32_t maxLen, char *pubKeyStr, uint32_t pubKeyStrLen);

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
    char pubkey[65] = {0};
    GetUpdatePubKey(pubkey);
    FormatPublicKeyHexStr(pubkey, sizeof(pubkey), pubkeyStr, sizeof(pubkeyStr));
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
    g_verifyParam = (PubkeySignatureVerifyParam_t *)param;
    PubkeySignatureVerifyParam_t *verifyParam = g_verifyParam;
    PrintArray("Pubkey from CLI", (uint8_t *)verifyParam->pubkey, verifyParam->pubkeyLen);
    PrintArray("Signature from CLI", (uint8_t *)verifyParam->signature, 64);

    if (k1_verify_signature((uint8_t *)verifyParam->signature, (uint8_t *)verifyParam->pubkey, (uint8_t *)verifyParam->pubkey) == false) {
        printf("GuiPrepareKeyPairOnPubkeyVerifySuccess: signature verification failed\n");
        return;
    } else {
        printf("GuiPrepareKeyPairOnPubkeyVerifySuccess: signature verification success\n");
        char pubkeyStr[256] = {0};
        FormatPublicKeyHexStr(verifyParam->pubkey, verifyParam->pubkeyLen, pubkeyStr, sizeof(pubkeyStr));
        lv_label_set_text(g_prepareKeyPairWidget.pubkeyLabel, pubkeyStr);
        GuiPrepareKeyPairNextTile();
    }
}

void GuiPrepareKeyPairNextTile(void)
{
    switch (g_prepareKeyPairWidget.currentTile) {
    case PREPARE_KEY_PAIR_NOTICE:
        break;
    case PREPARE_KEY_PAIR_PLUG_TO_PC:
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
    printf("%s %d..\n", __func__, __LINE__);
    switch (g_prepareKeyPairWidget.currentTile) {
    case PREPARE_KEY_PAIR_NOTICE:
        printf("%s %d..\n", __func__, __LINE__);
        GuiCloseCurrentWorkingView();
        return;
    case PREPARE_KEY_PAIR_PLUG_TO_PC:
        break;
    default:
        break;
    }
    g_prepareKeyPairWidget.currentTile--;
    GuiPrepareKeyPairRefresh();
    lv_obj_set_tile_id(g_prepareKeyPairWidget.tileView, g_prepareKeyPairWidget.currentTile, 0, LV_ANIM_OFF);
}

static void ConfirmSetPubkey(void)
{
    printf("confirm set pubkey\n");
    GuiFrameOpenView(&g_setupDoneView);
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