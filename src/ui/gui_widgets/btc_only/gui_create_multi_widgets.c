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
#include "gui_keyboard_hintbox.h"
#include "account_public_info.h"
#include "gui_fullscreen_mode.h"
#include "cjson/cJSON.h"
#include "keystore.h"
#ifdef COMPILE_SIMULATOR
#include "simulator_model.h"
#else
#include "user_fatfs.h"
#endif
#define MULTI_WALLET_DEFAULT_CO_SINGERS             (3)
#define MULTI_WALLET_DEFAULT_SIGNERS                (2)

typedef enum {
    CREATE_MULTI_SET_NAME = 0,
    CREATE_MULTI_SELECT_SLICE,
    CREATE_MULTI_SELECT_FORMAT,
    CREATE_MULTI_CONFIRM_CO_SIGNERS,
    CREATE_MULTI_SELECT_SDCARD_XPUB,
    CREATE_MULTI_IMPORT_SDCARD_XPUB,
    CREATE_MULTI_WALLET_INFO,
    CREATE_MULTI_CREATE_SUCCESS,

    CREATE_MULTI_BUTT,
} CREATE_MULTI_ENUM;


typedef struct CreateMultiWidget {
    uint8_t currentTile;
    uint8_t currentSinger;
    lv_obj_t *tileView;
    lv_obj_t *selectSlice;
    lv_obj_t *stepCont;
    lv_obj_t *stepBtn;
    lv_obj_t *confirmBtn;
} CreateMultiWidget_t;

typedef struct {
    lv_obj_t *cont;
    uint8_t coSingers;
    uint8_t singers;
    lv_obj_t *stepLabel;
    lv_obj_t *noticeLabel;
    lv_obj_t *coSingersKb;
    lv_obj_t *singersKb;
} SelectSliceWidget_t;

typedef struct {
    lv_obj_t *titleLabel;
    lv_obj_t *noticeLabel;
    lv_obj_t *xpubLabel;
} CustodianWidget_t;

typedef struct {
    lv_obj_t *mfpLabel;
    lv_obj_t *xpubLabel;
    lv_obj_t *pathLabel;
} XpubWidget_t;

typedef struct {
    char mfp[9];
    char xpub[BUFFER_SIZE_128 - 9];
    char path[16];
} XpubWidgetCache_t;

typedef struct {
    lv_obj_t *cont;
    lv_obj_t *xpubCont;
    lv_obj_t *walletName;
    lv_obj_t *policy;
    lv_obj_t *format;
} MultiWalletInfo_t;

typedef struct {
    char *title;
    char *subTitle;
    char *multiPath;
    ChainType type;
} AddressSettingsItem_t;

static CreateMultiWidget_t g_createMultiTileView;
static SelectSliceWidget_t g_selectSliceTile;
static CustodianWidget_t g_custodianTile;
static XpubWidget_t g_xPubTile;
static XpubWidgetCache_t *g_xpubCache;
static MultiWalletInfo_t g_multiWalletInfo;
static lv_obj_t *g_formatCheckBox[3];
static KeyBoard_t *g_nameWalletKb = NULL;
static lv_obj_t *g_noticeWindow = NULL;
static lv_obj_t *g_importXpubBtn = NULL;
static PageWidget_t *g_pageWidget;
static ChainType g_chainType = XPUB_TYPE_BTC_MULTI_SIG_P2WSH;
static KeyboardWidget_t *g_keyboardWidget = NULL;
static char g_fileList[10][64] = {0};

static void SelectCheckBoxHandler(lv_event_t* e);
static void GuiCreateNameWalletWidget(lv_obj_t *parent);
static void GuiCreateAddressSettingsWidget(lv_obj_t *parent);
static void OpenFileNextTileHandler(lv_event_t *e);
static void GuiShowKeyboardHandler(lv_event_t *e);
lv_obj_t* CreateUTXOReceiveQRCode(lv_obj_t* parent, uint16_t w, uint16_t h);
static void SelectFormatHandler(lv_event_t *e);

