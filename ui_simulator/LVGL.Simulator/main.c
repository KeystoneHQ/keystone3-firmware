#include <stdio.h>
#include "stdint.h"

#include <Windows.h>
#include "resource.h"
#include <process.h>
#include <conio.h>

#include "lvgl.h"
//#include "lvgl/examples/lv_examples.h"
//#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/win32drv/win32drv.h"
#include "lv_demos.h"
#include "gui_framework.h"
#include "gui_views.h"
#include "gui.h"
#include "gui_api.h"
#include <stdlib.h>
#include "err_code.h"
#include "gui_power_option_widgets.h"
#include "gui_firmware_process_widgets.h"
#include "gui_pop_message_box.h"

HANDLE hStartEvent;
static DWORD mainThreadId;
static DWORD uiThreadId;
void GuiWalletRecoveryWriteSe(bool result);

static void MainEventHandler(void)
{
    while (1) {
        char ch = _getch();
        printf("key pressed:'%c'\n", ch);
        static uint16_t single = 0;
        switch (ch) {
        case 'a':
            GuiFrameDebugging();
            // GuiApiEmitSignal(SIG_SETUP_VIEW_START, &ch, sizeof(ch));
            break;
        case 27:
            //ESC
            //GuiPowerOptionInit();
            break;
        case 'u':
            //GuiPopUsbConnectionMsgBox(true);
            break;
        case 'f':
            //GuiFirmwareProcessInit();
            break;
        case 'b':
            single = SIG_LOCK_VIEW_VERIFY_PIN;
            // GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
            // GuiWalletRecoveryWriteSe(false);
            // GuiWriteSeResult(false, ERR_KEYSTORE_MNEMONIC_INVALID);
            GuiCloseToTargetView(&g_homeView);
            break;
        case 'c':
            GuiCLoseCurrentWorkingView();
            break;
        default:
            break;
        }
    }
    return;
}

DWORD GetUiThreadId(void)
{
    return uiThreadId;
}

static void UIEventHandler(void)
{
    static uint16_t ret = 0;
    MSG msg;
    GuiEmitMsg_t *guiMsg = NULL;
    if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        return;
    }
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;

    printf("event received, id:0x%x\n", msg.message);
    switch (msg.message) {
    case SIG_SETUP_VIEW_START:
        GuiFrameOpenView(&g_setupView);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        guiMsg = (GuiEmitMsg_t *)(msg.wParam);
        printf("guiMsg->signal = %d\n", guiMsg->signal);
        memcpy(&ret, guiMsg->param, 1);
        printf("ret = %d\n", ret);
        GuiEmitSignal(SIG_VERIFY_PASSWORD_PASS, &ret, sizeof(ret));
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        guiMsg = (GuiEmitMsg_t *)(msg.wParam);
        memcpy(&ret, guiMsg->param, 1);
        printf("ret = %d\n", ret);
        GuiEmitSignal(SIG_VERIFY_PASSWORD_FAIL, &ret, sizeof(ret));
        break;
    case SIG_SETTING_SET_PIN:
        GuiEmitSignal(SIG_SETTING_SET_PIN, NULL, 0);
        break;
    case SIG_SETTING_REPEAT_PIN:
        GuiEmitSignal(SIG_SETTING_REPEAT_PIN, NULL, 0);
        break;
    case SIG_SETTING_PASSWORD_RESETTING:
        printf("%s %d\n", __func__, __LINE__);
        GuiEmitSignal(SIG_SETTING_PASSWORD_RESET_PASS, NULL, 0);
        break;
    case SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC:
        Sleep(10);
        GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_UPDATE_MNEMONIC, &ret, sizeof(ret));
        break;
    case SIG_CREATE_SHARE_UPDATE_MNEMONIC:
        Sleep(10);
        GuiEmitSignal(SIG_CREATE_SHARE_UPDATE_MNEMONIC, &ret, sizeof(ret));
        break;
    case SIG_SETTING_CHANGE_PASSWORD_PASS:
        printf("%s %d\n", __func__, __LINE__);
        GuiEmitSignal(SIG_SETTING_CHANGE_PASSWORD_PASS, &ret, sizeof(ret));
        break;
    case SIG_SETTING_CHANGE_PASSWORD_FAIL:
        printf("%s %d\n", __func__, __LINE__);
        GuiEmitSignal(SIG_SETTING_CHANGE_PASSWORD_FAIL, &ret, sizeof(ret));
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_SUCCESS:
        printf("%s %d\n", __func__, __LINE__);
        GuiEmitSignal(SIG_CREATE_SINGLE_PHRASE_WRITESE_PASS, &ret, sizeof(ret));
        // GuiEmitSignal(SIG_CREATE_SINGLE_PHRASE_WRITESE_PASS, &ret, sizeof(ret));
        break;
    case SIG_LOCK_VIEW_SCREEN_ON_VERIFY:
        printf("%s %d\n", __func__, __LINE__);
        GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
        break;
    case SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL:
        printf("%s %d\n", __func__, __LINE__);
        single = ERR_KEYSTORE_MNEMONIC_REPEAT;
        GuiEmitSignal(SIG_CREAT_SINGLE_PHRASE_WRITE_SE_FAIL, &single, sizeof(single));
        break;
    default:
        break;
    }
}

void gui_setting_view_open(void)
{
    static uint16_t lockParam = SIG_LOCK_VIEW_VERIFY_PIN;
    GuiFrameOpenViewWithParam(&g_lockView, &lockParam, sizeof(lockParam));
    GuiFrameOpenView(&g_settingView);
}

unsigned __stdcall UITaskEntry(void *param)
{
    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    if (!SetEvent(hStartEvent)) {
        printf("set start event failed, errno:%d\n", GetLastError());
        return 1;
    }
    GuiStyleInit();

    uiThreadId = GetCurrentThreadId();
    GuiFrameOpenView(&g_initView);
    // lv_example_textarea_1();
    // gui_keyboard_example_1();
    // gui_setting_view_open();
    // lv_example_tabview_1();
    // gui_file_vfs_example_1();
    // lv_example_slider_2();
    // lv_example_switch_1();
    // lv_example_list_1();
    // lv_example_emoji_keyboard_1();
    // lv_example_btn_1();
    // gui_button_example_1();
    // lv_example_btnmatrix_3();
    // gui_hintbox_example_1();
    // gui_button_example_1();
    // lv_example_img_3();

    while (1) {
        /* Periodically call the lv_task handler.
        * It could be done in a timer interrupt or an OS task too.*/
        lv_task_handler();
        UIEventHandler();
        Sleep(10);       /*Just to let the system breathe */
    }

    return 0;
}

int main()
{
    lv_init();
    if (!lv_win32_init(GetModuleHandleW(NULL), SW_SHOW, 480, 800, LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDI_LVGL)))) {
        return -1;
    }

    lv_win32_add_all_input_devices_to_group(NULL);

    mainThreadId = GetCurrentThreadId();

    hStartEvent = CreateEvent(0, FALSE, FALSE, 0);
    if (hStartEvent == 0) {
        printf("create start event failed, errno:%d\n", GetLastError());
        return 1;
    }

    unsigned uiThreadId;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &UITaskEntry, NULL, 0, &uiThreadId);
    if (hThread == 0) {
        printf("start thread failed, errno:%d\n", GetLastError());
        CloseHandle(hStartEvent);
        return 1;
    }

    WaitForSingleObject(hStartEvent, INFINITE);
    CloseHandle(hStartEvent);

    while (1) {
        MainEventHandler();
        Sleep(100);
    }
    return 0;
}
