#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_web_auth_widgets.h"
#include "gui_web_auth_result_widgets.h"
#include "gui_firmware_update_widgets.h"
#include "gui_setup_widgets.h"
#include "gui_qr_hintbox.h"
#include "gui_page.h"

static void GuiWebAuthQrcodeHandler(lv_event_t *e);
void GuiWebAuthHidePending();
void GuiWebAuthRenderAuthCode(lv_obj_t *parent);
void GuiWebAuthInitNVSBar();
void GuiWebAuthDeInitNVSBar();
static void GuiGoToFirmwareUpdateViewHandler(lv_event_t *e);
typedef struct WebAuthWidget {
    lv_obj_t *cont;
} WebAuthWidget_t;
static WebAuthWidget_t g_webAuthWidget;

static uint8_t g_webAuthEntry = WEB_AUTH_ENTRY_BUTT;
static uint8_t g_firmwareWareEntry = FIRMWARE_UPDATE_ENTRY_SETUP;
static PageWidget_t *g_pageWidget;
static lv_obj_t *g_instructions;
static lv_obj_t *g_keyCheckPanel;
static lv_obj_t *g_scanButton;

static void GuiWebAuthResizeInstructions(void)
{
    lv_obj_t *footer = g_keyCheckPanel != NULL ? g_keyCheckPanel : g_scanButton;
    lv_obj_update_layout(footer);
    lv_obj_set_height(g_instructions, LV_MAX(0, lv_obj_get_y(footer) - 12));
    lv_obj_update_layout(g_instructions);
    lv_obj_readjust_scroll(g_instructions, LV_ANIM_OFF);
}

static lv_obj_t *GuiWebAuthCreateKeyCheckPanel(void)
{
    lv_obj_t *panel = GuiCreateContainerWithParent(lv_obj_get_parent(g_scanButton), 408, 118);
    lv_obj_align_to(panel, g_scanButton, LV_ALIGN_OUT_TOP_MID, 0, -38);
    return panel;
}

static void GuiWebAuthShowUpdatingCode(void)
{
    g_keyCheckPanel = GuiWebAuthCreateKeyCheckPanel();
    lv_obj_t *ring = GuiCreateImg(g_keyCheckPanel, &imgRing);
    lv_obj_align(ring, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_t *dot = GuiCreateImg(g_keyCheckPanel, &imgCircular);
    lv_obj_align(dot, LV_ALIGN_TOP_MID, 0, 13);
    lv_img_set_pivot(dot, 5, 25);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, dot);
    lv_anim_set_exec_cb(&anim, GuiSetAngle);
    lv_anim_set_values(&anim, 0, 3600);
    lv_anim_set_time(&anim, 1000);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anim);

    lv_obj_t *label = GuiCreateNoticeLabel(g_keyCheckPanel, "Updating code...");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 84);
}

static void GuiWebAuthFinishKeyCheck(void)
{
    GUI_DEL_OBJ(g_keyCheckPanel)
    lv_obj_clear_state(g_scanButton, LV_STATE_DISABLED);
    lv_obj_clear_state(lv_obj_get_child(g_scanButton, 0), LV_STATE_DISABLED);
}

void GuiWebAuthSetEntry(uint8_t entry)
{
    g_webAuthEntry = entry;
}

void GuiWebAuthSuccessCbSetup()
{
    GuiFrameOpenViewWithParam(&g_firmwareUpdateView, &g_firmwareWareEntry, sizeof(g_firmwareWareEntry));
}

void GuiWebAuthSuccessCbSetting()
{
    GuiFrameCLoseView(&g_webAuthView); // web_auth view
}

void GuiWebAuthScanQRCodeHandler(lv_event_t *e)
{
    if (g_scanButton == NULL || lv_obj_has_state(g_scanButton, LV_STATE_DISABLED)) {
        return;
    }
    GuiFrameOpenView(&g_scanView);
    if (g_webAuthEntry == WEB_AUTH_ENTRY_SETUP) {
        GuiWebAuthResultSetSuccessCb(GuiWebAuthSuccessCbSetup);
    } else {
        GuiWebAuthResultSetSuccessCb(GuiWebAuthSuccessCbSetting);
    }
}

