#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "version.h"
#include "err_code.h"
#include "firmware_update.h"
#include "gui_page.h"
#include "gui_multi_sig_import_wallet_info_widgets.h"
#include "librust_c.h"
#include "keystore.h"
#ifndef COMPILE_SIMULATOR
#include "safe_str_lib.h"
#else
#include "simulator_mock_define.h"
#endif

#define MAX_LABEL_LENGTH 64

static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;

static void *g_multisig_wallet_info_data;
static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static MultiSigWallet *g_wallet = NULL;

static void GuiImportWalletInfoNVSBarInit();
static void GuiImportWalletInfoContent(lv_obj_t *parent);

void GuiSetMultisigImportWalletData(URParseResult *urResult, URParseMultiResult *multiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = multiResult;
    g_isMulti = multi;
    g_multisig_wallet_info_data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
}

void GuiImportWalletInfoWidgetsInit()
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    g_wallet = NULL;

    Ptr_Response_MultiSigWallet result = import_multi_sig_wallet_by_ur(g_multisig_wallet_info_data, mfp, 4, MainNet);
    if (result->error_code != 0)
    {
        printf("%s\r\n", result->error_message);
        // TODO: add error modal;
        GuiCLoseCurrentWorkingView();
        return;
    }
    else
    {
        g_wallet = result->data;
    }

    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    g_cont = cont;
    GuiImportWalletInfoContent(cont);
}

void GuiImportWalletInfoWidgetsDeInit()
{
    GUI_DEL_OBJ(g_cont)
    if (g_pageWidget != NULL)
    {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiImportWalletInfoWidgetsRefresh()
{
    GuiImportWalletInfoNVSBarInit();
}

void GuiImportWalletInfoWidgetsRestart()
{
}

static void GuiImportWalletInfoNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Wallet Info"));
}

void GuiImportWalletInfoContent(lv_obj_t *parent)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, 514);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *section = GuiCreateContainerWithParent(cont, 408, 100);
    lv_obj_align(section, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(section, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 24, LV_PART_MAIN);
    lv_obj_t *obj = GuiCreateNoticeLabel(section, _("Wallet Name"));
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 24, 16);
    obj = GuiCreateIllustrateLabel(section, g_wallet->name);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 24, 54);

    lv_obj_t *prev = section;
    section = GuiCreateContainerWithParent(cont, 408, 62);
    lv_obj_align_to(section, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_set_style_bg_color(section, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 24, LV_PART_MAIN);
    obj = GuiCreateNoticeLabel(section, _("Policy"));
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 24, 16);
    obj = GuiCreateIllustrateLabel(section, g_wallet->policy);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 94, 20);

    prev = section;
    section = GuiCreateContainerWithParent(cont, 408, 62);
    lv_obj_align_to(section, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_set_style_bg_color(section, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 24, LV_PART_MAIN);
    obj = GuiCreateNoticeLabel(section, _("Format"));
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 24, 16);
    obj = GuiCreateIllustrateLabel(section, g_wallet->format);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 108, 20);

    prev = section;
    uint32_t sectionHeight = 16 + (16 + 188) * g_wallet->derivations->size;
    section = GuiCreateContainerWithParent(cont, 408, sectionHeight);
    lv_obj_align_to(section, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
    lv_obj_set_style_bg_color(section, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(section, 24, LV_PART_MAIN);
    for (int i = 0; i < g_wallet->derivations->size; i++)
    {
        lv_obj_t *container = GuiCreateContainerWithParent(section, 360, 188);
        if (i == 0)
        {
            lv_obj_align(container, LV_ALIGN_TOP_LEFT, 24, 16);
        }
        else
        {
            lv_obj_align_to(container, prev, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);
        }
        lv_obj_set_style_bg_color(container, DARK_BG_COLOR, LV_PART_MAIN);
        char text[MAX_LABEL_LENGTH];
        snprintf_s(text, MAX_LABEL_LENGTH, "#F5870A %d/%d#", i + 1, g_wallet->derivations->size);
        obj = GuiCreateNoticeLabel(container, text);
        lv_obj_t *label = obj;
        lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_label_set_recolor(obj, true);

        obj = GuiCreateIllustrateLabel(container, g_wallet->xpub_items->data[i].xfp);
        lv_obj_align_to(obj, label, LV_ALIGN_OUT_TOP_RIGHT, 16, 0);

        obj = GuiCreateNoticeLabel(container, g_wallet->xpub_items->data[i].xpub);
        lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
        lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 20);
        lv_obj_set_style_width(obj, 360, LV_PART_MAIN);

        obj = GuiCreateNoticeLabel(container, g_wallet->derivations->data[i]);
        lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, 148);

        prev = container;
    }

    lv_obj_t *btn = GuiCreateBtn(parent, _("Confirm"));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_add_event_cb(btn, NULL, LV_EVENT_CLICKED, NULL);
}
