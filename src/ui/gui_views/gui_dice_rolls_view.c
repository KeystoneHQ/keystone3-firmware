#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_dice_rolls_widgets.h"

static int32_t GuiDiceRollsViewInit(uint8_t seed_type)
{
    GuiDiceRollsWidgetsInit(seed_type);
    return SUCCESS_CODE;
}

static int32_t GuiDiceRollsViewDeInit(void)
{
    GuiDiceRollsWidgetsDeInit();
    return SUCCESS_CODE;
}

int32_t GuiDiceRollsViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint8_t seed_type;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        if (param != NULL) {
            seed_type = *(uint8_t *)param;
        } else {
            return ERR_GUI_ERROR;
        }
        return GuiDiceRollsViewInit(seed_type);
    case GUI_EVENT_OBJ_DEINIT:
        return GuiDiceRollsViewDeInit();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_diceRollsView = {
    .id = SCREEN_DICE_ROLLS,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiDiceRollsViewEventProcess,
};
