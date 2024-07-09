#ifndef BTC_ONLY
#include "gui_standard_receive_widgets.h"
#include "gui_status_bar.h"
#include "gui_chain.h"
#include "gui_views.h"
#include "gui_hintbox.h"
#include "account_public_info.h"
#include "librust_c.h"
#include "assert.h"
#include "gui_keyboard.h"
#include "draw/lv_draw_mask.h"
#include "stdio.h"
#include "user_utils.h"
#include "inttypes.h"
#include "gui_home_widgets.h"
#include "gui_fullscreen_mode.h"
#include "keystore.h"
#include "gui_page.h"
#include "gui.h"
#include "gui_tutorial_widgets.h"
#include "account_manager.h"
#include "gui_button.h"
#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif


#define GENERAL_ADDRESS_INDEX_MAX                           999999999
#define LEDGER_LIVE_ADDRESS_INDEX_MAX                       9
#define ADDRESS_LONE_MODE_LEN                               (24)

typedef enum {
    RECEIVE_TILE_QRCODE = 0,
    RECEIVE_TILE_SWITCH_ACCOUNT,

    RECEIVE_TILE_BUTT,
} StandardReceiveTile;

typedef struct {
    lv_obj_t *addressCountLabel;
    lv_obj_t *addressLabel;
    lv_obj_t *checkBox;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} SwitchAddressWidgetsItem_t;

typedef struct {
    lv_obj_t *checkBox;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} PathWidgetsItem_t;

typedef struct {
    lv_obj_t *cont;
    lv_obj_t *tileView;
    lv_obj_t *tileQrCode;
    lv_obj_t *tileSwitchAccount;
    lv_obj_t *tileChangePath;
    lv_obj_t *attentionCont;
    lv_obj_t *qrCodeCont;
    lv_obj_t *qrCode;
    lv_obj_t *addressLabel;
    lv_obj_t *addressCountLabel;
    lv_obj_t *moreCont;
    lv_obj_t *addressButton;
    lv_obj_t *leftBtnImg;
    lv_obj_t *rightBtnImg;
    lv_obj_t *confirmBtn;
    lv_obj_t *inputAddressLabel;
    lv_obj_t *overflowLabel;
    lv_obj_t *confirmAccountBtn;
    lv_obj_t *confirmIndexBtn;
    lv_obj_t *inputAccountLabel;
    lv_obj_t *inputAccountCont;
    lv_obj_t *inputAccountKeyboard;
    SwitchAddressWidgetsItem_t switchAddressWidgets[5];
} StandardReceiveWidgets_t;

typedef struct {
    uint32_t index;
    char address[128];
    char path[32];
} AddressDataItem_t;

typedef struct {
    char title[32];
    char subTitle[32];
    char path[32];
} PathItem_t;

static void GuiCreateMoreWidgets(lv_obj_t *parent);
static void GuiStandardReceiveGotoTile(StandardReceiveTile tile);
static void GuiCreateQrCodeWidget(lv_obj_t *parent);
static void GuiCreateSwitchAddressWidget(lv_obj_t *parent);
static void GuiCreateSwitchAddressButtons(lv_obj_t *parent);

static void RefreshQrCode(void);
static void RefreshSwitchAccount(void);
static int GetMaxAddressIndex(void);
static void GetAttentionText(char* text);

static void CloseAttentionHandler(lv_event_t *e);
static void MoreHandler(lv_event_t *e);
static void TutorialHandler(lv_event_t *e);
static void LeftBtnHandler(lv_event_t *e);
static void RightBtnHandler(lv_event_t *e);
static bool IsAccountSwitchable();
static bool HasMoreBtn();
static void SwitchAddressHandler(lv_event_t *e);
static void OpenSwitchAddressHandler(lv_event_t *e);
static void SetCurrentSelectIndex(uint32_t selectIndex);
static uint32_t GetCurrentSelectIndex();
static void ConfirmHandler(lv_event_t *e);
static void UpdateConfirmBtn(void);
static bool ShouldRenderSwitchBtn();

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item);
static void JumpToAccountHandler(lv_event_t *e);
static void GuiCreateGotoAddressWidgets(lv_obj_t *parent);
static void CloseSwitchAddressHandler(lv_event_t *e);
static void InputAddressIndexKeyboardHandler(lv_event_t *e);
static void SetKeyboardValid(bool validation);
static void UpdateConfirmIndexBtn(void);
static void RefreshSwitchAddress(void);
static bool IsIndexSelectChanged();
static uint32_t* GetCosmosChainCurrentSelectIndex();


static StandardReceiveWidgets_t g_standardReceiveWidgets;
static StandardReceiveTile g_StandardReceiveTileNow;
static HOME_WALLET_CARD_ENUM g_chainCard;

