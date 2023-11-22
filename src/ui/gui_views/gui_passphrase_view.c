#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_passphrase_widgets.h"

static int32_t GuiPassphraseViewInit(void)
{
    GuiPassphraseInit();
    return SUCCESS_CODE;
}

static int32_t GuiPassphraseViewDeInit(void)
{
    GuiPassphraseDeInit();
    return SUCCESS_CODE;
}

static int32_t GuiPassphraseViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiPassphraseViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiPassphraseViewDeInit();
    case GUI_EVENT_DISACTIVE:
        break;
    case GUI_EVENT_REFRESH:
        GuiPassphraseRefresh();
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
        GuiPassphrasePrevTile();
        break;
    case SIG_SETTING_WRITE_PASSPHRASE_PASS:
        GuiPassphraseDone();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_passphraseView = {
    .id = SCREEN_PASSPHRASE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiPassphraseViewEventProcess,
};
