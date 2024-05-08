#ifndef BTC_ONLY
#include "gui_connect_ada_widgets.h"
#include "gui_connect_wallet_widgets.h"
#include "gui.h"
#include "gui_page.h"
#include "gui_hintbox.h"
#include "gui_button.h"

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_cont;
static lv_obj_t *g_openMoreHintBox;
static WALLET_LIST_INDEX_ENUM g_walletIndex;

static void GuiCreatePageContent(lv_obj_t *parent);
static void GotoScanQRCodeHandler(lv_event_t *e);
static void CleanHandler(lv_event_t *e);
static void OpenTutorialHandler(lv_event_t *e);
static void OpenMoreHandler(lv_event_t *e);

bool ConnectADAWidgetExist()
{
    return g_pageWidget != NULL;
}

void CleanConnectADAWidget()
{
    GUI_PAGE_DEL(g_pageWidget);
}

void GuiCreateConnectADAWalletWidget(uint8_t index)
{
    g_walletIndex = index;
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style(cont, NULL, LV_PART_SCROLLBAR);
    g_cont = cont;
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CleanHandler, NULL);
    SetWallet(g_pageWidget->navBarWidget, index, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, NULL);
    GuiCreatePageContent(g_cont);
}

static void GuiCreatePageContent(lv_obj_t *parent)
{
    lv_obj_t *button, *label, *cont;
    label = GuiCreateIllustrateLabel(parent, _("connect_wallet_instruction"));
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 8);

    cont = GuiCreateContainerWithParent(parent, 408, 270);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 62);

    char number[BUFFER_SIZE_16] = {0};

    strcpy_s(number, BUFFER_SIZE_16, "#F5870A 1#");
    label = GuiCreateIllustrateLabel(cont, number);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

    label = GuiCreateIllustrateLabel(cont, _("connect_wallet_ada_step1"));
    lv_label_set_text_fmt(label, _("connect_wallet_ada_step1"), GetWalletNameByIndex(g_walletIndex));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 0);

    strcpy_s(number, BUFFER_SIZE_16, "#F5870A 2#");
    label = GuiCreateIllustrateLabel(cont, number);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 72);

    label = GuiCreateIllustrateLabel(cont, _("connect_wallet_ada_step2"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 72);

    strcpy_s(number, BUFFER_SIZE_16, "#F5870A 3#");
    label = GuiCreateIllustrateLabel(cont, number);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 114);

    label = GuiCreateIllustrateLabel(cont, _("connect_wallet_ada_step3"));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 114);

    strcpy_s(number, BUFFER_SIZE_16, "#F5870A 4#");
    label = GuiCreateIllustrateLabel(cont, number);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 186);

    label = GuiCreateIllustrateLabel(cont, _("connect_wallet_ada_step4"));
    lv_label_set_text_fmt(label, _("connect_wallet_ada_step4"), GetWalletNameByIndex(g_walletIndex));
    lv_obj_set_width(label, 384);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 186);

    button = GuiCreateBtn(parent, USR_SYMBOL_ARROW_NEXT);
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(button, GotoScanQRCodeHandler, LV_EVENT_CLICKED, NULL);
}

static void CleanHandler(lv_event_t *e)
{
    CleanConnectADAWidget();
}

static void GotoScanQRCodeHandler(lv_event_t *e)
{
    GuiFrameOpenView(&g_scanView);
}

static void OpenTutorialHandler(lv_event_t *e)
{
    WALLET_LIST_INDEX_ENUM *wallet = lv_event_get_user_data(e);
    GuiFrameOpenViewWithParam(&g_walletTutorialView, wallet, sizeof(WALLET_LIST_INDEX_ENUM));
    GUI_DEL_OBJ(g_openMoreHintBox);
}

static void OpenMoreHandler(lv_event_t *e)
{
    int hintboxHeight = 132;
    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_openMoreHintBox);
    lv_obj_t *label = GuiCreateTextLabel(g_openMoreHintBox, _("Tutorial"));
    lv_obj_t *img = GuiCreateImg(g_openMoreHintBox, &imgTutorial);

    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {76, 0},
        },
    };
    lv_obj_t *btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                    OpenTutorialHandler, &g_walletIndex);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
}
#endif