// to do: stored.
static uint32_t g_showIndex;
static uint32_t g_tmpIndex = 0;
static uint32_t g_selectIndex[3] = {0};
static uint32_t g_suiSelectIndex[3] = {0};
static uint32_t g_stellarSelectIndex[3] = {0};
static uint32_t g_aptosSelectIndex[3] = {0};
static uint32_t g_xrpSelectIndex[3] = {0};
static uint32_t g_tiaChainSelectIndex[3] = {0};
static uint32_t g_dymChainSelectIndex[3] = {0};
static uint32_t g_osmoChainSelectIndex[3] = {0};
static uint32_t g_injChainSelectIndex[3] = {0};
static uint32_t g_atomChainSelectIndex[3] = {0};
static uint32_t g_croChainSelectIndex[3] = {0};
static uint32_t g_kavaChainSelectIndex[3] = {0};
static uint32_t g_lunaChainSelectIndex[3] = {0};
static uint32_t g_axlChainSelectIndex[3] = {0};
static uint32_t g_aktChainSelectIndex[3] = {0};
static uint32_t g_strdChainSelectIndex[3] = {0};
static uint32_t g_scrtChainSelectIndex[3] = {0};
static uint32_t g_bldChainSelectIndex[3] = {0};
static uint32_t g_ctkChainSelectIndex[3] = {0};
static uint32_t g_evmosChainSelectIndex[3] = {0};
static uint32_t g_starsChainSelectIndex[3] = {0};
static uint32_t g_xprtChainSelectIndex[3] = {0};
static uint32_t g_sommChainSelectIndex[3] = {0};
static uint32_t g_junoChainSelectIndex[3] = {0};
static uint32_t g_irisChainSelectIndex[3] = {0};
static uint32_t g_dvpnChainSelectIndex[3] = {0};
static uint32_t g_rowanChainSelectIndex[3] = {0};
static uint32_t g_regenChainSelectIndex[3] = {0};
static uint32_t g_bootChainSelectIndex[3] = {0};
static uint32_t g_gravChainSelectIndex[3] = {0};
static uint32_t g_ixoChainSelectIndex[3] = {0};
static uint32_t g_ngmChainSelectIndex[3] = {0};
static uint32_t g_iovChainSelectIndex[3] = {0};
static uint32_t g_umeeChainSelectIndex[3] = {0};
static uint32_t g_qckChainSelectIndex[3] = {0};
static uint32_t g_tgdChainSelectIndex[3] = {0};


static PageWidget_t *g_pageWidget;
static uint32_t g_selectedIndex[3] = {0};
static bool g_inputAccountValid = true;



static void JumpToAccountHandler(lv_event_t *e)
{
    GuiCreateGotoAddressWidgets(g_standardReceiveWidgets.tileSwitchAccount);
}

static void CloseSwitchAddressHandler(lv_event_t *e)
{
    lv_obj_add_flag(g_standardReceiveWidgets.inputAccountCont, LV_OBJ_FLAG_HIDDEN);
}

static void SetKeyboardValid(bool validation)
{
    if (validation) {
        if (lv_btnmatrix_has_btn_ctrl(g_standardReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED)) {
            lv_btnmatrix_clear_btn_ctrl(g_standardReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
        }
        lv_btnmatrix_set_btn_ctrl(g_standardReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED);
    } else {
        if (lv_btnmatrix_has_btn_ctrl(g_standardReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED)) {
            lv_btnmatrix_clear_btn_ctrl(g_standardReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED);
        }
        lv_btnmatrix_set_btn_ctrl(g_standardReceiveWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
    }
}

static void UpdateConfirmIndexBtn(void)
{
    // g_tmpIndex is your input address index
    // g_showIndex is the first address index of the current page
    RefreshSwitchAddress();
    RefreshQrCode();
    UpdateConfirmBtn();
}

static bool IsIndexSelectChanged()
{
    return g_tmpIndex != g_selectedIndex[GetCurrentAccountIndex()];
}
static void RefreshSwitchAddress(void)
{
    AddressDataItem_t addressDataItem;

    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, "%s-%u", _("Address"), (addressDataItem.index));
        char string[128] = {0};
        CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
        lv_label_set_text(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, string);
        if (end) {
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == g_tmpIndex) {
            lv_obj_add_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
        if (index == GetMaxAddressIndex()) {
            end = true;
        }
        index++;
    }
}



static void InputAddressIndexKeyboardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    lv_obj_draw_part_dsc_t *dsc;
    const char *txt;
    char input[16];
    uint32_t len;
    uint64_t longInt;

    if (code == LV_EVENT_CLICKED) {
        txt = lv_btnmatrix_get_btn_text(obj, id);
        strcpy_s(input, sizeof(input), lv_label_get_text(g_standardReceiveWidgets.inputAccountLabel));
        if (strcmp(txt, LV_SYMBOL_OK) == 0) {
            if (g_inputAccountValid) {
                sscanf(input, "%u", &g_tmpIndex);
                g_showIndex = g_tmpIndex / 5 * 5;
                RefreshSwitchAddress();
                lv_obj_add_flag(g_standardReceiveWidgets.inputAccountCont, LV_OBJ_FLAG_HIDDEN);
                g_inputAccountValid = false;
                UpdateConfirmIndexBtn();
            }
        } else if (strcmp(txt, "-") == 0) {
            len = strlen(input);
            if (len >= 1) {
                input[len - 1] = '\0';
                lv_label_set_text(g_standardReceiveWidgets.inputAccountLabel, input);
                lv_obj_add_flag(g_standardReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
                if (strlen(input) >= 1) {
                    g_inputAccountValid = true;
                } else {
                    g_inputAccountValid = false;
                }
            }
        } else if (strlen(input) < 15) {
            strcat(input, txt);
            longInt = strtol(input, NULL, 10);
            if (longInt >= GENERAL_ADDRESS_INDEX_MAX) {
                input[9] = '\0';
                lv_obj_clear_flag(g_standardReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_standardReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
            }
            if (longInt > 0) {
                if (input[0] == '0') {
                    lv_label_set_text(g_standardReceiveWidgets.inputAccountLabel, input + 1);
                } else {
                    lv_label_set_text(g_standardReceiveWidgets.inputAccountLabel, input);
                }
            } else {
                lv_label_set_text(g_standardReceiveWidgets.inputAccountLabel, "0");
            }
            g_inputAccountValid = true;
        } else {
            g_inputAccountValid = false;
            printf("input to long\r\n");
        }
        SetKeyboardValid(g_inputAccountValid);
    } else if (code == LV_EVENT_DRAW_PART_BEGIN) {
        dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            /*Change the draw descriptor of the 12th button*/
            if (dsc->id == 9) {
                dsc->label_dsc->opa = LV_OPA_TRANSP;
            } else if (dsc->id == 11) {
                dsc->rect_dsc->bg_color = ORANGE_COLOR;
                dsc->label_dsc->opa = LV_OPA_TRANSP;
            } else {
                dsc->rect_dsc->bg_color = DARK_GRAY_COLOR;
            }
        }
    } else if (code == LV_EVENT_DRAW_PART_END) {
        dsc = lv_event_get_draw_part_dsc(e);
        /*When the button matrix draws the buttons...*/
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            /*Add custom content to the 4th button when the button itself was drawn*/
            if (dsc->id == 9 || dsc->id == 11) {
                lv_img_header_t header;
                lv_draw_img_dsc_t img_draw_dsc;
                lv_area_t a;
                const lv_img_dsc_t *imgDsc;
                lv_res_t res;
                imgDsc = dsc->id == 9 ? &imgBackspace : &imgCheck;
                res = lv_img_decoder_get_info(imgDsc, &header);
                if (res != LV_RES_OK)
                    return;
                a.x1 = dsc->draw_area->x1 + (lv_area_get_width(dsc->draw_area) - header.w) / 2;
                a.x2 = a.x1 + header.w - 1;
                a.y1 = dsc->draw_area->y1 + (lv_area_get_height(dsc->draw_area) - header.h) / 2;
                a.y2 = a.y1 + header.h - 1;
                lv_draw_img_dsc_init(&img_draw_dsc);
                img_draw_dsc.recolor = lv_color_black();
                if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
                    img_draw_dsc.recolor_opa = LV_OPA_30;

                lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, imgDsc);
            }
        }
    }

}



static void GuiCreateGotoAddressWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *label, *line, *closeBtn;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    g_inputAccountValid = false;

    if (g_standardReceiveWidgets.inputAccountCont == NULL) {
        g_standardReceiveWidgets.inputAccountCont = GuiCreateHintBox(530);
        lv_obj_add_event_cb(lv_obj_get_child(g_standardReceiveWidgets.inputAccountCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_standardReceiveWidgets.inputAccountCont);
        cont = g_standardReceiveWidgets.inputAccountCont;

        label = GuiCreateNoticeLabel(cont, _("receive_btc_receive_change_address_title"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 30 + 270);
        label = GuiCreateNoticeLabel(cont, "");
        lv_label_set_text_fmt(label, "%s-", _("Address"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 108 + 270);
        g_standardReceiveWidgets.inputAccountLabel = GuiCreateTextLabel(cont, "");
        lv_obj_align(g_standardReceiveWidgets.inputAccountLabel, LV_ALIGN_TOP_LEFT, 38 + lv_obj_get_self_width(label), 108 + 270);
        label = GuiCreateIllustrateLabel(cont, _("receive_btc_receive_change_address_limit"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 170 + 270);
        lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        g_standardReceiveWidgets.overflowLabel = label;

        line = GuiCreateLine(cont, points, 2);
        lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 160 + 270);

        lv_obj_t *keyboard = GuiCreateNumKeyboard(cont, InputAddressIndexKeyboardHandler, NUM_KEYBOARD_NORMAL, NULL);
        lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_add_style(keyboard, &g_enterPressBtnmStyle, LV_STATE_PRESSED | LV_PART_ITEMS);
        lv_btnmatrix_set_btn_ctrl(keyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
        g_standardReceiveWidgets.inputAccountKeyboard = keyboard;

        closeBtn = GuiCreateImgButton(cont, &imgClose, 40, CloseSwitchAddressHandler, NULL);
        lv_obj_align(closeBtn, LV_ALIGN_TOP_RIGHT, -36, 27 + 270);
    } else {
        lv_label_set_text(g_standardReceiveWidgets.inputAccountLabel, "");
        lv_obj_clear_flag(g_standardReceiveWidgets.inputAccountCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_standardReceiveWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiStandardReceiveInit(uint8_t chain)
{
    g_chainCard = chain;
    g_showIndex = GetCurrentSelectIndex() / 5 * 5;
    g_pageWidget = CreatePageWidget();
    g_standardReceiveWidgets.cont = g_pageWidget->contentZone;
    g_standardReceiveWidgets.tileView = lv_tileview_create(g_standardReceiveWidgets.cont);
    lv_obj_set_style_bg_opa(g_standardReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_standardReceiveWidgets.tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    g_standardReceiveWidgets.tileQrCode = lv_tileview_add_tile(g_standardReceiveWidgets.tileView, RECEIVE_TILE_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(g_standardReceiveWidgets.tileQrCode);
    if (IsAccountSwitchable()) {
        g_standardReceiveWidgets.tileSwitchAccount = lv_tileview_add_tile(g_standardReceiveWidgets.tileView, RECEIVE_TILE_SWITCH_ACCOUNT, 0, LV_DIR_HOR);
        GuiCreateSwitchAddressWidget(g_standardReceiveWidgets.tileSwitchAccount);
        GuiCreateSwitchAddressButtons(g_standardReceiveWidgets.tileSwitchAccount);
    }
    lv_obj_clear_flag(g_standardReceiveWidgets.tileView, LV_OBJ_FLAG_SCROLLABLE);

    GuiStandardReceiveRefresh();
}

void GuiStandardReceiveDeInit(void)
{
    GUI_DEL_OBJ(g_standardReceiveWidgets.moreCont)
    GUI_DEL_OBJ(g_standardReceiveWidgets.attentionCont)
    GUI_DEL_OBJ(g_standardReceiveWidgets.cont)

    CLEAR_OBJECT(g_standardReceiveWidgets);
    g_StandardReceiveTileNow = 0;
    GuiFullscreenModeCleanUp();
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiStandardReceiveRefresh(void)
{
    char title[30];
    switch (g_StandardReceiveTileNow) {
    case RECEIVE_TILE_QRCODE:
        snprintf(title, sizeof(title), _("receive_coin_fmt"), GetCoinCardByIndex(g_chainCard)->coin);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseTimerCurrentViewHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SKIP, JumpToAccountHandler, NULL);
        SetCoinWallet(g_pageWidget->navBarWidget, g_chainCard, title);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, HasMoreBtn() ? NVS_BAR_MORE_INFO : NVS_RIGHT_BUTTON_BUTT, MoreHandler, NULL);
        RefreshQrCode();
        break;
    case RECEIVE_TILE_SWITCH_ACCOUNT:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("switch_account"));
        if (IsCosmosChain(g_chainCard)) {
            // only cosmos chain show the jump to account button
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SKIP, JumpToAccountHandler, NULL);
        } else {
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        }
        g_tmpIndex = GetCurrentSelectIndex();
        g_showIndex = g_tmpIndex / 5 * 5;
        if (ShouldRenderSwitchBtn()) {
            if (g_showIndex < 5) {
                lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
                lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            } else if (g_showIndex >= GetMaxAddressIndex() - 5) {
                lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
            } else {
                lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
            }
        }
        UpdateConfirmBtn();
        break;
    default:
        break;
    }
}

void GuiStandardReceivePrevTile(void)
{
    GuiStandardReceiveGotoTile(RECEIVE_TILE_QRCODE);
}

static void GuiCreateMoreWidgets(lv_obj_t *parent)
{
    lv_obj_t *cont, *btn, *img, *label;

    g_standardReceiveWidgets.moreCont = GuiCreateHintBox(132);
    lv_obj_add_event_cb(lv_obj_get_child(g_standardReceiveWidgets.moreCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_standardReceiveWidgets.moreCont);
    cont = g_standardReceiveWidgets.moreCont;

    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 456, 84);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 120 + 572);
    lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, TutorialHandler, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(btn, &imgTutorial);
    lv_obj_align(img, LV_ALIGN_CENTER, -186, 0);
    label = GuiCreateTextLabel(btn, _("Tutorial"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 4);
}

static void GuiStandardReceiveGotoTile(StandardReceiveTile tile)
{
    g_StandardReceiveTileNow = tile;
    GuiStandardReceiveRefresh();
    lv_obj_set_tile_id(g_standardReceiveWidgets.tileView, g_StandardReceiveTileNow, 0, LV_ANIM_OFF);
}

lv_obj_t* CreateStandardReceiveQRCode(lv_obj_t* parent, uint16_t w, uint16_t h)
{
    lv_obj_t* qrcode = lv_qrcode_create(parent, w, BLACK_COLOR, WHITE_COLOR);
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    lv_qrcode_update(qrcode, "", 0);
    return qrcode;
}

static uint16_t GetAddrYExtend(void)
{
    if (g_chainCard == HOME_WALLET_CARD_SUI || g_chainCard == HOME_WALLET_CARD_APT) {
        return 30;
    }
    return 0;
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *tempObj;
    uint16_t yOffset = 0;
    uint16_t addrYExtend = GetAddrYExtend();

    g_standardReceiveWidgets.qrCodeCont = GuiCreateContainerWithParent(parent, 408, 524 + addrYExtend);
    lv_obj_align(g_standardReceiveWidgets.qrCodeCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_standardReceiveWidgets.qrCodeCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_standardReceiveWidgets.qrCodeCont, 24, LV_PART_MAIN);

    yOffset += 36;
    g_standardReceiveWidgets.qrCode = CreateStandardReceiveQRCode(g_standardReceiveWidgets.qrCodeCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(CreateStandardReceiveQRCode, 420, 420);

    lv_obj_align(g_standardReceiveWidgets.qrCode, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 336;

    yOffset += 16;
    g_standardReceiveWidgets.addressLabel = GuiCreateNoticeLabel(g_standardReceiveWidgets.qrCodeCont, "");
    lv_obj_set_width(g_standardReceiveWidgets.addressLabel, 336);
    lv_obj_align(g_standardReceiveWidgets.addressLabel, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 60;

    yOffset += 16;
    g_standardReceiveWidgets.addressCountLabel = GuiCreateIllustrateLabel(g_standardReceiveWidgets.qrCodeCont, "");
    lv_obj_align(g_standardReceiveWidgets.addressCountLabel, LV_ALIGN_TOP_LEFT, 36, yOffset + addrYExtend);

    g_standardReceiveWidgets.addressButton = lv_btn_create(g_standardReceiveWidgets.qrCodeCont);
    lv_obj_set_size(g_standardReceiveWidgets.addressButton, 336, 36);
    lv_obj_align(g_standardReceiveWidgets.addressButton, LV_ALIGN_TOP_MID, 0, 464 + addrYExtend);
    lv_obj_set_style_bg_opa(g_standardReceiveWidgets.addressButton, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_standardReceiveWidgets.addressButton, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(g_standardReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(g_standardReceiveWidgets.addressButton, 0, LV_PART_MAIN);
    if (IsAccountSwitchable()) {
        lv_obj_add_event_cb(g_standardReceiveWidgets.addressButton, OpenSwitchAddressHandler, LV_EVENT_CLICKED, NULL);
        tempObj = GuiCreateImg(g_standardReceiveWidgets.addressButton, &imgArrowRight);
        lv_obj_set_style_img_opa(tempObj, LV_OPA_80, LV_PART_MAIN);
        lv_obj_align(tempObj, LV_ALIGN_CENTER, 150, 0);
    }

    const char* coin = GetCoinCardByIndex(g_chainCard)->coin;
    if (!GetFirstReceive(coin)) {
        char attentionText[1024];
        GetAttentionText(attentionText);
        g_standardReceiveWidgets.attentionCont = GuiCreateConfirmHintBox(&imgInformation, _("Attention"), attentionText, NULL, _("got_it"), WHITE_COLOR_OPA20);
        lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_standardReceiveWidgets.attentionCont), CloseAttentionHandler, LV_EVENT_CLICKED, NULL);
        SetFirstReceive(coin, true);
    }
}

void GetAttentionText(char* text)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_TRX:
        strcpy_s(text, 1024, _("receive_trx_hint"));
        break;
    default:
        snprintf_s(text, 1024, _("receive_coin_hint_fmt"), GetCoinCardByIndex(g_chainCard)->coin);
    }
}

static void GuiCreateSwitchAddressWidget(lv_obj_t *parent)
{
    // Create the account list page.
    uint32_t index;
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 514);
    lv_obj_t *line;
    static lv_point_t points[2] = {{0, 0}, {360, 0}};
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    index = 0;
    for (uint32_t i = 0; i < 5; i++) {
        g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel = GuiCreateTextLabel(cont, "");
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 20 + 103 * i);
        g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel = GuiCreateNoticeLabel(cont, "");
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * i);
        }

        g_standardReceiveWidgets.switchAddressWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, SwitchAddressHandler, LV_EVENT_CLICKED, NULL);

        g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg = GuiCreateImg(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg = GuiCreateImg(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

        index++;
    }
    RefreshSwitchAccount();
}

static bool IsSelectChanged()
{
    return g_tmpIndex != GetCurrentSelectIndex();
}

static void UpdateConfirmBtn(void)
{
    if (IsSelectChanged()) {
        lv_obj_set_style_bg_opa(g_standardReceiveWidgets.confirmBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_standardReceiveWidgets.confirmBtn, 0), LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_standardReceiveWidgets.confirmBtn, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_standardReceiveWidgets.confirmBtn, 0), LV_OPA_30, LV_PART_MAIN);
    }
}

static bool ShouldRenderSwitchBtn()
{
    return GetMaxAddressIndex() > 5;
}

static void GuiCreateSwitchAddressButtons(lv_obj_t *parent)
{
    lv_obj_t *btn;
    lv_obj_t *img;

    if (ShouldRenderSwitchBtn()) {
        btn = GuiCreateBtn(parent, "");
        lv_obj_set_size(btn, 96, 66);
        lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        img = GuiCreateImg(btn, &imgArrowLeft);
        lv_obj_set_align(img, LV_ALIGN_CENTER);
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
        }
        lv_obj_add_event_cb(btn, LeftBtnHandler, LV_EVENT_CLICKED, NULL);
        g_standardReceiveWidgets.leftBtnImg = img;

        btn = GuiCreateBtn(parent, "");
        lv_obj_set_size(btn, 96, 66);
        lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 156, -24);
        img = GuiCreateImg(btn, &imgArrowRight);
        lv_obj_set_align(img, LV_ALIGN_CENTER);
        lv_obj_set_style_opa(img, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_add_event_cb(btn, RightBtnHandler, LV_EVENT_CLICKED, NULL);
        g_standardReceiveWidgets.rightBtnImg = img;
    }

    btn = GuiCreateBtn(parent, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, ConfirmHandler, LV_EVENT_CLICKED, NULL);
    g_standardReceiveWidgets.confirmBtn = btn;
    UpdateConfirmBtn();
}

