#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_device_public_key_widgets.h"
#include "device_setting.h"
#include "keystore.h"
#include "user_utils.h"
#include "gui_page.h"
#include "account_manager.h"
#include "se_manager.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;

static void GuiDevicePublicKeyNVSBarInit(void);
static void GuiDevicePublicKeyEntranceWidget(lv_obj_t *parent);

void GuiDevicePublicKeyWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    GuiDevicePublicKeyEntranceWidget(cont);
}

void GuiDevicePublicKeyWidgetsDeInit()
{
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiDevicePublicKeyWidgetsRefresh()
{
    GuiDevicePublicKeyNVSBarInit();
}

void GuiDevicePublicKeyWidgetsRestart()
{}

static void GuiDevicePublicKeyNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("about_info_device_uid"));
}

void GuiDevicePublicKeyEntranceWidget(lv_obj_t *parent)
{
    lv_obj_t * qrCodeCont = lv_obj_create(parent);
    lv_obj_set_size(qrCodeCont, 408, 612);
    lv_obj_align(qrCodeCont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_border_width(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(qrCodeCont, 0, 0);
    lv_obj_set_style_pad_all(qrCodeCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(qrCodeCont, 24, LV_PART_MAIN);
    lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(qrCodeCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(qrCodeCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(qrCodeCont, LV_OPA_10 + LV_OPA_2, LV_PART_MAIN);

    lv_obj_t * innerCont = lv_obj_create(qrCodeCont);
    lv_obj_set_size(innerCont, 336, 336);
    lv_obj_align(innerCont, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_clear_flag(innerCont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(innerCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(innerCont, WHITE_COLOR, LV_PART_MAIN);

    char hexStr[131] = {0};
    uint8_t pubkey[BUFFER_SIZE_64 + 1] = {0};
    int32_t ret = GetDevicePublicKey(pubkey);
    if (ret == 0) {
        ByteArrayToHexStr(pubkey, sizeof(pubkey), hexStr);
    } else {
        snprintf_s(hexStr, sizeof(hexStr), "%s%d", "get pubkey error, error code is ", ret);
    }

    printf("pubkey is %s\n", hexStr);

    char serialNumber[BUFFER_SIZE_64] = {0};
    GetSerialNumber(serialNumber);

    char qrData[BUFFER_SIZE_256] = {0};
    snprintf_s(qrData, BUFFER_SIZE_256, "%s#%s", serialNumber, hexStr);

    lv_obj_t * qrCode = lv_qrcode_create(innerCont, 294, BLACK_COLOR, WHITE_COLOR);
    lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 0);

    lv_qrcode_update(qrCode, qrData, (uint32_t)strnlen_s(qrData, BUFFER_SIZE_256));

    lv_obj_t * contentCont = lv_obj_create(qrCodeCont);
    lv_obj_set_size(contentCont, 336, 180);
    lv_obj_set_style_bg_opa(contentCont, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(contentCont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(contentCont, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_border_width(contentCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(contentCont, 0, 0);
    lv_obj_set_style_pad_all(contentCont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(contentCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(contentCont, LV_ALIGN_TOP_MID, 0, 396);

    char sn[BUFFER_SIZE_64 + 1] = {0};
    snprintf_s(sn, sizeof(sn), "SN:%s", serialNumber);

    char uid[BUFFER_SIZE_256] = {0};
    snprintf_s(uid, BUFFER_SIZE_256, "UID:%s", hexStr);

    char show[BUFFER_SIZE_256] = {0};
    snprintf_s(show, BUFFER_SIZE_256, "%s\n%s", sn, uid);

    lv_obj_t * label = GuiCreateIllustrateLabel(contentCont, show);
    lv_obj_set_width(label, 336);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}