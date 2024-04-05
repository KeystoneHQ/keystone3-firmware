#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "version.h"
#include "err_code.h"
#include "firmware_update.h"
#include "gui_page.h"
#include "gui_import_multisig_wallet_info_widgets.h"
#include "librust_c.h"
#include "keystore.h"
#include "fingerprint_process.h"
#include "account_public_info.h"
#include "multi_sig_wallet_manager.h"
#include "gui_chain.h"
#include "stdint.h"
#include "user_memory.h"
#ifndef COMPILE_SIMULATOR
#include "drv_sdcard.h"
#include "user_fatfs.h"
#else
#include "simulator_mock_define.h"
#endif
#include <gui_keyboard_hintbox.h>

typedef enum {
    IMPORT_MULTI_SELECT_METHOD = 0,
    IMPORT_MULTI_SELECT_SDCARD_FILE,

    IMPORT_MULTI_BUTT,
} IMPORT_MULTI_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *tileView;
} ImportMultiInfoWidget_t;

static ImportMultiInfoWidget_t g_importMultiInfo;
static PageWidget_t *g_pageWidget;
static lv_obj_t *g_noticeWindow = NULL;
static char g_fileList[10][64] = {0};

static void GuiSelectImportMultisigWidget(lv_obj_t *parent);
static void SelectMicroCardFileHandler(lv_event_t *e);
static void GuiMultiImportSdCardListWidget(lv_obj_t *parent);
static void OpenFileNextTileHandler(lv_event_t *e);

void GuiImportMultisigWalletInfoWidgetsInit(void)
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, IMPORT_MULTI_SELECT_METHOD, 0, LV_DIR_HOR);
    GuiSelectImportMultisigWidget(tile);

    tile = lv_tileview_add_tile(tileView, IMPORT_MULTI_SELECT_SDCARD_FILE, 0, LV_DIR_HOR);
    GuiMultiImportSdCardListWidget(tile);

    g_importMultiInfo.currentTile = IMPORT_MULTI_SELECT_METHOD;
    g_importMultiInfo.tileView = tileView;

    lv_obj_set_tile_id(g_importMultiInfo.tileView, g_importMultiInfo.currentTile, 0, LV_ANIM_OFF);
}

void GuiImportMultisigWalletInfoWidgetsDeInit()
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }

    ClearSecretCache();
}

void GuiImportMultisigWalletInfoWidgetsRefresh()
{
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    if (g_importMultiInfo.currentTile == IMPORT_MULTI_SELECT_METHOD) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, CloseCurrentViewHandler, NULL);
    } else if (g_importMultiInfo.currentTile == IMPORT_MULTI_SELECT_SDCARD_FILE) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("import_multi_wallet_via_micro_title"));
    }
}

void GuiImportMultisigWalletInfoWidgetsRestart()
{
}

static void GuiSelectImportMultisigWidget(lv_obj_t *parent)
{
    lv_obj_t *label, *img, *button, *imgArrow, *line;
    label = GuiCreateTitleLabel(parent, _("wallet_profile_import_multi_wallet"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);
    label = GuiCreateNoticeLabel(parent, _("wallet_profile_import_multi_wallet_desc"));
    lv_obj_set_width(label, 400);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 72);

    img = GuiCreateImg(parent, &imgScanImport);
    label = GuiCreateLittleTitleLabel(parent, _("import_multi_wallet_via_camera"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {24, 40},},
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {76, 40},},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {372, 40},},
    };
    button = GuiCreateButton(parent, 432, 120, table, NUMBER_OF_ARRAYS(table), UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 180);

    table[0].obj = GuiCreateImg(parent, &imgSdcardImport);
    table[1].obj = GuiCreateLittleTitleLabel(parent, _("import_multi_wallet_via_micro_card"));
    table[2].obj = GuiCreateImg(parent, &imgArrowRight);
    button = GuiCreateButton(parent, 432, 120, table, NUMBER_OF_ARRAYS(table), SelectMicroCardFileHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 300);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 180);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 300);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 419);
}

static void GuiMultiImportSdCardListWidget(lv_obj_t *parent)
{
    GuiAddObjFlag(parent, LV_OBJ_FLAG_SCROLLABLE);
    char *buffer = EXT_MALLOC(1024 * 5);
    uint32_t number = 0;
    int i = 0;
#ifdef COMPILE_SIMULATOR
    FatfsGetFileName("C:/assets/sd", buffer, &number, 1024 * 5);
#else
    FatfsGetFileName("0:", buffer, &number, 1024 * 5);
#endif
    char *token = strtok(buffer, " ");
    while (token != NULL) {
        strncpy(g_fileList[i], token, sizeof(g_fileList[i]));
        token = strtok(NULL, " ");
        lv_obj_t *btn = GuiCreateSelectButton(parent, g_fileList[i], &imgArrowRight, OpenFileNextTileHandler, g_fileList[i], false);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 84 * i);
        i++;
        if (i == 10) {
            break;
        }
    }
    EXT_FREE(buffer);
}

int8_t GuiImportMultiInfoNextTile(void)
{
    switch (g_importMultiInfo.currentTile) {
    case IMPORT_MULTI_SELECT_METHOD:
        break;
    case IMPORT_MULTI_SELECT_SDCARD_FILE:
        break;
    }
    g_importMultiInfo.currentTile++;
    GuiImportMultisigWalletInfoWidgetsRefresh();
    lv_obj_set_tile_id(g_importMultiInfo.tileView, g_importMultiInfo.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiImportMultiInfoPrevTile(void)
{
    switch (g_importMultiInfo.currentTile) {
    case IMPORT_MULTI_SELECT_METHOD:
        break;
    case IMPORT_MULTI_SELECT_SDCARD_FILE:
        break;
    }
    g_importMultiInfo.currentTile--;
    GuiImportMultisigWalletInfoWidgetsRefresh();
    lv_obj_set_tile_id(g_importMultiInfo.tileView, g_importMultiInfo.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

static void OpenFileNextTileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    char *path = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        char *walletConfig = FatfsFileRead(path);
        printf("walletconfig: %s\n", walletConfig);
        GuiFrameOpenViewWithParam(&g_importMultisigWalletView, walletConfig, strnlen_s(walletConfig, 1024));
    }
}

static void SelectMicroCardFileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (SdCardInsert()) {
            GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
        } else {
            g_noticeWindow = GuiCreateErrorCodeHintbox(ERR_UPDATE_SDCARD_NOT_DETECTED, &g_noticeWindow);
        }
    }
}