static void RefreshQrCode(void)
{
    AddressDataItem_t addressDataItem;

    ModelGetAddress(GetCurrentSelectIndex(), &addressDataItem);
    lv_qrcode_update(g_standardReceiveWidgets.qrCode, addressDataItem.address, strnlen_s(addressDataItem.address, ADDRESS_MAX_LEN));
    lv_obj_t *fullscreenQrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
    if (fullscreenQrcode) {
        lv_qrcode_update(fullscreenQrcode, addressDataItem.address, strnlen_s(addressDataItem.address, ADDRESS_MAX_LEN));
    }
    if (g_chainCard == HOME_WALLET_CARD_ARWEAVE) {
        SimpleResponse_c_char *fixedAddress = fix_arweave_address(addressDataItem.address);
        if (fixedAddress->error_code == 0) {
            lv_label_set_text(g_standardReceiveWidgets.addressLabel, fixedAddress->data);
        }
        free_simple_response_c_char(fixedAddress);
    } else if (g_chainCard == HOME_WALLET_CARD_XLM) {
        char addressString[128];
        CutAndFormatString(addressString, sizeof(addressString), addressDataItem.address, 40);
        lv_label_set_text(g_standardReceiveWidgets.addressLabel, addressString);
    } else if (g_chainCard == HOME_WALLET_CARD_TON) {
        char address[128];
        snprintf_s(address, 128, "%.22s\n%s", addressDataItem.address, &addressDataItem.address[22]);
        lv_label_set_text(g_standardReceiveWidgets.addressLabel, address);
    } else {
        lv_label_set_text(g_standardReceiveWidgets.addressLabel, addressDataItem.address);
    }
    lv_label_set_text_fmt(g_standardReceiveWidgets.addressCountLabel, "%s-%u", _("account_head"), addressDataItem.index);
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem;
    char string[128];
    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, "%s-%u", _("account_head"), addressDataItem.index);
        CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
        lv_label_set_text(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, string);
        if (end) {
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == GetCurrentSelectIndex()) {
            lv_obj_add_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
        if (index == GetMaxAddressIndex()) {
            end = true;
        }
        index++;
    }
}

