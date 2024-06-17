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
    SETUP_ENGLISH = 0,
    SETUP_RUSSIAN,
    SETUP_CHINESE,
    SETUP_KOREAN,
    SETUP_SPANISH,
    SETUP_GERMAN,
    SETUP_JAPANESE,

    SETUP_LANGUAGE_BUTT,
} SETUP_LANGUAGE_ENUM;

static const char *g_languageList[] = {
    "English",
    "Русский язык",
    "简体中文",
    "한국어",
    "Español",
    "Deutsch",
    "日本語",
};

typedef enum {
    SETUP_WELCOME = 0,
    SETUP_SET_LANGUAGE,

    SETUP_BUTT,
} SETUP_ENUM;

typedef struct SetupWidget {
    uint8_t currentTile;
    lv_obj_t *tileView;
    lv_obj_t *welcome;
    lv_obj_t *setLanguage;
} SetupWidget_t;
static SetupWidget_t g_setupTileView;
static lv_obj_t *g_languageCheck[SETUP_LANGUAGE_BUTT];
static lv_obj_t *g_selectLanguageCont;
static bool g_setup = false;
static uint32_t g_clickLogoCount = 0;
static void GoToDeviceUIDPage(lv_event_t *e);
static lv_timer_t *g_resetClickCountTimer;
static void ResetClickCountHandler(lv_timer_t *timer);
static void DestroyTimer(void);

SETUP_PHASE_ENUM lastShutDownPage;
static PageWidget_t *g_pageWidget;
#ifdef BTC_ONLY
#define SUPPORT_WALLET_INDEX SETUP_ENGLISH
#else
#define SUPPORT_WALLET_INDEX SETUP_JAPANESE
#endif

static void GuiWelcomeWidget(lv_obj_t *parent)
{
    lv_obj_t *img = GuiCreateImg(parent, &imgLogoGraphL);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 84 - GUI_NAV_BAR_HEIGHT);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GoToDeviceUIDPage, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = GuiCreateTitleLabel(parent, _("Keystone"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 268 - GUI_NAV_BAR_HEIGHT);

    label = GuiCreateNoticeLabel(parent, GetSoftwareVersionString());
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 328 - GUI_NAV_BAR_HEIGHT);

    lv_obj_t *btn = GuiCreateBtn(parent, USR_SYMBOL_ARROW_NEXT);
    lv_obj_set_size(btn, 96, 96);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -96);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_CLICKED, NULL);
}

static void SelectLanguageHandler(lv_event_t *e)
{
    int newCheckIndex = 0;
    lv_obj_t *newCheckBox = lv_event_get_user_data(e);
    for (int i = SETUP_ENGLISH; i <= SUPPORT_WALLET_INDEX; i++) {
        if (newCheckBox == g_languageCheck[i]) {
            newCheckIndex = i;
            lv_obj_add_state(g_languageCheck[i], LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(g_languageCheck[i], LV_STATE_CHECKED);
        }
    }

    LanguageSwitch(newCheckIndex);
    GuiEmitSignal(GUI_EVENT_CHANGE_LANGUAGE, NULL, 0);
}

void GuiOpenWebAuthHandler(lv_event_t *e)
{
    GuiWebAuthSetEntry(WEB_AUTH_ENTRY_SETUP);
    GuiFrameOpenView(&g_webAuthView);
}

void GuiCreateLanguageWidget(lv_obj_t *parent, uint16_t offset)
{
    uint8_t lang = LanguageGetIndex();
    static lv_point_t linePoints[2] = {{36, 0}, {444, 0}};
    lv_obj_t *label;
    for (int i = SETUP_ENGLISH; i <= SUPPORT_WALLET_INDEX; i++) {
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(parent, "");
        g_languageCheck[i] = checkBox;
        if (i == SETUP_CHINESE) {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &cnText);
        } else if (i == SETUP_RUSSIAN) {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &ruText);
        } else if (i == SETUP_KOREAN) {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &koText);
        } else if (i == SETUP_SPANISH) {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &esText);
        } else if (i == SETUP_GERMAN) {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &deText);
        } else if (i == SETUP_JAPANESE) {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &jaText);
        } else {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &openSansEnText);
        }
        if (i == lang) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        }
        GuiButton_t table[] = {
            {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {24, 0}},
            {.obj = checkBox, .align = LV_ALIGN_TOP_MID, .position = {0, 24}}
        };
        lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                           SelectLanguageHandler, g_languageCheck[i]);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, i * 84 + offset);
        lv_obj_t *line = GuiCreateLine(parent, linePoints, 2);
        lv_obj_align(line, LV_ALIGN_DEFAULT, 0, i * 84 + offset + 84);
    }
}

