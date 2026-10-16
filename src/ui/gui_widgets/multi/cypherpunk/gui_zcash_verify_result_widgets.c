#include "gui.h"
#include "gui_page.h"
#include "gui_views.h"
#include "gui_qr_hintbox.h"
#include "gui_status_bar.h"
#include "account_manager.h"
#include "screen_manager.h"
#include "rust.h"
#include "gui_zcash_verify_result_widgets.h"

static PageWidget_t *g_pageWidget = NULL;

static void OpenZcashVerifyAboutHandler(lv_event_t *e)
{
    GuiQRCodeHintBoxOpen(_("zcash_verify_about_link"), _("zcash_verify_about_title"),
                         _("zcash_verify_about_link"));
}

static const char *ZcashVerifyTitle(ZcashAddressVerification result)
{
    switch (result) {
    case ZcashAddressOwned:
        return _("zcash_verify_title_owned");
    case ZcashAddressUnsupported:
        return _("zcash_verify_title_not_fully");
    default:
        return _("zcash_verify_title_not_verified");
    }
}

void GuiZcashVerifyResultWidgetsInit(const char *address)
{
    g_pageWidget = CreatePageWidget();
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    char ufvk[ZCASH_UFVK_BUFFER_SIZE] = {0};
    ZcashAddressVerification result = ZcashAddressKeyUnavailable;
    const uint8_t accountIndex = GetCurrentAccountIndex();
    if (address != NULL && accountIndex <= 2 && IsZcashSupportedForCurrentMnemonic() &&
            GetZcashUFVK(accountIndex, ufvk) == SUCCESS_CODE && ufvk[0] != '\0') {
        result = verify_zcash_address(ufvk, (char *)address);
    }
    CLEAR_ARRAY(ufvk);
    const char *message;
    switch (result) {
    case ZcashAddressOwned:
        message = _("zcash_verify_owned");
        break;
    case ZcashAddressNotOwned:
        message = _("zcash_verify_not_owned");
        break;
    case ZcashAddressInvalid:
        message = _("zcash_verify_invalid");
        break;
    case ZcashAddressUnsupported:
        message = _("zcash_verify_unsupported");
        break;
    default:
        message = _("zcash_verify_unavailable");
        break;
    }
    SetPageLockScreen(true);
    lv_obj_t *cont = g_pageWidget->contentZone;
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_style_pad_hor(cont, 36, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_row(cont, 24, LV_PART_MAIN);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_t *label = GuiCreateLittleTitleLabel(cont, ZcashVerifyTitle(result));
    lv_obj_set_width(label, 408);
    label = GuiCreateIllustrateLabel(cont, message);
    lv_obj_set_width(label, 408);
    lv_obj_set_style_text_color(label, lv_color_hex(result == ZcashAddressOwned ? 0x00CC88 : 0xF5870A), LV_PART_MAIN);
    label = GuiCreateTextLabel(cont, address == NULL ? "" : address);
    lv_obj_set_width(label, 408);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_t *learnMore = GuiCreateContainerWithParent(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_flag(learnMore, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_flex_flow(learnMore, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(learnMore, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(learnMore, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(learnMore, LV_OPA_0, LV_PART_MAIN);
    lv_obj_add_event_cb(learnMore, OpenZcashVerifyAboutHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *learnLabel = GuiCreateIllustrateLabel(learnMore, _("learn_more"));
    lv_obj_set_style_text_color(learnLabel, BLUE_GREEN_COLOR, LV_PART_MAIN);
    lv_obj_t *learnImg = GuiCreateImg(learnMore, &imgQrcodeTurquoise);
    lv_obj_clear_flag(learnLabel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(learnImg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_update_layout(learnMore);
}

void GuiZcashVerifyResultWidgetsDeInit(void)
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiZcashVerifyResultWidgetsRefresh(void)
{
    SetPageLockScreen(true);
}
