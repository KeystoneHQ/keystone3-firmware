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
#include "device_setting.h"
#include "gui_setup_widgets.h"
#include "low_power.h"
#include "account_manager.h"

static void UsbTask(void *argument);
void ClearUSBRequestId(void);

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
    #ifndef BTC_ONLY    
    ClearUSBRequestId();
    #endif
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
#if (USB_POP_WINDOW_ENABLE == 1)
    CloseUsb();
#else
    if (GetUSBSwitch() && GetUsbDetectState()) {
        OpenUsb();
    }
#endif
    while (1) {
        ret = osMessageQueueGet(g_usbQueue, &rcvMsg, NULL, 10000);
        if (ret == osOK) {
            switch (rcvMsg.id) {
            case USB_MSG_ISR_HANDLER: {
                ClearLockScreenTime();
                USBD_OTG_ISR_Handler((USB_OTG_CORE_HANDLE *)rcvMsg.value);
                NVIC_ClearPendingIRQ(USB_IRQn);
                NVIC_EnableIRQ(USB_IRQn);
            }
            break;
            case USB_MSG_SET_STATE: {
                GuiApiEmitSignal(SIG_INIT_USB_STATE_CHANGE, NULL, 0);
            }
            break;
            case USB_MSG_INIT: {
                g_usbState = true;
                UsbInit();
                SetUsbState(true);
            }
            break;
            case USB_MSG_DEINIT: {
                g_usbState = false;
                UsbDeInit();
                SetUsbState(false);
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
    if (strcmp(argv[0], "open") == 0) {
        printf("open usb\n");
        OpenUsb();
    } else if (strcmp(argv[0], "close") == 0) {
        printf("close usb\n");
        CloseUsb();
    }
}

