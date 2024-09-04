#include "gui_multisig_transaction_signature_widgets.h"
#include "gui_page.h"
#include "user_memory.h"
#include "gui_views.h"
#include "gui_btc.h"
#include "gui_hintbox.h"
#include "sdcard_manager.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_model.h"
#else
#include "user_fatfs.h"
#include "drv_rtc.h"
#include "drv_sdcard.h"
#endif

#define MAX_SIGN_STATUS_LEN 24
#define MAX_PSBT_LEN 20000
#define MAX_PSBT_NAME_LEN 128

static PageWidget_t *g_pageWidget = NULL;
static lv_obj_t *g_noticeWindow;
static lv_obj_t *g_cont = NULL;
static lv_obj_t *g_qrCont = NULL;
static lv_obj_t *g_signStatusLabel = NULL;
static lv_obj_t *g_signStatusView = NULL;
static lv_obj_t *g_hintView = NULL;

static char *g_signStatus = NULL;
static bool g_signCompleted = false;
static char *g_psbtName = NULL;
static uint8_t *g_psbtHex = NULL;
static uint32_t g_psbtLen = 0;

static void GuiMultisigTransactionSignatureContent(lv_obj_t *parent);
static void GuiMultisigTransactionSignatureSetupUR(lv_obj_t *parent);
static void GuiSDCardHandler(lv_event_t *);
static void GuiShowSDCardNotDetected();
static void GuiCloseHintBoxHandler(lv_event_t *);
static void GuiShowSDCardExport();
static void GuiShowSDCardExportSuccess();
static void GuiShowSDCardExportFailed();

static void GuiSDCardHandler(lv_event_t *e)
{
    if (SdCardInsert()) {
        GuiShowSDCardExport();
    } else {
        GuiShowSDCardNotDetected();
    }
    return;
}

static void GuiCloseHintBoxHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow);
}

static void GuiWriteSDCardHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow);
    char *filename = lv_event_get_user_data(e);
    int ret = FileWrite(filename, g_psbtHex, g_psbtLen);
    if (ret) {
        GuiShowSDCardExportFailed();
    } else {
        GuiShowSDCardExportSuccess();
    }
    return;
}

static void GuiShowSDCardNotDetected()
{
    g_noticeWindow = GuiCreateHintBox(356);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("firmware_update_sd_failed_access_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("firmware_update_sd_failed_access_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateTextBtn(g_noticeWindow, _("OK"));
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiCloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowSDCardExport()
{
    g_noticeWindow = GuiCreateHintBox(356);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgSdCardL);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateScrollLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard"), 408);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("about_info_export_file_name"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    SimpleResponse_c_char *result = generate_psbt_file_name(g_psbtHex, g_psbtLen, GetCurrentStampTime());
    if (result->error_code != 0) {
        printf("errorMessage: %s\r\n", result->error_message);
        GUI_DEL_OBJ(g_noticeWindow);
        GuiShowSDCardExportFailed();
        return;
    }

    char *filename = SRAM_MALLOC(MAX_PSBT_NAME_LEN);
    strncpy_s(filename, MAX_PSBT_NAME_LEN, result->data, strnlen_s(result->data, MAX_PSBT_NAME_LEN));
    label = GuiCreateIllustrateLabel(g_noticeWindow, filename);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 670);

    lv_obj_t *btn = GuiCreateTextBtn(g_noticeWindow, _("got_it"));
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiWriteSDCardHandler, LV_EVENT_CLICKED, filename);
}

static void GuiShowSDCardExportSuccess()
{
    g_noticeWindow = GuiCreateHintBox(356);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgSuccess);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard_success"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_signature_export_to_sdcard_success_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateTextBtn(g_noticeWindow, _("Done"));
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiCloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowSDCardExportFailed()
{
    g_noticeWindow = GuiCreateHintBox(356);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard_failed"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_export_to_sdcard_failed_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateTextBtn(g_noticeWindow, _("OK"));
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiCloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
}

void GuiMultisigTransactionSginatureSetPsbtName(char *psbtName)
{
    if (g_psbtName) {
        SRAM_FREE(g_psbtName);
    }
    g_psbtName = SRAM_MALLOC(MAX_PSBT_NAME_LEN);
    strncpy_s(g_psbtName, MAX_SIGN_STATUS_LEN, psbtName, strnlen_s(psbtName, MAX_PSBT_NAME_LEN));
}

