#include "gui_multisig_read_sdcard_widgets.h"
#include "gui.h"
#include "gui_page.h"
#include "stdlib.h"
#include "gui_button.h"
#include "gui_import_multisig_wallet_info_widgets.h"
#include "gui_btc.h"
#include "gui_model.h"
#include "gui_hintbox.h"
#ifndef COMPILE_SIMULATOR
#include "drv_sdcard.h"
#include "user_fatfs.h"
#else
#include "simulator_mock_define.h"
#include "simulator_model.h"
#endif

static PageWidget_t *g_pageWidget;
static char g_fileList[10][64] = {0};

static void GuiContent(lv_obj_t *);
static void GuiSelectFileHandler(lv_event_t *e);
static FileFilterType g_fileFilterType = ALL;
static ViewType g_viewType;
static lv_obj_t *g_scanErrorHintBox = NULL;

static void ThrowError();
static void GuiDealScanErrorResult(int errorType);
static void CloseErrorDataHandler(lv_event_t *e);


void GuiMultisigReadSdcardWidgetsInit(uint8_t fileFilterType)
{
    g_fileFilterType = fileFilterType;
    g_pageWidget = CreatePageWidget();
    GuiContent(g_pageWidget->contentZone);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("import_multi_wallet_via_micro_title"));
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
}
void GuiMultisigReadSdcardWidgetsDeInit()
{
    DestroyPageWidget(g_pageWidget);
    g_pageWidget = NULL;
}

static void GuiContent(lv_obj_t *parent)
{
    GuiAddObjFlag(parent, LV_OBJ_FLAG_SCROLLABLE);
    char *buffer = EXT_MALLOC(1024 * 5);
    uint32_t number = 0;
    int i = 0;
    char *suffix = NULL;
    switch (g_fileFilterType) {
    case ALL:
        break;
    case ONLY_TXT:
        suffix = ".txt";
        break;
    case ONLY_PSBT:
        suffix = ".psbt";
        break;
    case ONLY_JSON:
        suffix = ".json";
        break;
    default:
        break;
    }
    printf("suffix is %s\r\n", suffix);
#ifdef COMPILE_SIMULATOR
    FatfsGetFileName("C:/assets/sd", buffer, &number, 1024 * 5, suffix);
#else
    FatfsGetFileName("0:", buffer, &number, 1024 * 5, suffix);
#endif
    char *token = strtok(buffer, " ");
    while (token != NULL) {
        printf("token is %s\r\n", token);
        strncpy(g_fileList[i], token, sizeof(g_fileList[i]) - 1);
        token = strtok(NULL, " ");
        lv_obj_t *btn = GuiCreateSelectButton(parent, g_fileList[i], &imgArrowRight, GuiSelectFileHandler, g_fileList[i], false);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 84 * i);
        i++;
        if (i == 10) {
            break;
        }
    }
    EXT_FREE(buffer);
}

static void GuiSelectFileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    char *path = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {

        switch (g_fileFilterType) {
        case ALL:
            break;
        case ONLY_TXT: {
            char *walletConfig = FatfsFileRead(path);
            GuiSetMultisigImportWalletDataBySDCard(walletConfig);
            GuiFrameOpenView(&g_importMultisigWalletInfoView);
        }
        break;
        case ONLY_PSBT: {
            uint32_t readBytes = 0;
            uint8_t *psbtBytes = FatfsFileReadBytes(path, &readBytes);

            // for debug
            char *psbtStr = EXT_MALLOC(readBytes * 2 + 1);
            psbtStr[readBytes * 2] = 0;
            ByteArrayToHexStr(psbtBytes, readBytes, psbtStr);
            printf("psbt is %s\n", psbtStr);
            EXT_FREE(psbtStr);

            GuiSetPsbtStrData(psbtBytes, readBytes);
            g_viewType = BtcTx;
            GuiModelCheckTransaction(g_viewType);
        }
        break;
        case ONLY_JSON:

            break;
        default:
            break;
        }

    }
}



void GuiPSBtTransactionCheckPass(void)
{
    GuiModelTransactionCheckResultClear();
    GuiFrameOpenViewWithParam(&g_transactionDetailView, &g_viewType, sizeof(g_viewType));
}

void GuiPSBTTransactionCheckFaild(PtrT_TransactionCheckResult result)
{
    GuiModelTransactionCheckResultClear();
    ThrowError();
}

static void ThrowError()
{
    GuiDealScanErrorResult(0);
}

static void GuiDealScanErrorResult(int errorType)
{
    g_scanErrorHintBox = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_scanErrorHintBox, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_scanErrorHintBox, _("scan_qr_code_error_invalid_qrcode"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_scanErrorHintBox, _("scan_qr_code_error_invalid_qrcode_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_scanErrorHintBox, _("OK"), g_defTextFont);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, CloseErrorDataHandler, LV_EVENT_CLICKED, NULL);
}

static void CloseErrorDataHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_scanErrorHintBox)
    }
}
