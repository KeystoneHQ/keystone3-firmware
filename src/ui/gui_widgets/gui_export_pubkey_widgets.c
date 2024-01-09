#include "gui_export_pubkey_widgets.h"
#include "gui_home_widgets.h"
#include "gui_page.h"
#include "gui_fullscreen_mode.h"

typedef enum
{
    TILEVIEW_QRCODE = 0,
    TILEVIEW_PUBKEY,
} TILEVIEW_INDEX_ENUM;

typedef struct
{
    lv_obj_t *cont;
    lv_obj_t *tileview;
    lv_obj_t *qrTileview;
    lv_obj_t *qrCont;
    lv_obj_t *qrCode;
    lv_obj_t *title;
    lv_obj_t *desc;
    lv_obj_t *xpub;
} ExportPubkeyWidgets_t;

static HOME_WALLET_CARD_ENUM g_chain;
static PageWidget_t *g_pageWidget;
static ExportPubkeyWidgets_t g_widgets;
static TILEVIEW_INDEX_ENUM g_tileviewIndex;

static void GuiCreateQrCodeWidget(lv_obj_t *parent);

static GuiRefreshTileview()
{
    switch (g_tileviewIndex)
    {
    case TILEVIEW_QRCODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Extended Public Key"));
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    
    default:
        break;
    }
}

static GuiGotoTileview(TILEVIEW_INDEX_ENUM index)
{
    g_tileviewIndex = index;
    lv_obj_set_tile_id(g_widgets.tileview, g_tileviewIndex, 0, LV_ANIM_OFF);
    GuiRefreshTileview();
}

void GuiExportPubkeyInit(uint8_t chain)
{
    g_chain = chain;
    g_pageWidget = CreatePageWidget();
    g_widgets.cont = g_pageWidget->contentZone;

    g_widgets.tileview = lv_tileview_create(g_widgets.cont);
    lv_obj_set_style_bg_opa(g_widgets.tileview, LV_OPA_0, LV_PART_SCROLLBAR & LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(g_widgets.tileview, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    g_widgets.qrTileview = lv_tileview_add_tile(g_widgets.tileview, TILEVIEW_QRCODE, 0, LV_DIR_HOR);
    GuiCreateQrCodeWidget(g_widgets.qrTileview);

    GuiGotoTileview(TILEVIEW_QRCODE);
}

void GuiExportPubkeyDeInit(void)
{
    CLEAR_OBJECT(g_widgets);
    GuiFullscreenModeCleanUp();

    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

lv_obj_t* CreateExportPubkeyQRCode(lv_obj_t* parent, uint16_t w, uint16_t h)
{
    lv_obj_t* qrcode = lv_qrcode_create(parent, w, BLACK_COLOR, WHITE_COLOR);
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    lv_qrcode_update(qrcode, "", 0);
    return qrcode;
}

static void GuiCreateQrCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *tempObj;
    uint16_t yOffset = 0;

    g_widgets.qrCont = GuiCreateContainerWithParent(parent, 408, 552);
    lv_obj_align(g_widgets.qrCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(g_widgets.qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_widgets.qrCont, 24, LV_PART_MAIN);

    yOffset += 36;
    g_widgets.title = GuiCreateNoticeLabel(g_widgets.qrCont, "ABC");
    lv_obj_align(g_widgets.title, LV_ALIGN_TOP_MID, 0, yOffset);

    yOffset += 36;
    g_widgets.desc = GuiCreateNoticeLabel(g_widgets.qrCont, "DEF");
    lv_obj_align(g_widgets.desc, LV_ALIGN_TOP_MID, 0, yOffset);

    yOffset += 36;
    g_widgets.qrCode = CreateExportPubkeyQRCode(g_widgets.qrCont, 336, 336);
    GuiFullscreenModeInit(480, 800, WHITE_COLOR);
    GuiFullscreenModeCreateObject(CreateExportPubkeyQRCode, 420, 420);
    lv_obj_align(g_widgets.qrCode, LV_ALIGN_TOP_MID, 0, yOffset);
    yOffset += 336;

    yOffset += 16;
    g_widgets.xpub = GuiCreateNoticeLabel(g_widgets.qrCont, "xpub");
    lv_obj_set_width(g_widgets.xpub, 336);
    lv_obj_align(g_widgets.xpub, LV_ALIGN_TOP_MID, 0, yOffset);
}
