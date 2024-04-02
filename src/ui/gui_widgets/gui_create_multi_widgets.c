#include "gui.h"

#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_create_wallet_widgets.h"
#include "slip39.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "background_task.h"
#include "user_utils.h"
#include "motor_manager.h"
#include "gui_page.h"
#include <stdlib.h>

#define MULTI_WALLET_DEFAULT_CO_SINGERS             (5)
#define MULTI_WALLET_DEFAULT_SIGNERS                (3)

typedef enum {
    CREATE_MULTI_SET_NAME = 0,
    CREATE_MULTI_SELECT_SLICE,
    CREATE_MULTI_SELECT_FORMAT,
    CREATE_MULTI_CONFIRM_CO_SIGNERS,
    CREATE_MULTI_IMPORT_SDCARD_XPUB,
    CREATE_MULTI_CONFIRM,
    CREATE_MULTI_WRITE_SE,

    CREATE_MULTI_BUTT,
} CREATE_MULTI_ENUM;

typedef struct CreateMultiWidget {
    uint8_t     currentTile;
    uint8_t     currentSlice;
    lv_obj_t    *tileView;
    lv_obj_t    *selectSlice;
    lv_obj_t    *stepCont;
} CreateMultiWidget_t;
static CreateMultiWidget_t g_createMultiTileView;

typedef struct {
    lv_obj_t    *cont;
    uint8_t     coSingers;
    uint8_t     singers;
    lv_obj_t    *stepLabel;
    lv_obj_t    *noticeLabel;
    lv_obj_t    *coSingersKb;
    lv_obj_t    *singersKb;
} SelectSliceWidget_t;
static SelectSliceWidget_t g_selectSliceTile;

typedef struct {
    lv_obj_t    *cont;
    lv_obj_t    *titleLabel;
    lv_obj_t    *noticeLabel;
} CustodianWidget_t;
static CustodianWidget_t g_custodianTile;

typedef struct {
    char *title;
    char *subTitle;
    char *path;
} AddressSettingsItem_t;

typedef struct {
    MnemonicKeyBoard_t *keyBoard;
    lv_obj_t    *nextCont;
    lv_obj_t    *noticeLabel;
} MultiBackupWidget_t;
static MultiBackupWidget_t g_multiBackupTile;
static MultiBackupWidget_t g_multiConfirmTile;

static uint8_t g_phraseCnt = 20;
static KeyBoard_t *g_nameWalletKb = NULL;
static uint8_t g_currId = 0;
static char g_randomBuff[BUFFER_SIZE_512];
static lv_obj_t *g_noticeWindow = NULL;
static uint8_t g_entropyMethod;
static PageWidget_t *g_pageWidget;
static void SelectCheckBoxHandler(lv_event_t* e);
static void GuiCreateNameWalletWidget(lv_obj_t *parent);
static void GuiCreateAddressSettingsWidget(lv_obj_t *parent);
static void ImportSdCardXpubHandler(lv_event_t *e);

static const AddressSettingsItem_t g_mainNetAddressSettings[] = {
    {"Native SegWit",   "P2WPKH",           "m/84'/0'/0'"},
    {"Nested SegWit",   "P2SH-P2WPKH",      "m/49'/0'/0'"},
    {"Legacy",          "P2PKH",            "m/44'/0'/0'"},
};

static const AddressSettingsItem_t g_testNetAddressSettings[] = {
    {"Native SegWit",   "P2WPKH",           "m/84'/1'/0'"},
    {"Nested SegWit",   "P2SH-P2WPKH",      "m/49'/1'/0'"},
    {"Legacy",          "P2PKH",            "m/44'/1'/0'"},
};

static uint32_t g_addressSettingsNum = sizeof(g_mainNetAddressSettings) / sizeof(g_mainNetAddressSettings[0]);
static const AddressSettingsItem_t *g_addressSettings = g_mainNetAddressSettings;

static void ImportMultiXpubHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MoreInfoTable_t moreInfoTable[] = {
        {.name = _("create_multi_wallet_import_xpub_qr"), .src = &imgScanImport, .callBack = UnHandler, NULL},
        {.name = _("create_multi_wallet_import_xpub_sdcard"), .src = &imgSdcardImport, .callBack = NextTileHandler, NULL},
    };

    if (code == LV_EVENT_CLICKED) {
        g_noticeWindow = GuiCreateMoreInfoHintBox(NULL, NULL, &moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), true);
    }
}

static void MultiUpdateTileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    }
}

