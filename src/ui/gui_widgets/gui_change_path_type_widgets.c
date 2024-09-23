#ifndef BTC_ONLY
#include "gui_change_path_type_widgets.h"

typedef struct {
    char title[PATH_ITEM_MAX_LEN];
    char subTitle[PATH_ITEM_MAX_LEN];
    char path[PATH_ITEM_MAX_LEN];
} PathItem_t;

static const PathItem_t g_ethPaths[] = {
    {"BIP44 Standard",          "",     "m/44'/60'/0'"  },
    {"Ledger Live",             "",     "m/44'/60'"     },
    {"Ledger Legacy",           "",     "m/44'/60'/0'"  },
};

typedef struct {
    uint32_t index;
    char address[ADDRESS_MAX_LEN];
    char path[PATH_ITEM_MAX_LEN];
} AddressDataItem_t;
static lv_obj_t *g_egCont = NULL;
static char **g_derivationPathDescs = NULL;

typedef struct {
    lv_obj_t *checkBox;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} PathWidgetsItem_t;

static uint32_t g_selectType = 0;
static uint8_t g_currentAccountIndex = 0;
static lv_obj_t *g_addressLabel[2];

static PathWidgetsItem_t g_changePathWidgets[3];
static lv_obj_t *g_confirmAddrTypeBtn;
static HOME_WALLET_CARD_ENUM g_currentChain = HOME_WALLET_CARD_BUTT;
static lv_obj_t *g_derivationPathDescLabel = NULL;

static lv_event_cb_t g_changed_cb = NULL;

static void GetChangePathLabelHint(char* hint);
static uint32_t GetDerivedPathTypeCount();
static const char* GetChangePathItemTitle(uint32_t i);
static void GetPathItemSubTitle(char* subTitle, int index, uint32_t maxLen);
static void GetEthPathItemSubTittle(char* subTitle, int index, uint32_t maxLen);
static void GetSolPathItemSubTitle(char* subTitle, int index, uint32_t maxLen);
static void GetADAPathItemSubTitle(char* subTitle, int index, uint32_t maxLen);
static void UpdateAddrTypeCheckbox(uint8_t i, bool isChecked);
static void UpdateConfirmAddrTypeBtn(void);
static void ShowEgAddressCont(lv_obj_t *egCont);
static void InitDerivationPathDesc(uint8_t chain);
static void SetPathIndex(uint32_t index);
static void ConfirmAddrTypeHandler(lv_event_t *e);
static bool IsAddrTypeSelectChanged();
static uint32_t GetPathIndex(void);
static bool IsOnlyOneAddress(uint8_t addrType);
static void ChangePathCheckHandler(lv_event_t *e);
static void RefreshDefaultAddress(void);
static void ModelGetAddress(uint32_t index, AddressDataItem_t *item);
static void ModelGetADAAddress(uint32_t index, AddressDataItem_t *item, uint8_t type);

