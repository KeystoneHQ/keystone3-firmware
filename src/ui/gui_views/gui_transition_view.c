#include "gui_views.h"
#include "gui_qr_hintbox.h"
#include "gui_firmware_update_widgets.h"
#include "version.h"
#include "gui_api.h"

static lv_obj_t *g_transitionPage = NULL;
static const char g_firmwareUrl[] = "https://keyst.one/firmware";

static void ShowSourceHash(lv_event_t *e)
{
    lv_obj_add_flag(g_transitionPage, LV_OBJ_FLAG_HIDDEN);
    if (GuiFrameOpenView(&g_aboutInfoView) != SUCCESS_CODE) {
        lv_obj_clear_flag(g_transitionPage, LV_OBJ_FLAG_HIDDEN);
    }
}

static void OpenUpdate(bool viaUsb)
{
    static int entry = FIRMWARE_UPDATE_ENTRY_SETTING;
    lv_obj_add_flag(g_transitionPage, LV_OBJ_FLAG_HIDDEN);
    if (GuiFrameOpenViewWithParam(&g_firmwareUpdateView, &entry, sizeof(entry)) == SUCCESS_CODE) {
        GuiFirmwareUpdateShowMethod(viaUsb);
        if (viaUsb) {
            GuiApiEmitSignalWithValue(SIG_INIT_USB_CONNECTION, 1);
        }
    } else {
        lv_obj_clear_flag(g_transitionPage, LV_OBJ_FLAG_HIDDEN);
    }
}

static void UpdateViaSdCard(lv_event_t *e)
{
    OpenUpdate(false);
}

static void UpdateViaUsb(lv_event_t *e)
{
    OpenUpdate(true);
}

static void ShowFirmwareQr(lv_event_t *e)
{
    GuiQRCodeHintBoxOpen(g_firmwareUrl, _("firmware_update_title"), g_firmwareUrl);
}

static lv_obj_t *CreateBullet(lv_obj_t *parent, const char *text, const lv_font_t *font, lv_coord_t y)
{
    lv_obj_t *label = GuiCreateLabelWithFont(parent, "", font);
    lv_label_set_text_fmt(label, "    %s", text);
    lv_obj_set_width(label, 408);
    lv_obj_set_style_text_color(label, lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_set_style_text_line_space(label, 0, LV_PART_MAIN);
    lv_obj_set_pos(label, 0, y);

    lv_obj_t *bullet = GuiCreateContainerWithParent(parent, 4, 4);
    lv_obj_set_style_radius(bullet, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bullet, lv_color_hex(0xFF5B23), LV_PART_MAIN);
    lv_obj_set_pos(bullet, 1, y + 12);
    lv_obj_update_layout(label);
    return label;
}

static void CreateTransitionPage(void)
{
    g_transitionPage = GuiCreateContainer(480, 800);
    lv_obj_add_flag(g_transitionPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *parent = g_transitionPage;
    lv_obj_t *img = GuiCreateImg(parent, &imgUpdate);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 64);

    lv_obj_t *title = GuiCreateLittleTitleLabel(parent, _("transition_firmware_title"));
    lv_obj_set_width(title, 408);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 170);
    lv_obj_update_layout(title);
    if (lv_obj_get_height(title) > 40) {
        lv_obj_set_style_text_font(title, g_defIllustrateFont, LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 176);
    }

    char versionText[32] = "v";
    GetSoftWareVersionNumber(&versionText[1]);
    lv_obj_t *version = GuiCreateIllustrateLabel(parent, versionText);
    lv_obj_set_style_text_color(version, lv_color_hex(0x999999), LV_PART_MAIN);
    lv_obj_align(version, LV_ALIGN_TOP_MID, 0, 226);

    /* Long translations can scroll without covering the upgrade buttons. */
    lv_obj_t *body = GuiCreateContainerWithParent(parent, 408, 300);
    lv_obj_set_pos(body, 36, 258);
    lv_obj_add_flag(body, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scroll_dir(body, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(body, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_t *notice = CreateBullet(body, _("transition_firmware_completed"), g_defIllustrateFont, 0);
    notice = CreateBullet(body, _("transition_firmware_continue"), g_defIllustrateFont, lv_obj_get_y2(notice) + 30);

    lv_obj_t *link = GuiCreateIllustrateLabel(body, g_firmwareUrl);
    lv_obj_set_style_text_color(link, lv_color_hex(0x1BE0C6), LV_PART_MAIN);
    lv_obj_align_to(link, notice, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_add_flag(link, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(link, ShowFirmwareQr, LV_EVENT_CLICKED, NULL);
    img = GuiCreateImg(body, &imgQrcodeTurquoise);
    lv_obj_align_to(img, link, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, ShowFirmwareQr, LV_EVENT_CLICKED, NULL);
    lv_obj_update_layout(link);
    CreateBullet(body, _("transition_firmware_assets_safe"), g_defIllustrateFont, lv_obj_get_y2(link) + 32);

    lv_obj_t *button = GuiCreateBtnWithFont(parent, _("firmware_update_via_usb"), g_defTextFont);
    lv_obj_set_size(button, 408, 66);
    lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -123);
    lv_obj_add_event_cb(button, UpdateViaUsb, LV_EVENT_CLICKED, NULL);

    button = GuiCreateBtnWithFont(parent, _("firmware_update_via_sd"), g_defTextFont);
    lv_obj_set_size(button, 408, 66);
    lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -36);
    lv_obj_add_event_cb(button, UpdateViaSdCard, LV_EVENT_CLICKED, NULL);

    button = GuiCreateTextBtn(parent, _("about_info_verify_source_code_title"));
    lv_obj_set_size(button, 408, 36);
    lv_obj_set_style_bg_opa(button, LV_OPA_0, LV_PART_MAIN);
    lv_obj_t *label = lv_obj_get_child(button, 0);
    lv_obj_set_style_text_font(label, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_set_pos(button, 36, 567);
    lv_obj_add_event_cb(button, ShowSourceHash, LV_EVENT_CLICKED, NULL);
}

static int32_t TransitionViewEventProcess(void *self, uint16_t event, void *param, uint16_t len)
{
    switch (event) {
    case GUI_EVENT_OBJ_INIT:
        CreateTransitionPage();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiQRHintBoxRemove();
        GUI_DEL_OBJ(g_transitionPage);
        break;
    case GUI_EVENT_REFRESH:
        if (GuiCheckIfTopView(&g_transitionView)) {
            lv_obj_clear_flag(g_transitionPage, LV_OBJ_FLAG_HIDDEN);
        }
        break;
    case GUI_EVENT_DISACTIVE:
        GuiQRHintBoxRemove();
        lv_obj_add_flag(g_transitionPage, LV_OBJ_FLAG_HIDDEN);
        break;
    case SIG_SETUP_VIEW_TILE_PREV:
    case SIG_INIT_GET_ACCOUNT_NUMBER:
        /* There is no route from this page to setup, unlock or wallet screens. */
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_transitionView = {
    .id = SCREEN_TRANSITION,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = TransitionViewEventProcess,
};