static void ContinueStopCreateHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(obj));
        g_noticeWindow = NULL;
        if (lv_event_get_user_data(e) != NULL) {
            g_createMultiTileView.currentSlice = 0;
            GuiCLoseCurrentWorkingView();
        }
    }
}

static void ResetBtnTest(void)
{
    lv_btnmatrix_clear_btn_ctrl_all(g_multiConfirmTile.keyBoard->btnm, LV_BTNMATRIX_CTRL_CHECKED);
    GuiUpdateMnemonicKeyBoard(g_multiConfirmTile.keyBoard, g_randomBuff, true);
}

static void ResetBtnHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ResetBtnTest();
    }
}

static void StopCreateViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_noticeWindow = GuiCreateHintBox(lv_scr_act(), 480, 416, false);
        lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgWarn);
        lv_obj_align(img, LV_ALIGN_DEFAULT, 36, 432);
        lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("shamir_phrase_cancel_create_title"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 528);
        label = GuiCreateIllustrateLabel(g_noticeWindow, _("shamir_phrase_cancel_create_desc"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 580);
        lv_obj_t *btn = GuiCreateBtn(g_noticeWindow, _("shamir_phrase_continue_btn"));
        lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 36, 710);
        lv_obj_set_size(btn, 162, 66);
        lv_obj_add_event_cb(btn, ContinueStopCreateHandler, LV_EVENT_CLICKED, NULL);

        btn = GuiCreateBtn(g_noticeWindow, _("shamir_phrase_cancel_create_btn"));
        lv_obj_set_style_bg_color(btn, RED_COLOR, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 229, 710);
        lv_obj_set_size(btn, 215, 66);
        lv_obj_add_event_cb(btn, ContinueStopCreateHandler, LV_EVENT_CLICKED, g_noticeWindow);
    }
}

static void NumSelectSliceHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    uint32_t currentId = lv_btnmatrix_get_selected_btn(obj);
    if (code == LV_EVENT_DRAW_PART_BEGIN) {
        if (currentId >= 0xFFFF) {
            return;
        }
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            if (currentId == dsc->id) {
                dsc->rect_dsc->border_color = ORANGE_COLOR;
            } else {
                dsc->rect_dsc->border_color = DARK_BG_COLOR;
            }
        }
    } else if (code == LV_EVENT_CLICKED) {
        Vibrate(SLIGHT);
        if (currentId >= 0xFFFF) {
            lv_btnmatrix_set_selected_btn(g_selectSliceTile.singersKb, g_selectSliceTile.singers - 2);
            lv_btnmatrix_set_selected_btn(g_selectSliceTile.coSingersKb, g_selectSliceTile.coSingers - 2);
            return;
        }
        if (obj == g_selectSliceTile.coSingersKb) {
            g_selectSliceTile.coSingers = currentId + 2;
            lv_btnmatrix_set_selected_btn(g_selectSliceTile.singersKb, g_selectSliceTile.singers - 2);
            GuiUpdateSsbKeyBoard(g_selectSliceTile.singersKb, g_selectSliceTile.coSingers);
            if (g_selectSliceTile.singers > g_selectSliceTile.coSingers) {
                g_selectSliceTile.singers = g_selectSliceTile.coSingers;
                lv_btnmatrix_set_selected_btn(g_selectSliceTile.singersKb, g_selectSliceTile.coSingers - 2);
            }
        } else {
            g_selectSliceTile.singers = currentId + 2;
            lv_btnmatrix_set_selected_btn(g_selectSliceTile.coSingersKb, g_selectSliceTile.coSingers - 2);
        }
        char tempBuf[BUFFER_SIZE_16];
        snprintf_s(tempBuf, BUFFER_SIZE_16, "%d/%d", g_selectSliceTile.singers, g_selectSliceTile.coSingers);
        lv_label_set_text(g_selectSliceTile.stepLabel, tempBuf);
    }
}