static void GuiSetLanguageWidget(lv_obj_t *parent)
{
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_height(parent, 800 - GUI_MAIN_AREA_OFFSET - 114);
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("language_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("language_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN | LV_STATE_DEFAULT);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    GuiCreateLanguageWidget(parent, 270 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_ARROW_NEXT);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, GuiOpenWebAuthHandler, LV_EVENT_CLICKED, NULL);
    g_selectLanguageCont = cont;
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
}

void GuiSetupAreaInit(void)
{
    g_setup = true;
    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, SETUP_WELCOME, 0, LV_DIR_RIGHT);
    g_setupTileView.welcome = tile;
    GuiWelcomeWidget(tile);

    tile = lv_tileview_add_tile(tileView, SETUP_SET_LANGUAGE, 0, LV_DIR_HOR);
    g_setupTileView.setLanguage = tile;
    GuiSetLanguageWidget(tile);

    g_setupTileView.currentTile = SETUP_WELCOME;
    g_setupTileView.tileView = tileView;

    lv_obj_set_tile_id(g_setupTileView.tileView, g_setupTileView.currentTile, 0, LV_ANIM_OFF);
}

uint8_t GuiSetupNextTile(void)
{
    switch (g_setupTileView.currentTile) {
    case SETUP_WELCOME:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        lv_obj_clear_flag(g_selectLanguageCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case SETUP_SET_LANGUAGE:
        return SUCCESS_CODE;
    }
    g_setupTileView.currentTile++;
    lv_obj_set_tile_id(g_setupTileView.tileView, g_setupTileView.currentTile, 0, LV_ANIM_OFF);
    if (g_setupTileView.currentTile == SETUP_SET_LANGUAGE) {
        GuiSetSetupPhase(SETUP_PAHSE_LANGUAGE);
        if (g_reboot) {
            if (!GuiJudgeCurrentPahse(SETUP_PAHSE_LANGUAGE)) {
                GuiWebAuthSetEntry(WEB_AUTH_ENTRY_SETUP);
                GuiFrameOpenView(&g_webAuthView);
            } else {
                g_reboot = false;
            }
        }
    }

    return SUCCESS_CODE;
}

uint8_t GuiSetupPrevTile(void)
{
    switch (g_setupTileView.currentTile) {
    case SETUP_WELCOME:
        return SUCCESS_CODE;
    case SETUP_SET_LANGUAGE:
        lv_obj_add_flag(g_selectLanguageCont, LV_OBJ_FLAG_HIDDEN);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        break;
    }
    g_setupTileView.currentTile--;
    lv_obj_set_tile_id(g_setupTileView.tileView, g_setupTileView.currentTile, 0, LV_ANIM_OFF);
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
    if (g_setupTileView.currentTile == SETUP_WELCOME) {
        lv_obj_add_flag(g_selectLanguageCont, LV_OBJ_FLAG_HIDDEN);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        GuiNvsBarSetWalletIcon(NULL);
        GuiNvsBarSetWalletName("");
        if (g_reboot) {
            lastShutDownPage = GetSetupStep();
        }
        GuiSetSetupPhase(SETUP_PAHSE_WELCOME);
        if (g_reboot) {
            if (!GuiJudgeCurrentPahse(SETUP_PAHSE_WELCOME) && !GuiJudgeCurrentPahse(SETUP_PAHSE_LANGUAGE)) {
                GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
            } else {
                g_reboot = false;
            }
        }
    } else {
        lv_obj_clear_flag(g_selectLanguageCont, LV_OBJ_FLAG_HIDDEN);
        GuiSetSetupPhase(SETUP_PAHSE_LANGUAGE);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    }
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
    if (GetSetupStep() != pahaseEnum) {
        SetSetupStep(pahaseEnum);
        SaveDeviceSettings();
    }
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