void GuiCreateSwitchPathTypeWidget(lv_obj_t *parent, HOME_WALLET_CARD_ENUM chain, lv_event_cb_t changed_cb)
{
    g_changed_cb = changed_cb;
    g_currentChain = chain;
    g_currentAccountIndex = GetCurrentAccountIndex();
    if (chain == HOME_WALLET_CARD_ADA) {
        SetPathIndex(GetReceivePageAdaXPubType());
    }
    g_selectType = GetPathIndex();
    InitDerivationPathDesc(chain);

    lv_obj_t *cont, *line, *label;
    static lv_point_t points[2] =  {{0, 0}, {360, 0}};
    char lableText[BUFFER_SIZE_128] = {0};
    GetChangePathLabelHint(lableText);
    lv_obj_t *scrollCont = GuiCreateContainerWithParent(parent, 408, 542);
    lv_obj_align(scrollCont, LV_ALIGN_DEFAULT, 36, 0);
    lv_obj_add_flag(scrollCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(scrollCont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *labelHint = GuiCreateIllustrateLabel(scrollCont, lableText);
    lv_obj_set_style_text_opa(labelHint, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(labelHint, LV_ALIGN_TOP_LEFT, 0, 0);

    cont = GuiCreateContainerWithParent(scrollCont, 408, 103 * GetDerivedPathTypeCount() - 1);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);

    for (uint32_t i = 0; i < GetDerivedPathTypeCount(); i++) {
        label = GuiCreateTextLabel(cont, GetChangePathItemTitle(i));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 32 + 103 * i);
        if (i < GetDerivedPathTypeCount() - 1) {
            line = GuiCreateLine(cont, points, 2);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102 * (i + 1));
        }
        g_changePathWidgets[i].checkBox = lv_btn_create(cont);
        lv_obj_set_size(g_changePathWidgets[i].checkBox, 408, 82);
        lv_obj_align(g_changePathWidgets[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_set_style_bg_opa(g_changePathWidgets[i].checkBox, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_changePathWidgets[i].checkBox, LV_OPA_TRANSP, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(g_changePathWidgets[i].checkBox, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_outline_width(g_changePathWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(g_changePathWidgets[i].checkBox, 0, LV_PART_MAIN);
        lv_obj_add_flag(g_changePathWidgets[i].checkBox, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(g_changePathWidgets[i].checkBox, ChangePathCheckHandler, LV_EVENT_CLICKED, NULL);

        g_changePathWidgets[i].checkedImg = GuiCreateImg(g_changePathWidgets[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_changePathWidgets[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_changePathWidgets[i].uncheckedImg = GuiCreateImg(g_changePathWidgets[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_changePathWidgets[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_clear_flag(g_changePathWidgets[g_selectType].checkedImg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_changePathWidgets[g_selectType].uncheckedImg, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *egCont = GuiCreateContainerWithParent(scrollCont, 408, 186);
    lv_obj_align_to(egCont, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_bg_color(egCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(egCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);
    lv_obj_set_style_radius(egCont, 24, LV_PART_MAIN);
    g_egCont = egCont;
    ShowEgAddressCont(g_egCont);

    lv_obj_t *tmCont = GuiCreateContainerWithParent(parent, 480, 114);
    lv_obj_align(tmCont, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_color(tmCont, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_t *btn = GuiCreateBtn(tmCont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -36, 0);
    lv_obj_add_event_cb(btn, ConfirmAddrTypeHandler, LV_EVENT_CLICKED, NULL);
    g_confirmAddrTypeBtn = btn;

    UpdateConfirmAddrTypeBtn();
}

void GuiDestroySwitchPathTypeWidget(void)
{
    g_currentChain = HOME_WALLET_CARD_BUTT;
    g_currentAccountIndex = 0;
    g_selectType = 0;
    g_egCont = NULL;
    g_derivationPathDescs = NULL;
    g_confirmAddrTypeBtn = NULL;
    g_derivationPathDescLabel = NULL;
    for (uint32_t i = 0; i < 3; i++) {
        g_changePathWidgets[i].checkBox = NULL;
        g_changePathWidgets[i].checkedImg = NULL;
        g_changePathWidgets[i].uncheckedImg = NULL;
    }
    for (uint32_t i = 0; i < 2; i++) {
        g_addressLabel[i] = NULL;
    }
}

static void ConfirmAddrTypeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED && IsAddrTypeSelectChanged()) {
        SetPathIndex(g_selectType);
        SetAccountReceivePath(GetCoinCardByIndex(g_currentChain)->coin, g_selectType);
        if (g_changed_cb != NULL) {
            SetReceivePageAdaXPubType(g_selectType);
            g_changed_cb(e);
        }
        ReturnHandler(e);
    }
}

static bool IsAddrTypeSelectChanged()
{
    return g_selectType != GetPathIndex();
}

static uint32_t GetPathIndex(void)
{
    return GetAccountReceivePath(GetCoinCardByIndex(g_currentChain)->coin);
}

static void SetPathIndex(uint32_t index)
{
    SetAccountReceivePath(GetCoinCardByIndex(g_currentChain)->coin, index);
}

static void InitDerivationPathDesc(uint8_t chain)
{
    switch (chain) {
    case HOME_WALLET_CARD_ETH:
        g_derivationPathDescs = GetDerivationPathDescs(ETH_DERIVATION_PATH_DESC);
        break;
    case HOME_WALLET_CARD_SOL:
    case HOME_WALLET_CARD_HNT:
        g_derivationPathDescs = GetDerivationPathDescs(SOL_DERIVATION_PATH_DESC);
        break;
    case HOME_WALLET_CARD_ADA:
        g_derivationPathDescs = GetDerivationPathDescs(ADA_DERIVATION_PATH_DESC);
        break;
    default:
        break;
    }
}

static void ShowEgAddressCont(lv_obj_t *egCont)
{

    if (egCont == NULL) {
        printf("egCont is NULL, cannot show eg address\n");
        return;
    }

    lv_obj_clean(egCont);

    lv_obj_t *prevLabel, *label;

    int egContHeight = 12;
    label = GuiCreateNoticeLabel(egCont, g_derivationPathDescs[g_selectType]);
    lv_obj_set_width(label, 360);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    lv_obj_update_layout(label);
    egContHeight += lv_obj_get_height(label);
    g_derivationPathDescLabel = label;
    prevLabel = label;

    int gap = 4;
    if (strnlen_s(g_derivationPathDescs[g_selectType], ADDRESS_MAX_LEN) == 0) {
        egContHeight -= lv_obj_get_height(label);
        lv_obj_set_height(label, 0);
        gap = 0;
    }

    const char *desc = _("derivation_path_address_eg");
    label = GuiCreateNoticeLabel(egCont, desc);
    lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, gap);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(label);
    egContHeight =  egContHeight + 4 + lv_obj_get_height(label);
    prevLabel = label;

    lv_obj_t *index = GuiCreateNoticeLabel(egCont, _("0"));
    lv_obj_align_to(index, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_label_set_long_mode(index, LV_LABEL_LONG_WRAP);
    lv_obj_update_layout(index);
    egContHeight =  egContHeight + 4 + lv_obj_get_height(index);
    prevLabel = index;

    label = GuiCreateIllustrateLabel(egCont, "");
    lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
    g_addressLabel[0] = label;

    // g_selectType means the derivation path type
    if (!IsOnlyOneAddress(g_selectType)) {
        index = GuiCreateNoticeLabel(egCont, _("1"));
        lv_obj_align_to(index, prevLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        lv_label_set_long_mode(index, LV_LABEL_LONG_WRAP);
        lv_obj_update_layout(index);
        egContHeight =  egContHeight + 4 + lv_obj_get_height(index);
        prevLabel = index;

        label = GuiCreateIllustrateLabel(egCont, "");
        lv_obj_align_to(label, prevLabel, LV_ALIGN_OUT_RIGHT_MID, 12, 0);
        g_addressLabel[1] = label;
    }
    egContHeight += 12;
    lv_obj_set_height(egCont, egContHeight);

    RefreshDefaultAddress();
}

static void RefreshDefaultAddress(void)
{
    char string[128];

    AddressDataItem_t addressDataItem;

    ModelGetAddress(0, &addressDataItem);
    CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
    lv_label_set_text(g_addressLabel[0], string);

    if (!IsOnlyOneAddress(g_selectType)) {
        ModelGetAddress(1, &addressDataItem);
        CutAndFormatString(string, sizeof(string), addressDataItem.address, 24);
        lv_label_set_text(g_addressLabel[1], string);
    }
}

static void ModelGetAddress(uint32_t index, AddressDataItem_t *item)
{
    switch (g_currentChain) {
    case HOME_WALLET_CARD_ADA:
        ModelGetADAAddress(index, item, 0);
        break;
    default:
        break;
    }
}

static void ModelGetADAAddress(uint32_t index, AddressDataItem_t *item, uint8_t type)
{
    char *xPub = NULL, hdPath[BUFFER_SIZE_128] = {0};
    SimpleResponse_c_char *result = NULL;
    xPub = GetCurrentAccountPublicKey(GetAdaXPubTypeByIndexAndDerivationType(g_selectType, index));
    snprintf_s(hdPath, BUFFER_SIZE_128, "m/1852'/1815'/%u'", index);
    switch (type) {
    case 1:
        result = cardano_get_enterprise_address(xPub, 0, 1);
        break;
    case 2:
        result = cardano_get_stake_address(xPub, 0, 1);
        break;
    default:
        result = cardano_get_base_address(xPub, 0, 1);
        break;
    }
    item->index = index;
    strcpy_s(item->address, ADDRESS_MAX_LEN, result->data);
    strcpy_s(item->path, PATH_ITEM_MAX_LEN, hdPath);
    free_simple_response_c_char(result);
}

static void UpdateConfirmAddrTypeBtn(void)
{
    if (IsAddrTypeSelectChanged()) {
        lv_obj_set_style_bg_opa(g_confirmAddrTypeBtn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_confirmAddrTypeBtn, 0), LV_OPA_COVER, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_opa(g_confirmAddrTypeBtn, LV_OPA_30, LV_PART_MAIN);
        lv_obj_set_style_text_opa(lv_obj_get_child(g_confirmAddrTypeBtn, 0), LV_OPA_30, LV_PART_MAIN);
    }
}

static bool IsOnlyOneAddress(uint8_t addrType)
{
    if (g_currentChain == HOME_WALLET_CARD_SOL && addrType == 1) {
        return true;
    }
    if (g_currentChain == HOME_WALLET_CARD_HNT && addrType == 1) {
        return true;
    }
    return false;
}

static void ChangePathCheckHandler(lv_event_t *e)
{
    lv_obj_t *checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < GetDerivedPathTypeCount(); i++) {
        bool isChecked = checkBox == g_changePathWidgets[i].checkBox;
        UpdateAddrTypeCheckbox(i, isChecked);
    }
    UpdateConfirmAddrTypeBtn();
}

static void UpdateAddrTypeCheckbox(uint8_t i, bool isChecked)
{
    if (isChecked) {
        lv_obj_add_state(g_changePathWidgets[i].checkBox, LV_STATE_CHECKED);
        lv_obj_clear_flag(g_changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        g_selectType = i;
        ShowEgAddressCont(g_egCont);
    } else {
        lv_obj_clear_state(g_changePathWidgets[i].checkBox, LV_STATE_CHECKED);
        lv_obj_add_flag(g_changePathWidgets[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_changePathWidgets[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_currentChain == HOME_WALLET_CARD_ADA && isChecked) {
        RefreshDefaultAddress();
    }
}

static uint32_t GetDerivedPathTypeCount()
{
    switch (g_currentChain) {
    case HOME_WALLET_CARD_ETH:
        return 3;
    case HOME_WALLET_CARD_SOL:
    case HOME_WALLET_CARD_HNT:
        return 3;
    case HOME_WALLET_CARD_ADA:
        return 2;
    default:
        return 3;
    }
}

static void GetPathItemSubTitle(char* subTitle, int index, uint32_t maxLen)
{
    switch (g_currentChain) {
    case HOME_WALLET_CARD_ETH:
        GetEthPathItemSubTittle(subTitle, index, maxLen);
        break;
    case HOME_WALLET_CARD_SOL:
    case HOME_WALLET_CARD_HNT:
        GetSolPathItemSubTitle(subTitle, index, maxLen);
        break;
    case HOME_WALLET_CARD_ADA:
        GetADAPathItemSubTitle(subTitle, index, maxLen);
        break;
    default:
        break;
    }
}

static void GetEthPathItemSubTittle(char* subTitle, int index, uint32_t maxLen)
{
    switch (index) {
    case 0:
        strcpy_s(subTitle, maxLen, "m/44'/60'/0'/0/#F5870A X#");
        break;
    case 1:
        strcpy_s(subTitle, maxLen, "m/44'/60'/#F5870A X#'/0/0");
        break;
    case 2:
        strcpy_s(subTitle, maxLen, "m/44'/60'/0'/#F5870A X#");
        break;
    default:
        break;
    }
}

static void GetSolPathItemSubTitle(char* subTitle, int index, uint32_t maxLen)
{
    switch (index) {
    case 0:
        snprintf_s(subTitle, maxLen, "m/44'/501'/#F5870A X#'");
        break;
    case 1:
        snprintf_s(subTitle, maxLen, "m/44'/501'");
        break;
    case 2:
        snprintf_s(subTitle, maxLen, "m/44'/501'/#F5870A X#'/0'");
        break;
    default:
        break;
    }
}

static void GetADAPathItemSubTitle(char* subTitle, int index, uint32_t maxLen)
{
    snprintf_s(subTitle, maxLen, "m/1852'/1815'/#F5870A X#'");
}

static const char* GetChangePathItemTitle(uint32_t i)
{
    switch (g_currentChain) {
    case HOME_WALLET_CARD_ETH:
        return (char *)g_ethPaths[i].title;
    case HOME_WALLET_CARD_SOL:
    case HOME_WALLET_CARD_HNT:
        if (i == 0) {
            return _("receive_sol_more_t_base_path");
        } else if (i == 1) {
            return _("receive_sol_more_t_single_path");
        } else if (i == 2) {
            return _("receive_sol_more_t_sub_path");
        }
    case HOME_WALLET_CARD_ADA:
        if (i == 0) {
            return _("receive_ada_more_t_standard");
        } else if (i == 1) {
            return _("receive_ada_more_t_ledger");
        }
    default:
        break;
    }
    return "";
}

static void GetChangePathLabelHint(char* hint)
{
    switch (g_currentChain) {
    case HOME_WALLET_CARD_ETH:
        snprintf_s(hint, BUFFER_SIZE_128, _("derivation_path_select_eth"));
        return;
    case HOME_WALLET_CARD_SOL:
    case HOME_WALLET_CARD_HNT:
        snprintf_s(hint, BUFFER_SIZE_128, _("derivation_path_select_sol"));
        return;
    case HOME_WALLET_CARD_ADA:
        snprintf_s(hint, BUFFER_SIZE_128, _("derivation_path_select_ada"));
        return;
    default:
        break;
    }
}
#endif