static int GetMaxAddressIndex(void)
{
    if (g_chainCard == HOME_WALLET_CARD_SUI || g_chainCard == HOME_WALLET_CARD_APT) {
        return 10;
    }
    if (g_chainCard == HOME_WALLET_CARD_XLM) {
        return 5;
    }
    if (g_chainCard == HOME_WALLET_CARD_XRP) {
        return 200;
    }
    return GENERAL_ADDRESS_INDEX_MAX;
}

static void CloseAttentionHandler(lv_event_t *e)
{
    lv_obj_add_flag(g_standardReceiveWidgets.attentionCont, LV_OBJ_FLAG_HIDDEN);
}

static void MoreHandler(lv_event_t *e)
{
    if (g_standardReceiveWidgets.moreCont == NULL) {
        GuiCreateMoreWidgets(g_standardReceiveWidgets.tileQrCode);
    } else {
        lv_obj_del(g_standardReceiveWidgets.moreCont);
        g_standardReceiveWidgets.moreCont = NULL;
    }
}

static void TutorialHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_standardReceiveWidgets.moreCont);

    TUTORIAL_LIST_INDEX_ENUM index = TUTORIAL_ETH_RECEIVE;
    GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
}

static void LeftBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showIndex >= 5) {
        g_showIndex -= 5;
        RefreshSwitchAccount();
    }
    if (g_showIndex < 5) {
        lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_30, LV_PART_MAIN);
    }
}