static lv_obj_t *GuiWebAuthCreateStep(lv_obj_t *parent, lv_obj_t *previous,
                                     const char *number, const char *text, lv_coord_t gap)
{
    lv_obj_t *numberLabel = GuiCreateOrangeIllustrateLabel(parent, number);
    lv_obj_align_to(numberLabel, previous, LV_ALIGN_OUT_BOTTOM_LEFT, 0, gap);
    lv_obj_set_x(numberLabel, 36);

    lv_obj_t *label = GuiCreateIllustrateLabel(parent, text);
    lv_obj_set_width(label, 376);
    lv_obj_align_to(label, numberLabel, LV_ALIGN_OUT_RIGHT_TOP, 12, 0);
    return label;
}

static void GuiWebAuthIntroWidget(lv_obj_t *parent)
{
    g_instructions = GuiCreateContainerWithParent(parent, 480, 1);
    lv_obj_add_flag(g_instructions, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(g_instructions, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(g_instructions, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_bottom(g_instructions, 8, LV_PART_MAIN);

    lv_obj_t *title = GuiCreateScrollTitleLabel(g_instructions, _("verify_title"));
    lv_obj_align(title, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *description = GuiCreateIllustrateLabel(g_instructions, _("verify_desc"));
    lv_obj_set_width(description, 408);
    lv_obj_set_style_text_opa(description, LV_OPA_80, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(description, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    lv_obj_t *step1 = GuiWebAuthCreateStep(g_instructions, description, "1", _("verify_cont1"), 40);
    lv_label_set_recolor(step1, true);
    lv_obj_t *cont = GuiCreateContainerWithParent(g_instructions, 244, 30);
    lv_obj_align_to(cont, step1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, GuiWebAuthQrcodeHandler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = GuiCreateIllustrateLabel(cont, _("verify_qr_link"));
    lv_obj_set_style_text_color(label, BLUE_GREEN_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 0, 0);

    lv_obj_t *img;
    img = GuiCreateImg(cont, &imgQrcodeTurquoise);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 220, 0);

    lv_obj_t *step2 = GuiWebAuthCreateStep(g_instructions, cont, "2", _("verify_cont2"), 12);
    GuiWebAuthCreateStep(g_instructions, step2, "3", _("verify_cont3"), 12);

    lv_obj_t *btn = GuiCreateTextBtn(parent, _("verify_scan_qr_code"));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_add_event_cb(btn, GuiWebAuthScanQRCodeHandler, LV_EVENT_CLICKED, NULL);
    g_scanButton = btn;
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x6B707C), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_state(btn, LV_STATE_DISABLED);
    lv_obj_t *btnLabel = lv_obj_get_child(btn, 0);
    lv_obj_set_style_text_color(btnLabel, lv_color_hex(0xC7C9CE), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_state(btnLabel, LV_STATE_DISABLED);
    GuiWebAuthShowUpdatingCode();
    GuiWebAuthResizeInstructions();
}

void GuiWebAuthAreaInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(cont, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(cont, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    GuiWebAuthIntroWidget(cont);
    g_webAuthWidget.cont = cont;
    GuiModelCheckWebAuthKey();
}

void GuiWebAuthAreaDeInit()
{
    GUI_DEL_OBJ(g_webAuthWidget.cont)
    g_instructions = NULL;
    g_keyCheckPanel = NULL;
    g_scanButton = NULL;
    g_webAuthEntry = WEB_AUTH_ENTRY_BUTT;
    if (GuiQRHintBoxIsActive()) {
        GuiQRHintBoxRemove();
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiWebAuthShowKeyCheck(const char *keyCheck)
{
    if (g_scanButton == NULL || keyCheck == NULL) {
        return;
    }
    GuiWebAuthFinishKeyCheck();
    if (keyCheck[0] != '\0') {
        g_keyCheckPanel = GuiCreateNoticeLabel(lv_obj_get_parent(g_scanButton), keyCheck);
        lv_obj_set_width(g_keyCheckPanel, 408);
        lv_obj_align_to(g_keyCheckPanel, g_scanButton, LV_ALIGN_OUT_TOP_LEFT, 0, -16);
    }
    GuiWebAuthResizeInstructions();
}

void GuiWebAuthShowVerificationId(uint32_t crc)
{
    if (g_scanButton == NULL) {
        return;
    }
    GuiWebAuthFinishKeyCheck();
    g_keyCheckPanel = GuiWebAuthCreateKeyCheckPanel();
    GuiWebAuthResizeInstructions();
    lv_obj_set_style_bg_color(g_keyCheckPanel, lv_color_hex(0x0C0C0C), LV_PART_MAIN);
    lv_obj_set_style_radius(g_keyCheckPanel, 12, LV_PART_MAIN);
    lv_obj_set_style_border_width(g_keyCheckPanel, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(g_keyCheckPanel, ORANGE_COLOR, LV_PART_MAIN);

    lv_obj_t *title = GuiCreateOrangeIllustrateLabel(g_keyCheckPanel, "Device Verification ID");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 16);

    char id[9];
    snprintf(id, sizeof(id), "%08lX", (unsigned long)crc);
    int16_t x = 20;
    for (uint8_t i = 0; i < sizeof(id) - 1; i++) {
        if (i == 4) {
            x += 12;
        }
        lv_obj_t *cell = GuiCreateContainerWithParent(g_keyCheckPanel, 36, 48);
        lv_obj_set_style_radius(cell, 6, LV_PART_MAIN);
        lv_obj_set_style_bg_color(cell, DARK_BG_COLOR, LV_PART_MAIN);
        lv_obj_align(cell, LV_ALIGN_TOP_LEFT, x, 54);
        char digit[2] = {id[i], '\0'};
        lv_obj_t *label = GuiCreateLabelWithFont(cell, digit, &openSansEnLittleTitle);
        lv_obj_center(label);
        x += 45;
    }
}

// Unlock, Return from QRCode, Return from Purpose(in setup)
void GuiWebAuthAreaRefresh()
{
    GuiWebAuthInitNVSBar();
    if (g_webAuthEntry == WEB_AUTH_ENTRY_SETUP) {
        GuiSetSetupPhase(SETUP_PHASE_WEB_AUTH);
        if (g_reboot) {
            if (!GuiJudgeCurrentPahse(SETUP_PHASE_WEB_AUTH)) {
                GuiFrameOpenViewWithParam(&g_firmwareUpdateView, &g_firmwareWareEntry, sizeof(g_firmwareWareEntry));
            } else {
                g_reboot = false;
            }
        }
    }
}

void GuiWebAuthAreaRestart()
{
    GuiWebAuthAreaDeInit();
    GuiWebAuthAreaInit();
}

void GuiWebAuthInitNVSBar()
{
    if (g_webAuthEntry == WEB_AUTH_ENTRY_SETUP) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_NEW_SKIP, GuiGoToFirmwareUpdateViewHandler, NULL);
    } else {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    }
}

static void GuiGoToFirmwareUpdateViewHandler(lv_event_t *e)
{
    GuiFrameOpenViewWithParam(&g_firmwareUpdateView, &g_firmwareWareEntry, sizeof(g_firmwareWareEntry));
}

static void GuiOpenQRCodeHintBox()
{
    GuiQRCodeHintBoxOpen(_("verify_qr_link"), _("verify_title_text"), _("verify_qr_link"));
}

static void GuiWebAuthQrcodeHandler(lv_event_t *e)
{
    GuiOpenQRCodeHintBox();
}
