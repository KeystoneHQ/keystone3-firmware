#include "gui_multisig_select_import_method_widgets.h"
#include "gui_page.h"
#include "gui_button.h"
#include "gui_views.h"
#include "gui_multisig_read_sdcard_widgets.h"
#include "gui_scan_widgets.h"
#include "gui_qr_hintbox.h"

#ifndef COMPILE_SIMULATOR
#include "drv_sdcard.h"
#else
#include "simulator_model.h"
#endif

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_noticeWindow;

static void GuiContent(lv_obj_t *parent);
static void SelectMicroCardFileHandler(lv_event_t *e);
static void SelectCameraHandler(lv_event_t *e);
static void OpenMultisigMoreInfoHandler(lv_event_t *e);

void GuiMultisigSelectImportMethodWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    GuiContent(g_pageWidget->contentZone);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, OpenMultisigMoreInfoHandler, NULL);
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
}

void GuiMultisigSelectImportMethodWidgetsDeInit()
{
    DestroyPageWidget(g_pageWidget);
    g_pageWidget = NULL;
    GUI_DEL_OBJ(g_noticeWindow);
}

static void GuiContent(lv_obj_t *parent)
{
    lv_obj_t *label, *img, *button, *imgArrow, *line;
    label = GuiCreateTitleLabel(parent, _("wallet_profile_import_multi_wallet"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 12);
    label = GuiCreateNoticeLabel(parent, _("wallet_profile_import_multi_wallet_desc"));
    lv_obj_set_width(label, 400);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 72);

    img = GuiCreateImg(parent, &imgScanImport);
    label = GuiCreateLittleTitleLabel(parent, _("import_multi_wallet_via_camera"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 40},
        },
        {
            .obj = label,
            .align = LV_ALIGN_DEFAULT,
            .position = {76, 40},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_DEFAULT,
            .position = {372, 40},
        },
    };
    button = GuiCreateButton(parent, 432, 120, table, NUMBER_OF_ARRAYS(table), SelectCameraHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 180);

    table[0].obj = GuiCreateImg(parent, &imgSdcardImport);
    table[1].obj = GuiCreateLittleTitleLabel(parent, _("import_multi_wallet_via_micro_card"));
    table[2].obj = GuiCreateImg(parent, &imgArrowRight);
    button = GuiCreateButton(parent, 432, 120, table, NUMBER_OF_ARRAYS(table), SelectMicroCardFileHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 300);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 180);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 300);
    line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 419);
}

static void SelectMicroCardFileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (SdCardInsert()) {
            static uint8_t fileFilterType = ONLY_TXT;
            GuiFrameOpenViewWithParam(&g_multisigReadSdcardView, &fileFilterType, sizeof(fileFilterType));
        } else {
            g_noticeWindow = GuiCreateErrorCodeWindow(ERR_UPDATE_SDCARD_NOT_DETECTED, &g_noticeWindow, NULL);
        }
    }
}

static void SelectCameraHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ViewType viewType = MultisigWalletImport;
        GuiFrameOpenView(&g_scanView);
        GuiSetScanViewTypeFiler(&viewType, 1);
    }
}

static void OpenMultisigMoreInfoHandler(lv_event_t *e)
{
    GuiQRCodeHintBoxOpen(_("multisig_decoding_qr_link"), _("multisig_decoding_qr_title"), _("multisig_decoding_qr_link"));
}