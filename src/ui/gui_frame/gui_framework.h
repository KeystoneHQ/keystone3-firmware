#ifndef _GUI_FRAMEWORK_H
#define _GUI_FRAMEWORK_H

#include "gui_obj.h"

int32_t GuiEmitSignal(uint16_t usEvent, void *param, uint16_t usLen);
int32_t GuiPubEvent(uint16_t usEvent, void *param, uint16_t usLen);
int32_t GuiCLoseCurrentWorkingView(void);
int32_t GuiFrameCLoseView(GUI_VIEW *view);
int32_t GuiFrameOpenViewWithParam(GUI_VIEW *view, void *param, uint16_t usLen);
int32_t GuiFrameOpenView(GUI_VIEW *view);
bool GuiCheckIfViewOpened(GUI_VIEW *viewToOpen);
int32_t GuiFrameWorkViewHandleMsg(void *data, uint16_t len);
void GuiFrameDebugging(void);
int32_t GuiCloseToTargetView(GUI_VIEW *view);
bool GuiCheckIfTopView(GUI_VIEW *view);

#define GUI_FRAME_TIMER_EVENT_PERIOD 1000

typedef enum {
    GUI_EVENT_OBJ_INIT = 0,
    GUI_EVENT_REFRESH,
    GUI_EVENT_RESTART,
    GUI_EVENT_OBJ_DEINIT,
    GUI_EVENT_TIMER,
    GUI_EVENT_DISACTIVE,
    GUI_EVENT_UPDATE_KEYBOARD,

    GUI_EVENT_FRAME_WORK_RSV = 100,
} GUI_FRAME_EVENT_ENUM;

#endif /* _GUI_FRAMEWORK_H */

