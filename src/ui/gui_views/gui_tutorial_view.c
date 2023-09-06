#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_tutorial_widgets.h"
#include "err_code.h"

static int32_t GuiTutorialViewInit(uint8_t index)
{
    GuiTutorialInit(index);
    return SUCCESS_CODE;
}

static int32_t GuiTutorialViewDeInit(void)
{
    GuiTutorialDeInit();
    return SUCCESS_CODE;
}

int32_t GuiTutorialViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_tutorialView.isActive);

    uint8_t index = 0;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            index = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiTutorialViewInit(index);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiTutorialViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiTutorialRefresh();
        break;
    case GUI_EVENT_DISACTIVE:
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_tutorialView = {
    .id = SCREEN_TUTORIAL,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiTutorialViewEventProcess,
};

