#ifdef BTC_ONLY
#include "gui_btc_wallet_profile_widgets.h"
#include "gui.h"
#include "gui_views.h"
#include "gui_model.h"
#include "gui_status_bar.h"
#include "gui_page.h"

static PageWidget_t *g_pageWidget;

void GuiBtcWalletProfileInit(void)
{
    printf("%s\n", __func__);
    g_pageWidget = CreatePageWidget();
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "Wallet Profile");
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
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
}


int8_t GuiBtcWalletProfileNextTile(uint8_t tileIndex)
{
    printf("%s\n", __func__);
}


static void CreateBtcWalletProfileEntranceWidget(lv_obj_t *parent)
{
    //GuiButton_t table[4] = {
    //    {
    //        .obj = img,
    //        .align = LV_ALIGN_DEFAULT,
    //        .position = {24, 24},
    //    },
    //    {
    //        .obj = label,
    //        .align = LV_ALIGN_DEFAULT,
    //        .position = {76, 24},
    //    },
    //    {
    //        .obj = labelDesc,
    //        .align = LV_ALIGN_DEFAULT,
    //        .position = {76, 64},
    //    },
    //    {
    //        .obj = imgArrow,
    //        .align = LV_ALIGN_DEFAULT,
    //        .position = {396, 24},
    //    },
    //};
}

#endif