static void RightBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_standardReceiveWidgets.leftBtnImg, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showIndex < GetMaxAddressIndex() - 5) {
        g_showIndex += 5;
        RefreshSwitchAccount();
    }
    if (g_showIndex >= GetMaxAddressIndex() - 5) {
        lv_obj_set_style_img_opa(g_standardReceiveWidgets.rightBtnImg, LV_OPA_30, LV_PART_MAIN);
    }
}

static void ConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED && IsSelectChanged()) {
        SetCurrentSelectIndex(g_tmpIndex);
        ReturnHandler(e);
    }
}

static bool IsAccountSwitchable()
{
    // all cosmos chain can switch account
    if (IsCosmosChain(g_chainCard)) {
        return true;
    }
    switch (g_chainCard) {
    case HOME_WALLET_CARD_TRX:
    case HOME_WALLET_CARD_SUI:
    case HOME_WALLET_CARD_APT:
    case HOME_WALLET_CARD_XRP:
    case HOME_WALLET_CARD_XLM:
        return true;

    default:
        return false;
    }
}

static bool HasMoreBtn()
{
    return false;
}

static void SwitchAddressHandler(lv_event_t *e)
{
    lv_obj_t *checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < 5; i++) {
        if (checkBox == g_standardReceiveWidgets.switchAddressWidgets[i].checkBox) {
            lv_obj_add_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            g_tmpIndex = g_showIndex + i;
        } else {
            lv_obj_clear_state(g_standardReceiveWidgets.switchAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_standardReceiveWidgets.switchAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_standardReceiveWidgets.switchAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
    }
    UpdateConfirmBtn();
}

static void OpenSwitchAddressHandler(lv_event_t *e)
{
    GuiStandardReceiveGotoTile(RECEIVE_TILE_SWITCH_ACCOUNT);
    RefreshSwitchAccount();
}

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item)
{
    char *xPub, hdPath[BUFFER_SIZE_128];
    SimpleResponse_c_char *result;

    switch (g_chainCard) {
    case HOME_WALLET_CARD_TRX:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_TRX);
        snprintf_s(hdPath, BUFFER_SIZE_128, "m/44'/195'/0'/0/%u", index);
        result = tron_get_address(hdPath, xPub);
        break;
    case HOME_WALLET_CARD_SUI:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_SUI_0 + index);
        snprintf_s(hdPath, BUFFER_SIZE_128, "m/44'/784'/%u'/0'/0'", index);
        result = sui_generate_address(xPub);
        break;
    case HOME_WALLET_CARD_APT:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_APT_0 + index);
        snprintf_s(hdPath, BUFFER_SIZE_128, "m/44'/637'/%u'/0'/0'", index);
        result = aptos_generate_address(xPub);
        break;
    case HOME_WALLET_CARD_XRP:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_XRP);
        snprintf_s(hdPath, BUFFER_SIZE_128, "m/44'/144'/0'/0/%u", index);
        result = xrp_get_address(hdPath, xPub, "m/44'/144'/0'/");
        break;
    case HOME_WALLET_CARD_ARWEAVE:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_ARWEAVE);
        result = arweave_get_address(xPub);
        break;
    case HOME_WALLET_CARD_XLM:
        xPub = GetCurrentAccountPublicKey(XPUB_TYPE_STELLAR_0 + index);
        snprintf_s(hdPath, BUFFER_SIZE_64, "m/44'/148'/%u'", index);
        result = stellar_get_address(xPub);
        break;
    case HOME_WALLET_CARD_TON: {
        bool isTonNative = GetMnemonicType() == MNEMONIC_TYPE_TON;
        if (isTonNative) {
            xPub = GetCurrentAccountPublicKey(XPUB_TYPE_TON_NATIVE);
            result = ton_get_address(xPub);
        } else {
            ASSERT(false);
            //remains for bip39 ton
        }
        break;
    }
    default:
        if (IsCosmosChain(g_chainCard)) {
            result = GetCosmosChainAddressByCoinTypeAndIndex(g_chainCard, index);
        } else {
            printf("Standard Receive ModelGetAddress cannot match %d\r\n", index);
            return;
        }
    }
    if (result->error_code == 0) {
        item->index = index;
        strcpy(item->address, result->data);
        strcpy(item->path, hdPath);
    }
    free_simple_response_c_char(result);
}