static void GuiMultiSelectSliceWidget(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateNoticeLabel(parent, _("create_multi_wallet_co_signers"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 12);

    label = GuiCreateIllustrateLabel(parent, _("create_multi_wallet_signers"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 438 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("sdcard_format_confirm"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 576 - GUI_MAIN_AREA_OFFSET);

    g_selectSliceTile.coSingers = MULTI_WALLET_DEFAULT_CO_SINGERS;
    g_selectSliceTile.singers = MULTI_WALLET_DEFAULT_SIGNERS;

    lv_obj_t *btnm = GuiCreateNumKeyboard(parent, NumSelectSliceHandler, NUM_KEYBOARD_SLICE, NULL);
    lv_obj_align(btnm, LV_ALIGN_DEFAULT, 36, 198 - GUI_MAIN_AREA_OFFSET);
    lv_btnmatrix_set_selected_btn(btnm, g_selectSliceTile.coSingers - 2);
    g_selectSliceTile.coSingersKb = btnm;

    btnm = GuiCreateNumKeyboard(parent, NumSelectSliceHandler, NUM_KEYBOARD_SLICE, g_selectSliceTile.singersKb);
    lv_obj_align(btnm, LV_ALIGN_DEFAULT, 36, 438 - GUI_MAIN_AREA_OFFSET);
    lv_btnmatrix_set_selected_btn(btnm, g_selectSliceTile.singers - 2);
    g_selectSliceTile.singersKb = btnm;
    GuiUpdateSsbKeyBoard(g_selectSliceTile.singersKb, g_selectSliceTile.coSingers);

    label = GuiCreateTextLabel(g_createMultiTileView.stepCont, "3 of 5");
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 24);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_selectSliceTile.stepLabel = label;

    label = GuiCreateNoticeLabel(g_createMultiTileView.stepCont, _("create_multi_wallet_co_sign_policy"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 60);
    g_selectSliceTile.noticeLabel = label;
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
}

static void GuiMultiSelectFormatWidget(lv_obj_t *parent)
{
    uint16_t hintHeight = 48;
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateNoticeLabel(parent, _("create_multi_wallet_select_format"));
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 0);
    g_custodianTile.titleLabel = label;

    GuiCreateAddressSettingsWidget(parent);
}

static void GuiMultiConfirmSignersWidget(lv_obj_t *parent)
{
    char tempBuff[BUFFER_SIZE_32] = {0};
    snprintf(tempBuff, sizeof(tempBuff), "%s %d/%d", _("create_multi_wallet_co_signers"), 1, g_selectSliceTile.coSingers);
    lv_obj_t *label = GuiCreateTitleLabel(parent, tempBuff);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("create_multi_wallet_co_signers_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);
    g_multiBackupTile.noticeLabel = label;

    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 296);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 382 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateNoticeLabel(cont, _("create_multi_wallet_multi_xpub"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 24);

    label = GuiCreateIllustrateLabel(cont, "xpub69cicR2MFe9QbMVTMHN882fGtXBKQV4g9gqWNZN7aEM9RASi3WmUzgPF9md8fLfUNuF4znQ8937VQrjG2bG8VgU7rjhUR8qCfBL9hJDQogL");
    lv_obj_set_width(label, 360);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 70);

    lv_obj_t *button = GuiCreateImgLabelAdaptButton(cont, _("Import"), &imgImport, ImportMultiXpubHandler, NULL);
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 12, -12);
    lv_obj_set_size(button, 384, 56);
}

static void GuiMultiConfirmWidget(lv_obj_t *parent)
{
    lv_obj_t *btn = GuiCreateSelectButton(cont, table[i].name, table[i].src, table[i].callBack, table[i].param, false);
}

void GuiCreateMultiStepCont(void)
{
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    g_createMultiTileView.stepCont = cont;
    lv_obj_add_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);

    // label = GuiCreateTextLabel(cont, "3 of 5");
    // lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    // lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 24);
    // g_selectSliceTile.stepLabel = label;
    // label = GuiCreateIllustrateLabel(cont, _("create_multi_wallet_co_sign_policy"));
    // lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 60);

    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 348, 24);
    lv_obj_add_event_cb(btn, MultiUpdateTileHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
}

void GuiCreateMultiInit(void)
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, CREATE_MULTI_SET_NAME, 0, LV_DIR_HOR);
    GuiCreateNameWalletWidget(tile);

    GuiCreateMultiStepCont();

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_SELECT_SLICE, 0, LV_DIR_HOR);
    GuiMultiSelectSliceWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_SELECT_FORMAT, 0, LV_DIR_HOR);
    GuiMultiSelectFormatWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_CONFIRM_CO_SIGNERS, 0, LV_DIR_HOR);
    GuiMultiConfirmSignersWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_IMPORT_SDCARD_XPUB, 0, LV_DIR_HOR);
    GuiMultiConfirmWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_WRITE_SE, 0, LV_DIR_HOR);
    GuiWriteSeWidget(tile);

    g_createMultiTileView.currentTile = CREATE_MULTI_SET_NAME;
    g_createMultiTileView.tileView = tileView;

    lv_obj_set_tile_id(g_createMultiTileView.tileView, 0, 0, LV_ANIM_OFF);
}

