#include "gui_select_address_widgets.h"
#include "gui_home_widgets.h"
#include "account_public_info.h"
#include "gui_page.h"

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

static PageWidget_t *g_pageWidget;
static SelectAddressWidgetsItem_t g_selectAddressWidgets[5];
static uint32_t g_showIndex = 0;
static GuiChainCoinType g_chainCoinType;
static uint32_t g_selectIndex = 0;
static lv_obj_t *g_leftBtn;
static lv_obj_t *g_rightBtn;
static SetSelectAddressIndexFunc g_setSelectIndexFunc;

static void SetCurrentSelectIndex(uint32_t selectIndex);

static void SelectAddressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *checkBox;

    if (code == LV_EVENT_CLICKED) {
        checkBox = lv_event_get_target(e);
        for (uint32_t i = 0; i < 5; i++) {
            if (checkBox == g_selectAddressWidgets[i].checkBox) {
                lv_obj_add_state(g_selectAddressWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_clear_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                SetCurrentSelectIndex(g_showIndex + i);
            } else {
                lv_obj_clear_state(g_selectAddressWidgets[i].checkBox, LV_STATE_CHECKED);
                lv_obj_add_flag(g_selectAddressWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(g_selectAddressWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item)
{
    switch (g_chainCoinType) {
    case CHAIN_XRP:
        item->index = index;
        strcpy(item->address, GuiGetXrpAddressByIndex(index));
        break;
    case CHAIN_NEAR:
        item->index = index;
        strcpy(item->address, GetCurrentAccountPublicKey(XPUB_TYPE_NEAR_LEDGER_LIVE_0 + index));
        break;

    default:
        printf("ModelGetAddress cannot match %d\r\n", index);
        return;
    }
}

static void AddressLongModeCut(char *out, const char *address)
{
    uint32_t len = strlen(address);
    if (len <= 24) {
        strcpy(out, address);
        return;
    }
    strncpy(out, address, 12);
    out[12] = 0;
    strcat(out, "...");
    strcat(out, address + len - 12);
}

static void SetCurrentSelectIndex(uint32_t selectIndex)
{
    switch (g_chainCoinType)
    {
    default:
        g_selectIndex = selectIndex;
    }
}

static uint32_t GetCurrentSelectAddressIndex()
{
    switch (g_chainCoinType)
    {
    default:
        return g_selectIndex;
    }
}

static int GetMaxAddressIndex(void)
{
    switch (g_chainCoinType)
    {
    case CHAIN_XRP:
        return 200;
    default:
        return 999999999;
    }
}

static void RefreshSwitchAccount(void)
{
    AddressDataItem_t addressDataItem;
    char string[128];
    uint32_t index = g_showIndex;
    bool end = false;
    for (uint32_t i = 0; i < 5; i++) {
        ModelGetAddress(index, &addressDataItem);
        lv_label_set_text_fmt(g_selectAddressWidgets[i].addressCountLabel, "Account-%u", (addressDataItem.index + 1));
        AddressLongModeCut(string, addressDataItem.address);
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
        g_selectAddressWidgets[i].addressCountLabel = GuiCreateLabelWithFont(cont, "account", &openSans_24);
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
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_rightBtn, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex >= 5) {
            g_showIndex -= 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex < 5) {
            lv_obj_set_style_img_opa(g_leftBtn, LV_OPA_30, LV_PART_MAIN);
        }
    }
}

static void RightBtnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_set_style_img_opa(g_leftBtn, LV_OPA_COVER, LV_PART_MAIN);
        if (g_showIndex < GetMaxAddressIndex() - 5) {
            g_showIndex += 5;
            RefreshSwitchAccount();
        }
        if (g_showIndex >= GetMaxAddressIndex() - 5) {
            lv_obj_set_style_img_opa(g_rightBtn, LV_OPA_30, LV_PART_MAIN);
        }
    }
}

static void GuiCreatePaginationBtns(lv_obj_t *parent)
{
    lv_obj_t *btn;
    lv_obj_t *img;

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
    g_leftBtn = img;

    btn = GuiCreateBtn(parent, "");
    lv_obj_set_size(btn, 96, 66);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    img = GuiCreateImg(btn, &imgArrowRight);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(img, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, RightBtnHandler, LV_EVENT_CLICKED, NULL);
    g_rightBtn = img;
}

void GuiDestroySelectAddressWidget()
{
    GUI_PAGE_DEL(g_pageWidget);
}

static void BackHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        GuiDestroySelectAddressWidget();
        g_setSelectIndexFunc(GetCurrentSelectAddressIndex());
    }
}

lv_obj_t *GuiCreateSelectAddressWidget(GuiChainCoinType chainCoinType, uint32_t selectIndex, SetSelectAddressIndexFunc setIndex)
{
    g_chainCoinType = chainCoinType;
    SetCurrentSelectIndex(selectIndex);
    g_setSelectIndexFunc = setIndex;
    g_showIndex = selectIndex / 5 * 5;

    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, BackHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("switch_account"));
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);

    GuiCreateSelectAddressList(cont);
    GuiCreatePaginationBtns(cont);

    return cont;
}
