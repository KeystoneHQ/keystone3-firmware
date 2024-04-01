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
#include "fingerprint_process.h"
#include "account_public_info.h"
#include "multi_sig_wallet_manager.h"
#ifndef COMPILE_SIMULATOR
#include "safe_str_lib.h"
#else
#include "simulator_mock_define.h"
#endif
#include <gui_keyboard_hintbox.h>

#define MAX_LABEL_LENGTH 64
#define MAX_NAME_LENGTH 64
#define MAX_NETWORK_LENGTH 24
#define MAX_VERIFY_CODE_LENGTH 12
#define MAX_WALLET_CONFIG_TEXT_LENGTH 2048

static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;
static KeyboardWidget_t *g_keyboardWidget = NULL;

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

void GuiImportMultisigWalletInfoWidgetsInit()
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

void GuiImportMultisigWalletInfoWidgetsDeInit()
{
    GUI_DEL_OBJ(g_cont)
    if (g_pageWidget != NULL)
    {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiImportMultisigWalletInfoWidgetsRefresh()
{
    GuiImportWalletInfoNVSBarInit();
}

void GuiImportMultisigWalletInfoWidgetsRestart()
{
}

void GuiImportMultisigWalletInfoVerifyPasswordSuccess(void){
    MultiSigWalletManager_t *manager = initMultiSigWalletManager();
    char *password = SecretCacheGetPassword();
    MultiSigWalletGet(GetCurrentAccountIndex(), password, manager);
    MultiSigWalletItem_t* wallet = manager->findNode(manager->list, g_wallet->verify_code);
    if(wallet != NULL){
        //throw for existing
        GuiCLoseCurrentWorkingView();
        return;
    }
    MultiSigWalletItem_t *walletItem = SRAM_MALLOC(sizeof(MultiSigWalletItem_t));
    
    walletItem->name = SRAM_MALLOC(MAX_NAME_LENGTH);
    strcpy_s(walletItem->name, MAX_NAME_LENGTH, g_wallet->name);
    
    walletItem->network = g_wallet->network;
    
    walletItem->order = manager->getLength(manager->list);
    
    walletItem->verifyCode = SRAM_MALLOC(MAX_VERIFY_CODE_LENGTH);
    strcpy_s(walletItem->verifyCode, MAX_VERIFY_CODE_LENGTH, g_wallet->verify_code);
    
    walletItem->walletConfig = SRAM_MALLOC(MAX_WALLET_CONFIG_TEXT_LENGTH);
    strcpy_s(walletItem->walletConfig, MAX_WALLET_CONFIG_TEXT_LENGTH, g_wallet->config_text);
    
    manager->insertNode(manager->list, walletItem);
    manager->saveToFlash(password, manager);
}

static void GuiImportWalletInfoNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Wallet Info"));
}

static void SignByPasswordCb(bool cancel)
{
    if (cancel) {
        FpCancelCurOperate();
    }
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_MULTISIG_WALLET_IMPORT_VERIFY_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void GuiConfirmHandler(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        SignByPasswordCb(false);
    }
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
    lv_obj_add_event_cb(btn, GuiConfirmHandler, LV_EVENT_CLICKED, NULL);
}

