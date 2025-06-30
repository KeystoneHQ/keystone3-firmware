#include "gui.h"
#include "gui_animating_qrcode.h"
#include "gui_model.h"
#include "gui_pending_hintbox.h"
#ifndef COMPILE_WIN_SIMULATOR
#include "lvgl/src/extra/libs/qrcode/lv_qrcode.h"
#endif
#include "gui_fullscreen_mode.h"

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

lv_obj_t* CreateQRCode(lv_obj_t* parent, uint16_t w, uint16_t h)
{
    lv_obj_t* qrcode = lv_qrcode_create(parent, w, QR_FG_COLOR, QR_BG_COLOR);
    lv_obj_add_flag(qrcode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(qrcode, GuiFullscreenModeHandler, LV_EVENT_CLICKED, NULL);
    lv_qrcode_update(qrcode, "", 0);
    return qrcode;
}

void GuiAnimatingQRCodeInitWithLoadingParams(lv_obj_t* parent, GenerateUR dataFunc, bool showPending, char *title, char *subtitle)
{
    GuiFullscreenModeInit(SCREEN_WIDTH, SCREEN_HEIGHT, QR_BG_COLOR);
    GuiFullscreenModeCreateObject(CreateQRCode, QR_SIZE_FULL, QR_SIZE_FULL);

    bool isValid = g_qrcode != NULL && lv_obj_is_valid(g_qrcode);
    if (g_qrcode == NULL || g_qrcode->parent != parent || !isValid) {
        if (isValid) {
            lv_obj_del(g_qrcode);
        }
        g_qrcode = CreateQRCode(parent, QR_SIZE_REGULAR, QR_SIZE_REGULAR);
        lv_obj_align(g_qrcode, LV_ALIGN_DEFAULT, 0, 0);
    }

    g_showPending = showPending;
    if (showPending) {
        GuiPendingHintBoxOpen(title, subtitle);
    }

    GuiModelURGenerateQRCode(dataFunc);

}

void GuiAnimatingQRCodeInit(lv_obj_t* parent, GenerateUR dataFunc, bool showPending)
{
    GuiAnimatingQRCodeInitWithLoadingParams(parent, dataFunc, showPending, _("Pending"), _("generating_qr_codes"));
}

void GuiAnimatingQRCodeInitWithCustomSize(lv_obj_t* parent, GenerateUR dataFunc, bool showPending, uint16_t w, uint16_t h, char *loadingTitle)
{
    GuiFullscreenModeInit(SCREEN_WIDTH, SCREEN_HEIGHT, QR_BG_COLOR);
    GuiFullscreenModeCreateObject(CreateQRCode, QR_SIZE_FULL, QR_SIZE_FULL);

    bool isValid = g_qrcode != NULL && lv_obj_is_valid(g_qrcode);
    if (g_qrcode == NULL || g_qrcode->parent != parent || !isValid) {
        if (isValid) {
            lv_obj_del(g_qrcode);
        }
        g_qrcode = CreateQRCode(parent, w, h);
        lv_obj_align(g_qrcode, LV_ALIGN_DEFAULT, 0, 0);
    }

    g_showPending = showPending;
    if (showPending) {
        GuiPendingHintBoxOpen(loadingTitle, NULL);
    }

    GuiModelURGenerateQRCode(dataFunc);
}

void GuiAnimantingQRCodeFirstUpdate(char* data, uint16_t len)
{
    GuiAnimatingQRCodeUpdate(data, len);
    g_timer = lv_timer_create(TimerHandler, TIMER_UPDATE_INTERVAL, NULL);
    if (g_showPending) {
        GuiPendingHintBoxRemove();
    }
}

void GuiAnimatingQRCodeUpdate(char* data, uint16_t len)
{
    if (g_qrcode == NULL) {
        return;
    }
    lv_qrcode_update(g_qrcode, data, len);

    lv_obj_t *fullscreen_qrcode = GuiFullscreenModeGetCreatedObjectWhenVisible();
    if (fullscreen_qrcode) {
        lv_qrcode_update(fullscreen_qrcode, data, len);
    }
}

void GuiAnimatingQRCodeDestroyTimer()
{
    if (g_timer) {
        lv_timer_del(g_timer);
        g_timer = NULL;
    }
    GuiModelURClear();

    GuiFullscreenModeCleanUp();
}