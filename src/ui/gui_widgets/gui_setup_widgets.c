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

typedef enum {
    SETUP_ENGLISH = 0,
    SETUP_CHINESE,
    SETUP_RUSSIAN,
    SETUP_SPANISH,
    SETUP_KOREAN,

    SETUP_LANGUAGE_BUTT,
} SETUP_LANGUAGE_ENUM;

static const char *g_languageList[] = {
    "English",
    "繁體中文",
    "Русский язык",
    "Español",
    "korean", // "한국인",
    "Japanese",
    "German",
};

typedef enum {
    SETUP_WELCOME = 0,
    SETUP_SET_LANGUAGE,

    SETUP_BUTT,
} SETUP_ENUM;

typedef struct SetupWidget {
    uint8_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    lv_obj_t *welcome;
    lv_obj_t *setLanguage;
} SetupWidget_t;
static SetupWidget_t g_setupTileView;
static uint32_t currentIndex = 0;
static lv_obj_t *g_languageCheck[SETUP_LANGUAGE_BUTT];

static uint32_t g_clickLogoCount = 0;
static void GoToDeviceUIDPage(lv_event_t *e);
static lv_timer_t *g_resetClickCountTimer;
static void ResetClickCountHandler(lv_timer_t *timer);
static void DestroyTimer(void);

SETUP_PHASE_ENUM lastShutDownPage;
static PageWidget_t *g_pageWidget;

static void GuiWelcomeWidget(lv_obj_t *parent)
{
    lv_obj_t *img = GuiCreateImg(parent, &imgLogoGraphL);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 84 - GUI_NAV_BAR_HEIGHT);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, GoToDeviceUIDPage, LV_EVENT_ALL, NULL);


    lv_obj_t *label = GuiCreateTitleLabel(parent, _("Keystone"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 268 - GUI_NAV_BAR_HEIGHT);

    label = GuiCreateNoticeLabel(parent, GetSoftwareVersionString());
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 328 - GUI_NAV_BAR_HEIGHT);

    lv_obj_t *btn = GuiCreateBtn(parent, USR_SYMBOL_ARROW_NEXT);
    lv_obj_set_size(btn, 96, 96);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -96);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_ALL, NULL);
}

static void SelectLanguageHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        int newCheckIndex = 0;
        lv_obj_t *newCheckBox = lv_event_get_user_data(e);
        for (int i = SETUP_ENGLISH; i < SETUP_LANGUAGE_BUTT; i++) {
            if (newCheckBox == g_languageCheck[i]) {
                newCheckIndex = i;
                break;
            }
        }

        lv_obj_add_state(newCheckBox, LV_STATE_CHECKED);
        if (newCheckIndex <= 1)
            LanguageSwitch(newCheckIndex);
        GuiEmitSignal(GUI_EVENT_RESTART, NULL, 0);
    }
}

void GuiOpenWebAuthHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiWebAuthSetEntry(WEB_AUTH_ENTRY_SETUP);
        GuiFrameOpenView(&g_webAuthView);
    }
}

static void GuiSetLanguageWidget(lv_obj_t *parent)
{
    static lv_point_t linePoints[2] = {{36, 0}, {444, 0}};
    uint8_t lang = LanguageGetIndex();
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("language_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("language_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);

    for (int i = SETUP_ENGLISH; i < SETUP_ENGLISH + 1; i++) {
        lv_obj_t *checkBox = GuiCreateSingleCheckBox(parent, "");
        g_languageCheck[i] = checkBox;
        if (i == SETUP_CHINESE) {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &openSansCnText);
        } else {
            label = GuiCreateLabelWithFont(parent, g_languageList[i], &openSansLanguage);
        }
        if (i == lang) {
            lv_obj_add_state(checkBox, LV_STATE_CHECKED);
            currentIndex = i;
        }
        GuiButton_t table[] = {
            {
                .obj = label,
                .align = LV_ALIGN_LEFT_MID,
                .position = {24, 0},
            },
            {
                .obj = checkBox,
                .align = LV_ALIGN_TOP_MID,
                .position = {0, 24},
            },
        };
        lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                           SelectLanguageHandler, g_languageCheck[i]);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 270 - GUI_MAIN_AREA_OFFSET + i * 84);
        lv_obj_t *line = GuiCreateLine(parent, linePoints, 2);
        lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 354 - GUI_MAIN_AREA_OFFSET + i * 84);
    }

    lv_obj_t *btn = GuiCreateBtn(parent, USR_SYMBOL_ARROW_NEXT);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, GuiOpenWebAuthHandler, LV_EVENT_ALL, NULL);
}

