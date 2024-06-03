#include "define.h"
#include "gui.h"
#include "lvgl.h"
#include "gui_framework.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_connect_wallet_widgets.h"
#include "gui_wallet_tutorial_widgets.h"
#include "gui_qr_hintbox.h"
#include "gui_page.h"
#if BTC_ONLY
#include "gui_btc_home_widgets.h"
#endif

typedef struct WalletTutorialItem {
    const char *walletName;
    const char *url;
    const char *qrTitle;
    const char *qrUrl;
} WalletTutorialItem_t;

typedef struct WalletTutorial {
    WalletTutorialItem_t items[3];
    uint8_t len;
    const char *desc;
    const char *title;
} WalletTutorial_t;

static WalletTutorial_t g_tutorials[WALLET_LIST_BUTT];

typedef struct GuiWalletTutorialWidget {
    lv_obj_t *cont;
} GuiWalletTurorialWidget_t;

static GuiWalletTurorialWidget_t g_walletTutorialWidget;
static void GuiWalletTutorialQrcodeHandler(lv_event_t *e);
static PageWidget_t *g_pageWidget = NULL;

static void WalletTutorialsInit()
{
#ifndef BTC_ONLY
    // WALLET_LIST_BLUE
    g_tutorials[WALLET_LIST_BLUE].len = 1;
    g_tutorials[WALLET_LIST_BLUE].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_BLUE].items[0].walletName = _("connect_bw_title");
    g_tutorials[WALLET_LIST_BLUE].items[0].url = _("connect_bw_link");
    g_tutorials[WALLET_LIST_BLUE].items[0].qrTitle = _("connect_bw_title");
    g_tutorials[WALLET_LIST_BLUE].items[0].qrUrl = _("connect_bw_link");

    // WALLET_LIST_METAMASK
    g_tutorials[WALLET_LIST_METAMASK].len = 2;
    g_tutorials[WALLET_LIST_METAMASK].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_METAMASK].items[0].walletName = _("connect_mm_title");
    g_tutorials[WALLET_LIST_METAMASK].items[0].url = _("connect_mm_link");
    g_tutorials[WALLET_LIST_METAMASK].items[0].qrTitle = _("connect_mm_title");
    g_tutorials[WALLET_LIST_METAMASK].items[0].qrUrl = _("connect_mm_link");

    g_tutorials[WALLET_LIST_METAMASK].items[1].walletName = _("connect_mm_title2");
    g_tutorials[WALLET_LIST_METAMASK].items[1].url = _("connect_mm_link2");
    g_tutorials[WALLET_LIST_METAMASK].items[1].qrTitle = _("connect_mm_title2");
    g_tutorials[WALLET_LIST_METAMASK].items[1].qrUrl = _("connect_mm_link2");

    // WALLET_LIST_OKX
    g_tutorials[WALLET_LIST_OKX].len = 2;
    g_tutorials[WALLET_LIST_OKX].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_OKX].items[0].walletName = _("connect_okx_title");
    g_tutorials[WALLET_LIST_OKX].items[0].url = _("connect_okx_link");
    g_tutorials[WALLET_LIST_OKX].items[0].qrTitle = _("connect_okx_title");
    g_tutorials[WALLET_LIST_OKX].items[0].qrUrl = _("connect_okx_link");

    g_tutorials[WALLET_LIST_OKX].items[1].walletName = _("connect_okx_title2");
    g_tutorials[WALLET_LIST_OKX].items[1].url = _("connect_okx_link2");
    g_tutorials[WALLET_LIST_OKX].items[1].qrTitle = _("connect_okx_title2");
    g_tutorials[WALLET_LIST_OKX].items[1].qrUrl = _("connect_okx_link2");

    // WALLET_LIST_SOLFARE
    g_tutorials[WALLET_LIST_SOLFARE].len = 1;
    g_tutorials[WALLET_LIST_SOLFARE].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_SOLFARE].items[0].walletName = _("connect_solflare_title");
    g_tutorials[WALLET_LIST_SOLFARE].items[0].url = _("connect_solflare_link");
    g_tutorials[WALLET_LIST_SOLFARE].items[0].qrTitle = _("connect_solflare_title");
    g_tutorials[WALLET_LIST_SOLFARE].items[0].qrUrl = _("connect_solflare_link");

    // WALLET_LIST_ETERNL
    g_tutorials[WALLET_LIST_ETERNL].len = 1;
    g_tutorials[WALLET_LIST_ETERNL].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_ETERNL].items[0].walletName = _("connect_eternl_title");
    g_tutorials[WALLET_LIST_ETERNL].items[0].url = _("connect_eternl_link");
    g_tutorials[WALLET_LIST_ETERNL].items[0].qrTitle = _("connect_eternl_title");
    g_tutorials[WALLET_LIST_ETERNL].items[0].qrUrl = _("connect_eternl_link");

    // WALLET_LIST_YOROI
    //  g_tutorials[WALLET_LIST_YOROI].len = 1;
    //  g_tutorials[WALLET_LIST_YOROI].desc = _("connect_wallet_desc");
    //  g_tutorials[WALLET_LIST_YOROI].items[0].walletName = _("connect_yoroi_title");
    //  g_tutorials[WALLET_LIST_YOROI].items[0].url = _("connect_yoroi_link");
    //  g_tutorials[WALLET_LIST_YOROI].items[0].qrTitle = _("connect_yoroi_qr_title");
    //  g_tutorials[WALLET_LIST_YOROI].items[0].qrUrl = _("connect_yoroi_qr_link");

    // WALLET_LIST_Typhon
    g_tutorials[WALLET_LIST_TYPHON].len = 1;
    g_tutorials[WALLET_LIST_TYPHON].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_TYPHON].items[0].walletName = _("connect_typhon_title");
    g_tutorials[WALLET_LIST_TYPHON].items[0].url = _("connect_typhon_link");
    g_tutorials[WALLET_LIST_TYPHON].items[0].qrTitle = _("connect_typhon_title");
    g_tutorials[WALLET_LIST_TYPHON].items[0].qrUrl = _("connect_typhon_link");

    // WALLET_LIST_RABBY
    g_tutorials[WALLET_LIST_RABBY].len = 1;
    g_tutorials[WALLET_LIST_RABBY].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_RABBY].items[0].walletName = _("connect_rabby_title");
    g_tutorials[WALLET_LIST_RABBY].items[0].url = _("connect_rabby_link");
    g_tutorials[WALLET_LIST_RABBY].items[0].qrTitle = _("connect_rabby_title");
    g_tutorials[WALLET_LIST_RABBY].items[0].qrUrl = _("connect_rabby_link");

    // WALLET_LIST_BLOCK_WALLET
    g_tutorials[WALLET_LIST_BLOCK_WALLET].len = 1;
    g_tutorials[WALLET_LIST_BLOCK_WALLET].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_BLOCK_WALLET].items[0].walletName = _("connect_block_title");
    g_tutorials[WALLET_LIST_BLOCK_WALLET].items[0].url = _("connect_block_link");
    g_tutorials[WALLET_LIST_BLOCK_WALLET].items[0].qrTitle = _("connect_block_title");
    g_tutorials[WALLET_LIST_BLOCK_WALLET].items[0].qrUrl = _("connect_block_link");

    // WALLET_LIST_SAFE
    g_tutorials[WALLET_LIST_SAFE].len = 2;
    g_tutorials[WALLET_LIST_SAFE].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_SAFE].items[0].walletName = _("connect_safe_title");
    g_tutorials[WALLET_LIST_SAFE].items[0].url = _("connect_safe_link");
    g_tutorials[WALLET_LIST_SAFE].items[0].qrTitle = _("connect_safe_title");
    g_tutorials[WALLET_LIST_SAFE].items[0].qrUrl = _("connect_safe_link");

    g_tutorials[WALLET_LIST_SAFE].items[1].walletName = _("connect_safe_title2");
    g_tutorials[WALLET_LIST_SAFE].items[1].url = _("connect_safe_link2");
    g_tutorials[WALLET_LIST_SAFE].items[1].qrTitle = _("connect_safe_title2");
    g_tutorials[WALLET_LIST_SAFE].items[1].qrUrl = _("connect_safe_link2");

    // WALLET_LIST_ZAPPER
    g_tutorials[WALLET_LIST_ZAPPER].len = 1;
    g_tutorials[WALLET_LIST_ZAPPER].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_ZAPPER].items[0].walletName = _("connect_zapper_title");
    g_tutorials[WALLET_LIST_ZAPPER].items[0].url = _("connect_zapper_link");
    g_tutorials[WALLET_LIST_ZAPPER].items[0].qrTitle = _("connect_zapper_title");
    g_tutorials[WALLET_LIST_ZAPPER].items[0].qrUrl = _("connect_zapper_link");

    // WALLET_LIST_YEARN_FINANCE
    g_tutorials[WALLET_LIST_YEARN_FINANCE].len = 1;
    g_tutorials[WALLET_LIST_YEARN_FINANCE].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_YEARN_FINANCE].items[0].walletName = _("connect_yearn_title");
    g_tutorials[WALLET_LIST_YEARN_FINANCE].items[0].url = _("connect_yearn_link");
    g_tutorials[WALLET_LIST_YEARN_FINANCE].items[0].qrTitle = _("connect_yearn_title");
    g_tutorials[WALLET_LIST_YEARN_FINANCE].items[0].qrUrl = _("connect_yearn_link");

    // WALLET_LIST_SUSHISWAP
    g_tutorials[WALLET_LIST_SUSHISWAP].len = 1;
    g_tutorials[WALLET_LIST_SUSHISWAP].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_SUSHISWAP].items[0].walletName = _("connect_sushi_title");
    g_tutorials[WALLET_LIST_SUSHISWAP].items[0].url = _("connect_sushi_link");
    g_tutorials[WALLET_LIST_SUSHISWAP].items[0].qrTitle = _("connect_sushi_title");
    g_tutorials[WALLET_LIST_SUSHISWAP].items[0].qrUrl = _("connect_sushi_link");

    // WALLET_LIST_IMTOKEN
    g_tutorials[WALLET_LIST_IMTOKEN].len = 1;
    g_tutorials[WALLET_LIST_IMTOKEN].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_IMTOKEN].items[0].walletName = _("connect_imtoken_title");
    g_tutorials[WALLET_LIST_IMTOKEN].items[0].url = _("connect_imtoken_link");
    g_tutorials[WALLET_LIST_IMTOKEN].items[0].qrTitle = _("connect_imtoken_title");
    g_tutorials[WALLET_LIST_IMTOKEN].items[0].qrUrl = _("connect_imtoken_link");

    // WALLET_LIST_SPARROW
    g_tutorials[WALLET_LIST_SPARROW].len = 1;
    g_tutorials[WALLET_LIST_SPARROW].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_SPARROW].items[0].walletName = _("connect_sparrow_title");
    g_tutorials[WALLET_LIST_SPARROW].items[0].url = _("connect_sparrow_link");
    g_tutorials[WALLET_LIST_SPARROW].items[0].qrTitle = _("connect_sparrow_title");
    g_tutorials[WALLET_LIST_SPARROW].items[0].qrUrl = _("connect_sparrow_link");

    // WALLET_LIST_UNISAT
    g_tutorials[WALLET_LIST_UNISAT].len = 1;
    g_tutorials[WALLET_LIST_UNISAT].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_UNISAT].items[0].walletName = _("connect_unisat_title");
    g_tutorials[WALLET_LIST_UNISAT].items[0].url = _("connect_unisat_link");
    g_tutorials[WALLET_LIST_UNISAT].items[0].qrTitle = _("connect_unisat_title");
    g_tutorials[WALLET_LIST_UNISAT].items[0].qrUrl = _("connect_unisat_link");

    // WALLET_LIST_KEPLR
    g_tutorials[WALLET_LIST_KEPLR].len = 1;
    g_tutorials[WALLET_LIST_KEPLR].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_KEPLR].items[0].walletName = _("connect_keplr_title");
    g_tutorials[WALLET_LIST_KEPLR].items[0].url = _("connect_keplr_link");
    g_tutorials[WALLET_LIST_KEPLR].items[0].qrTitle = _("connect_keplr_title");
    g_tutorials[WALLET_LIST_KEPLR].items[0].qrUrl = _("connect_keplr_link");

    //WALLET_LIST_ARCONNECT
    g_tutorials[WALLET_LIST_ARCONNECT].len = 1;
    g_tutorials[WALLET_LIST_ARCONNECT].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_ARCONNECT].items[0].walletName = _("connect_arconnect_title");
    g_tutorials[WALLET_LIST_ARCONNECT].items[0].url = _("connect_arconnect_link");
    g_tutorials[WALLET_LIST_ARCONNECT].items[0].qrTitle = _("connect_arconnect_title");
    g_tutorials[WALLET_LIST_ARCONNECT].items[0].qrUrl = _("connect_arconnect_link");

    //WALLET_LIST_FEWCHA
    g_tutorials[WALLET_LIST_FEWCHA].len = 1;
    g_tutorials[WALLET_LIST_FEWCHA].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_FEWCHA].items[0].walletName = _("connect_fewcha_title");
    g_tutorials[WALLET_LIST_FEWCHA].items[0].url = _("connect_fewcha_link");
    g_tutorials[WALLET_LIST_FEWCHA].items[0].qrTitle = _("connect_fewcha_title");
    g_tutorials[WALLET_LIST_FEWCHA].items[0].qrUrl = _("connect_fewcha_link");

    // WALLET_LIST_PETRA
    g_tutorials[WALLET_LIST_PETRA].len = 1;
    g_tutorials[WALLET_LIST_PETRA].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_PETRA].items[0].walletName = _("connect_petra_title");
    g_tutorials[WALLET_LIST_PETRA].items[0].url = _("connect_petra_link");
    g_tutorials[WALLET_LIST_PETRA].items[0].qrTitle = _("connect_petra_title");
    g_tutorials[WALLET_LIST_PETRA].items[0].qrUrl = _("connect_petra_link");

    // WALLET_LIST_XRP_TOOLKIT
    g_tutorials[WALLET_LIST_XRP_TOOLKIT].len = 1;
    g_tutorials[WALLET_LIST_XRP_TOOLKIT].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_XRP_TOOLKIT].items[0].walletName = _("connect_xrp_toolkit_title");
    g_tutorials[WALLET_LIST_XRP_TOOLKIT].items[0].url = _("connect_xrp_toolkit_link");
    g_tutorials[WALLET_LIST_XRP_TOOLKIT].items[0].qrTitle = _("connect_xrp_toolkit_title");
    g_tutorials[WALLET_LIST_XRP_TOOLKIT].items[0].qrUrl = _("connect_xrp_toolkit_link");

    //WALLET_LIST_TONKEEPER
    g_tutorials[WALLET_LIST_TONKEEPER].len = 1;
    g_tutorials[WALLET_LIST_TONKEEPER].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_TONKEEPER].items[0].walletName = _("connect_tonkeeper_title");
    g_tutorials[WALLET_LIST_TONKEEPER].items[0].url = _("connect_tonkeeper_link");
    g_tutorials[WALLET_LIST_TONKEEPER].items[0].qrTitle = _("connect_tonkeeper_title");
    g_tutorials[WALLET_LIST_TONKEEPER].items[0].qrUrl = _("connect_tonkeeper_link");
