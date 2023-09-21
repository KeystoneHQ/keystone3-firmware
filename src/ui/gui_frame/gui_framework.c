/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_framework.c
 * Description:
 * author     : stone wang
 * data       : 2023-01-06 13:59
**********************************************************************/
#include "gui_framework.h"
#include "gui_views.h"
#include "gui_api.h"
#include "screen_manager.h"
#include "gui_model.h"

#define OPENED_VIEW_MAX 20          // the litmit of views
static GUI_VIEW *g_workingView = NULL;
static uint32_t g_viewCnt = 0;      // Record how many views are opened

bool GuiLockScreenIsTop(void);

typedef struct {
    GUI_VIEW view;
    GUI_VIEW *pview;
} GuiFrameDebug_t;
GuiFrameDebug_t g_debugView[OPENED_VIEW_MAX];

static void GuiFrameIdToName(SCREEN_ID_ENUM ID);

bool GuiViewHandleEvent(GUI_VIEW *view, uint16_t usEvent, void *param, uint16_t usLen)
{
    int32_t ret;
    bool handled = false;
    if (NULL != view->pEvtHandler) {
        if (GuiLockScreenIsTop()) {
            if (usEvent == GUI_EVENT_REFRESH || usEvent == SIG_INIT_SDCARD_CHANGE) {
                g_lockView.pEvtHandler(&g_lockView, usEvent, param, usLen);
                return SUCCESS_CODE;
            }
        }

        ret = view->pEvtHandler(view, usEvent, param, usLen);
        if (ERR_GUI_UNHANDLED != ret) {
            handled = true;
            if (SUCCESS_CODE != ret) {
                //
            }
        }
    }
    return handled;
}

int32_t GuiEmitSignal(uint16_t usEvent, void *param, uint16_t usLen)
{
    bool sigHandled;
    GUI_VIEW *pView = g_workingView;
    uint32_t loopCnt = 0;
    GuiFrameDebugging();
    if (GuiLockScreenIsTop()) {
        //verify failed
        if (usEvent == SIG_VERIFY_PASSWORD_FAIL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            if (SIG_LOCK_VIEW_VERIFY_PIN == *(uint16_t *)passwordVerifyResult->signal) {
                sigHandled = GuiViewHandleEvent(&g_lockView, usEvent, param, usLen);
                return SUCCESS_CODE;
            }
        }
        //verify success
        if (param != NULL && (*(uint16_t *)param == SIG_LOCK_VIEW_VERIFY_PIN)) {
            sigHandled = GuiViewHandleEvent(&g_lockView, usEvent, param, usLen);
            return SUCCESS_CODE;
        }
    }
    do {
        sigHandled = GuiViewHandleEvent(pView, usEvent, param, usLen);
        if (sigHandled) {
            printf("usEvENT:%d sig has handled:", usEvent);
            GuiFrameIdToName(pView->id);
            return SUCCESS_CODE;
        }
        pView = pView->previous;
        ++loopCnt;
        if (loopCnt > g_viewCnt + 10) {
            return ERR_GUI_ERROR;
        }
    } while (pView != NULL);
    return SUCCESS_CODE;
}

bool GuiCheckIfViewOpened(GUI_VIEW *viewToOpen)
{
    // todo some error proofing
    if (viewToOpen->isActive) {
        return true;
    }

    return false;
}

int32_t GuiFrameOpenView(GUI_VIEW *view)
{
    // todo some error proofing
    if (GuiCheckIfViewOpened(view)) {
        printf("err:gui has already opened!\r\n");
        return ERR_GUI_ERROR;
    }

    GuiFrameDebugging();

    g_debugView[g_viewCnt].view.id = view->id;
    g_debugView[g_viewCnt].view.pEvtHandler = view->pEvtHandler;
    g_debugView[g_viewCnt].view.previous = view->previous;
    g_debugView[g_viewCnt].view.isActive = view->isActive;
    g_debugView[g_viewCnt].pview = view;
    g_viewCnt++;
    if (NULL != g_workingView) {
        GuiViewHandleEvent(g_workingView, GUI_EVENT_DISACTIVE, NULL, 0);
        // g_workingView->isActive = false;
    }
    view->previous = g_workingView;
    g_workingView = view;
    g_workingView->isActive = true;
    GuiFrameDebugging();
    GuiViewHandleEvent(view, GUI_EVENT_OBJ_INIT, NULL, 0);
    GuiViewHandleEvent(view, GUI_EVENT_REFRESH, NULL, 0);
    return SUCCESS_CODE;
}

