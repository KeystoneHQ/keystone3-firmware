#include "define.h"
#include "gui.h"
#include "lvgl.h"
#include "gui_framework.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_tutorial_widgets.h"
#include "gui_qr_hintbox.h"
#include "user_memory.h"
#include "gui_page.h"

typedef struct Tutorial {
    const char *title;
    const char *desc;
    const char *link;
    const char *qrTitle;
    const char *qrSubtitle;
    const char *qrCode;
} Tutorial_t;

typedef struct TutorialList {
    size_t len;
    Tutorial_t tutorials[3];
} TutorialList_t;

static TutorialList_t g_tutorials[TUTORIAL_LIST_INDEX_BUTT];

typedef struct GuiTurorialWidget {
    lv_obj_t *cont;
} GuiTurorialWidget_t;

static GuiTurorialWidget_t g_tutorialWidget;
static PageWidget_t *g_pageWidget = NULL;

static void TutorialsInit()
{
    // TUTORIAL_SHAMIR_BACKUP
    g_tutorials[TUTORIAL_SHAMIR_BACKUP].len = 1;
    g_tutorials[TUTORIAL_SHAMIR_BACKUP].tutorials[0].title = _("single_backup_learn_more_title");
    g_tutorials[TUTORIAL_SHAMIR_BACKUP].tutorials[0].desc = _("single_backup_learn_more_desc");
    g_tutorials[TUTORIAL_SHAMIR_BACKUP].tutorials[0].link = _("learn_more");
    g_tutorials[TUTORIAL_SHAMIR_BACKUP].tutorials[0].qrTitle = _("single_backup_learn_more_title");
    g_tutorials[TUTORIAL_SHAMIR_BACKUP].tutorials[0].qrSubtitle = _("single_backup_learn_more_qr_link");
    g_tutorials[TUTORIAL_SHAMIR_BACKUP].tutorials[0].qrCode = _("single_backup_learn_more_qr_link");

    // TUTORIAL_BTC_RECEIVE
    g_tutorials[TUTORIAL_BTC_RECEIVE].len = 3;
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[0].title = _("receive_btc_more_t_title1");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[0].desc = _("receive_btc_more_t_desc1");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[0].link = _("learn_more");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[0].qrTitle = _("receive_btc_more_t_title1");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[0].qrSubtitle = _("receive_more_t_qr_link");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[0].qrCode = _("receive_more_t_qr_link");

    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[1].title = _("receive_btc_more_t_title2");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[1].desc = _("receive_btc_more_t_desc2");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[1].link = _("learn_more");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[1].qrTitle = _("receive_btc_more_t_title2");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[1].qrSubtitle = _("receive_more_t_qr_link");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[1].qrCode = _("receive_more_t_qr_link");

    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[2].title = _("receive_btc_more_t_title3");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[2].desc = _("receive_btc_more_t_desc3");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[2].link = _("learn_more");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[2].qrTitle = _("receive_btc_more_t_title3");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[2].qrSubtitle = _("receive_more_t_qr_link");
    g_tutorials[TUTORIAL_BTC_RECEIVE].tutorials[2].qrCode = _("receive_more_t_qr_link");

    // TUTORIAL_ETH_RECEIVE
    g_tutorials[TUTORIAL_ETH_RECEIVE].len = 1;
    g_tutorials[TUTORIAL_ETH_RECEIVE].tutorials[0].title = _("receive_eth_more_t_title1");
    g_tutorials[TUTORIAL_ETH_RECEIVE].tutorials[0].desc = _("receive_eth_more_t_desc1");
    g_tutorials[TUTORIAL_ETH_RECEIVE].tutorials[0].link = _("learn_more");
    g_tutorials[TUTORIAL_ETH_RECEIVE].tutorials[0].qrTitle = _("receive_eth_more_t_title1");
    g_tutorials[TUTORIAL_ETH_RECEIVE].tutorials[0].qrSubtitle = _("receive_more_t_qr_link");
    g_tutorials[TUTORIAL_ETH_RECEIVE].tutorials[0].qrCode = _("receive_more_t_qr_link");

    // TUTORIAL_ETH_RECEIVE
    g_tutorials[TUTORIAL_ADA_RECEIVE].len = 2;
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[0].title = _("receive_ada_more_t_title1");
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[0].desc = _("receive_ada_more_t_desc1");
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[0].link = NULL;
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[0].qrTitle = NULL;
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[0].qrSubtitle = NULL;
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[0].qrCode = NULL;

    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[1].title = _("receive_ada_more_t_title2");
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[1].desc = _("receive_ada_more_t_desc2");
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[1].link = _("learn_more");
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[1].qrTitle = _("receive_ada_more_t_title2");
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[1].qrSubtitle = _("receive_more_t_qr_link");
    g_tutorials[TUTORIAL_ADA_RECEIVE].tutorials[1].qrCode = _("receive_more_t_qr_link");

    // TUTORIAL_SOL_RECEIVE
    g_tutorials[TUTORIAL_SOL_RECEIVE].len = 1;
    g_tutorials[TUTORIAL_SOL_RECEIVE].tutorials[0].title = _("receive_sol_more_t_title1");
    g_tutorials[TUTORIAL_SOL_RECEIVE].tutorials[0].desc = _("receive_sol_more_t_desc1");
    g_tutorials[TUTORIAL_SOL_RECEIVE].tutorials[0].link = _("learn_more");
    g_tutorials[TUTORIAL_SOL_RECEIVE].tutorials[0].qrTitle = _("receive_sol_more_t_title1");
    g_tutorials[TUTORIAL_SOL_RECEIVE].tutorials[0].qrSubtitle = _("receive_more_t_qr_link");
    g_tutorials[TUTORIAL_SOL_RECEIVE].tutorials[0].qrCode = _("receive_more_t_qr_link");

    // TUTORIAL_CHANGE_ENTROPY
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].len = 2;
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[0].title = _("tutorial_change_entropy_title1");
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[0].desc = _("tutorial_change_entropy_desc1");
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[0].link = NULL;
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[0].qrTitle = NULL;
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[0].qrSubtitle = NULL;
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[0].qrCode = NULL;

    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[1].title = _("tutorial_change_entropy_title2");
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[1].desc = _("tutorial_change_entropy_desc2");
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[1].link = NULL;
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[1].qrTitle = NULL;
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[1].qrSubtitle = NULL;
    g_tutorials[TUTORIAL_CHANGE_ENTROPY].tutorials[1].qrCode = NULL;

    //TUTORIAL_TON_MNEMONIC
    g_tutorials[TUTORIAL_TON_MNEMONIC].len = 1;
    g_tutorials[TUTORIAL_TON_MNEMONIC].tutorials[0].title = _("tutorial_ton_mnemonic_title");
    g_tutorials[TUTORIAL_TON_MNEMONIC].tutorials[0].desc = _("tutorial_ton_mnemonic_desc");
    g_tutorials[TUTORIAL_TON_MNEMONIC].tutorials[0].link = _("learn_more");
    g_tutorials[TUTORIAL_TON_MNEMONIC].tutorials[0].qrTitle = _("tutorial_ton_mnemonic_title");
    g_tutorials[TUTORIAL_TON_MNEMONIC].tutorials[0].qrSubtitle = _("tutorial_ton_mnemonic_url");
    g_tutorials[TUTORIAL_TON_MNEMONIC].tutorials[0].qrCode = _("tutorial_ton_mnemonic_url");
}

