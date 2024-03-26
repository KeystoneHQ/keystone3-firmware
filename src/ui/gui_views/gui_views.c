#include "gui_views.h"
#include "gui_model.h"
#include "gui_hintbox.h"
#include "gui_lock_widgets.h"
#include "gui_keyboard.h"

static lv_obj_t *g_hintBox = NULL;
static lv_obj_t **g_hintParam = NULL;
void UnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    }
}

void OpenImportWalletHandler(lv_event_t *e)
{

}

void OpenCreateWalletHandler(lv_event_t *e)
{

}

void OpenViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenView(lv_event_get_user_data(e));
    }
}

void CloseTimerCurrentViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiCLoseCurrentWorkingView();
    }
}

void GoToHomeViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    }
}

void CloseCurrentViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiCLoseCurrentWorkingView();
    }
}

void ReturnHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

void NextTileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    }
}

void CloseToTargetTileView(uint8_t currentIndex, uint8_t targetIndex)
{
    for (int i = currentIndex; i > targetIndex; i--) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

void CloseCurrentParentHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    }
}

void CloseParentAndNextHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        void **param = lv_event_get_user_data(e);
        if (param != NULL) {
            *param = NULL;
        }
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    }
}

void CloseCurrentUserDataHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiViewHintBoxClear();
        GuiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
    }
}

void CloseCurrentParentAndCloseViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;

    if (code == LV_EVENT_CLICKED) {

    }
}

void CloseWaringPageHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_event_get_user_data(e));
        if (g_hintParam != NULL) {
            *g_hintParam = NULL;
        }
    }
}

void ToggleSwitchBoxHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *switchBox = lv_event_get_user_data(e);
        bool en = lv_obj_has_state(switchBox, LV_STATE_CHECKED);
        if (en) {
            lv_obj_clear_state(switchBox, LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(switchBox, LV_STATE_CHECKED);
        }
        lv_event_send(switchBox, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

void GuiWriteSeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTextLabel(parent, _("create_wallet_generating_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 403 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("create_wallet_generating_desc"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 457 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
}

void DuplicateShareHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiCLoseCurrentWorkingView();
        GuiCLoseCurrentWorkingView();
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        GuiViewHintBoxClear();
    }
}

void GuiViewHintBoxClear(void)
{
    if (g_hintBox != NULL) {
        GUI_DEL_OBJ(g_hintBox)
    }
}

void GuiDoNothingHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    }
}

void GuiWriteSeResult(bool en, int32_t errCode)
{
}

void *GuiCreateErrorCodeHintbox(int32_t errCode, lv_obj_t **param)
{
}