void GuiResetCurrentStandardAddressIndex(uint8_t index)
{
    if (index > 2) {
        return;
    }
    g_selectIndex[index] = 0;
    g_suiSelectIndex[index] = 0;
    g_stellarSelectIndex[index] = 0;
    g_aptosSelectIndex[index] = 0;
    g_xrpSelectIndex[index] = 0;
    g_tiaChainSelectIndex[index] = 0;
    g_dymChainSelectIndex[index] = 0;
    g_osmoChainSelectIndex[index] = 0;
    g_injChainSelectIndex[index] = 0;
    g_atomChainSelectIndex[index] = 0;
    g_croChainSelectIndex[index] = 0;
    g_kavaChainSelectIndex[index] = 0;
    g_lunaChainSelectIndex[index] = 0;
    g_axlChainSelectIndex[index] = 0;
    g_lunaChainSelectIndex[index] = 0;
    g_aktChainSelectIndex[index] = 0;
    g_strdChainSelectIndex[index] = 0;
    g_scrtChainSelectIndex[index] = 0;
    g_bldChainSelectIndex[index] = 0;
    g_ctkChainSelectIndex[index] = 0;
    g_evmosChainSelectIndex[index] = 0;
    g_starsChainSelectIndex[index] = 0;
    g_xprtChainSelectIndex[index] = 0;
    g_sommChainSelectIndex[index] = 0;
    g_junoChainSelectIndex[index] = 0;
    g_irisChainSelectIndex[index] = 0;
    g_dvpnChainSelectIndex[index] = 0;
    g_rowanChainSelectIndex[index] = 0;
    g_regenChainSelectIndex[index] = 0;
    g_bootChainSelectIndex[index] = 0;
    g_gravChainSelectIndex[index] = 0;
    g_ixoChainSelectIndex[index] = 0;
    g_ngmChainSelectIndex[index] = 0;
    g_iovChainSelectIndex[index] = 0;
    g_umeeChainSelectIndex[index] = 0;
    g_qckChainSelectIndex[index] = 0;
    g_tgdChainSelectIndex[index] = 0;
}




void GuiResetAllStandardAddressIndex(void)
{
    memset_s(g_selectIndex, sizeof(g_selectIndex), 0, sizeof(g_selectIndex));
    memset_s(g_suiSelectIndex, sizeof(g_suiSelectIndex), 0, sizeof(g_suiSelectIndex));
    memset_s(g_stellarSelectIndex, sizeof(g_stellarSelectIndex), 0, sizeof(g_stellarSelectIndex));
    memset_s(g_aptosSelectIndex, sizeof(g_aptosSelectIndex), 0, sizeof(g_aptosSelectIndex));
    memset_s(g_xrpSelectIndex, sizeof(g_xrpSelectIndex), 0, sizeof(g_xrpSelectIndex));
    memset_s(g_tiaChainSelectIndex, sizeof(g_tiaChainSelectIndex), 0, sizeof(g_tiaChainSelectIndex));
    memset_s(g_dymChainSelectIndex, sizeof(g_dymChainSelectIndex), 0, sizeof(g_dymChainSelectIndex));
    memset_s(g_osmoChainSelectIndex, sizeof(g_osmoChainSelectIndex), 0, sizeof(g_osmoChainSelectIndex));
    memset_s(g_injChainSelectIndex, sizeof(g_injChainSelectIndex), 0, sizeof(g_injChainSelectIndex));
    memset_s(g_atomChainSelectIndex, sizeof(g_atomChainSelectIndex), 0, sizeof(g_atomChainSelectIndex));
    memset_s(g_croChainSelectIndex, sizeof(g_croChainSelectIndex), 0, sizeof(g_croChainSelectIndex));
    memset_s(g_kavaChainSelectIndex, sizeof(g_kavaChainSelectIndex), 0, sizeof(g_kavaChainSelectIndex));
    memset_s(g_lunaChainSelectIndex, sizeof(g_lunaChainSelectIndex), 0, sizeof(g_lunaChainSelectIndex));
    memset_s(g_axlChainSelectIndex, sizeof(g_axlChainSelectIndex), 0, sizeof(g_axlChainSelectIndex));
    memset_s(g_lunaChainSelectIndex, sizeof(g_lunaChainSelectIndex), 0, sizeof(g_lunaChainSelectIndex));
    memset_s(g_aktChainSelectIndex, sizeof(g_aktChainSelectIndex), 0, sizeof(g_aktChainSelectIndex));
    memset_s(g_strdChainSelectIndex, sizeof(g_strdChainSelectIndex), 0, sizeof(g_strdChainSelectIndex));
    memset_s(g_scrtChainSelectIndex, sizeof(g_scrtChainSelectIndex), 0, sizeof(g_scrtChainSelectIndex));
    memset_s(g_bldChainSelectIndex, sizeof(g_bldChainSelectIndex), 0, sizeof(g_bldChainSelectIndex));
    memset_s(g_ctkChainSelectIndex, sizeof(g_ctkChainSelectIndex), 0, sizeof(g_ctkChainSelectIndex));
    memset_s(g_evmosChainSelectIndex, sizeof(g_evmosChainSelectIndex), 0, sizeof(g_evmosChainSelectIndex));
    memset_s(g_starsChainSelectIndex, sizeof(g_starsChainSelectIndex), 0, sizeof(g_starsChainSelectIndex));
    memset_s(g_xprtChainSelectIndex, sizeof(g_xprtChainSelectIndex), 0, sizeof(g_xprtChainSelectIndex));
    memset_s(g_sommChainSelectIndex, sizeof(g_sommChainSelectIndex), 0, sizeof(g_sommChainSelectIndex));
    memset_s(g_junoChainSelectIndex, sizeof(g_junoChainSelectIndex), 0, sizeof(g_junoChainSelectIndex));
    memset_s(g_irisChainSelectIndex, sizeof(g_irisChainSelectIndex), 0, sizeof(g_irisChainSelectIndex));
    memset_s(g_dvpnChainSelectIndex, sizeof(g_dvpnChainSelectIndex), 0, sizeof(g_dvpnChainSelectIndex));
    memset_s(g_rowanChainSelectIndex, sizeof(g_rowanChainSelectIndex), 0, sizeof(g_rowanChainSelectIndex));
    memset_s(g_regenChainSelectIndex, sizeof(g_regenChainSelectIndex), 0, sizeof(g_regenChainSelectIndex));
    memset_s(g_bootChainSelectIndex, sizeof(g_bootChainSelectIndex), 0, sizeof(g_bootChainSelectIndex));
    memset_s(g_gravChainSelectIndex, sizeof(g_gravChainSelectIndex), 0, sizeof(g_gravChainSelectIndex));
    memset_s(g_ixoChainSelectIndex, sizeof(g_ixoChainSelectIndex), 0, sizeof(g_ixoChainSelectIndex));
    memset_s(g_ngmChainSelectIndex, sizeof(g_ngmChainSelectIndex), 0, sizeof(g_ngmChainSelectIndex));
    memset_s(g_iovChainSelectIndex, sizeof(g_iovChainSelectIndex), 0, sizeof(g_iovChainSelectIndex));
    memset_s(g_umeeChainSelectIndex, sizeof(g_umeeChainSelectIndex), 0, sizeof(g_umeeChainSelectIndex));
    memset_s(g_qckChainSelectIndex, sizeof(g_qckChainSelectIndex), 0, sizeof(g_qckChainSelectIndex));
    memset_s(g_tgdChainSelectIndex, sizeof(g_tgdChainSelectIndex), 0, sizeof(g_tgdChainSelectIndex));
}