static const AddressSettingsItem_t g_mainNetAddressSettings[] = {
    {"Native SegWit", "P2WPKH", "m/48'/0'/0'/2'", XPUB_TYPE_BTC_MULTI_SIG_P2WSH},
    {"Nested SegWit", "P2SH-P2WPKH", "m/48'/0'/0'/1'", XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH},
    {"Legacy", "P2PKH", "m/45'", XPUB_TYPE_BTC_MULTI_SIG_P2SH},
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
        {.name = _("create_multi_wallet_import_xpub_sdcard"), .src = &imgSdcardImport, .callBack = CloseParentAndNextHandler, &g_noticeWindow},
    };

    if (code == LV_EVENT_CLICKED) {
        g_noticeWindow = GuiCreateMoreInfoHintBox(NULL, NULL, moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), true);
    }
}

static void StopCreateViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_noticeWindow = GuiCreateGeneralHintBox(lv_scr_act(), &imgWarn, _("create_multi_wallet_cancel_title"), _("create_multi_wallet_cancel_desc"), NULL,
                         _("not_now"), WHITE_COLOR_OPA20, _("Cancel"), DEEP_ORANGE_COLOR);
        lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
        lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
        lv_obj_add_event_cb(rightBtn, CloseCurrentViewHandler, LV_EVENT_CLICKED, NULL);
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

    label = GuiCreateNoticeLabel(parent, _("create_multi_wallet_signers"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 294);

    label = GuiCreateNoticeLabel(parent, _("sdcard_format_confirm"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 576 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, "Native Segwit");
    lv_obj_t *img = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[] = {
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
        {.obj = img, .align = LV_ALIGN_RIGHT_MID, .position = {-18, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 408, 60, table, NUMBER_OF_ARRAYS(table),
                                       SelectFormatHandler, NULL);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 36, 474);

    g_selectSliceTile.coSingers = MULTI_WALLET_DEFAULT_CO_SINGERS;
    g_selectSliceTile.singers = MULTI_WALLET_DEFAULT_SIGNERS;

    lv_obj_t *btnm = GuiCreateNumKeyboard(parent, NumSelectSliceHandler, NUM_KEYBOARD_SLICE, NULL);
    lv_obj_align(btnm, LV_ALIGN_DEFAULT, 36, 198 - GUI_MAIN_AREA_OFFSET);
    lv_btnmatrix_set_selected_btn(btnm, g_selectSliceTile.coSingers - 2);
    g_selectSliceTile.coSingersKb = btnm;

    btnm = GuiCreateNumKeyboard(parent, NumSelectSliceHandler, NUM_KEYBOARD_SLICE, g_selectSliceTile.singersKb);
    lv_obj_align(btnm, LV_ALIGN_DEFAULT, 36, 336);
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
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateNoticeLabel(parent, _("create_multi_wallet_select_format"));
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 0);

    GuiCreateAddressSettingsWidget(parent);
}

static void GuiMultiConfirmSignersWidget(lv_obj_t *parent)
{
    char tempBuff[BUFFER_SIZE_32] = {0};
    snprintf(tempBuff, sizeof(tempBuff), "%s %d/%d", _("create_multi_wallet_co_signers"), 1, g_selectSliceTile.coSingers);
    lv_obj_t *label = GuiCreateTitleLabel(parent, tempBuff);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);
    g_custodianTile.titleLabel = label;

    label = GuiCreateIllustrateLabel(parent, _("create_multi_wallet_co_signers_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);
    g_custodianTile.noticeLabel = label;

    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 296);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 382 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateNoticeLabel(cont, _("create_multi_wallet_multi_xpub"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 24);

    label = GuiCreateIllustrateLabel(cont, "");
    lv_obj_set_width(label, 360);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 70);
    g_custodianTile.xpubLabel = label;

    lv_obj_t *button = GuiCreateImgLabelAdaptButton(cont, _("Import"), &imgImport, ImportMultiXpubHandler, NULL);
    lv_obj_set_style_text_color(lv_obj_get_child(button, 1), ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 12, -12);
    lv_obj_set_size(button, 384, 56);
    lv_obj_add_flag(button, LV_OBJ_FLAG_HIDDEN);
    g_importXpubBtn = button;
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

static void GuiMultiImportSdCardXpubWidget(lv_obj_t *parent)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 220);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 0);

    lv_obj_t *label = GuiCreateNoticeLabel(cont, "");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    g_xPubTile.mfpLabel = label;

    label = GuiCreateIllustrateLabel(cont, "");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 50);
    lv_obj_set_width(label, 360);
    g_xPubTile.xpubLabel = label;

    label = GuiCreateNoticeLabel(cont, "");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 174);
    g_xPubTile.pathLabel = label;

    lv_obj_t *btn = GuiCreateBtn(parent, _("Import"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiMultiUpdateWalletInfoWidget(void)
{
    lv_obj_set_height(g_multiWalletInfo.xpubCont, g_selectSliceTile.coSingers * 220 - 16);
    for (int i = 0; i < g_selectSliceTile.coSingers; i++) {
        char buff[8] = {0};
        snprintf(buff, sizeof(buff), "%d/%d", i + 1, g_selectSliceTile.coSingers);
        lv_obj_t *label = GuiCreateIllustrateLabel(g_multiWalletInfo.xpubCont, buff);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 16);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

        label = GuiCreateNoticeLabel(g_multiWalletInfo.xpubCont, g_xpubCache[i].mfp);
        GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
        label = GuiCreateIllustrateLabel(g_multiWalletInfo.xpubCont, g_xpubCache[i].xpub);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 50);
        lv_obj_set_width(label, 360);
        label = GuiCreateNoticeLabel(g_multiWalletInfo.xpubCont, g_xpubCache[i].path);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 174);
    }
}

