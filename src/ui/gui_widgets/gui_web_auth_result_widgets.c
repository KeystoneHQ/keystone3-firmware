#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_qrcode_widgets.h"
#include "gui_web_auth_widgets.h"
#include "gui_web_auth_result_widgets.h"
#include "screen_manager.h"
#include "gui_page.h"

static void *g_web_auth_data;
static bool g_isMulti = false;
static void *g_urResult = NULL;
static char *g_authCode = NULL;
static lv_obj_t *g_hintBox = NULL;
static PageWidget_t *g_pageWidget;

typedef struct WebAuthResultWidget {
    uint8_t currentTile;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    lv_obj_t *result;
    lv_obj_t *failed;
    lv_obj_t *qrCodeCont;
} WebAuthResultWidget_t;
static WebAuthResultWidget_t g_WebAuthResultTileView;

static WebAuthSuccessCb g_webAuthSuccessCb = NULL;
void GuiWebAuthCalculateAuthCode();

void GuiWebAuthResultRenderAuthCode(lv_obj_t *parent);
void GuiWebAuthResultAreaInit();
void GuiWebAuthResultHidePending();
void GuiWebAuthResultInitNVSBar();
void GuiWebAuthResultShowPending();
static void WebAuthWipeDeviceHandler(lv_event_t *e);
static void WebAuthWipeDevice(void);
void GuiWebAuthResultShowPending();

void GuiWebAuthResultSetSuccessCb(WebAuthSuccessCb cb)
{
    g_webAuthSuccessCb = cb;
}

typedef enum {
    WEB_AUTH_RESULT_CODE = 1,
    WEB_AUTH_RESULT_FAILED,

    WEB_AUTH_RESULT_BUTT,
} WEB_AUTH_RESULT_ENUM;

void GuiSetWebAuthResultData(void *data, bool multi)
{
#ifndef COMPILE_SIMULATOR
    g_urResult = data;
    g_isMulti = multi;
    g_web_auth_data = g_isMulti ? ((URParseMultiResult *)g_urResult)->data : ((URParseResult *)g_urResult)->data;
#endif
}

void GuiWebAuthResultSuccessHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_webAuthSuccessCb != NULL) {
            g_webAuthSuccessCb();
        }
        GuiFrameCLoseView(&g_webAuthResultView);
    }
}

void GuiWebAuthResultCodeWidget(lv_obj_t *parent)
{
    lv_obj_t *btn, *label;

    label = GuiCreateTitleLabel(parent, _("verification_code_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("verification_code_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 352 - GUI_MAIN_AREA_OFFSET);

    btn = GuiCreateBtn(parent, _("Failed"));
    lv_obj_set_style_bg_color(btn, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_set_size(btn, 192, 66);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_ALL, NULL);

    btn = GuiCreateBtn(parent, _("Success"));
    lv_obj_set_style_bg_color(btn, GREEN_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 252, -24);
    lv_obj_set_size(btn, 192, 66);

    lv_obj_add_event_cb(btn, GuiWebAuthResultSuccessHandler, LV_EVENT_CLICKED, NULL);
}

void GuiWebAuthResultCreateAuthCode(lv_obj_t *parent, char *code, int xOffset)
{
    lv_obj_t *label, *cont;
    cont = GuiCreateContainerWithParent(parent, 44, 56);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, xOffset, 108);
    lv_obj_set_style_radius(cont, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x2d2d2d), 0);
    label = GuiCreateLittleTitleLabel(cont, code);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void GuiWebAuthResultRenderAuthCode(lv_obj_t *parent)
{
    if (g_authCode != NULL && strlen(g_authCode) >= 8) {
        char c1[2] = {g_authCode[0], '\0'};
        char c2[2] = {g_authCode[1], '\0'};
        char c3[2] = {g_authCode[2], '\0'};
        char c4[2] = {g_authCode[3], '\0'};
        char c5[2] = {g_authCode[4], '\0'};
        char c6[2] = {g_authCode[5], '\0'};
        char c7[2] = {g_authCode[6], '\0'};
        char c8[2] = {g_authCode[7], '\0'};

        GuiWebAuthResultCreateAuthCode(parent, c1, 36);
        GuiWebAuthResultCreateAuthCode(parent, c2, 86);
        GuiWebAuthResultCreateAuthCode(parent, c3, 136);
        GuiWebAuthResultCreateAuthCode(parent, c4, 186);

        GuiWebAuthResultCreateAuthCode(parent, c5, 250);
        GuiWebAuthResultCreateAuthCode(parent, c6, 300);
        GuiWebAuthResultCreateAuthCode(parent, c7, 350);
        GuiWebAuthResultCreateAuthCode(parent, c8, 400);
    }
}

static void WebAuthWipeDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (CHECK_BATTERY_LOW_POWER()) {
            g_hintBox = GuiCreateErrorCodeHintbox(ERR_KEYSTORE_SAVE_LOW_POWER, &g_hintBox);
        } else {
            WebAuthWipeDevice();
        }
    }
}

static void WebAuthWipeDevice(void)
{
    GuiWebAuthAreaDeInit();

    SetPageLockScreen(false);
    lv_obj_t *g_cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                          GUI_STATUS_BAR_HEIGHT);
    lv_obj_align(g_cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT);
    lv_obj_add_flag(g_cont, LV_OBJ_FLAG_CLICKABLE);
    GuiCreateCircleAroundAnimation(g_cont, -84);
    lv_obj_set_size(g_cont, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                    GUI_STATUS_BAR_HEIGHT);

    lv_obj_t *label = GuiCreateTextLabel(g_cont, "Resetting Device");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 355);

    label = GuiCreateNoticeLabel(g_cont, "Erasing Secure Element...");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 410);

    label = GuiCreateNoticeLabel(g_cont, "Do not power off your device while the installation process is underway");
    lv_obj_set_width(label, 408);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 612);

    GuiModelLockedDeviceDelAllWalletDesc();
}