int8_t GuiCreateMultiNextSlice(void)
{
    g_createMultiTileView.currentSlice++;
    if (g_createMultiTileView.currentSlice == g_selectSliceTile.coSingers) {
        // GuiModelWriteSe();
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
        return SUCCESS_CODE;
    }
    lv_label_set_text_fmt(g_custodianTile.titleLabel, _("shamir_phrase_multi_number_fmt"), g_createMultiTileView.currentSlice + 1, g_selectSliceTile.coSingers);
    lv_label_set_text_fmt(g_custodianTile.noticeLabel, _("shamir_phrase_multi_notice_fmt"), g_createMultiTileView.currentSlice + 1);
    lv_label_set_text_fmt(g_multiBackupTile.noticeLabel, _("shamir_phrase_multi_backup_notice_fmt"), g_createMultiTileView.currentSlice + 1);
    lv_label_set_text_fmt(g_multiConfirmTile.noticeLabel, _("shamir_phrase_multi_confirm_notice_fmt"), g_createMultiTileView.currentSlice + 1);

    // g_createMultiTileView.currentTile = CREATE_MULTI_CUSTODIAN;
    // ResetBtnTest();
    // GuiUpdateMnemonicKeyBoard(g_multiBackupTile.keyBoard, SecretCacheGetSlip39Mnemonic(g_createMultiTileView.currentSlice), false);
    // SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    // lv_obj_set_tile_id(g_createMultiTileView.tileView, g_createMultiTileView.currentTile, 0, LV_ANIM_OFF);
    // ArrayRandom(SecretCacheGetSlip39Mnemonic(g_createMultiTileView.currentSlice), g_randomBuff, g_phraseCnt);
    // GuiUpdateMnemonicKeyBoard(g_multiConfirmTile.keyBoard, g_randomBuff, true);
    return SUCCESS_CODE;
}