#else
    g_tutorials[WALLET_LIST_BLUE].len = 1;
    g_tutorials[WALLET_LIST_BLUE].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_BLUE].items[0].walletName = _("connect_bw_title");
    g_tutorials[WALLET_LIST_BLUE].items[0].url = _("connect_bw_link");
    g_tutorials[WALLET_LIST_BLUE].items[0].qrTitle = _("connect_bw_title");
    g_tutorials[WALLET_LIST_BLUE].items[0].qrUrl = _("connect_bw_link");

    g_tutorials[WALLET_LIST_SPECTER].len = 1;
    g_tutorials[WALLET_LIST_SPECTER].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_SPECTER].items[0].walletName = _("connect_specter_title");
    g_tutorials[WALLET_LIST_SPECTER].items[0].url = _("connect_specter_link");
    g_tutorials[WALLET_LIST_SPECTER].items[0].qrTitle = _("connect_specter_title");
    g_tutorials[WALLET_LIST_SPECTER].items[0].qrUrl = _("connect_specter_link");

    g_tutorials[WALLET_LIST_SPARROW].len = 1;
    g_tutorials[WALLET_LIST_SPARROW].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_SPARROW].items[0].walletName = _("connect_sparrow_title");
    g_tutorials[WALLET_LIST_SPARROW].items[0].url = _("connect_sparrow_link");
    g_tutorials[WALLET_LIST_SPARROW].items[0].qrTitle = _("connect_sparrow_title");
    g_tutorials[WALLET_LIST_SPARROW].items[0].qrUrl = _("connect_sparrow_link");

    g_tutorials[WALLET_LIST_UNISAT].len = 1;
    g_tutorials[WALLET_LIST_UNISAT].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_UNISAT].items[0].walletName = _("connect_unisat_title");
    g_tutorials[WALLET_LIST_UNISAT].items[0].url = _("connect_unisat_link");
    g_tutorials[WALLET_LIST_UNISAT].items[0].qrTitle = _("connect_unisat_qr_title");
    g_tutorials[WALLET_LIST_UNISAT].items[0].qrUrl = _("connect_unisat_qr_link");

    g_tutorials[WALLET_LIST_NUNCHUK].len = 1;
    g_tutorials[WALLET_LIST_NUNCHUK].desc = _("connect_wallet_desc");
    g_tutorials[WALLET_LIST_NUNCHUK].items[0].walletName = _("connect_nunchuk_title");
    g_tutorials[WALLET_LIST_NUNCHUK].items[0].url = _("connect_nunchuk_link");
    g_tutorials[WALLET_LIST_NUNCHUK].items[0].qrTitle = _("connect_nunchuk_title");
    g_tutorials[WALLET_LIST_NUNCHUK].items[0].qrUrl = _("connect_nunchuk_link");
