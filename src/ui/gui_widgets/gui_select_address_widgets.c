#ifndef BTC_ONLY
#include "gui_select_address_widgets.h"
#include "gui_home_widgets.h"
#include "account_public_info.h"
#include "gui_page.h"
#include "gui_obj.h"
#include "gui_button.h"


#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif
#define GENERAL_ADDRESS_INDEX_MAX                           999999999
typedef struct {
    lv_obj_t *addressCountLabel;
    lv_obj_t *addressLabel;
    lv_obj_t *checkBox;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} SelectAddressWidgetsItem_t;

typedef struct {
    uint32_t index;
    char address[128];
    char path[32];
} AddressDataItem_t;


typedef struct {
    lv_obj_t *tileSwitchAccount;
    lv_obj_t *overflowLabel;
    lv_obj_t *confirmIndexBtn;
    lv_obj_t *inputAccountLabel;
    lv_obj_t *inputAccountCont;
    lv_obj_t *inputAccountKeyboard;
    uint32_t g_inputTmpIndex;
    bool g_inputAccountValid;
    SelectAddressWidgetsItem_t switchAddressWidgets[5];
} JumpButtonWidgets_t;


static PageWidget_t *g_pageWidget;
static SelectAddressWidgetsItem_t g_selectAddressWidgets[5];
static uint32_t g_showIndex = 0;
static GuiChainCoinType g_chainCoinType;
static uint32_t g_selectIndex = 0;
static uint32_t g_initedSelectIndex = 0;
static lv_obj_t *g_leftBtn;
static lv_obj_t *g_rightBtn;
static SetSelectAddressIndexFunc g_setSelectIndexFunc;
static lv_obj_t *g_confirmBtn;

static JumpButtonWidgets_t g_standardJumpButtonWidgets ;


static void SetCurrentSelectIndex(uint32_t selectIndex);
static void UpdateConfirmBtn(void);
static void BackHandler(lv_event_t *e);
static void ConfirmHandler(lv_event_t *e);
static void JumpToAccountHandler(lv_event_t *e);
static void CloseSwitchAddressHandler(lv_event_t *e);
static void InputAddressIndexKeyboardHandler(lv_event_t *e);
static void SetKeyboardValid(bool validation);
static void RefreshSwitchAccount(void);

static void CloseSwitchAddressHandler(lv_event_t *e)
{
    lv_obj_add_flag(g_standardJumpButtonWidgets.inputAccountCont, LV_OBJ_FLAG_HIDDEN);
}


