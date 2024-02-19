#ifdef BTC_ONLY
#include "gui_btc_wallet_profile_widgets.h"
#include "gui.h"
#include "gui_views.h"
#include "gui_model.h"
#include "gui_status_bar.h"
#include "gui_page.h"
#include "gui_button.h"

static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent);
static void EmptyHandler(lv_event_t *e);

static PageWidget_t *g_pageWidget;

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
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), EmptyHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 109);

    table[0].obj = GuiCreateImg(parent, &imgExport);
    table[1].obj = GuiCreateTextLabel(parent, _("wallet_profile_export_title"));
    table[2].obj = GuiCreateImg(parent, &imgArrowRight);
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), EmptyHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 205);
}


static void EmptyHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("do nothing\n");
    }
}


#endif
