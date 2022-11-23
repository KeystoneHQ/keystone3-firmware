#include "lvgl.h"
#include "gui.h"
#include "gui_animating_qrcode.h"
#include "gui_model.h"
#include "gui_pending_hintbox.h"

lv_timer_t *g_timer = NULL;
lv_obj_t *g_qrcode = NULL;
bool g_showPending = false;

static void TimerHandler(lv_timer_t *timer)
{
    GuiModelURUpdate();
}

void GuiAnimatingQRCodeControl(bool pause)
{
    if (g_timer) {
        if (pause) {
            lv_timer_pause(g_timer);
        } else {
            lv_timer_resume(g_timer);
        }
    }
}

void GuiAnimatingQRCodeInit(lv_obj_t* parent, GenerateUR dataFunc, bool showPending)
{
    lv_obj_t *qrcode = lv_qrcode_create(parent, 294, BLACK_COLOR, WHITE_COLOR);
    lv_obj_align(qrcode, LV_ALIGN_DEFAULT, 0, 0);
    lv_qrcode_update(qrcode, "", 0);

    g_qrcode = qrcode;

    GuiModelURGenerateQRCode(dataFunc);

    g_showPending = showPending;

    if (showPending) {
        GuiPendingHintBoxOpen(_("Pending"), _("Generating QR Codes"));
    }
}

void GuiAnimantingQRCodeFirstUpdate(char* data, uint16_t len)
{
    GuiAnimatingQRCodeUpdate(data, len);
    g_timer = lv_timer_create(TimerHandler, 200, NULL);
    if (g_showPending) {
        GuiPendingHintBoxRemove();
    }
}

void GuiAnimatingQRCodeUpdate(char* data, uint16_t len)
{
    lv_qrcode_update(g_qrcode, data, len);
}

void GuiAnimatingQRCodeDestroyTimer()
{
    if (g_timer) {
        lv_timer_del(g_timer);
        g_timer = NULL;
    }
    GuiModelURClear();
    // dont clear g_qrcode because parent will clear it;
    // if (g_qrcode)
    // {
    //     GUI_DEL_OBJ(g_qrcode)
    // }
}