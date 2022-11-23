/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: usb task.
 * Author: leon sun
 * Create: 2023-4-17
 ************************************************************************************************/

#include "usb_task.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "drv_usb.h"
#include "user_memory.h"
#include "user_msg.h"
#include "err_code.h"
#include "log.h"
#include "user_utils.h"
#include "screen_manager.h"
#include "gui_views.h"
#include "gui_api.h"
#include "drv_aw32001.h"

#ifdef BUILD_PRODUCTION
#define USB_POP_WINDOW_ENABLE           1
#endif

static void UsbTask(void *argument);

static osThreadId_t g_usbTaskHandle;
static volatile bool g_usbState = false;

void CreateUsbTask(void)
{
    const osThreadAttr_t usbTaskAttributes = {
        .name = "UsbTask",
        .stack_size = 1024 * 16,
        .priority = (osPriority_t) osPriorityNormal,
    };
    g_usbTaskHandle = osThreadNew(UsbTask, NULL, &usbTaskAttributes);
    printf("g_usbTaskHandle=%d\r\n", g_usbTaskHandle);
}


void SetUsbState(bool enable)
{
    PubValueMsg(USB_MSG_SET_STATE, enable);
}

void CloseUsb()
{
    PubValueMsg(USB_MSG_DEINIT, 0);
}

void OpenUsb()
{
    PubValueMsg(USB_MSG_INIT, 0);
}

bool GetUsbState(void)
{
    return g_usbState;
}

static void UsbTask(void *argument)
{
    Message_t rcvMsg;
    osStatus_t ret;

    osDelay(1000);
    while (1) {
        ret = osMessageQueueGet(g_usbQueue, &rcvMsg, NULL, 10000);
        if (ret == osOK) {
            switch (rcvMsg.id) {
            case USB_MSG_ISR_HANDLER: {
                ClearLockScreenTime();
#if (USB_POP_WINDOW_ENABLE == 1)
                if (g_usbState == false && GetUSBSwitch() == true) {
                    osDelay(50);
                    if (GetUsbDetectState() == true) {
                        printf("pop the USB connection message box and deinit USB driver\n");
                        UsbDeInit();
                        GuiApiEmitSignalWithValue(SIG_INIT_USB_CONNECTION, 1);
                    } else {
                        printf("!debug!\n");
                    }
                }
#endif
                if (GetUSBSwitch()) {
                    USBD_OTG_ISR_Handler((USB_OTG_CORE_HANDLE *)rcvMsg.value);
                    NVIC_ClearPendingIRQ(USB_IRQn);
                    NVIC_EnableIRQ(USB_IRQn);
                }
            }
            break;
            case USB_MSG_SET_STATE: {
                g_usbState = rcvMsg.value != 0;
                GuiApiEmitSignal(SIG_INIT_USB_STATE_CHANGE, NULL, 0);
                UsbInit();
            }
            break;
            case USB_MSG_INIT: {
                UsbInit();
            }
            break;
            case USB_MSG_DEINIT: {
                UsbDeInit();
            }
            break;
            default:
                break;
            }
            if (rcvMsg.buffer != NULL) {
                SRAM_FREE(rcvMsg.buffer);
            }
        }
    }
}


/// @brief
/// @param argc Test arg count.
/// @param argv Test arg values.
void UsbTest(int argc, char *argv[])
{
    if (strcmp(argv[0], "init") == 0) {
        printf("usb init\n");
        UsbInit();
    } else if (strcmp(argv[0], "deinit") == 0) {
        printf("usb deinit\n");
        UsbDeInit();
    } else if (strcmp(argv[0], "enable") == 0) {
        printf("usb enable\n");
        SetUsbState(true);
    } else if (strcmp(argv[0], "disable") == 0) {
        printf("usb disable\n");
        SetUsbState(false);
    }
}

