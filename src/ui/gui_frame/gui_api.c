#include "gui_api.h"

#ifdef COMPILE_SIMULATOR
#ifndef COMPILE_MAC_SIMULATOR
#include <Windows.h>
#else
#include <stdlib.h>
#include <string.h>
#endif
#else
#include <stdlib.h>
#include <string.h>
#include "user_msg.h"
#include "user_memory.h"
#endif

int32_t GuiApiEmitSignal(uint16_t signal, void *param, uint16_t usLen)
{
#ifdef COMPILE_SIMULATOR
    GuiEmitMsg_t *msg = (GuiEmitMsg_t *)malloc(sizeof(GuiEmitMsg_t) + usLen);
    if (NULL == msg) {
        return ERR_GUI_ERROR;
    }
    msg->signal = signal;

    if (param != NULL) {
        memcpy(msg->param, param, usLen);
    }
#ifndef COMPILE_MAC_SIMULATOR
    if (!PostThreadMessage(GetUiThreadId(), signal, (WPARAM)msg, (LPARAM)(sizeof(GuiEmitMsg_t) + usLen))) {
        printf("post message failed, errno:%d\n", GetLastError());
    }
#endif
#else
    GuiEmitMsg_t *msg = (GuiEmitMsg_t *)SRAM_MALLOC(sizeof(GuiEmitMsg_t) + usLen);
    msg->signal = signal;
    if (param != NULL) {
        memcpy(msg->param, param, usLen);
    }
    PubBufferMsg(UI_MSG_EMIT_SIGNAL_TO_WORKING_VIEW, (void *)msg, sizeof(GuiEmitMsg_t) + usLen);
    SRAM_FREE(msg);
#endif
    return SUCCESS_CODE;
}


int32_t GuiApiEmitSignalWithValue(uint16_t signal, uint32_t value)
{
#ifdef COMPILE_SIMULATOR
#else
    GuiEmitMsg_t *msg = (GuiEmitMsg_t *)SRAM_MALLOC(sizeof(GuiEmitMsg_t) + sizeof(value));
    msg->signal = signal;
    memcpy(msg->param, &value, sizeof(value));
    PubBufferMsg(UI_MSG_EMIT_SIGNAL_TO_WORKING_VIEW, (void *)msg, sizeof(GuiEmitMsg_t) + sizeof(value));
    SRAM_FREE(msg);
#endif
    return SUCCESS_CODE;
}
