#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_home_widgets.h"
#include "gui_pending_hintbox.h"
#include "gui_lock_widgets.h"
#include "gui_scan_widgets.h"

static int32_t GuiHomeViewInit(void)
{
    return SUCCESS_CODE;
}

static int32_t GuiHomeViewDeInit(void)
{
    return SUCCESS_CODE;
}

int32_t GuiHomeViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_homeView = {
    .id = SCREEN_HOME,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiHomeViewEventProcess,
};
