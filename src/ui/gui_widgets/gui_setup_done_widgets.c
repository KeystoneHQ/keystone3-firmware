#include "define.h"
#include "gui.h"
#include "lvgl.h"
#include "gui_framework.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_qr_hintbox.h"
#include "gui_page.h"
#include "gui_button.h"
#include "drv_sdcard.h"
#include "gui_api.h"
#include "presetting.h"

static lv_obj_t *g_setupDoneTileView = NULL;
static bool g_setupDone = false;
static void StartUpdateHandler(lv_event_t *e);

int32_t FormatPublicKeyHexStr(uint8_t *pubkey, uint32_t maxLen, char *pubKeyStr, uint32_t pubKeyStrLen)
{
    char hexAlphaLookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    uint8_t pubKey[65] = {0};
    uint32_t copyLen = maxLen < sizeof(pubKey) ? maxLen : sizeof(pubKey);
    char tempStr[200] = {0};
    int pos = 0;

    memcpy_s(pubKey, sizeof(pubKey), pubkey, copyLen);

    pos += sprintf(tempStr + pos, "#F5870A ");
    
    for (int i = 0; i < 64; i++) {
        if (i > 0 && i % 2 == 0 && i != 4 && i != 60) {
            tempStr[pos++] = ' ';
        }
        
        if (i == 4) {
            pos += sprintf(tempStr + pos, "# ");
        }
        if (i == 60) {
            pos += sprintf(tempStr + pos, " #F5870A ");
        }
        
        tempStr[pos++] = hexAlphaLookup[pubKey[i + 1] >> 4];
        tempStr[pos++] = hexAlphaLookup[pubKey[i + 1] & 0x0F];
    }
    
    pos += sprintf(tempStr + pos, "#");
    
    strncpy(pubKeyStr, tempStr, pubKeyStrLen - 1);
    pubKeyStr[pubKeyStrLen - 1] = '\0';
    return SUCCESS_CODE;
}

void GuiSetupDoneInit(void)
{
    g_setupDone = true;
    g_setupDoneTileView = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET_NEW);
    lv_obj_align(g_setupDoneTileView, LV_ALIGN_TOP_MID, 0, GUI_MAIN_AREA_OFFSET_NEW);
    lv_obj_t *parent = g_setupDoneTileView;
    lv_obj_t *img = GuiCreateImg(parent, &imgWarn);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 36);

    lv_obj_t *label = GuiCreateLittleTitleLabel(parent, "Setup Completed");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 127);
    label = GuiCreateOrangeIllustrateLabel(parent, "Secure your firmware signing private key! ");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 179);

    lv_obj_t *number = GuiCreateOrangeIllustrateLabel(parent, "1");
    lv_obj_align(number, LV_ALIGN_TOP_LEFT, 36, 229);

    label = GuiCreateIllustrateLabel(parent, "Prepare your custom firmware or try our Hello World firmware here:");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 229);

    GuiButton_t table[] = {
        {.obj = GuiCreateColorIllustrateLabel(parent, "https://github.com/keystonehq/devkit", 0x1BE0C6), .align = LV_ALIGN_TOP_LEFT, .position = {0, 0},},
        {.obj = GuiCreateImg(parent, &imgQrcodeTurquoise), .align = LV_ALIGN_RIGHT_MID, .position = {0, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 389, 30, table, NUMBER_OF_ARRAYS(table), OpenCliConfigurationHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 60, 293);

    number = GuiCreateOrangeIllustrateLabel(parent, "2");
    lv_obj_align(number, LV_ALIGN_TOP_LEFT, 36, 335);
    label = GuiCreateIllustrateLabel(parent, "Copy the firmware to the SD card, insert it into the device, and click #F5870A Update# to install.");
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 60, 335);

    label = GuiCreateIllustrateLabel(parent, "Public Key:");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 486);
    char pubkeyStr[256] = {0};
    uint8_t pubkey[65] = {0};
    GetUpdatePubKey(pubkey);
    FormatPublicKeyHexStr(pubkey, sizeof(pubkey), pubkeyStr, sizeof(pubkeyStr));
    label = GuiCreateIllustrateLabel(parent, pubkeyStr);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 536);

    lv_obj_t *btn = GuiCreateBtn(parent, "Update");
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_add_event_cb(btn, StartUpdateHandler, LV_EVENT_CLICKED, NULL);
}

static void StartUpdateHandler(lv_event_t *e)
{
    printf("StartUpdateHandler\n");
    bool sdCardState = SdCardInsert();
    printf("sd card state = %d\n", sdCardState);
    GuiApiEmitSignalWithValue(SIG_INIT_SDCARD_CHANGE, !sdCardState);
}

void GuiSetupDoneDeInit(void)
{
}

void GuiSetupDoneRefresh(void)
{
}

bool AllowDeviceUpdateViaSdCard(void)
{
    return g_setupDone;
}