static uint32_t* GetCosmosChainCurrentSelectIndex()
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_TIA:
        return &g_tiaChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_DYM:
        return &g_dymChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_OSMO:
        return &g_osmoChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_INJ:
        return &g_injChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_ATOM:
        return &g_atomChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_CRO:
        return &g_croChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_KAVA:
        return &g_kavaChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_LUNC:
        return &g_lunaChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_AXL:
        return &g_axlChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_LUNA:
        return &g_lunaChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_AKT:
        return &g_aktChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_STRD:
        return &g_strdChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_SCRT:
        return &g_scrtChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_BLD:
        return &g_bldChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_CTK:
        return &g_ctkChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_EVMOS:
        return &g_evmosChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_STARS:
        return &g_starsChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_XPRT:
        return &g_xprtChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_SOMM:
        return &g_sommChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_JUNO:
        return &g_junoChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_IRIS:
        return &g_irisChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_DVPN:
        return &g_dvpnChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_ROWAN:
        return &g_rowanChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_REGEN:
        return &g_regenChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_BOOT:
        return &g_bootChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_GRAV:
        return &g_gravChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_IXO:
        return &g_ixoChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_NGM:
        return &g_ngmChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_IOV:
        return &g_iovChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_UMEE:
        return &g_umeeChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_QCK:
        return &g_qckChainSelectIndex[GetCurrentAccountIndex()];
        break;
    case HOME_WALLET_CARD_TGD:
        return &g_tgdChainSelectIndex[GetCurrentAccountIndex()];
        break;
    default:
        return NULL;
    }
}

static void SetCurrentSelectIndex(uint32_t selectIndex)
{
    switch (g_chainCard) {
    case HOME_WALLET_CARD_SUI:
        g_suiSelectIndex[GetCurrentAccountIndex()] = selectIndex;
        break;
    case HOME_WALLET_CARD_XLM:
        g_stellarSelectIndex[GetCurrentAccountIndex()] = selectIndex;
        break;
    case HOME_WALLET_CARD_APT:
        g_aptosSelectIndex[GetCurrentAccountIndex()] = selectIndex;
        break;
    case HOME_WALLET_CARD_XRP:
        g_xrpSelectIndex[GetCurrentAccountIndex()] = selectIndex;
        break;
    default:
        if (IsCosmosChain(g_chainCard)) {
            uint32_t *ptr = GetCosmosChainCurrentSelectIndex();
            *ptr = selectIndex;
            break;
        } else {
            g_selectIndex[GetCurrentAccountIndex()] = selectIndex;
            break;
        }
    }
}

static uint32_t GetCurrentSelectIndex()
{
    if (!IsAccountSwitchable()) {
        return 0;
    }
    switch (g_chainCard) {
    case HOME_WALLET_CARD_SUI:
        return g_suiSelectIndex[GetCurrentAccountIndex()];
    case HOME_WALLET_CARD_XLM:
        return g_stellarSelectIndex[GetCurrentAccountIndex()];
    case HOME_WALLET_CARD_APT:
        return g_aptosSelectIndex[GetCurrentAccountIndex()];
    case HOME_WALLET_CARD_XRP:
        return g_xrpSelectIndex[GetCurrentAccountIndex()];
    default:
        if (IsCosmosChain(g_chainCard)) {
            uint32_t *ptr = GetCosmosChainCurrentSelectIndex();
            return *ptr;
        } else {
            return g_selectIndex[GetCurrentAccountIndex()];
        }
    }
}
#endif
