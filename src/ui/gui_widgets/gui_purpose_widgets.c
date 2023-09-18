#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_single_phrase_widgets.h"
#include "gui_create_wallet_widgets.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "background_task.h"
#include "gui_lock_widgets.h"
#include "gui_web_auth_widgets.h"
#include "gui_setup_widgets.h"
#include "gui_page.h"

static lv_obj_t *container;
static PageWidget_t *g_pageWidget;

void GuiPurposeAreaInit()
{
    g_pageWidget = CreatePageWidget();
    container = g_pageWidget->contentZone;

    lv_obj_t *label = GuiCreateTitleLabel(container, _("purpose_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateNoticeLabel(container, _("purpose_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *line = GuiCreateDividerLine(container);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 324 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *img = GuiCreateImg(container, &imgWallet);
    label = GuiCreateLittleTitleLabel(container, _("purpose_new_wallet"));
    lv_obj_t *right = GuiCreateImg(container, &imgArrowRight);

    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_DEFAULT,
            .position = {28, 42},
        },
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 40},
        },
        {
            .obj = right,
            .align = LV_ALIGN_DEFAULT,
            .position = {400, 42},
        },
    };
    lv_obj_t *button = GuiCreateButton(container, 456, 120, table, NUMBER_OF_ARRAYS(table), OpenCreateWalletHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 324 - GUI_MAIN_AREA_OFFSET);

    line = GuiCreateDividerLine(container);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 443 - GUI_MAIN_AREA_OFFSET);

    img = GuiCreateImg(container, &imgImport);
    label = GuiCreateTextLabel(container, _("purpose_import_wallet"));
    lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN | LV_STATE_DEFAULT);
    table[0].obj = img;
    table[0].position.x = -lv_obj_get_self_width(label) / 2 - 5;
    table[0].position.y = 0;
    table[0].align = LV_ALIGN_CENTER;

    table[1].obj = label;
    table[1].position.x = lv_obj_get_self_width(img) / 2 + 5;
    table[1].position.y = 0;
    table[1].align = LV_ALIGN_CENTER;
    button = GuiCreateButton(container, 228, 50, table, 2, OpenImportWalletHandler, NULL);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -26);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
}
void GuiPurposeAreaDeInit()
{
    lv_obj_del(container);
    container = NULL;
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}
void GuiPurposeAreaRefresh()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);

    GuiSetSetupPhase(SETUP_PAHSE_CREATE_WALLET);
    if (g_reboot) {
        if (!GuiJudgeCurrentPahse(SETUP_PAHSE_CREATE_WALLET)) {
            GuiFrameOpenView(&g_homeView);
        } else {
            g_reboot = false;
        }
    }
}
void GuiPurposeAreaRestart()
{
    GuiPurposeAreaDeInit();
    GuiPurposeAreaInit();
}