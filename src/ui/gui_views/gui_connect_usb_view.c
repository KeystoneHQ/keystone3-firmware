#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_connect_usb_widgets.h"
#include "err_code.h"

int32_t GuiConnectUsbViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    uint32_t rcvValue;

    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiConnectUsbWidgetsInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiConnectUsbWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiConnectUsbWidgetsRefresh();
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        GuiConnectUsbPasswordPass();
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        GuiConnectUsbPasswordErrorCount(param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_connectUsbView = {
    .id = SCREEN_CONNECT_USB,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiConnectUsbViewEventProcess,
};
