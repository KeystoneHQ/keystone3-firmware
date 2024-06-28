#include "gui_nft_screen_widgets.h"
#include "gui.h"
#include "gui_pop_message_box.h"
#include "gui_hintbox.h"
#include "gui_views.h"
#include "gui_api.h"
#include "device_setting.h"

static void GuiTransNftScreenInit(void);
static void GuiTransNftScreenDeinit(void);
static void ConfirmNftScreenSaverHandler(lv_event_t *e);
static void NotSetNftNowHandler(lv_event_t *e);

static lv_obj_t *g_noticeWindow = NULL;

const GuiMsgBox_t g_guiMsgBoxNftScreen = {
    GuiTransNftScreenInit,
    GuiTransNftScreenDeinit,
    GUI_TRANSFER_NFT_SCREENSAVER,
};

static void GuiTransNftScreenInit(void)
{
    g_noticeWindow = GuiCreateGeneralHintBox(&imgWarn, _("nft_screen_saver_title"), _("nft_screen_saver_desc"), NULL,
                     _("not_now"), WHITE_COLOR_OPA20, _("Confirm"), DEEP_ORANGE_COLOR);
    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_add_event_cb(leftBtn, NotSetNftNowHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_add_event_cb(rightBtn, ConfirmNftScreenSaverHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiTransNftScreenDeinit(void)
{
    GUI_DEL_OBJ(g_noticeWindow)
}

static void NotSetNftNowHandler(lv_event_t *e)
{
    SetNftScreenSaver(false);
    SaveDeviceSettings();
    GuiApiEmitSignalWithValue(SIG_INIT_TRANSFER_NFT_SCREEN, 0);
}

static void ConfirmNftScreenSaverHandler(lv_event_t *e)
{
    SetNftScreenSaver(true);
    SaveDeviceSettings();
    GuiApiEmitSignalWithValue(SIG_INIT_TRANSFER_NFT_SCREEN, 0);
}