void GuiSetupAreaInit(void)
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = lv_tileview_create(cont);
    lv_obj_clear_flag(tileView, LV_OBJ_FLAG_SCROLLABLE);
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(tileView, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(tileView, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *tile = lv_tileview_add_tile(tileView, SETUP_WELCOME, 0, LV_DIR_RIGHT);
    g_setupTileView.welcome = tile;
    GuiWelcomeWidget(tile);

    tile = lv_tileview_add_tile(tileView, SETUP_SET_LANGUAGE, 0, LV_DIR_HOR);
    g_setupTileView.setLanguage = tile;
    GuiSetLanguageWidget(tile);

    g_setupTileView.currentTile = SETUP_WELCOME;
    g_setupTileView.tileView = tileView;
    g_setupTileView.cont = cont;

    lv_obj_set_tile_id(g_setupTileView.tileView, g_setupTileView.currentTile, 0, LV_ANIM_OFF);


}

uint8_t GuiSetupNextTile(void)
{
    switch (g_setupTileView.currentTile) {
    case SETUP_WELCOME:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
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
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        break;
    }
    g_setupTileView.currentTile--;
    lv_obj_set_tile_id(g_setupTileView.tileView, g_setupTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiSetupAreaDeInit(void)
{
    lv_obj_del(g_setupTileView.cont);
    g_setupTileView.cont = NULL;
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiSetupAreaRefresh(void)
{
    if (g_setupTileView.currentTile == SETUP_WELCOME) {
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

    // uint32_t currentPhase = GetSetupStep();
    // bool valid = false;
    // switch (pahaseEnum) {
    // case SETUP_PAHSE_WELCOME:
    //     if (currentPhase == SETUP_PAHSE_WELCOME) {
    //         valid = true;
    //     }
    //     break;
    // case SETUP_PAHSE_LANGUAGE:
    //     if (currentPhase == SETUP_PAHSE_WELCOME) {
    //         valid = true;
    //     }
    //     break;
    // case SETUP_PAHSE_WEB_AUTH:
    //     if (currentPhase == SETUP_PAHSE_LANGUAGE) {
    //         valid = true;
    //     }
    //     break;
    // case SETUP_PAHSE_FIRMWARE_UPDATE:
    //     if (currentPhase == SETUP_PAHSE_WEB_AUTH) {
    //         valid = true;
    //     }
    //     break;
    // case SETUP_PAHSE_CREATE_WALLET:
    //     if (currentPhase == SETUP_PAHSE_FIRMWARE_UPDATE) {
    //         valid = true;
    //     }
    //     break;
    // case SETUP_PAHSE_DONE:
    //     if (currentPhase == SETUP_PAHSE_CREATE_WALLET) {
    //         valid = true;
    //     }
    //     break;
    // default:
    //     break;
    // }

    // if (valid) {
    SetSetupStep(pahaseEnum);
    SaveDeviceSettings();
    return 0;
    // }
    // return -1;
}

bool GuiJudgeCurrentPahse(SETUP_PHASE_ENUM pahaseEnum)
{
    // uint32_t currentPhase = GetSetupStep();
    return lastShutDownPage == pahaseEnum;
}

static void GoToDeviceUIDPage(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        g_clickLogoCount++;
        DestroyTimer();
        g_resetClickCountTimer = lv_timer_create(ResetClickCountHandler, 1000, NULL);
        if (g_clickLogoCount == 3) {
            GuiFrameOpenView(&g_aboutInfoView);
        }
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