int32_t GuiFrameOpenViewWithParam(GUI_VIEW *view, void *param, uint16_t usLen)
{
    // todo some error proofing
    g_debugView[g_viewCnt].view.id = view->id;
    g_debugView[g_viewCnt].view.pEvtHandler = view->pEvtHandler;
    g_debugView[g_viewCnt].view.previous = view->previous;
    g_debugView[g_viewCnt].view.isActive = view->isActive;
    g_debugView[g_viewCnt].pview = view;

    g_viewCnt++;
    if (NULL != g_workingView) {
        // g_workingView->isActive = false;
    }
    view->previous = g_workingView;
    g_workingView = view;
    g_workingView->isActive = true;
    GuiFrameDebugging();
    GuiViewHandleEvent(view, GUI_EVENT_OBJ_INIT, param, usLen);
    GuiViewHandleEvent(view, GUI_EVENT_REFRESH, NULL, 0);
    return SUCCESS_CODE;
}

int32_t GuiCLoseCurrentWorkingView(void)
{
    g_viewCnt--;
    GuiViewHandleEvent(g_workingView, GUI_EVENT_OBJ_DEINIT, NULL, 0);
    g_workingView->isActive = false;
    g_workingView = g_workingView->previous;
    g_workingView->isActive = true;
    GuiViewHandleEvent(g_workingView, GUI_EVENT_REFRESH, NULL, 0);

    return SUCCESS_CODE;
}

int32_t GuiFrameCLoseView(GUI_VIEW *view)
{
    if (g_workingView == view) {
        return GuiCLoseCurrentWorkingView();
    } else {
        GUI_VIEW *current = g_workingView;
        while (current != NULL && current->previous != view) {
            current = current->previous;
        }
        if (current == NULL) {
            // not found view
            return ERR_GENERAL_FAIL;
        }
        //remove view
        GUI_VIEW *parent = current -> previous;
        g_viewCnt--;
        GuiViewHandleEvent(parent, GUI_EVENT_OBJ_DEINIT, NULL, 0);
        parent->isActive = false;
        parent = parent->previous;
        current->previous = parent;
    }
    return SUCCESS_CODE;
}



int32_t GuiFrameWorkViewHandleMsg(void *data, uint16_t len)
{
    GuiEmitMsg_t *msg = data;
    GuiEmitSignal(msg->signal, (void *)msg->param, len - 2);
    // GuiViewHandleEvent(g_workingView, msg->signal, (void *)msg->param, len - 2);
    return SUCCESS_CODE;
}

void GuiFrameDebugging(void)
{
    // printf("frame debug..\n");
    // for (int i = 0; i < g_viewCnt; i++) {
    //     printf("GuiFrameOpenView %d %p  ", i, g_debugView[i].pview);
    //     GuiFrameIdToName(g_debugView[i].view.id);
    // }
}

int32_t GuiCloseToTargetView(GUI_VIEW *view)
{
    GuiViewHintBoxClear();
    if (g_workingView == &g_homeView) {
        GuiViewHandleEvent(view, GUI_EVENT_RESTART, NULL, 0);
    } else {
        while (g_workingView != view) {
            GuiCLoseCurrentWorkingView();
        }
        GuiViewHandleEvent(view, GUI_EVENT_REFRESH, NULL, 0);
    }
    return SUCCESS_CODE;
}

static void GuiFrameIdToName(SCREEN_ID_ENUM ID)
{
    const char *str =
        "SCREEN_INIT\0" "SCREEN_LOCK\0" "SCREEN_HOME\0" "SCREEN_SETUP\0" "CREATE_WALLET\0" "CREATE_SHARE\0"
        "IMPORT_SHARE\0" "SINGLE_PHRASE\0" "IMPORT_SINGLE_PHRASE\0" "CONNECT_WALLET\0" "SCREEN_SETTING\0" "SCREEN_QRCODE\0"
        "USB_TRANSPORT\0";
    SCREEN_ID_ENUM i;

    for (i = SCREEN_INIT; i != ID && *str; i++) {
        while (*str++) ;
    }
    printf("id = %d name = %s\n", ID, str);
}

bool GuiCheckIfTopView(GUI_VIEW *view)
{
    return view == g_workingView;
}