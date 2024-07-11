#include "gui_ton_mnemonic_hint_widgets.h"
#include "gui_page.h"
#include <gui_create_wallet_widgets.h>

PageWidget_t *g_tonPhraseHintPage;

static void TonNewMnemonicHandler(lv_event_t *e)
{
    uint8_t entropyMethod = WALLET_TYPE_TON;
    GuiFrameOpenViewWithParam(&g_singlePhraseView, &entropyMethod, sizeof(entropyMethod));
}

void GuiTonMnemonicHintWidgetsInit()
{
    g_tonPhraseHintPage = CreatePageWidget();
    SetNavBarLeftBtn(g_tonPhraseHintPage->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    lv_obj_t *parent = g_tonPhraseHintPage->contentZone;

    lv_obj_t *label, *btn, *img, *container;
    container = GuiCreateContainerWithParent(parent, 408, lv_obj_get_height(lv_scr_act()) -
                GUI_MAIN_AREA_OFFSET);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);

    img = GuiCreateImg(container, &imgBlueInformation);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 168, 12);

    label = GuiCreateTextLabel(container, _("create_ton_wallet"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 90);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(container, _("create_ton_wallet_hint"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 144);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    btn = GuiCreateBtn(container, _("Next"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -36);
    if (GetBatterPercent() < 60) {
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(btn, LV_OPA_30, LV_PART_MAIN);
    }

    lv_obj_add_event_cb(btn, TonNewMnemonicHandler, LV_EVENT_CLICKED, NULL);
}
void GuiTonMnemonicHintWidgetsDeInit()
{
    GUI_PAGE_DEL(g_tonPhraseHintPage);
}