static void SetKeyboardValid(bool validation)
{
    if (validation) {
        if (lv_btnmatrix_has_btn_ctrl(g_standardJumpButtonWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED)) {
            lv_btnmatrix_clear_btn_ctrl(g_standardJumpButtonWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
        }
        lv_btnmatrix_set_btn_ctrl(g_standardJumpButtonWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED);
    } else {
        if (lv_btnmatrix_has_btn_ctrl(g_standardJumpButtonWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED)) {
            lv_btnmatrix_clear_btn_ctrl(g_standardJumpButtonWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_CHECKED);
        }
        lv_btnmatrix_set_btn_ctrl(g_standardJumpButtonWidgets.inputAccountKeyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
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
        strcpy_s(input, sizeof(input), lv_label_get_text(g_standardJumpButtonWidgets.inputAccountLabel));
        if (strcmp(txt, LV_SYMBOL_OK) == 0) {
            if (g_standardJumpButtonWidgets.g_inputAccountValid) {
                sscanf(input, "%u", &g_standardJumpButtonWidgets.g_inputTmpIndex);
                g_showIndex = g_standardJumpButtonWidgets.g_inputTmpIndex / 5 * 5;
                g_selectIndex = g_standardJumpButtonWidgets.g_inputTmpIndex;
                RefreshSwitchAccount();
                UpdateConfirmBtn();
                lv_obj_add_flag(g_standardJumpButtonWidgets.inputAccountCont, LV_OBJ_FLAG_HIDDEN);
                g_standardJumpButtonWidgets.g_inputAccountValid = false;
            }
        } else if (strcmp(txt, "-") == 0) {
            len = strlen(input);
            if (len >= 1) {
                input[len - 1] = '\0';
                lv_label_set_text(g_standardJumpButtonWidgets.inputAccountLabel, input);
                lv_obj_add_flag(g_standardJumpButtonWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
                if (strlen(input) >= 1) {
                    g_standardJumpButtonWidgets.g_inputAccountValid = true;
                } else {
                    g_standardJumpButtonWidgets.g_inputAccountValid = false;
                }
            }
        } else if (strlen(input) < 15) {
            strcat(input, txt);
            longInt = strtol(input, NULL, 10);
            if (longInt >= GENERAL_ADDRESS_INDEX_MAX) {
                input[9] = '\0';
                lv_obj_clear_flag(g_standardJumpButtonWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_standardJumpButtonWidgets.overflowLabel, LV_OBJ_FLAG_HIDDEN);
            }
            if (longInt > 0) {
                if (input[0] == '0') {
                    lv_label_set_text(g_standardJumpButtonWidgets.inputAccountLabel, input + 1);
                } else {
                    lv_label_set_text(g_standardJumpButtonWidgets.inputAccountLabel, input);
                }
            } else {
                lv_label_set_text(g_standardJumpButtonWidgets.inputAccountLabel, "0");
            }
            g_standardJumpButtonWidgets.g_inputAccountValid = true;
        } else {
            g_standardJumpButtonWidgets.g_inputAccountValid = false;
            printf("input to long\r\n");
        }
        SetKeyboardValid(g_standardJumpButtonWidgets.g_inputAccountValid);
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


static void JumpToAccountHandler(lv_event_t *e)
{
    lv_obj_t *cont, *label, *line, *closeBtn;
    static lv_point_t points[2] = {{0, 0}, {408, 0}};
    g_standardJumpButtonWidgets.g_inputAccountValid = false;

    g_standardJumpButtonWidgets.inputAccountCont = GuiCreateHintBox(530);
    lv_obj_add_event_cb(lv_obj_get_child(g_standardJumpButtonWidgets.inputAccountCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_standardJumpButtonWidgets.inputAccountCont);
    cont = g_standardJumpButtonWidgets.inputAccountCont;

    label = GuiCreateNoticeLabel(cont, _("receive_btc_receive_change_address_title"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 30 + 270);
    label = GuiCreateNoticeLabel(cont, "");
    lv_label_set_text_fmt(label, "%s-", _("Address"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 108 + 270);
    g_standardJumpButtonWidgets.inputAccountLabel = GuiCreateTextLabel(cont, "");
    lv_obj_align(g_standardJumpButtonWidgets.inputAccountLabel, LV_ALIGN_TOP_LEFT, 38 + lv_obj_get_self_width(label), 108 + 270);
    label = GuiCreateIllustrateLabel(cont, _("receive_btc_receive_change_address_limit"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 170 + 270);
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_standardJumpButtonWidgets.overflowLabel = label;

    line = GuiCreateLine(cont, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 160 + 270);

    lv_obj_t *keyboard = GuiCreateNumKeyboard(cont, InputAddressIndexKeyboardHandler, NUM_KEYBOARD_NORMAL, NULL);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_add_style(keyboard, &g_enterPressBtnmStyle, LV_STATE_PRESSED | LV_PART_ITEMS);
    lv_btnmatrix_set_btn_ctrl(keyboard, 11, LV_BTNMATRIX_CTRL_DISABLED);
    g_standardJumpButtonWidgets.inputAccountKeyboard = keyboard;

    closeBtn = GuiCreateImgButton(cont, &imgClose, 40, CloseSwitchAddressHandler, NULL);
    lv_obj_align(closeBtn, LV_ALIGN_TOP_RIGHT, -36, 27 + 270);
}

static void SelectAddressHandler(lv_event_t *e)
{
    lv_obj_t *checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < 5; i++) {
        if (checkBox == g_selectAddressWidgets[i].checkBox) {
            lv_obj_add_state(g_selectAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            SetCurrentSelectIndex(g_showIndex + i);
            UpdateConfirmBtn();
        } else {
            lv_obj_clear_state(g_selectAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item)
{
    switch (g_chainCoinType) {
#ifndef BTC_ONLY
    case CHAIN_XRP:
        item->index = index;
        strcpy(item->address, GuiGetXrpAddressByIndex(index));
        break;
    case CHAIN_ADA:
        item->index = index;
        char *xpub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndexAndDerivationType(
                GetConnectWalletPathIndex(GetWalletNameByIndex(GuiConnectWalletGetWalletIndex())),
                index));
        strcpy(item->address, GuiGetADABaseAddressByXPub(xpub));
        break;
    case CHAIN_ATOM:
        item->index = index;
        // we dont need to show the address,only show the account index. thus the address is empty.
        strcpy(item->address, "");
        break;
#endif
    default:
        printf("ModelGetAddress cannot match %d\r\n", index);
        return;
    }
}

static void SetCurrentSelectIndex(uint32_t selectIndex)
{
    g_selectIndex = selectIndex;
}

static uint32_t GetCurrentSelectAddressIndex()
{
    return g_selectIndex;
}

static bool IsSelectChanged()
{
    return g_selectIndex != g_initedSelectIndex;
}

static int GetMaxAddressIndex(void)
{
    switch (g_chainCoinType) {
#ifndef BTC_ONLY
    case CHAIN_XRP:
        return 200;
    case CHAIN_ADA:
        return 23;
#endif
    default:
        return 999999999;
    }
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem = {0};
    char string[128];
    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_selectAddressWidgets[i].addressCountLabel, "%s-%u", _("account_head"), addressDataItem.index);
        CutAndFormatString(string, sizeof(string), addressDataItem.address, 28);
        lv_label_set_text(g_selectAddressWidgets[i].addressLabel, string);
        if (end) {
            lv_obj_add_flag(g_selectAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_selectAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_selectAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(g_selectAddressWidgets[i].addressCountLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_selectAddressWidgets[i].addressLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_selectAddressWidgets[i].checkBox, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        if (index == GetCurrentSelectAddressIndex()) {
            lv_obj_add_state(g_selectAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_state(g_selectAddressWidgets[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        }
        if (index == GetMaxAddressIndex()) {
            end = true;
        }
        index++;
    }
}

static void GuiCreateSelectAddressList(lv_obj_t *parent)
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
        g_selectAddressWidgets[i].addressCountLabel = GuiCreateTextLabel(cont, "account");
        lv_obj_align(g_selectAddressWidgets[i].addressCountLabel, LV_ALIGN_TOP_LEFT, 24, 30 + 103 * i);
        g_selectAddressWidgets[i].addressLabel = GuiCreateNoticeLabel(cont, "address");
        lv_obj_align(g_selectAddressWidgets[i].addressLabel, LV_ALIGN_TOP_LEFT, 24, 56 + 103 * i);
        if (i > 0) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * i);
        }

        g_selectAddressWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_selectAddressWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_selectAddressWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_selectAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_selectAddressWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_selectAddressWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_selectAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_selectAddressWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_selectAddressWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_selectAddressWidgets[i].checkBox, SelectAddressHandler, LV_EVENT_CLICKED, NULL);

        g_selectAddressWidgets[i].checkedImg = GuiCreateImg(g_selectAddressWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_selectAddressWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_selectAddressWidgets[i].uncheckedImg = GuiCreateImg(g_selectAddressWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_selectAddressWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

        index++;
    }
    RefreshSwitchAccount();
}

static void LeftBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_rightBtn, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showIndex >= 5) {
        g_showIndex -= 5;
        RefreshSwitchAccount();
    }
    if (g_showIndex < 5) {
        lv_obj_set_style_img_opa(g_leftBtn, LV_OPA_30, LV_PART_MAIN);
    }
}

static void RightBtnHandler(lv_event_t *e)
{
    lv_obj_set_style_img_opa(g_leftBtn, LV_OPA_COVER, LV_PART_MAIN);
    if (g_showIndex < GetMaxAddressIndex() - 5) {
        g_showIndex += 5;
        RefreshSwitchAccount();
    }
    if (g_showIndex >= GetMaxAddressIndex() - 5) {
        lv_obj_set_style_img_opa(g_rightBtn, LV_OPA_30, LV_PART_MAIN);
    }
}

static void UpdateConfirmBtn(void)
{
    if (IsSelectChanged()) {
        lv_obj_set_style_bg_opa(g_confirmBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_confirmBtn, 0), LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_confirmBtn, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_confirmBtn, 0), LV_OPA_30, LV_PART_MAIN);
    }
}


static void GuiCreatePaginationBtns(lv_obj_t *parent)
{
    lv_obj_t *btn = GuiCreateImgButton(parent, &imgArrowLeft, 66, LeftBtnHandler, NULL);
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_t *img = lv_obj_get_child(btn, 0);
    if (g_showIndex < 5) {
        lv_obj_set_style_img_opa(img, LV_OPA_30, LV_PART_MAIN);
    }
    g_leftBtn = img;

    btn = GuiCreateImgButton(parent, &imgArrowRight, 66, RightBtnHandler, NULL);
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 156, -24);
    img = lv_obj_get_child(btn, 0);
    g_rightBtn = img;

    btn = GuiCreateBtn(parent, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, ConfirmHandler, LV_EVENT_CLICKED, NULL);
    g_confirmBtn = btn;
    UpdateConfirmBtn();
}

void GuiDestroySelectAddressWidget()
{
    GUI_PAGE_DEL(g_pageWidget);
}

static void BackHandler(lv_event_t *e)
{
    GuiDestroySelectAddressWidget();
    g_setSelectIndexFunc(g_initedSelectIndex);
}

static void ConfirmHandler(lv_event_t *e)
{
    if (IsSelectChanged()) {
        GuiDestroySelectAddressWidget();
        g_setSelectIndexFunc(GetCurrentSelectAddressIndex());
    }
}

lv_obj_t *GuiCreateSelectAddressWidget(GuiChainCoinType chainCoinType, uint32_t selectIndex, SetSelectAddressIndexFunc setIndex)
{
    g_chainCoinType = chainCoinType;
    g_initedSelectIndex = selectIndex;
    SetCurrentSelectIndex(selectIndex);
    // pass the selected address index to the connect wallet page through g_setSelectIndexFunc
    g_setSelectIndexFunc = setIndex;
    g_showIndex = selectIndex / 5 * 5;

    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, BackHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("switch_account"));
    switch (g_chainCoinType) {
    case CHAIN_ATOM:
        // add jump button at the right of the nav bar
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SKIP, JumpToAccountHandler, NULL);
        break;
    default:
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    };
    GuiCreateSelectAddressList(cont);
    GuiCreatePaginationBtns(cont);
    return cont;
}

#endif