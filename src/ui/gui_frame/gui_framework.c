#include "gui_framework.h"
#include "gui_views.h"
#include "gui_api.h"
#include "screen_manager.h"
#include "gui_model.h"
#ifndef COMPILE_SIMULATOR
#include "user_msg.h"
#endif

/* DEFINES */
#define OPENED_VIEW_MAX 20          // the litmit of views

/* TYPEDEFS */
typedef struct {
    GUI_VIEW view;
    GUI_VIEW *pview;
} GuiFrameDebug_t;
GuiFrameDebug_t g_debugView[OPENED_VIEW_MAX];

/* FUNC DECLARATION*/
bool GuiLockScreenIsTop(void);
static const char *GuiFrameIdToName(SCREEN_ID_ENUM ID);

/* STATIC VARIABLES */
static GUI_VIEW *g_workingView = NULL;
static uint32_t g_viewCnt = 0;      // Record how many views are opened

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
    if (GuiLockScreenIsTop()) {
        //verify failed
        if (usEvent == SIG_VERIFY_PASSWORD_FAIL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            if (SIG_LOCK_VIEW_VERIFY_PIN == passwordVerifyResult->signal) {
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
            printf("usEvENT:%d sig has handled: id=%d name=%s\n", usEvent, pView->id, GuiFrameIdToName(pView->id));
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

    printf("open view %s freeHeap %d\n", GuiFrameIdToName(view->id), xPortGetFreeHeapSize());

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
    GuiViewHandleEvent(view, GUI_EVENT_OBJ_INIT, param, usLen);
    GuiViewHandleEvent(view, GUI_EVENT_REFRESH, NULL, 0);
    return SUCCESS_CODE;
}

int32_t GuiCloseCurrentWorkingView(void)
{
    g_viewCnt--;
    GuiViewHandleEvent(g_workingView, GUI_EVENT_OBJ_DEINIT, NULL, 0);
    printf("close view %s freeHeap %d\n", GuiFrameIdToName(g_workingView->id), xPortGetFreeHeapSize());
    g_workingView->isActive = false;
    g_workingView = g_workingView->previous;
    g_workingView->isActive = true;
    printf("refresh view %s freeHeap %d\n", GuiFrameIdToName(g_workingView->id), xPortGetFreeHeapSize());
    GuiViewHandleEvent(g_workingView, GUI_EVENT_REFRESH, NULL, 0);

    return SUCCESS_CODE;
}

int32_t GuiFrameCLoseView(GUI_VIEW *view)
{
    if (g_workingView == view) {
        return GuiCloseCurrentWorkingView();
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

bool GuiCheckIfTopView(GUI_VIEW *view)
{
    return view == g_workingView;
}

int32_t GuiCloseToTargetView(GUI_VIEW *view)
{
    GuiViewHintBoxClear();
    if (g_workingView == &g_homeView) {
        GuiViewHandleEvent(view, GUI_EVENT_RESTART, NULL, 0);
    } else {
        while (g_workingView != view) {
            GuiCloseCurrentWorkingView();
        }
        GuiViewHandleEvent(view, GUI_EVENT_REFRESH, NULL, 0);
    }
    return SUCCESS_CODE;
}

static const char *const g_screenNames[] = {
    SCREEN_LIST(ITEM_STR)
};

static const char *GuiFrameIdToName(SCREEN_ID_ENUM ID)
{
    if (ID < SCREEN_INIT || ID >= SCREEN_TOTAL) {
        return "UNKNOWN_SCREEN";
    }
    return g_screenNames[ID];
}
