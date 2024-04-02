#include "gui_multisig_import_wallet_success_widgets.h"
#include "gui_fullscreen_mode.h"
#include "gui_page.h"
#include "gui_obj.h"
#include "multi_sig_wallet_manager.h"
#include "keystore.h"
#include "user_memory.h"
#include "define.h"
#ifndef COMPILE_SIMULATOR
#include "safe_str_lib.h"
#else
#include "simulator_mock_define.h"
#endif

#define MAX_ADDRESS_LEN 256

static lv_obj_t *g_cont, *g_qrcode, *g_eg;
static PageWidget_t *g_pageWidget;
static MultiSigWalletManager_t *manager = NULL;
static MultiSigWalletItem_t *g_wallet = NULL;

static void GuiImportWalletSuccessContent(lv_obj_t *parent);
static void ModelGenerateAddress(char *addr);
static void SetEgContent();
static char* convertFormatLabel(char *format);

void CutAndFormatAddress(char *out, uint32_t maxLen, const char *address, uint32_t targetLen);

static void GuiDoneHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        GuiCLoseCurrentWorkingView();
    }
}

static void GuiImportWalletSuccessNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Import Success"));
}

void GuiImportMultisigWalletSuccessWidgetsInit(char *verifyCode)
{
    g_wallet = GetMultisigWalletByVerifyCode(verifyCode);
    if (g_wallet == NULL)
    {
        // TODO: Throw error;
        GuiCLoseCurrentWorkingView();
        return;
    }

    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    GuiImportWalletSuccessNVSBarInit();
    GuiImportWalletSuccessContent(cont);
}

static UREncodeResult *GuiGenerateUR()
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    return export_multi_sig_wallet_by_ur(mfp, 4, g_wallet->walletConfig, MainNet);
}

static void GuiImportWalletSuccessContent(lv_obj_t *parent)
{
    lv_obj_t *cont, *text, *qr_cont, *btn;

    cont = GuiCreateContainerWithParent(parent, 480, 800 - 48 - 96 - 114);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    text = GuiCreateNoticeLabel(cont, _("multisig_import_success_hint"));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 8);
    lv_obj_set_style_text_align(text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    cont = GuiCreateContainerWithParent(cont, 408, 134 + 384);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 36, 122);

    qr_cont = GuiCreateContainerWithParent(cont, 336, 336);
    lv_obj_align(qr_cont, LV_ALIGN_TOP_LEFT, 36, 36);
    GuiAnimatingQRCodeInitWithCustomSize(qr_cont, GuiGenerateUR, false, 336, 336, NULL);

    text = GuiCreateIllustrateLabel(cont, convertFormatLabel(g_wallet->format));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 384);

    text = GuiCreateNoticeLabel(cont, _("Sample Address:"));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 34 + 384);

    text = GuiCreateIllustrateLabel(cont, "text_placeholder");
    lv_obj_set_width(text, 360);
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 34 + 384 + 30);
    lv_label_set_recolor(text, true);
    g_eg = text;
    SetEgContent();

    btn = GuiCreateBtn(parent, _("Done"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_add_event_cb(btn, GuiDoneHandler, LV_EVENT_CLICKED, NULL);
}

static char* convertFormatLabel(char *format){
    if(strcmp(format, FORMAT_P2WSH) == 0) {
        return "Native Segwit";
    }
    if(strcmp(format, FORMAT_P2WSH_P2SH) == 0) {
        return "Nested Segwit";
    }
    return "Legacy";
}

static void SetEgContent()
{
    char eg[BUFFER_SIZE_64] = {0};
    char prefix[8] = {0};
    char rest[BUFFER_SIZE_64] = {0};
    char addr[BUFFER_SIZE_128] = {0};
    char addrShot[BUFFER_SIZE_64] = {0};
    int8_t prefixLen = (strcmp(g_wallet->format, FORMAT_P2WSH) == 0) ? 4 : 1;
    memset_s(addrShot, BUFFER_SIZE_64, 0, BUFFER_SIZE_64);
    ModelGenerateAddress(addr);
    CutAndFormatAddress(addrShot, sizeof(addrShot), addr, 24);
    strncpy(prefix, addrShot, prefixLen);
    strncpy(rest, addrShot + prefixLen, strnlen_s(addrShot, BUFFER_SIZE_64) - prefixLen);
    snprintf_s(eg, sizeof(eg), "#F5870A %s#%s", prefix, rest);
    lv_label_set_text(g_eg, eg);
}

void GuiImportMultisigWalletSuccessWidgetsDeInit()
{
    GUI_DEL_OBJ(g_cont)
    if (g_pageWidget != NULL)
    {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiImportMultisigWalletSuccessWidgetsRefresh()
{
}

void GuiImportMultisigWalletSuccessWidgetsRestart()
{
}

static void ModelGenerateAddress(char *address)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    SimpleResponse_c_char *result = generate_address_for_multisig_wallet_config(g_wallet->walletConfig, 0, 0, mfp, 4, MainNet);
    if (result->error_code != 0)
    {
        printf("errorMessage: %s\r\n", result->error_message);
        GuiCLoseCurrentWorkingView();
        return;
    }
    strncpy_s(address, MAX_ADDRESS_LEN, result->data, strnlen_s(result->data, MAX_ADDRESS_LEN));
}