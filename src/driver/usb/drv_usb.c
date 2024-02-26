#include <stdio.h>
#include "drv_usb.h"
#include "usbd_composite.h"
#include "usbd_usr.h"
#include "user_msg.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE g_usbDev __ALIGN_END;

static volatile bool g_usbInit = false;

void UsbInit(void)
{
    USBPHY_CR1_TypeDef usbphy_cr1;
    printf("usb init\r\n");

    if (g_usbInit == false) {
        SYSCTRL_AHBPeriphClockCmd(SYSCTRL_AHBPeriph_USB, ENABLE);
        SYSCTRL_AHBPeriphResetCmd(SYSCTRL_AHBPeriph_USB, ENABLE);

        usbphy_cr1.d32 = SYSCTRL->USBPHY_CR1;

        usbphy_cr1.b.commononn           = 0;
        usbphy_cr1.b.stop_ck_for_suspend = 0;

        SYSCTRL->USBPHY_CR1 = usbphy_cr1.d32;

        memset_s(&g_usbDev, sizeof(g_usbDev), 0x00, sizeof(g_usbDev));

        USBD_Init(&g_usbDev, USB_OTG_FS_CORE_ID, &USR_desc, DeviceCallback, &USRD_cb);
        g_usbInit = true;
    }
}

void USB_IRQHandler(void)
{
    NVIC_DisableIRQ(USB_IRQn);
    PubValueMsg(USB_MSG_ISR_HANDLER, (uint32_t)&g_usbDev);
}

void UsbLoop(void)
{

}

void UsbDeInit(void)
{
    if (g_usbInit == true) {
        USBD_DeInit(&g_usbDev);
        g_usbInit = false;
        ConnectUsbMutexRestrict();
    }
}

void UsbSetIRQ(bool enable)
{
    if (enable) {
        NVIC_EnableIRQ(USB_IRQn);
    } else {
        NVIC_DisableIRQ(USB_IRQn);
    }
}

bool UsbInitState(void)
{
    return g_usbInit;
}