#endif
#if BTC_ONLY
    // multisig
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        g_tutorials[WALLET_LIST_BLUE].items[0].url = _("connect_bw_multisig_link");
        g_tutorials[WALLET_LIST_BLUE].items[0].qrUrl = _("connect_bw_multisig_link");

        g_tutorials[WALLET_LIST_SPARROW].items[0].url = _("connect_sparrow_multisig_link");
        g_tutorials[WALLET_LIST_SPARROW].items[0].qrUrl = _("connect_sparrow_multisig_link");

        g_tutorials[WALLET_LIST_NUNCHUK].items[0].url = _("connect_nunchuk_multisig_link");
        g_tutorials[WALLET_LIST_NUNCHUK].items[0].qrUrl = _("connect_nunchuk_multisig_link");
    } else {
        g_tutorials[WALLET_LIST_BLUE].items[0].url = _("connect_bw_link");
        g_tutorials[WALLET_LIST_BLUE].items[0].qrUrl = _("connect_bw_link");

        g_tutorials[WALLET_LIST_SPARROW].items[0].url = _("connect_sparrow_link");
        g_tutorials[WALLET_LIST_SPARROW].items[0].qrUrl = _("connect_sparrow_link");

        g_tutorials[WALLET_LIST_NUNCHUK].items[0].url = _("connect_nunchuk_link");
        g_tutorials[WALLET_LIST_NUNCHUK].items[0].qrUrl = _("connect_nunchuk_link");
    }