int8_t GuiCreateMultiNextTile(void)
{
    switch (g_createMultiTileView.currentTile) {
    case CREATE_MULTI_SET_NAME:
        lv_obj_clear_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_MULTI_SELECT_SLICE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        break;
    case CREATE_MULTI_SELECT_FORMAT:
        printf("%s %d..\n", __func__, __LINE__);
        break;
    case CREATE_MULTI_CONFIRM_CO_SIGNERS:
        // SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, _("single_phrase_reset"));
        // SetRightBtnCb(g_pageWidget->navBarWidget, ResetBtnHandler, NULL);
        // lv_obj_add_flag(g_multiBackupTile.nextCont, LV_OBJ_FLAG_HIDDEN);
        printf("%s %d..\n", __func__, __LINE__);
        break;
    case CREATE_MULTI_CONFIRM:
        // SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        // SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        // GuiModelSlip39WriteSe(g_phraseCnt);
        printf("%s %d..\n", __func__, __LINE__);
        break;
    }
    g_createMultiTileView.currentTile++;
    lv_obj_set_tile_id(g_createMultiTileView.tileView, g_createMultiTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiCreateMultiPrevTile(void)
{
    switch (g_createMultiTileView.currentTile) {
    case CREATE_MULTI_SELECT_SLICE:
        break;
    case CREATE_MULTI_SELECT_FORMAT:
        lv_obj_clear_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_MULTI_CONFIRM_CO_SIGNERS:
        lv_obj_add_flag(g_multiBackupTile.nextCont, LV_OBJ_FLAG_HIDDEN);
        break;
    }

    g_createMultiTileView.currentTile--;
    lv_obj_set_tile_id(g_createMultiTileView.tileView, g_createMultiTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiCreateMultiDeInit(void)
{
    GUI_DEL_OBJ(g_noticeWindow)
    g_currId = 0;
    g_phraseCnt = 20;
    g_selectSliceTile.coSingers = SLIP39_DEFAULT_MEMBER_COUNT;
    g_selectSliceTile.singers = SLIP39_DEFAULT_MEMBER_THRESHOLD;
    memset_s(g_randomBuff, 512, 0, 512);
    GUI_DEL_OBJ(g_multiBackupTile.nextCont)
    GUI_DEL_OBJ(g_createMultiTileView.stepCont)
    CLEAR_OBJECT(g_createMultiTileView);
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiCreateMultiRefresh(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    if (g_createMultiTileView.currentTile == CREATE_MULTI_SELECT_SLICE) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    } else if (g_createMultiTileView.currentTile == CREATE_MULTI_CONFIRM_CO_SIGNERS) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
    } else if (g_createMultiTileView.currentTile == CREATE_MULTI_CONFIRM) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, ResetBtnHandler, NULL);
    } else if (g_createMultiTileView.currentTile == CREATE_MULTI_WRITE_SE) {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    }
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
}

static void GuiCreateNameWalletWidget(lv_obj_t *parent)
{
    char tempBuf[BUFFER_SIZE_16] = {0};
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("single_backup_namewallet_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateNoticeLabel(parent, _("create_multi_wallet_name_desc"));
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    g_nameWalletKb = GuiCreateFullKeyBoard(parent, ReadyNextTileHandler, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_nameWalletKb, 0);
    lv_obj_set_size(g_nameWalletKb->ta, 300, 60);
    lv_obj_set_style_text_opa(g_nameWalletKb->ta, LV_OPA_100, 0);
    lv_obj_align(g_nameWalletKb->ta, LV_ALIGN_DEFAULT, 36, 164);
    lv_textarea_set_max_length(g_nameWalletKb->ta, 16);
    lv_textarea_set_one_line(g_nameWalletKb->ta, true);
    lv_textarea_set_text(g_nameWalletKb->ta, _("create_multi_wallet_default_name"));
    snprintf_s(tempBuf, BUFFER_SIZE_16, "%d/16", strnlen_s(_("create_multi_wallet_default_name"), 16));
    lv_obj_t *progressLabel = GuiCreateNoticeLabel(parent, tempBuf);
    lv_obj_align(progressLabel, LV_ALIGN_TOP_RIGHT, -36, 384 - GUI_MAIN_AREA_OFFSET);
    GuiSetEnterProgressLabel(progressLabel);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align_to(line, g_nameWalletKb->ta, LV_ALIGN_BOTTOM_LEFT, -50, 10);
}

static void GuiCreateAddressSettingsWidget(lv_obj_t *parent)
{
    lv_obj_t *cont, *line, *label;
    static lv_point_t points[2] = {{0, 0}, {360, 0}};
    char string[BUFFER_SIZE_64];
    char labelText[ADDRESS_MAX_LEN] = {0};

    cont = GuiCreateContainerWithParent(parent, 408, 306);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
#ifdef BTC_ONLY
    g_addressSettings = GetIsTestNet() ? g_testNetAddressSettings : g_mainNetAddressSettings;
#endif
    for (uint32_t i = 0; i < g_addressSettingsNum; i++) {
        label = GuiCreateTextLabel(cont, g_addressSettings[i].title);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 20 + 103 * i);
        snprintf_s(string, BUFFER_SIZE_64, "%s (%s)", g_addressSettings[i].subTitle, g_addressSettings[i].path);
        label = GuiCreateNoticeLabel(cont, string);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i != (g_addressSettingsNum - 1)) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * (i + 1));
        }
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(cont, "");
        lv_obj_align(checkBox, LV_ALIGN_TOP_LEFT, -20, 10 + 102 * i);
        // lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        // lv_obj_set_style_bg_opa(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        // lv_obj_set_style_border_width(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_outline_width(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, 0, LV_PART_MAIN);
        // lv_obj_set_style_shadow_width(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, 0, LV_PART_MAIN);
        // lv_obj_add_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        // lv_obj_add_event_cb(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, AddressSettingsCheckHandler, LV_EVENT_CLICKED, NULL);

        // g_utxoReceiveWidgets.addressSettingsWidgets[i].checkedImg = GuiCreateImg(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, &imgMessageSelect);
        // lv_obj_align(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        // lv_obj_add_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        // g_utxoReceiveWidgets.addressSettingsWidgets[i].uncheckedImg = GuiCreateImg(g_utxoReceiveWidgets.addressSettingsWidgets[i].checkBox, &imgUncheckCircle);
        // lv_obj_align(g_utxoReceiveWidgets.addressSettingsWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        // lv_obj_clear_flag(g_utxoReceiveWidgets.addressSettingsWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    // lv_obj_clear_flag(g_utxoReceiveWidgets.addressSettingsWidgets[g_addressType[g_currentAccountIndex]].checkedImg, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_flag(g_utxoReceiveWidgets.addressSettingsWidgets[g_addressType[g_currentAccountIndex]].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *egCont = GuiCreateContainerWithParent(parent, 408, 186);
    lv_obj_align_to(egCont, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);
    // ShowEgAddressCont(g_egCont);

    lv_obj_t *tmCont = GuiCreateContainerWithParent(parent, 480, 114);
    lv_obj_align(tmCont, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(tmCont, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_t *btn = GuiCreateBtn(tmCont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -36, 0);
    // lv_obj_add_event_cb(btn, ConfirmAddrTypeHandler, LV_EVENT_CLICKED, NULL);
    // g_utxoReceiveWidgets.confirmAddrTypeBtn = btn;
    // UpdateConfirmAddrTypeBtn();
}
