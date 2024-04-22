#include "gui_firmware_update_deny_widgets.h"
#include "gui.h"
#include "gui_pop_message_box.h"
#include "gui_hintbox.h"
#include "gui_views.h"
#include "gui_api.h"

static void GuiFirmwareUpdateDenyInit(void);
static void GuiFirmwareUpdateDenyDeInit(void);
static void ButtonHandler(lv_event_t *e);

static lv_obj_t *g_firmwareUpdateDenyHintBox = NULL;

const GuiMsgBox_t g_guiMsgBoxFirmwareUpdateDeny = {
    GuiFirmwareUpdateDenyInit,
    GuiFirmwareUpdateDenyDeInit,
    GUI_FIRMWARE_UPDATE_DENY_PRIORITY,
};

static void GuiFirmwareUpdateDenyInit(void)
{
    lv_obj_t *img, *label, *button;
    if (g_firmwareUpdateDenyHintBox == NULL) {
        g_firmwareUpdateDenyHintBox = GuiCreateHintBox(386);
        img = GuiCreateImg(g_firmwareUpdateDenyHintBox, &imgWarn);
        lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 36, -266);

        label = GuiCreateLittleTitleLabel(g_firmwareUpdateDenyHintBox, _("firmware_update_deny_title"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -202);
        label = GuiCreateIllustrateLabel(g_firmwareUpdateDenyHintBox, _("firmware_update_deny_desc"));
        lv_obj_set_width(label, 408);
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -130);

        button = GuiCreateTextBtn(g_firmwareUpdateDenyHintBox, _("firmware_update_deny_input_password"));
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_set_size(button, 234, 66);
        lv_obj_set_style_bg_color(button, DARK_GRAY_COLOR, LV_PART_MAIN);
        lv_obj_add_event_cb(button, ButtonHandler, LV_EVENT_CLICKED, NULL);
    }
}

static void GuiFirmwareUpdateDenyDeInit(void)
{
    if (g_firmwareUpdateDenyHintBox) {
        lv_obj_del(g_firmwareUpdateDenyHintBox);
        g_firmwareUpdateDenyHintBox = NULL;
    }
}

static void ButtonHandler(lv_event_t *e)
{
        GuiApiEmitSignalWithValue(SIG_INIT_FIRMWARE_UPDATE_DENY, 0);
}
