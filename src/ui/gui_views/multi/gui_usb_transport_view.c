#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_usb_transport_widgets.h"

int32_t GuiUSBTransportViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    EAPDUResultPage_t *pageParam = (EAPDUResultPage_t *)param;
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiUSBTransportWidgetsInit(pageParam);
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiUSBTransportWidgetsDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiUSBTransportWidgetsRefresh();
        break;
    case SIG_CLOSE_USB_TRANSPORT:
        UsbGoToHomeView();
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_USBTransportView = {
    .id = SCREEN_USB_TRANSPORT,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiUSBTransportViewEventProcess,
};