void GuiMultisigTransactionSignatureSetSignStatus(char *signStatus, bool signCompleted, uint8_t *psbtHex, uint32_t psbtLen)
{
    if (g_signStatus)
        SRAM_FREE(g_signStatus);
    if (g_psbtHex)
        EXT_FREE(g_psbtHex);
    g_signStatus = SRAM_MALLOC(MAX_SIGN_STATUS_LEN);
    strncpy_s(g_signStatus, MAX_SIGN_STATUS_LEN, signStatus, strnlen_s(signStatus, MAX_SIGN_STATUS_LEN));
    g_psbtHex = EXT_MALLOC(MAX_PSBT_LEN);
    memcpy_s(g_psbtHex, MAX_PSBT_LEN, psbtHex, psbtLen);
    g_psbtLen = psbtLen;
    g_signCompleted = signCompleted;
    // This might be confusing. This function is actually called in `GuiGetSignQrCodeData`
    // which is in `GuiMultisigTransactionSignatureSetupUR`, so we need a refresh signal to update ui
    GuiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
}

void GuiMultisigTransactionSignatureNVSBarInit();

void GuiMultisigTransactionSignaureWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    GuiMultisigTransactionSignatureNVSBarInit();
    g_cont = g_pageWidget->contentZone;
    GuiMultisigTransactionSignatureSetupUR(g_cont);
}

static void GuiMultisigTransactionSignatureSetupUR(lv_obj_t *parent)
{
    g_qrCont = GuiCreateContainerWithParent(parent, 408, 450);
    lv_obj_set_style_bg_color(g_qrCont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(g_qrCont, 24, LV_PART_MAIN);
    lv_obj_align(g_qrCont, LV_ALIGN_TOP_LEFT, 36, 0);

    lv_obj_t *qrcode = GuiCreateContainerWithParent(g_qrCont, 336, 336);
    lv_obj_align(qrcode, LV_ALIGN_TOP_LEFT, 36, 36);
    bool showPending = true;
#if BTC_ONLY
    showPending = false;
#endif
    GuiAnimatingQRCodeInitWithCustomSize(qrcode, GuiGetSignQrCodeData, showPending, 336, 336, NULL);

    g_signStatusLabel = GuiCreateIllustrateLabel(g_qrCont, _("multi_signature_status"));
    lv_obj_align_to(g_signStatusLabel, qrcode, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
}

static void GuiMultisigTransactionSignatureContent(lv_obj_t *parent)
{

    if (g_signStatus != NULL) {
        char signStatus[64] = {0};
        if (strncmp(g_signStatus, "Completed", 9) == 0) {
            sprintf(signStatus, "#00FF00 %s#", _("multi_signature_completed"));
        } else {
            sprintf(signStatus, "#F5870A %s#", g_signStatus);
        }
        if (g_signStatusView != NULL) {
            lv_obj_del(g_signStatusView);
            g_signStatusView = NULL;
        }
        g_signStatusView = GuiCreateIllustrateLabel(g_qrCont, signStatus);
        lv_label_set_recolor(g_signStatusView, true);
        lv_obj_align_to(g_signStatusView, g_signStatusLabel, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    }

    char *hint = (char *)_("multisig_signature_hint_1");
    if (g_signCompleted) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("multi_signature_broadcast"));
        hint = (char *)_("connect_wallet_desc");
    }

    if (g_hintView != NULL) {
        lv_obj_del(g_hintView);
        g_hintView = NULL;
    }
    g_hintView = GuiCreateIllustrateLabel(g_cont, hint);
    lv_obj_align_to(g_hintView, g_qrCont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_text_align(g_hintView, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *btn = GuiCreateTextBtn(g_cont, _("Done"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_add_event_cb(btn, GoToHomeViewHandler, LV_EVENT_CLICKED, NULL);
}

void GuiMultisigTransactionSignatureNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, GoToHomeViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("multi_signature_title"));
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SDCARD, GuiSDCardHandler, NULL);
}

void GuiMultisigTransactionSignaureWidgetsDeInit()
{
    GUI_DEL_OBJ(g_noticeWindow);
    GuiAnimatingQRCodeDestroyTimer();
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
        g_signStatusView = NULL;
        g_hintView = NULL;
    }
    if (g_signStatus) {
        SRAM_FREE(g_signStatus);
        g_signStatus = NULL;
    }
    if (g_psbtName) {
        SRAM_FREE(g_psbtName);
        g_psbtName = NULL;
    }
    g_signCompleted = false;
}

void GuiMultisigTransactionSignaureWidgetsRefresh()
{
    GuiMultisigTransactionSignatureContent(g_cont);
    GuiAnimatingQRCodeControl(false);
}