static void GuiOpenQRHintBox(Tutorial_t *tutorial)
{
    GuiQRCodeHintBoxOpen(tutorial->qrCode, tutorial->qrTitle, tutorial->qrSubtitle);
}

static void GuiOpenQRHintBoxHandler(lv_event_t *e)
{
    Tutorial_t *t = lv_event_get_user_data(e);
    GuiOpenQRHintBox(t);
}

void GuiTutorialInit(TUTORIAL_LIST_INDEX_ENUM tutorialIndex)
{
    TutorialsInit();

    lv_obj_t *cont, *label, *img, *parent;
    cont = NULL;

    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_pageWidget = CreatePageWidget();
    parent = g_pageWidget->contentZone;
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    g_tutorialWidget.cont = parent;
    GuiAddObjFlag(parent, LV_OBJ_FLAG_SCROLLABLE);

    TutorialList_t *tutorialList = &g_tutorials[tutorialIndex];

    for (size_t i = 0; i < tutorialList->len; i++) {
        lv_obj_t *container, *learnMoreCont, *last;
        container = GuiCreateContainerWithParent(parent, lv_obj_get_width(lv_scr_act()) - 36 - 36, 0);
        if (cont == NULL) {
            lv_obj_align(container, LV_ALIGN_DEFAULT, 36, 12);
            cont = container;
        } else {
            lv_obj_align_to(container, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
            cont = container;
        }
        lv_coord_t height = 0, h;
        label = GuiCreateTextLabel(container, tutorialList->tutorials[i].title);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 0, 0);
        lv_obj_update_layout(label);
        h = lv_obj_get_height(label);

        height += h;
        last = label;

        label = GuiCreateIllustrateLabel(container, tutorialList->tutorials[i].desc);
        lv_obj_align_to(label, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_update_layout(label);
        last = label;

        h = lv_obj_get_height(label);
        height += h;

        if (tutorialList->tutorials[i].link != NULL) {
            learnMoreCont = GuiCreateContainerWithParent(container, 300, 30);
            lv_obj_add_flag(learnMoreCont, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(learnMoreCont, GuiOpenQRHintBoxHandler, LV_EVENT_CLICKED, &tutorialList->tutorials[i]);
            lv_obj_align_to(learnMoreCont, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
            last = learnMoreCont;

            h = lv_obj_get_height(learnMoreCont);
            height += h;

            label = GuiCreateIllustrateLabel(learnMoreCont, tutorialList->tutorials[i].link);
            lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_DEFAULT, 0, 0);

            img = GuiCreateImg(learnMoreCont, &imgQrcodeTurquoise);
            lv_obj_align_to(img, label, LV_ALIGN_RIGHT_MID, 30, 0);
        }

        lv_obj_t *line = GuiCreateDividerLine(container);
        lv_obj_align_to(line, last, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
        last = line;

        lv_obj_set_height(container, height + 24 + 24);
    }
}

void GuiTutorialRefresh()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseCurrentViewHandler, NULL);
}

void GuiTutorialDeInit()
{
    GUI_DEL_OBJ(g_tutorialWidget.cont);
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}