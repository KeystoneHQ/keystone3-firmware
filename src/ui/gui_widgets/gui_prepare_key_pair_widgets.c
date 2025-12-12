#include "define.h"
#include "gui.h"
#include "lvgl.h"
#include "gui_framework.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_qr_hintbox.h"
#include "gui_page.h"
#include "gui_button.h"

typedef enum {
    PREPARE_KEY_PAIR_NOTICE,
    PREPARE_KEY_PAIR_PLUG_TO_PC,
    PREPARE_KEY_PAIR_CONFIRM_PUBKEY,
    PREPARE_KEY_PAIR_SETUP_COMPLETE,

    PREPARE_KEY_BUTT,
} PREPARE_KEY_PAIR_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *tileView;
} PrepareKeyPairWidget_t;

static PrepareKeyPairWidget_t g_prepareKeyPairWidget;
static PageWidget_t *g_pageWidget = NULL;

static void GuiPrepareKeyPairCreateNoticeTile(lv_obj_t *parent);
static void GuiCreatePlugToPcTile(lv_obj_t *parent);

void GuiPrepareKeyPairInit(void)
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    g_prepareKeyPairWidget.tileView = tileView;
    lv_obj_t *tile = lv_tileview_add_tile(tileView, PREPARE_KEY_PAIR_NOTICE, 0, LV_DIR_RIGHT);
    GuiPrepareKeyPairCreateNoticeTile(tile);

    tile = lv_tileview_add_tile(tileView, PREPARE_KEY_PAIR_PLUG_TO_PC, 0, LV_DIR_RIGHT);
    GuiCreatePlugToPcTile(tile);

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
}

void GuiPrepareKeyPairNextTile()
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
    if (g_prepareKeyPairWidget.currentTile == PREPARE_KEY_PAIR_SETUP_COMPLETE) {
    } else {
        switch (g_prepareKeyPairWidget.currentTile) {
        case PREPARE_KEY_PAIR_NOTICE:
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Prepare Key Pair");
            break;
        case PREPARE_KEY_PAIR_PLUG_TO_PC:
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Plug to computer");
            break; 
        case PREPARE_KEY_PAIR_CONFIRM_PUBKEY:
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Confirm Public Key");
            break;
        }
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    }
}

void GuiPrepareKeyPairPrevTile(void)
{
    switch (g_prepareKeyPairWidget.currentTile) {
    case PREPARE_KEY_PAIR_NOTICE:
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