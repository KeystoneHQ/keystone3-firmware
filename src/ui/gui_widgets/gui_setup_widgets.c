#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_button.h"
#include "gui_setup_widgets.h"
#include "gui_obj.h"
#include "version.h"
#include "gui_web_auth_widgets.h"
#include "gui_web_auth_result_widgets.h"
#include "device_setting.h"
#include "gui_page.h"
#include "ui_display_task.h"

typedef enum {
    SETUP_WELCOME = 0,

    SETUP_BUTT,
} SETUP_ENUM;

typedef struct SetupWidget {
    uint8_t currentTile;
    lv_obj_t *tileView;
    lv_obj_t *welcome;
    lv_obj_t *setLanguage;
} SetupWidget_t;
static SetupWidget_t g_setupTileView;
static bool g_setup = false;
static uint32_t g_clickLogoCount = 0;
static void GoToDeviceUIDPage(lv_event_t *e);
static lv_timer_t *g_resetClickCountTimer;
static void ResetClickCountHandler(lv_timer_t *timer);
static void DestroyTimer(void);

SETUP_PHASE_ENUM lastShutDownPage;
static PageWidget_t *g_pageWidget;
static lv_obj_t *g_noticeWindow = NULL;
static lv_timer_t *g_countDownTimer;

static void CountDownTimerChangeLabelTextHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    lv_obj_t *label = lv_obj_get_child(obj, 0);
    char buff[32] = {0};
    static int8_t g_countDown = 5;
    --g_countDown;

    if (g_countDown == 0) {
        lv_label_set_text(label, "Got it");
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
        if (g_countDownTimer != NULL) {
            g_countDown = 0;
            lv_timer_del(g_countDownTimer);
            g_countDownTimer = NULL;
            UNUSED(g_countDownTimer);
        }
    } else {
        snprintf_s(buff, sizeof(buff), "Got it (%d)", g_countDown);
        lv_label_set_text(label, buff);
    }
}

static void StartSetupHandler(lv_event_t *e)
{
    g_noticeWindow = GuiCreateConfirmHintBox(&imgWarn, "Setup Your Device",
                     "Your device is running the official setup firmware. To install custom firmware, you must excute the upcoming instructions.", NULL, "Got it (5)",  ORANGE_COLOR);

    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_CLICKED, NULL);

    g_countDownTimer = lv_timer_create(CountDownTimerChangeLabelTextHandler, 1000, btn);
}

static void GuiWelcomeWidget(lv_obj_t *parent)
{
    // lv_obj_t *img = GuiCreateImg(parent, &imgLogoGraphL);
    lv_obj_t *img = GuiCreateImg(parent, &imgDevLogo);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 84 - GUI_NAV_BAR_HEIGHT);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GoToDeviceUIDPage, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = GuiCreateTitleLabel(parent, "ForgeBox");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 268 - GUI_NAV_BAR_HEIGHT);

    label = GuiCreateNoticeLabel(parent, GetSoftwareVersionString());
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 328 - GUI_NAV_BAR_HEIGHT);

    lv_obj_t *btn = GuiCreateBtn(parent, USR_SYMBOL_ARROW_NEXT);
    lv_obj_set_size(btn, 96, 96);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -96);
    lv_obj_add_event_cb(btn, StartSetupHandler, LV_EVENT_CLICKED, NULL);
}


void GuiOpenWebAuthHandler(lv_event_t *e)
{
    GuiWebAuthSetEntry(WEB_AUTH_ENTRY_SETUP);
    GuiFrameOpenView(&g_webAuthView);
}

void GuiSetupAreaInit(void)
{
    g_setup = true;
    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, SETUP_WELCOME, 0, LV_DIR_RIGHT);
    g_setupTileView.welcome = tile;
    GuiWelcomeWidget(tile);

    g_setupTileView.currentTile = SETUP_WELCOME;
    g_setupTileView.tileView = tileView;

    lv_obj_set_tile_id(g_setupTileView.tileView, g_setupTileView.currentTile, 0, LV_ANIM_OFF);
}

uint8_t GuiSetupNextTile(void)
{
    switch (g_setupTileView.currentTile) {
    case SETUP_WELCOME:
        GuiWebAuthSetEntry(WEB_AUTH_ENTRY_SETUP);
        GuiFrameOpenView(&g_webAuthView);
        break;
    }

    return SUCCESS_CODE;
}

uint8_t GuiSetupPrevTile(void)
{
    return SUCCESS_CODE;
}

void GuiSetupAreaDeInit(void)
{
    g_setup = false;
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

bool GuiIsSetup(void)
{
    return g_setup;
}

void GuiSetupAreaRefresh(void)
{
    GuiNvsBarSetWalletIcon(NULL);
    GuiNvsBarSetWalletName("");
}

void GuiSetupAreaRestart(void)
{
    uint8_t oldTile = g_setupTileView.currentTile;
    GuiSetupAreaDeInit();
    GuiSetupAreaInit();
    g_setupTileView.currentTile = oldTile;
    lv_obj_set_tile_id(g_setupTileView.tileView, oldTile, 0, LV_ANIM_OFF);
    GuiSetupAreaRefresh();
}

uint8_t GuiSetSetupPhase(SETUP_PHASE_ENUM pahaseEnum)
{
    return 0;
}

bool GuiJudgeCurrentPahse(SETUP_PHASE_ENUM pahaseEnum)
{
    return lastShutDownPage == pahaseEnum;
}

static void GoToDeviceUIDPage(lv_event_t *e)
{
    g_clickLogoCount++;
    DestroyTimer();
    g_resetClickCountTimer = lv_timer_create(ResetClickCountHandler, 1000, NULL);
    if (g_clickLogoCount == 3) {
        GuiFrameOpenView(&g_aboutInfoView);
    }
}

static void ResetClickCountHandler(lv_timer_t *timer)
{
    g_clickLogoCount = 0;
    DestroyTimer();
}

static void DestroyTimer(void)
{
    if (g_resetClickCountTimer != NULL) {
        lv_timer_del(g_resetClickCountTimer);
        g_resetClickCountTimer = NULL;
    }
}