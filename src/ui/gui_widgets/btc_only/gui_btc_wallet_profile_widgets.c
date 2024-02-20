#ifdef BTC_ONLY
#include "gui_btc_wallet_profile_widgets.h"
#include "gui.h"
#include "gui_views.h"
#include "gui_model.h"
#include "gui_status_bar.h"
#include "gui_page.h"
#include "gui_button.h"
#include "gui_btc_home_widgets.h"
#include "gui_hintbox.h"

typedef struct {
    lv_obj_t *button;
    lv_obj_t *label;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
} Checkbox_t;

static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent);
static void CreateBtcNetworkWidget(lv_obj_t *parent);
static void NetworkSelHandler(lv_event_t *e);
static void NetworkHandler(lv_event_t *e);
static void ExportXpubHandler(lv_event_t *e);
static void EmptyHandler(lv_event_t *e);

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_networkCont;
Checkbox_t g_networkCheckbox[2];

void GuiBtcWalletProfileInit(void)
{
    printf("%s\n", __func__);
    g_pageWidget = CreatePageWidget();
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_profile_mid_btn"));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    CreateBtcWalletProfileEntranceWidget(g_pageWidget->contentZone);
}


void GuiBtcWalletProfileDeInit(void)
{
    printf("%s\n", __func__);

    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}


void GuiBtcWalletProfileRefresh(void)
{
    printf("%s\n", __func__);
}


int8_t GuiBtcWalletProfilePrevTile(uint8_t tileIndex)
{
    printf("%s\n", __func__);
    return 0;
}


int8_t GuiBtcWalletProfileNextTile(uint8_t tileIndex)
{
    printf("%s\n", __func__);
    return 0;
}


static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent)
{
    lv_obj_t *label, *img, *imgArrow, *button, *line;

    img = GuiCreateImg(parent, &imgKey);
    label = GuiCreateTextLabel(parent, _("wallet_profile_single-sign_title"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    GuiButton_t table[3] = {
        {
            .obj = img,
            .align = LV_ALIGN_TOP_LEFT,
            .position = {24, 24},
        },
        {
            .obj = label,
            .align = LV_ALIGN_TOP_LEFT,
            .position = {76, 24},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_TOP_LEFT,
            .position = {396, 24},
        },
    };
    lv_obj_add_flag(table[2].obj, LV_OBJ_FLAG_HIDDEN);
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), EmptyHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 0);

    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, 96);

    table[0].obj = GuiCreateImg(parent, &imgNetwork);
    table[1].obj = GuiCreateTextLabel(parent, _("wallet_profile_network_title"));
    table[2].obj = GuiCreateImg(parent, &imgArrowRight);
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), NetworkHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 109);

    table[0].obj = GuiCreateImg(parent, &imgExport);
    table[1].obj = GuiCreateTextLabel(parent, _("wallet_profile_export_title"));
    table[2].obj = GuiCreateImg(parent, &imgArrowRight);
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), ExportXpubHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 205);
}


static void CreateBtcNetworkWidget(lv_obj_t *parent)
{
    lv_obj_t *label, *closeBtn, *closeImg;
    g_networkCont = GuiCreateHintBox(parent, 480, 282, true);
    lv_obj_add_event_cb(lv_obj_get_child(g_networkCont, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_networkCont);
    label = GuiCreateIllustrateLabel(g_networkCont, "Network");
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -222);

    closeBtn = GuiCreateContainerWithParent(g_networkCont, 64, 64);
    lv_obj_align(closeBtn, LV_ALIGN_BOTTOM_RIGHT, -24, -205);
    lv_obj_set_style_bg_opa(closeBtn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_add_flag(closeBtn, LV_OBJ_FLAG_CLICKABLE);
    closeImg = GuiCreateImg(closeBtn, &imgClose);
    lv_obj_align(closeImg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(closeBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_networkCont);

    char *netType[] = {"MainNet", "TestNet"};
    for (uint32_t i = 0; i < 2; i++) {
        g_networkCheckbox[i].button = GuiCreateContainerWithParent(g_networkCont, 456, 84);
        lv_obj_align(g_networkCheckbox[i].button, LV_ALIGN_TOP_MID, 0, 608 + i * 96);
        lv_obj_set_style_bg_opa(g_networkCheckbox[i].button, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_add_flag(g_networkCheckbox[i].button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_networkCheckbox[i].button, NetworkSelHandler, LV_EVENT_CLICKED, NULL);

        g_networkCheckbox[i].label = GuiCreateTextLabel(g_networkCheckbox[i].button, netType[i]);
        lv_obj_align(g_networkCheckbox[i].label, LV_ALIGN_LEFT_MID, 24, 0);

        g_networkCheckbox[i].checkedImg = GuiCreateImg(g_networkCheckbox[i].button, &imgMessageSelect);
        lv_obj_align(g_networkCheckbox[i].checkedImg, LV_ALIGN_RIGHT_MID, -28, 0);
        lv_obj_add_flag(g_networkCheckbox[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_networkCheckbox[i].uncheckedImg = GuiCreateImg(g_networkCheckbox[i].button, &imgUncheckCircle);
        lv_obj_align(g_networkCheckbox[i].uncheckedImg, LV_ALIGN_RIGHT_MID, -24, 0);
        lv_obj_clear_flag(g_networkCheckbox[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
    if (GetIsTestNet()) {
        lv_obj_add_flag(g_networkCheckbox[0].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_networkCheckbox[0].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_networkCheckbox[1].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_networkCheckbox[1].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(g_networkCheckbox[0].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_networkCheckbox[0].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_networkCheckbox[1].checkedImg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_networkCheckbox[1].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }
}


static void NetworkSelHandler(lv_event_t *e)
{
    uint32_t i, j;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        printf("net work sel\n");
        for (i = 0; i < 2; i++) {
            if (target == g_networkCheckbox[i].button) {
                printf("target is %d\n", i);
                SetIsTestNet(i == 1);
                for (j = 0; j < 2; j++) {
                    if (i == j) {
                        //checked
                        lv_obj_add_flag(g_networkCheckbox[j].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_clear_flag(g_networkCheckbox[j].checkedImg, LV_OBJ_FLAG_HIDDEN);
                    } else {
                        //unchecked
                        lv_obj_add_flag(g_networkCheckbox[j].checkedImg, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_clear_flag(g_networkCheckbox[j].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
                    }
                }
            }
        }
    }
}


static void NetworkHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        CreateBtcNetworkWidget(g_pageWidget->contentZone);
    }
}

static void ExportXpubHandler(lv_event_t *e)
{
    HOME_WALLET_CARD_ENUM chainCard = HOME_WALLET_CARD_BTC;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenViewWithParam(&g_exportPubkeyView, &chainCard, sizeof(chainCard));
    }
}


static void EmptyHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("do nothing\n");
    }
}


#endif
