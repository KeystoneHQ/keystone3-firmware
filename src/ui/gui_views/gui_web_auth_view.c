#include "gui_obj.h"
#include "gui_resource.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_web_auth_widgets.h"
#include "err_code.h"
#include <string.h>

int32_t GuiWebAuthViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    GUI_ASSERT(g_webAuthView.isActive);
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        GuiWebAuthAreaInit();
        break;
    case GUI_EVENT_OBJ_DEINIT:
        GuiWebAuthAreaDeInit();
        break;
    case GUI_EVENT_REFRESH:
        GuiWebAuthAreaRefresh();
        break;
    case GUI_EVENT_RESTART:
        GuiWebAuthAreaRestart();
        break;
    case SIG_WEB_AUTH_KEY_CHECK:
        GuiWebAuthShowKeyCheck(param);
        break;
    case SIG_WEB_AUTH_VERIFICATION_ID: {
        uint32_t crc;
        if (param == NULL || usLen != sizeof(crc)) {
            return ERR_GUI_ERROR;
        }
        memcpy(&crc, param, sizeof(crc));
        GuiWebAuthShowVerificationId(crc);
        break;
    }
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_webAuthView = {
    .id = SCREEN_WEB_AUTH,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiWebAuthViewEventProcess,
};

