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
#include "gui_chain.h"
#ifndef COMPILE_SIMULATOR
#include "safe_str_lib.h"
#else
#include "simulator_mock_define.h"
#endif
#include <gui_keyboard_hintbox.h>

#define MAX_LABEL_LENGTH 64

static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;
static KeyboardWidget_t *g_keyboardWidget = NULL;

static void *g_multisig_wallet_info_data;
static bool g_isMulti = false;
static URParseResult *g_urResult = NULL;
static URParseMultiResult *g_urMultiResult = NULL;
static MultiSigWallet *g_wallet = NULL;
static lv_obj_t *g_errorHintBox = NULL;

static void GuiImportWalletInfoNVSBarInit();
static void GuiImportWalletInfoContent(lv_obj_t *parent);
static void GuiOnFailedHandler(lv_event_t *event);
static void GuiShowInvalidQRCode();
static void GuiShowWalletExisted();

static void GuiOnFailedHandler(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_errorHintBox);
        GuiCLoseCurrentWorkingView();
    }
}

void GuiSetMultisigImportWalletData(URParseResult *urResult, URParseMultiResult *multiResult, bool multi)
{
    g_urResult = urResult;
    g_urMultiResult = multiResult;
    g_isMulti = multi;
    g_multisig_wallet_info_data = g_isMulti ? g_urMultiResult->data : g_urResult->data;
}

static void GuiShowInvalidQRCode(){
    g_errorHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_errorHintBox, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_errorHintBox, _("scan_qr_code_error_invalid_qrcode"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_errorHintBox, _("scan_qr_code_error_invalid_qrcode_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_errorHintBox, _("OK"), g_defTextFont);
    lv_obj_set_size(btn, 94, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiOnFailedHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowWalletExisted() {
    #define PLUS_PADDING + 384
    g_errorHintBox = GuiCreateHintBox(lv_scr_act(), 480, 416, false);
    lv_obj_t *img = GuiCreateImg(g_errorHintBox, &imgFailed);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 36, 48 PLUS_PADDING);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_errorHintBox, _("multisig_import_wallet_exist"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 144 PLUS_PADDING);

    label = GuiCreateIllustrateLabel(g_errorHintBox, _("multisig_import_wallet_exist_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 196 PLUS_PADDING);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_errorHintBox, _("OK"), g_defTextFont);
    lv_obj_set_size(btn, 94, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiOnFailedHandler, LV_EVENT_CLICKED, NULL);
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
        GuiShowInvalidQRCode();
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
    CHECK_FREE_UR_RESULT(g_urResult, false);
    CHECK_FREE_UR_RESULT(g_urMultiResult, true);
    free_MultiSigWallet(g_wallet);
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
    loadCurrentAccountMultisigWallet(manager, password);
    MultiSigWalletItem_t* wallet = getMultisigWalletByVerifyCode(manager, g_wallet->verify_code);
    if(wallet != NULL){
        //throw for existing
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        GuiShowWalletExisted();
        return;
    }
    addMultisigWalletToCurrentAccount(manager, g_wallet, password);
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

