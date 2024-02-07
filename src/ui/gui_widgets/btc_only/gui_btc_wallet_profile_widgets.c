#ifdef BTC_ONLY
#include "gui_btc_wallet_profile_widgets.h"

void GuiBtcWalletProfileInit(void)
{
    printf("%s\n", __func__);
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, DEVICE_SETTING, 0, LV_DIR_HOR);
    GuiSettingEntranceWidget(tile);

    g_deviceSetTileView.currentTile = DEVICE_SETTING;
    g_deviceSetTileView.tileView = tileView;
    g_deviceSetTileView.cont = cont;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].tile = tile;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].obj = NULL;
    strcpy(g_deviceSettingArray[g_deviceSetTileView.currentTile].midLabel, _("device_setting_mid_btn"));
    g_deviceSettingArray[g_deviceSetTileView.currentTile].destructCb = NULL;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].structureCb = NULL;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].rightBtn = NVS_RIGHT_BUTTON_BUTT;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].leftBtn = NVS_BAR_RETURN;
    g_deviceSettingArray[g_deviceSetTileView.currentTile].leftCb = ReturnHandler;

    lv_obj_set_tile_id(g_deviceSetTileView.tileView, g_deviceSetTileView.currentTile, 0, LV_ANIM_OFF);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, g_deviceSettingArray[g_deviceSetTileView.currentTile].midLabel);

}


void GuiBtcWalletProfileDeInit(void)
{
    printf("%s\n", __func__);
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

#endif