void GuiWebAuthResultFailedWidget(lv_obj_t *parent)
{
    lv_obj_t *img, *label;
    img = GuiCreateImg(parent, &imgWarn);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 36);

    label = GuiCreateLittleTitleLabel(parent, _("verification_code_failed_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 140);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    label = GuiCreateIllustrateLabel(parent, _("verification_code_failed_desc"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 232);
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    char mail[64] = {0};
    sprintf(mail, "#1BE0C6 %s#", _("verification_code_failed_link"));

    label = GuiCreateIllustrateLabel(parent, mail);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 420);
    lv_label_set_recolor(label, true);

    char erase[64] = {0};
    sprintf(erase, "#F55831 %s#", _("Wipe Device"));

    label = GuiCreateTextLabel(parent, erase);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -64);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, WebAuthWipeDeviceHandler, LV_EVENT_CLICKED, NULL);
    lv_label_set_recolor(label, true);
}

void GuiWebAuthResultAreaInit()
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

    lv_obj_t *tile = lv_tileview_add_tile(tileView, WEB_AUTH_RESULT_CODE, 0, LV_DIR_RIGHT);
    g_WebAuthResultTileView.result = tile;
    GuiWebAuthResultCodeWidget(tile);

    tile = lv_tileview_add_tile(tileView, WEB_AUTH_RESULT_FAILED, 0, LV_DIR_HOR);
    g_WebAuthResultTileView.failed = tile;
    GuiWebAuthResultFailedWidget(tile);

    g_WebAuthResultTileView.currentTile = WEB_AUTH_RESULT_CODE;
    g_WebAuthResultTileView.tileView = tileView;
    g_WebAuthResultTileView.cont = cont;

    lv_obj_set_tile_id(g_WebAuthResultTileView.tileView, g_WebAuthResultTileView.currentTile, 0, LV_ANIM_OFF);
    if (g_web_auth_data != NULL) {
        GuiWebAuthResultShowPending();
        GuiWebAuthCalculateAuthCode();
    }
}

void GuiWebAuthResultAreaDeInit()
{
    lv_obj_del(g_WebAuthResultTileView.cont);
    GUI_DEL_OBJ(g_hintBox)
    g_WebAuthResultTileView.cont = NULL;
    g_webAuthSuccessCb = NULL;
    GuiWebAuthResultHidePending();
    if (g_urResult != NULL) {
        if (g_isMulti) {
            // has already free
            ((URParseMultiResult *)g_urResult)->data = NULL;
            free_ur_parse_multi_result(g_urResult);
        } else {
            // has already free
            ((URParseMultiResult *)g_urResult)->data = NULL;
            free_ur_parse_result(g_urResult);
        }
        g_urResult = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

// Unlock, Return from QRCode, Return from Purpose(in setup)
void GuiWebAuthResultAreaRefresh()
{
    GuiWebAuthResultInitNVSBar();
}

void GuiWebAuthResultAreaRestart()
{
    GuiWebAuthResultAreaDeInit();
    GuiWebAuthResultAreaInit();
}

void GuiWebAuthResultInitNVSBar()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
}

uint32_t GuiWebAuthResultPrevTile()
{
    // it starts from result tile;
    switch (g_WebAuthResultTileView.currentTile) {
    case WEB_AUTH_RESULT_CODE:
        return SUCCESS_CODE;
    case WEB_AUTH_RESULT_FAILED:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        break;
    }
    g_WebAuthResultTileView.currentTile--;
    lv_obj_set_tile_id(g_WebAuthResultTileView.tileView, g_WebAuthResultTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

uint32_t GuiWebAuthResultNextTile()
{
    switch (g_WebAuthResultTileView.currentTile) {
    case WEB_AUTH_RESULT_CODE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, ReturnHandler, NULL);
        break;
    case WEB_AUTH_RESULT_FAILED:
        return SUCCESS_CODE;
    }
    g_WebAuthResultTileView.currentTile++;
    lv_obj_set_tile_id(g_WebAuthResultTileView.tileView, g_WebAuthResultTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

static lv_obj_t *g_WebAuthResultPendingCont;

void GuiWebAuthResultShowPending()
{
    g_WebAuthResultPendingCont = GuiCreateAnimHintBox(lv_scr_act(), 480, 326, 82);
    lv_obj_t *title = GuiCreateTextLabel(g_WebAuthResultPendingCont, _("Verifing"));
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -124);
    lv_obj_t *desc = GuiCreateNoticeLabel(g_WebAuthResultPendingCont, _("Calculating Auth Code..."));
    lv_obj_align(desc, LV_ALIGN_BOTTOM_MID, 0, -76);
}

void GuiWebAuthResultHidePending()
{
    if (g_WebAuthResultPendingCont != NULL) {
        lv_obj_del(g_WebAuthResultPendingCont);
        g_WebAuthResultPendingCont = NULL;
    }
}

void GuiWebAuthCalculateAuthCode()
{
    GuiModelCalculateWebAuthCode(g_web_auth_data);
}

void GuiWebAuthShowAuthCode(char *authCode)
{
    g_authCode = authCode;
    GuiWebAuthResultRenderAuthCode(g_WebAuthResultTileView.result);
    GuiWebAuthResultHidePending();
}