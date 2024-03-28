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
static bool g_tutorialsInitialized = false;
static PageWidget_t *g_pageWidget = NULL;

static void WalletTutorialsInit()
{
    if (!g_tutorialsInitialized) {
        g_tutorialsInitialized = true;
#ifndef BTC_ONLY
        //WALLET_LIST_BLUE
        g_tutorials[WALLET_LIST_BLUE].len = 1;
        g_tutorials[WALLET_LIST_BLUE].desc = _("connect_bw_desc");
        g_tutorials[WALLET_LIST_BLUE].items[0].walletName = _("connect_bw_title");
        g_tutorials[WALLET_LIST_BLUE].items[0].url = _("connect_bw_link");
        g_tutorials[WALLET_LIST_BLUE].items[0].qrTitle = _("connect_bw_qr_title");
        g_tutorials[WALLET_LIST_BLUE].items[0].qrUrl = _("connect_bw_qr_link");

        //WALLET_LIST_METAMASK
        g_tutorials[WALLET_LIST_METAMASK].len = 2;
        g_tutorials[WALLET_LIST_METAMASK].desc = _("connect_mm_desc");
        g_tutorials[WALLET_LIST_METAMASK].items[0].walletName = _("connect_mm_title");
        g_tutorials[WALLET_LIST_METAMASK].items[0].url = _("connect_mm_link");
        g_tutorials[WALLET_LIST_METAMASK].items[0].qrTitle = _("connect_mm_qr_title");
        g_tutorials[WALLET_LIST_METAMASK].items[0].qrUrl = _("connect_mm_qr_link");

        g_tutorials[WALLET_LIST_METAMASK].items[1].walletName = _("connect_mm_title2");
        g_tutorials[WALLET_LIST_METAMASK].items[1].url = _("connect_mm_link2");
        g_tutorials[WALLET_LIST_METAMASK].items[1].qrTitle = _("connect_mm_qr_title2");
        g_tutorials[WALLET_LIST_METAMASK].items[1].qrUrl = _("connect_mm_qr_link2");

        //WALLET_LIST_OKX
        g_tutorials[WALLET_LIST_OKX].len = 2;
        g_tutorials[WALLET_LIST_OKX].desc = _("connect_okx_desc");
        g_tutorials[WALLET_LIST_OKX].items[0].walletName = _("connect_okx_title");
        g_tutorials[WALLET_LIST_OKX].items[0].url = _("connect_okx_link");
        g_tutorials[WALLET_LIST_OKX].items[0].qrTitle = _("connect_okx_qr_title");
        g_tutorials[WALLET_LIST_OKX].items[0].qrUrl = _("connect_okx_qr_link");

        g_tutorials[WALLET_LIST_OKX].items[1].walletName = _("connect_okx_title2");
        g_tutorials[WALLET_LIST_OKX].items[1].url = _("connect_okx_link2");
        g_tutorials[WALLET_LIST_OKX].items[1].qrTitle = _("connect_okx_qr_title2");
        g_tutorials[WALLET_LIST_OKX].items[1].qrUrl = _("connect_okx_qr_link2");

        //WALLET_LIST_KEYSTONE
        g_tutorials[WALLET_LIST_KEYSTONE].len = 1;
        g_tutorials[WALLET_LIST_KEYSTONE].desc = _("connect_keyst_app_desc");
        g_tutorials[WALLET_LIST_KEYSTONE].items[0].walletName = _("connect_keyst_app_title");
        g_tutorials[WALLET_LIST_KEYSTONE].items[0].url = _("connect_keyst_app_link");
        g_tutorials[WALLET_LIST_KEYSTONE].items[0].qrTitle = _("connect_keyst_app_qr_title");
        g_tutorials[WALLET_LIST_KEYSTONE].items[0].qrUrl = _("connect_keyst_app_qr_link");

        //WALLET_LIST_SOLFARE
        g_tutorials[WALLET_LIST_SOLFARE].len = 1;
        g_tutorials[WALLET_LIST_SOLFARE].desc = _("connect_solflare_desc");
        g_tutorials[WALLET_LIST_SOLFARE].items[0].walletName = _("connect_solflare_qr_title");
        g_tutorials[WALLET_LIST_SOLFARE].items[0].url = _("connect_solflare_link");
        g_tutorials[WALLET_LIST_SOLFARE].items[0].qrTitle = _("connect_solflare_qr_title");
        g_tutorials[WALLET_LIST_SOLFARE].items[0].qrUrl = _("connect_solflare_qr_link");

        //WALLET_LIST_ETERNL
        g_tutorials[WALLET_LIST_ETERNL].len = 1;
        g_tutorials[WALLET_LIST_ETERNL].desc = _("connect_eternl_desc");
        g_tutorials[WALLET_LIST_ETERNL].items[0].walletName = _("connect_eternl_title");
        g_tutorials[WALLET_LIST_ETERNL].items[0].url = _("connect_eternl_link");
        g_tutorials[WALLET_LIST_ETERNL].items[0].qrTitle = _("connect_eternl_qr_title");
        g_tutorials[WALLET_LIST_ETERNL].items[0].qrUrl = _("connect_eternl_qr_link");

        //WALLET_LIST_RABBY
        g_tutorials[WALLET_LIST_RABBY].len = 1;
        g_tutorials[WALLET_LIST_RABBY].desc = _("connect_rabby_desc");
        g_tutorials[WALLET_LIST_RABBY].items[0].walletName = _("connect_rabby_title");
        g_tutorials[WALLET_LIST_RABBY].items[0].url = _("connect_rabby_link");
        g_tutorials[WALLET_LIST_RABBY].items[0].qrTitle = _("connect_rabby_qr_title");
        g_tutorials[WALLET_LIST_RABBY].items[0].qrUrl = _("connect_rabby_qr_link");

        //WALLET_LIST_BLOCK_WALLET
        g_tutorials[WALLET_LIST_BLOCK_WALLET].len = 1;
        g_tutorials[WALLET_LIST_BLOCK_WALLET].desc = _("connect_block_desc");
        g_tutorials[WALLET_LIST_BLOCK_WALLET].items[0].walletName = _("connect_block_title");
        g_tutorials[WALLET_LIST_BLOCK_WALLET].items[0].url = _("connect_block_link");
        g_tutorials[WALLET_LIST_BLOCK_WALLET].items[0].qrTitle = _("connect_block_qr_title");
        g_tutorials[WALLET_LIST_BLOCK_WALLET].items[0].qrUrl = _("connect_block_qr_link");

        //WALLET_LIST_SAFE
        g_tutorials[WALLET_LIST_SAFE].len = 2;
        g_tutorials[WALLET_LIST_SAFE].desc = _("connect_safe_desc");
        g_tutorials[WALLET_LIST_SAFE].items[0].walletName = _("connect_safe_title");
        g_tutorials[WALLET_LIST_SAFE].items[0].url = _("connect_safe_link");
        g_tutorials[WALLET_LIST_SAFE].items[0].qrTitle = _("connect_safe_qr_title");
        g_tutorials[WALLET_LIST_SAFE].items[0].qrUrl = _("connect_safe_qr_link");

        g_tutorials[WALLET_LIST_SAFE].items[1].walletName = _("connect_safe_title2");
        g_tutorials[WALLET_LIST_SAFE].items[1].url = _("connect_safe_link2");
        g_tutorials[WALLET_LIST_SAFE].items[1].qrTitle = _("connect_safe_qr_title2");
        g_tutorials[WALLET_LIST_SAFE].items[1].qrUrl = _("connect_safe_qr_link2");

        //WALLET_LIST_ZAPPER
        g_tutorials[WALLET_LIST_ZAPPER].len = 1;
        g_tutorials[WALLET_LIST_ZAPPER].desc = _("connect_zapper_desc");
        g_tutorials[WALLET_LIST_ZAPPER].items[0].walletName = _("connect_zapper_title");
        g_tutorials[WALLET_LIST_ZAPPER].items[0].url = _("connect_zapper_link");
        g_tutorials[WALLET_LIST_ZAPPER].items[0].qrTitle = _("connect_zapper_qr_title");
        g_tutorials[WALLET_LIST_ZAPPER].items[0].qrUrl = _("connect_zapper_qr_link");

        //WALLET_LIST_YEARN_FINANCE
        g_tutorials[WALLET_LIST_YEARN_FINANCE].len = 1;
        g_tutorials[WALLET_LIST_YEARN_FINANCE].desc = _("connect_yearn_desc");
        g_tutorials[WALLET_LIST_YEARN_FINANCE].items[0].walletName = _("connect_yearn_title");
        g_tutorials[WALLET_LIST_YEARN_FINANCE].items[0].url = _("connect_yearn_link");
        g_tutorials[WALLET_LIST_YEARN_FINANCE].items[0].qrTitle = _("connect_yearn_qr_title");
        g_tutorials[WALLET_LIST_YEARN_FINANCE].items[0].qrUrl = _("connect_yearn_qr_link");

        //WALLET_LIST_SUSHISWAP
        g_tutorials[WALLET_LIST_SUSHISWAP].len = 1;
        g_tutorials[WALLET_LIST_SUSHISWAP].desc = _("connect_sushi_desc");
        g_tutorials[WALLET_LIST_SUSHISWAP].items[0].walletName = _("connect_sushi_title");
        g_tutorials[WALLET_LIST_SUSHISWAP].items[0].url = _("connect_sushi_link");
        g_tutorials[WALLET_LIST_SUSHISWAP].items[0].qrTitle = _("connect_sushi_qr_title");
        g_tutorials[WALLET_LIST_SUSHISWAP].items[0].qrUrl = _("connect_sushi_qr_link");

        //WALLET_LIST_IMTOKEN
        g_tutorials[WALLET_LIST_IMTOKEN].len = 1;
        g_tutorials[WALLET_LIST_IMTOKEN].desc = _("connect_imtoken_desc");
        g_tutorials[WALLET_LIST_IMTOKEN].items[0].walletName = _("connect_imtoken_title");
        g_tutorials[WALLET_LIST_IMTOKEN].items[0].url = _("connect_imtoken_link");
        g_tutorials[WALLET_LIST_IMTOKEN].items[0].qrTitle = _("connect_imtoken_qr_title");
        g_tutorials[WALLET_LIST_IMTOKEN].items[0].qrUrl = _("connect_imtoken_qr_link");

        //WALLET_LIST_SPARROW
        g_tutorials[WALLET_LIST_SPARROW].len = 1;
        g_tutorials[WALLET_LIST_SPARROW].desc = _("connect_sparrow_desc");
        g_tutorials[WALLET_LIST_SPARROW].items[0].walletName = _("connect_sparrow_title");
        g_tutorials[WALLET_LIST_SPARROW].items[0].url = _("connect_sparrow_link");
        g_tutorials[WALLET_LIST_SPARROW].items[0].qrTitle = _("connect_sparrow_qr_title");
        g_tutorials[WALLET_LIST_SPARROW].items[0].qrUrl = _("connect_sparrow_qr_link");

        //WALLET_LIST_UNISAT
        g_tutorials[WALLET_LIST_UNISAT].len = 1;
        g_tutorials[WALLET_LIST_UNISAT].desc = _("connect_unisat_desc");
        g_tutorials[WALLET_LIST_UNISAT].items[0].walletName = _("connect_unisat_title");
        g_tutorials[WALLET_LIST_UNISAT].items[0].url = _("connect_unisat_link");
        g_tutorials[WALLET_LIST_UNISAT].items[0].qrTitle = _("connect_unisat_qr_title");
        g_tutorials[WALLET_LIST_UNISAT].items[0].qrUrl = _("connect_unisat_qr_link");

        //WALLET_LIST_KEPLR
        g_tutorials[WALLET_LIST_KEPLR].len = 1;
        g_tutorials[WALLET_LIST_KEPLR].desc = _("connect_keplr_desc");
        g_tutorials[WALLET_LIST_KEPLR].items[0].walletName = _("connect_keplr_title");
        g_tutorials[WALLET_LIST_KEPLR].items[0].url = _("connect_keplr_link");
        g_tutorials[WALLET_LIST_KEPLR].items[0].qrTitle = _("connect_keplr_qr_title");
        g_tutorials[WALLET_LIST_KEPLR].items[0].qrUrl = _("connect_keplr_qr_link");

        //WALLET_LIST_FEWCHA
        g_tutorials[WALLET_LIST_FEWCHA].len = 1;
        g_tutorials[WALLET_LIST_FEWCHA].desc = _("connect_fewcha_desc");
        g_tutorials[WALLET_LIST_FEWCHA].items[0].walletName = _("connect_fewcha_title");
        g_tutorials[WALLET_LIST_FEWCHA].items[0].url = _("connect_fewcha_link");
        g_tutorials[WALLET_LIST_FEWCHA].items[0].qrTitle = _("connect_fewcha_qr_title");
        g_tutorials[WALLET_LIST_FEWCHA].items[0].qrUrl = _("connect_fewcha_qr_link");

        //WALLET_LIST_PETRA
        g_tutorials[WALLET_LIST_PETRA].len = 1;
        g_tutorials[WALLET_LIST_PETRA].desc = _("connect_petra_desc");
        g_tutorials[WALLET_LIST_PETRA].items[0].walletName = _("connect_petra_title");
        g_tutorials[WALLET_LIST_PETRA].items[0].url = _("connect_petra_link");
        g_tutorials[WALLET_LIST_PETRA].items[0].qrTitle = _("connect_petra_qr_title");
        g_tutorials[WALLET_LIST_PETRA].items[0].qrUrl = _("connect_petra_qr_link");

        //WALLET_LIST_XRP_TOOLKIT
        g_tutorials[WALLET_LIST_XRP_TOOLKIT].len = 1;
        g_tutorials[WALLET_LIST_XRP_TOOLKIT].desc = _("connect_xrp_toolkit_desc");
        g_tutorials[WALLET_LIST_XRP_TOOLKIT].items[0].walletName = _("connect_xrp_toolkit_title");
        g_tutorials[WALLET_LIST_XRP_TOOLKIT].items[0].url = _("connect_xrp_toolkit_link");
        g_tutorials[WALLET_LIST_XRP_TOOLKIT].items[0].qrTitle = _("connect_xrp_toolkit_qr_title");
        g_tutorials[WALLET_LIST_XRP_TOOLKIT].items[0].qrUrl = _("connect_xrp_toolkit_qr_link");
#else
        g_tutorials[WALLET_LIST_BLUE].len = 1;
        g_tutorials[WALLET_LIST_BLUE].desc = _("connect_bw_desc");
        g_tutorials[WALLET_LIST_BLUE].items[0].walletName = _("connect_bw_title");
        g_tutorials[WALLET_LIST_BLUE].items[0].url = _("connect_bw_link");
        g_tutorials[WALLET_LIST_BLUE].items[0].qrTitle = _("connect_bw_qr_title");
        g_tutorials[WALLET_LIST_BLUE].items[0].qrUrl = _("connect_bw_qr_link");

        g_tutorials[WALLET_LIST_SPECTER].len = 1;
        g_tutorials[WALLET_LIST_SPECTER].desc = _("connect_specter_desc");
        g_tutorials[WALLET_LIST_SPECTER].items[0].walletName = _("connect_specter_title");
        g_tutorials[WALLET_LIST_SPECTER].items[0].url = _("connect_specter_link");
        g_tutorials[WALLET_LIST_SPECTER].items[0].qrTitle = _("connect_specter_qr_title");
        g_tutorials[WALLET_LIST_SPECTER].items[0].qrUrl = _("connect_specter_qr_link");

        g_tutorials[WALLET_LIST_SPARROW].len = 1;
        g_tutorials[WALLET_LIST_SPARROW].desc = _("connect_sparrow_desc");
        g_tutorials[WALLET_LIST_SPARROW].items[0].walletName = _("connect_sparrow_title");
        g_tutorials[WALLET_LIST_SPARROW].items[0].url = _("connect_sparrow_link");
        g_tutorials[WALLET_LIST_SPARROW].items[0].qrTitle = _("connect_sparrow_qr_title");
        g_tutorials[WALLET_LIST_SPARROW].items[0].qrUrl = _("connect_sparrow_qr_link");

        g_tutorials[WALLET_LIST_UNISAT].len = 1;
        g_tutorials[WALLET_LIST_UNISAT].desc = _("connect_unisat_desc");
        g_tutorials[WALLET_LIST_UNISAT].items[0].walletName = _("connect_unisat_title");
        g_tutorials[WALLET_LIST_UNISAT].items[0].url = _("connect_unisat_link");
        g_tutorials[WALLET_LIST_UNISAT].items[0].qrTitle = _("connect_unisat_qr_title");
        g_tutorials[WALLET_LIST_UNISAT].items[0].qrUrl = _("connect_unisat_qr_link");

        g_tutorials[WALLET_LIST_NUNCHUK].len = 1;
        g_tutorials[WALLET_LIST_NUNCHUK].desc = _("connect_nunchuk_desc");
        g_tutorials[WALLET_LIST_NUNCHUK].items[0].walletName = _("connect_nunchuk_title");
        g_tutorials[WALLET_LIST_NUNCHUK].items[0].url = _("connect_nunchuk_link");
        g_tutorials[WALLET_LIST_NUNCHUK].items[0].qrTitle = _("connect_nunchuk_qr_title");
        g_tutorials[WALLET_LIST_NUNCHUK].items[0].qrUrl = _("connect_nunchuk_qr_link");
#endif
    }
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

        img = GuiCreateImg(cont, &imgQrcode36px);
        lv_obj_align(img, LV_ALIGN_DEFAULT, 348, 33);
        lastTarget = cont;
    }
}

void GuiWalletTutorialRefresh()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("connect_block_t"));
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
    lv_event_code_t code = lv_event_get_code(e);
    WalletTutorialItem_t *item = (WalletTutorialItem_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        showQRHintBox(item);
    }
}
