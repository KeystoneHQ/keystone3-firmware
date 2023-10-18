#include "gui_key_derivation_request_widgets.h"
#include "gui.h"
#include "gui_page.h"

typedef struct KeyDerivationWidget
{
    uint8_t currentTile;
    PageWidget_t *pageWidget;
    lv_obj_t *cont;
} KeyDerivationWidget_t;

static void *g_data;
static void *g_urResult;
static bool g_isMulti;
static KeyDerivationWidget_t g_keyDerivationTileView;

static void GuiCreateContent(lv_obj_t *parent);

void GuiSetKeyDerivationRequestData(void *data, bool is_multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = data;
    g_isMulti = is_multi;
    g_data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
#endif
}

void GuiKeyDerivationRequestInit()
{
    GUI_PAGE_DEL(g_keyDerivationTileView.pageWidget);
    GUI_DEL_OBJ(g_keyDerivationTileView.cont);
    g_keyDerivationTileView.pageWidget = CreatePageWidget();
    g_keyDerivationTileView.cont = g_keyDerivationTileView.pageWidget->contentZone;
    GuiCreateContent(g_keyDerivationTileView.cont);
}
void GuiKeyDerivationRequestDeInit()
{
    GUI_PAGE_DEL(g_keyDerivationTileView.pageWidget);
    GUI_DEL_OBJ(g_keyDerivationTileView.cont);
}
void GuiKeyDerivationRequestRefresh()
{
}
void GuiKeyDerivationRequestNextTile()
{
}
void GuiKeyDerivationRequestPrevTile()
{
}

static void GuiCreateContent(lv_obj_t *parent)
{
}