#include "gui_framework.h"
#include "gui_views.h"
#include "gui_api.h"
#include "screen_manager.h"
#include "gui_model.h"
#include <string.h>
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
    if (view != NULL && view->pEvtHandler != NULL) {
        if (GuiLockScreenIsTop()) {
            if (usEvent == GUI_EVENT_REFRESH || usEvent == SIG_INIT_SDCARD_CHANGE) {
                if (g_lockView.pEvtHandler != NULL) {
                    g_lockView.pEvtHandler(&g_lockView, usEvent, param, usLen);
                    return true;
                }
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
    static bool walletAborted = false;
    if (walletAborted) {
        return SUCCESS_CODE;
    }
    if (usEvent == SIG_WALLET_OPERATION_ABORTED || usEvent == SIG_WALLET_PASSCODE_ABORTED) {
        walletAborted = true;
        GuiWalletOperationAborted(usEvent == SIG_WALLET_PASSCODE_ABORTED);
        return SUCCESS_CODE;
    }
    bool sigHandled;
    GUI_VIEW *pView = g_workingView;
    uint32_t loopCnt = 0;
    if (GuiLockScreenIsTop()) {
        //verify failed
        if (usEvent == SIG_VERIFY_PASSWORD_FAIL) {
            if (param == NULL || usLen < sizeof(PasswordVerifyResult_t)) {
                return ERR_GUI_ERROR;
            }
            uint16_t signal;
            memcpy(&signal, param, sizeof(signal));
            if (SIG_LOCK_VIEW_VERIFY_PIN == signal) {
                sigHandled = GuiViewHandleEvent(&g_lockView, usEvent, param, usLen);
                return SUCCESS_CODE;
            }
        }
        //verify success
        if (usEvent == SIG_VERIFY_PASSWORD_PASS || usEvent == SIG_LOCK_VIEW_SCREEN_ON_PASSPHRASE_PASS) {
            if (param == NULL || usLen < sizeof(uint16_t)) {
                return ERR_GUI_ERROR;
            }
            uint16_t signal;
            memcpy(&signal, param, sizeof(signal));
            if (signal == SIG_LOCK_VIEW_VERIFY_PIN) {
                GuiViewHandleEvent(&g_lockView, usEvent, param, usLen);
                return SUCCESS_CODE;
            }
        }
    }
    while (pView != NULL) {
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
    }
    return SUCCESS_CODE;
}

bool GuiCheckIfViewOpened(GUI_VIEW *viewToOpen)
{
    return viewToOpen != NULL && viewToOpen->isActive;
}

static int32_t GuiFrameOpenViewInternal(GUI_VIEW *view, void *param, uint16_t usLen, bool deactivatePrevious)
{
    if (view == NULL || g_viewCnt >= OPENED_VIEW_MAX || GuiCheckIfViewOpened(view)) {
        printf("err:invalid, duplicate or excessive gui view\r\n");
        return ERR_GUI_ERROR;
    }

    printf("open view %s freeHeap %d\n", GuiFrameIdToName(view->id), xPortGetFreeHeapSize());

    g_debugView[g_viewCnt].view.id = view->id;
    g_debugView[g_viewCnt].view.pEvtHandler = view->pEvtHandler;
    g_debugView[g_viewCnt].view.previous = view->previous;
    g_debugView[g_viewCnt].view.isActive = view->isActive;
    g_debugView[g_viewCnt].pview = view;
    g_viewCnt++;
    if (deactivatePrevious && g_workingView != NULL) {
        GuiViewHandleEvent(g_workingView, GUI_EVENT_DISACTIVE, NULL, 0);
        // g_workingView->isActive = false;
    }
    view->previous = g_workingView;
    g_workingView = view;
    g_workingView->isActive = true;
    GuiViewHandleEvent(view, GUI_EVENT_OBJ_INIT, param, usLen);
    GuiViewHandleEvent(view, GUI_EVENT_REFRESH, NULL, 0);
    return SUCCESS_CODE;
}

int32_t GuiFrameOpenView(GUI_VIEW *view)
{
    return GuiFrameOpenViewInternal(view, NULL, 0, true);
}

int32_t GuiFrameOpenViewWithParam(GUI_VIEW *view, void *param, uint16_t usLen)
{
    return GuiFrameOpenViewInternal(view, param, usLen, false);
}

int32_t GuiCloseCurrentWorkingView(void)
{
    if (g_workingView == NULL || g_workingView->previous == NULL || g_viewCnt == 0) {
        return ERR_GUI_ERROR;
    }
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
    if (view == NULL || g_workingView == NULL || g_viewCnt == 0) {
        return ERR_GUI_ERROR;
    }
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
    if (data == NULL || len < sizeof(GuiEmitMsg_t)) {
        return ERR_GUI_ERROR;
    }
    GuiEmitMsg_t *msg = data;
    return GuiEmitSignal(msg->signal, (void *)msg->param, len - sizeof(GuiEmitMsg_t));
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
    if (view == NULL || g_workingView == NULL) {
        return ERR_GUI_ERROR;
    }
    if (g_workingView != &g_homeView) {
        GUI_VIEW *current = g_workingView;
        uint32_t remaining = g_viewCnt;
        while (current != NULL && current != view && remaining > 0) {
            current = current->previous;
            remaining--;
        }
        if (current != view) {
            return ERR_GUI_ERROR;
        }
    }
    GuiViewHintBoxClear();
    if (g_workingView == &g_homeView) {
        GuiViewHandleEvent(view, GUI_EVENT_RESTART, NULL, 0);
    } else {
        while (g_workingView != view) {
            if (GuiCloseCurrentWorkingView() != SUCCESS_CODE) {
                return ERR_GUI_ERROR;
            }
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