static void GuiMultiShowWalletInfoWidget(lv_obj_t *parent)
{
    lv_obj_t *bgCont = GuiCreateContainerWithParent(parent, 480, 542);
    GuiAddObjFlag(bgCont, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *cont = GuiCreateContainerWithParent(bgCont, 408, 100);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 0);

    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("single_backup_namewallet_previnput"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    label = GuiCreateIllustrateLabel(cont, "12314");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 54);
    g_multiWalletInfo.walletName = label;

    cont = GuiCreateContainerWithParent(bgCont, 408, 62);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 116);

    label = GuiCreateNoticeLabel(cont, _("Policy"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 24, 0);
    label = GuiCreateIllustrateLabel(cont, "2 of 4");
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    g_multiWalletInfo.policy = label;

    cont = GuiCreateContainerWithParent(bgCont, 408, 62);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 194);

    label = GuiCreateNoticeLabel(cont, _("sdcard_format_confirm"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 24, 0);
    label = GuiCreateIllustrateLabel(cont, "");
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    g_multiWalletInfo.format = label;

    cont = GuiCreateContainerWithParent(bgCont, 408, 62);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 272);
    g_multiWalletInfo.xpubCont = cont;
}

static void GuiMultiCreateSuccessWidget(lv_obj_t *parent)
{
    GuiAddObjFlag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *label = GuiCreateNoticeLabel(parent, _("create_multi_wallet_success_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 518);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 122);

    lv_obj_t *qrcode = CreateUTXOReceiveQRCode(cont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(CreateUTXOReceiveQRCode, 420, 420);
    lv_obj_align(qrcode, LV_ALIGN_TOP_MID, 0, 36);

    label = GuiCreateIllustrateLabel(cont, _("Native SegWit"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 396);
    label = GuiCreateNoticeLabel(cont, _("Sample address:"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 430);
    label = GuiCreateIllustrateLabel(cont, _("Native SegWit"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 464);

    lv_obj_t *btn = GuiCreateBtn(parent, _("Done"));
    lv_obj_add_event_cb(btn, GoToHomeViewHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 666);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_set_style_outline_pad(btn, 24, LV_STATE_DEFAULT);
}

static void GuiCreateMultiStepCont(void)
{
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    g_createMultiTileView.stepCont = cont;
    lv_obj_add_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 348, 24);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    g_createMultiTileView.stepBtn = btn;

    btn = GuiCreateBtn(cont, _("Confirm"));
    lv_obj_add_event_cb(btn, GuiShowKeyboardHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    g_createMultiTileView.confirmBtn = btn;
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

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_SELECT_SDCARD_XPUB, 0, LV_DIR_HOR);
    GuiMultiImportSdCardListWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_IMPORT_SDCARD_XPUB, 0, LV_DIR_HOR);
    GuiMultiImportSdCardXpubWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_WALLET_INFO, 0, LV_DIR_HOR);
    GuiMultiShowWalletInfoWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_MULTI_CREATE_SUCCESS, 0, LV_DIR_HOR);
    GuiMultiCreateSuccessWidget(tile);

    g_createMultiTileView.currentTile = CREATE_MULTI_SET_NAME;
    g_createMultiTileView.tileView = tileView;

    lv_obj_set_tile_id(g_createMultiTileView.tileView, 0, 0, LV_ANIM_OFF);
}

int8_t GuiCreateMultiNextTile(uint8_t index)
{
    switch (g_createMultiTileView.currentTile) {
    case CREATE_MULTI_SET_NAME:
        lv_label_set_text(g_multiWalletInfo.walletName, lv_textarea_get_text(g_nameWalletKb->ta));
        lv_obj_clear_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_MULTI_SELECT_SLICE:
        if (index == 0xFF) {
            g_createMultiTileView.currentTile++;
        }
        lv_label_set_text_fmt(g_multiWalletInfo.policy, "%d of %d", g_selectSliceTile.singers, g_selectSliceTile.coSingers);
        lv_label_set_text_fmt(g_selectSliceTile.stepLabel, "%d of %d", g_selectSliceTile.singers, g_selectSliceTile.coSingers);
        g_xpubCache = SRAM_MALLOC(sizeof(XpubWidgetCache_t) * g_selectSliceTile.coSingers);
        char tempBuf[BUFFER_SIZE_16] = "12345";
        uint8_t mfp[4];
        GetMasterFingerPrint(mfp);
        for (int i = 0; i < sizeof(mfp); i++) {
            snprintf_s(&tempBuf[i * 2], BUFFER_SIZE_16 - strnlen_s(tempBuf, BUFFER_SIZE_16), "%02X", mfp[i]);
        }
        strcpy_s(g_xpubCache[g_createMultiTileView.currentSinger].mfp, sizeof(g_xpubCache[g_createMultiTileView.currentSinger].mfp), tempBuf);
        for (int i = 0; i < g_addressSettingsNum; i++) {
            if (g_addressSettings[i].type == g_chainType) {
                strcpy_s(g_xpubCache[g_createMultiTileView.currentSinger].path, sizeof(g_xpubCache[g_createMultiTileView.currentSinger].path), g_addressSettings[i].multiPath);
                break;
            }
        }
        strcpy_s(g_xpubCache[g_createMultiTileView.currentSinger].xpub, sizeof(g_xpubCache[g_createMultiTileView.currentSinger].xpub), GetCurrentAccountPublicKey(g_chainType));
        lv_label_set_text(g_custodianTile.xpubLabel, g_xpubCache[g_createMultiTileView.currentSinger].xpub);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, StopCreateViewHandler, NULL);
        break;
    case CREATE_MULTI_SELECT_FORMAT:
        for (int i = 0; i < g_addressSettingsNum; i++) {
            if (g_addressSettings[i].type == g_chainType) {
                lv_label_set_text(g_multiWalletInfo.format, g_addressSettings[i].multiPath);
                break;
            }
        }
        lv_label_set_text(g_custodianTile.xpubLabel, g_xpubCache[g_createMultiTileView.currentSinger].xpub);
        break;
    case CREATE_MULTI_CONFIRM_CO_SIGNERS:
        lv_obj_add_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        if (g_createMultiTileView.currentSinger == g_selectSliceTile.coSingers - 1) {
            GuiMultiUpdateWalletInfoWidget();
            lv_obj_clear_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_createMultiTileView.stepBtn, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_createMultiTileView.confirmBtn, LV_OBJ_FLAG_HIDDEN);
            g_createMultiTileView.currentTile = CREATE_MULTI_WALLET_INFO;
            lv_obj_set_tile_id(g_createMultiTileView.tileView, g_createMultiTileView.currentTile, 0, LV_ANIM_OFF);
            return SUCCESS_CODE;
        } else if (g_createMultiTileView.currentSinger == 0) {
            GuiClearObjFlag(g_createMultiTileView.stepBtn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(g_importXpubBtn, LV_OBJ_FLAG_HIDDEN);
        }
        g_createMultiTileView.currentSinger++;
        char tempBuff[BUFFER_SIZE_64];
        snprintf(tempBuff, sizeof(tempBuff), "%s %d/%d", _("create_multi_wallet_co_signers"), g_createMultiTileView.currentSinger + 1, g_selectSliceTile.coSingers);
        lv_label_set_text(g_custodianTile.titleLabel, tempBuff);
        lv_label_set_text_fmt(g_custodianTile.noticeLabel, _("create_multi_wallet_co_signers_desc_fmt"), g_createMultiTileView.currentSinger + 1);
        lv_label_set_text(g_custodianTile.xpubLabel, "");
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("create_multi_wallet_import_xpub_sdcard_title"));
        break;
    case CREATE_MULTI_SELECT_SDCARD_XPUB:
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("create_multi_wallet_import_xpub_title"));
        lv_obj_add_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(g_xPubTile.pathLabel, g_xpubCache[g_createMultiTileView.currentSinger].path);
        lv_label_set_text(g_xPubTile.mfpLabel, g_xpubCache[g_createMultiTileView.currentSinger].mfp);
        lv_label_set_text(g_xPubTile.xpubLabel, g_xpubCache[g_createMultiTileView.currentSinger].xpub);
        break;
    case CREATE_MULTI_IMPORT_SDCARD_XPUB:
        lv_label_set_text(g_custodianTile.xpubLabel, g_xpubCache[g_createMultiTileView.currentSinger].xpub);
        lv_obj_clear_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        g_createMultiTileView.currentTile = CREATE_MULTI_SELECT_FORMAT;
        if (g_createMultiTileView.currentSinger == g_selectSliceTile.coSingers - 1) {
            lv_obj_add_flag(g_importXpubBtn, LV_OBJ_FLAG_HIDDEN);
            GuiAddObjFlag(g_createMultiTileView.stepBtn, LV_OBJ_FLAG_CLICKABLE);
        } else {
            GuiClearObjFlag(g_createMultiTileView.stepBtn, LV_OBJ_FLAG_CLICKABLE);
        }
        break;
    case CREATE_MULTI_WALLET_INFO:
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        lv_obj_add_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        g_keyboardWidget = NULL;
        break;
    }
    g_createMultiTileView.currentTile++;
    lv_obj_set_tile_id(g_createMultiTileView.tileView, g_createMultiTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiCreateMultiPrevTile(void)
{
    switch (g_createMultiTileView.currentTile) {
    case CREATE_MULTI_SET_NAME:
        GuiCLoseCurrentWorkingView();
        break;
    case CREATE_MULTI_SELECT_SLICE:
        lv_obj_add_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_MULTI_SELECT_FORMAT:
        lv_obj_clear_flag(g_createMultiTileView.stepCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_MULTI_CONFIRM_CO_SIGNERS:
        break;
    }

    g_createMultiTileView.currentTile--;
    lv_obj_set_tile_id(g_createMultiTileView.tileView, g_createMultiTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiCreateMultiDeInit(void)
{
    GUI_DEL_OBJ(g_noticeWindow)
    GuiDeleteKeyBoard(g_nameWalletKb);
    g_nameWalletKb = NULL;
    g_selectSliceTile.coSingers = SLIP39_DEFAULT_MEMBER_COUNT;
    g_selectSliceTile.singers = SLIP39_DEFAULT_MEMBER_THRESHOLD;
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    GUI_DEL_OBJ(g_createMultiTileView.stepCont)
}

void GuiCreateMultiRefresh(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
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

static void SelectCheckBoxHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *newCheckBox = lv_event_get_user_data(e);
        for (int i = 0; i < 3; i++) {
            if (newCheckBox == g_formatCheckBox[i]) {
                lv_obj_add_state(newCheckBox, LV_STATE_CHECKED);
                g_chainType = i + XPUB_TYPE_BTC_MULTI_SIG_P2SH;
            } else {
                lv_obj_clear_state(g_formatCheckBox[i], LV_STATE_CHECKED);
            }
        }
    }
}

static void GuiCreateAddressSettingsWidget(lv_obj_t *parent)
{
    lv_obj_t *cont, *label;
    char string[BUFFER_SIZE_64];

    cont = GuiCreateContainerWithParent(parent, 408, 306);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
#ifdef BTC_ONLY
    g_addressSettings = GetIsTestNet() ? g_testNetAddressSettings : g_mainNetAddressSettings;
#endif
    for (uint32_t i = 0; i < g_addressSettingsNum; i++) {
        lv_obj_t *accountType = GuiCreateTextLabel(cont, g_addressSettings[i].title);
        snprintf_s(string, BUFFER_SIZE_64, "%s (%s)", g_addressSettings[i].subTitle, g_addressSettings[i].multiPath);
        lv_obj_t *path = GuiCreateNoticeLabel(cont, string);
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(cont, "");
        lv_obj_set_size(checkBox, 45, 45);
        g_formatCheckBox[i] = checkBox;

        GuiButton_t table[] = {
            {.obj = accountType, .align = LV_ALIGN_DEFAULT, .position = {24, 16},},
            {.obj = path, .align = LV_ALIGN_DEFAULT, .position = {24, 56},},
            {.obj = checkBox, .align = LV_ALIGN_RIGHT_MID, .position = {-24, 0}},
        };

        if (i == g_chainType - XPUB_TYPE_BTC_MULTI_SIG_P2SH) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        lv_obj_t *button = GuiCreateButton(cont, 408, 102, table, NUMBER_OF_ARRAYS(table),
                                           SelectCheckBoxHandler, g_formatCheckBox[i]);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, i * 102);
    }

    lv_obj_t *egCont = GuiCreateContainerWithParent(parent, 408, 86);
    lv_obj_align_to(egCont, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);

    label = GuiCreateNoticeLabel(egCont, _("create_multi_wallet_eg_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 12);
}

void GetMultiInfoFromFile(const char *path, XpubWidgetCache_t *xpub, ChainType chainType)
{
    char *jsonString = FatfsFileRead(path);
    cJSON *rootJson;
    char *derivKey = NULL;
    char *xpubKey = NULL;
    if (jsonString == NULL) {
        printf("Failed to read\n");
        return;
    }

    if (chainType == XPUB_TYPE_BTC_MULTI_SIG_P2SH) {
        derivKey = "p2sh_deriv";
        xpubKey = "p2sh";
    } else if (chainType == XPUB_TYPE_BTC_MULTI_SIG_P2WSH_P2SH) {
        derivKey = "p2sh_p2wsh_deriv";
        xpubKey = "p2sh_p2wsh";
    } else if (chainType == XPUB_TYPE_BTC_MULTI_SIG_P2WSH) {
        derivKey = "p2wsh_deriv";
        xpubKey = "p2wsh";
    }

    do {
        rootJson = cJSON_Parse(jsonString);
        if (rootJson == NULL) {
            break;
        }

        GetStringValue(rootJson, derivKey, xpub->path, sizeof(xpub->path));
        GetStringValue(rootJson, xpubKey, xpub->xpub, sizeof(xpub->xpub));
        GetStringValue(rootJson, "xfp", xpub->mfp, sizeof(xpub->mfp));
    } while (0);
    cJSON_Delete(rootJson);
    EXT_FREE(jsonString);
}

static void SelectFormatHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiCreateMultiNextTile(CREATE_MULTI_SELECT_SLICE);
    }
}

static void OpenFileNextTileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    char *path = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        GetMultiInfoFromFile(path, &g_xpubCache[g_createMultiTileView.currentSinger], g_chainType);
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    }
}

static void GuiShowKeyboardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        static uint16_t walletSetIndex = SIG_MULTISIG_WALLET_CREATE;
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
        SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
        SetKeyboardWidgetSig(g_keyboardWidget, &walletSetIndex);
    }
}
