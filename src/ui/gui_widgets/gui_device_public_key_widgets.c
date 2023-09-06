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

static lv_obj_t *g_cont;


static void GuiDevicePublicKeyNVSBarInit(void);
static void GuiDevicePublicKeyEntranceWidget(lv_obj_t *parent);

void GuiDevicePublicKeyWidgetsInit()
{
    GuiDevicePublicKeyNVSBarInit();

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    g_cont = cont;
    GuiDevicePublicKeyEntranceWidget(cont);
}

void GuiDevicePublicKeyWidgetsDeInit()
{
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
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
    GuiNvsBarSetLeftCb(NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "Device UID");
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
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
    uint8_t pubkey[65] = {0};
    int32_t ret = GetDevicePublicKey(pubkey);
    if (ret == 0) {
        ByteArrayToHexStr(pubkey, sizeof(pubkey), hexStr);
    } else {
        sprintf(hexStr, "%s%d", "get pubkey error, error code is ", ret);
    }

    printf("pubkey is %s\n", hexStr);

    char serialNumber[64] = {0};
    GetSerialNumber(serialNumber);

    char qrData[200] = {0};
    sprintf(qrData, "%s#%s", serialNumber, hexStr);

    lv_obj_t * qrCode = lv_qrcode_create(innerCont, 294, BLACK_COLOR, WHITE_COLOR);
    lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 0);

    lv_qrcode_update(qrCode, qrData, (uint32_t)strlen(qrData));

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

    char sn[65] = {0};
    sprintf(sn, "SN:%s", serialNumber);

    char uid[135] = {0};
    sprintf(uid, "UID:%s", hexStr);

    char show[200] = {0};
    sprintf(show, "%s\n%s", sn, uid);

    lv_obj_t * label = GuiCreateLabel(contentCont, show);
    lv_obj_set_width(label, 336);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}