#include "gui_api.h"
#include <string.h>

#ifdef COMPILE_SIMULATOR
#include "gui_framework.h"
#else
#include "user_msg.h"
#include "user_memory.h"
#endif

int32_t GuiApiEmitSignal(uint16_t signal, void *param, uint16_t usLen)
{
    if ((param == NULL && usLen != 0) || usLen > UINT16_MAX - sizeof(GuiEmitMsg_t)) {
        return ERR_GUI_ERROR;
    }
#ifdef COMPILE_SIMULATOR
    return GuiEmitSignal(signal, param, usLen);
#else
    GuiEmitMsg_t *msg = (GuiEmitMsg_t *)SRAM_MALLOC(sizeof(GuiEmitMsg_t) + usLen);
    if (msg == NULL) {
        return ERR_GUI_ERROR;
    }
    msg->signal = signal;
    if (param != NULL) {
        memcpy(msg->param, param, usLen);
    }
    uint32_t result = PubBufferMsg(UI_MSG_EMIT_SIGNAL_TO_WORKING_VIEW, (void *)msg, sizeof(GuiEmitMsg_t) + usLen);
    SRAM_FREE(msg);
    return result == MSG_SUCCESS ? SUCCESS_CODE : ERR_GUI_ERROR;
#endif
}

int32_t GuiApiEmitSignalWithValue(uint16_t signal, uint32_t value)
{
    return GuiApiEmitSignal(signal, &value, sizeof(value));
}