#endif
}

void GuiWalletTutorialInit(WALLET_LIST_INDEX_ENUM tutorialIndex)
{
    WalletTutorialsInit();

    printf("index: %d\r\n", tutorialIndex);

    WalletTutorial_t *tutorial = &g_tutorials[tutorialIndex];

    lv_obj_t *cont, *label, *img, *lastTarget;
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_pageWidget = CreatePageWidget();
    cont = g_pageWidget->contentZone;
    g_walletTutorialWidget.cont = cont;
    label = GuiCreateIllustrateLabel(cont, tutorial->desc);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 12);
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN | LV_STATE_DEFAULT);

    lastTarget = label;

    for (size_t i = 0; i < tutorial->len; i++) {
        cont = GuiCreateContainerWithParent(g_walletTutorialWidget.cont, 408, 102);
        lv_obj_align_to(cont, lastTarget, LV_ALIGN_OUT_BOTTOM_LEFT, 0, i == 0 ? 24 : 16);
        lv_obj_set_style_bg_color(cont, DARK_GRAY_COLOR, LV_PART_MAIN);
        lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
        lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(cont, GuiWalletTutorialQrcodeHandler, LV_EVENT_CLICKED, &tutorial->items[i]);

        label = GuiCreateTextLabel(cont, tutorial->items[i].walletName);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
        lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(cont, tutorial->items[i].url);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 56);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_width(label, 320);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

        img = GuiCreateImg(cont, &imgQrcodeTurquoise);
        lv_obj_align(img, LV_ALIGN_DEFAULT, 348, 33);
        lastTarget = cont;

        lv_obj_update_layout(label);
        lv_coord_t height = lv_obj_get_height(label) - 30;

        if (height > 0) {
            lv_obj_set_height(cont, 102 + height);
            lv_obj_align(img, LV_ALIGN_DEFAULT, 348, 33 + height / 2);
            lv_obj_update_layout(cont);
        }
    }
}

void GuiWalletTutorialRefresh()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Tutorial"));
}

void GuiWalletTutorialDeInit()
{
    GUI_DEL_OBJ(g_walletTutorialWidget.cont);
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

static void showQRHintBox(WalletTutorialItem_t *item)
{
    GuiQRCodeHintBoxOpen(item->qrUrl, item->qrTitle, item->qrUrl);
}

static void GuiWalletTutorialQrcodeHandler(lv_event_t *e)
{
    showQRHintBox(lv_event_get_user_data(e));
}
