/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "usbd_desc.h"
#include "usbd_hid_core.h"
#include "usbd_req.h"

#include "CircularBuffer.h"

static uint8_t RxBuffer[HID_OUT_PACKET_SIZE];

static CircularBufferStruct  SendCircularBuffer;
static CircularBufferStruct* InputBuffer;

static uint8_t SendByteBuffer[HID_IN_BUFFER_COUNT * HID_IN_PACKET_SIZE];

static volatile uint32_t TxPending = 0;

#if HID_OUT_BUFFER_COUNT
static CircularBufferStruct  ReadCircularBuffer;
static CircularBufferStruct* OutputBuffer;

static uint8_t ReadByteBuffer[HID_OUT_BUFFER_COUNT * HID_OUT_PACKET_SIZE];

static volatile uint32_t RxPending = 0;
#endif

static uint32_t USBD_HID_AltSet    = 0;
static uint32_t USBD_HID_Protocol  = 0;
static uint32_t USBD_HID_IdleState = 0;

/* USB HID device Configuration Descriptor */
static uint8_t USBD_HID_Config[] = {
    0x09,                        /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    NULL,                        /* wTotalLength: Bytes returned */
    NULL,                        /* Will calculate in GetDescriptor() */
    0x01,                        /* bNumInterfaces: 1 interface */
    0x01,                        /* bConfigurationValue: Configuration value */
    0x00,                        /* iConfiguration: Index of string descriptor describing the configuration */
    0xE0,                        /* bmAttributes: bus powered and Support Remote Wake-up */
    0x32,                        /* MaxPower 100 mA: this current is used for detecting Vbus */

    /************** Descriptor of HID interface ****************/
    0x09,                    /* bLength: Interface Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface descriptor type */
    0x00,                    /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
#ifdef HID_OUT_EP
    0x02, /* bNumEndpoints */
#else
    0x01, /* bNumEndpoints */
#endif
    HID_CLASS,         /* bInterfaceClass: HID */
    HID_SUBCLASS_NONE, /* bInterfaceSubClass : 1=BOOT, 0=no boot */
    HID_PROTOCOL_NONE, /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
    0x00,              /* iInterface: Index of string descriptor */

    /******************** Descriptor of HID Report ********************/
    HID_DESCRIPTOR_SIZE, // bLength
    HID_DESCRIPTOR,      // bDescriptorType
    LSB(HID_VERSION),    // bcdHID (LSB)
    MSB(HID_VERSION),    // bcdHID (MSB)
    0x00,                // bCountryCode
    0x01,                // bNumDescriptors
    REPORT_DESCRIPTOR,   // bDescriptorType
    NULL,                // wDescriptorLength (LSB)
    NULL,                // wDescriptorLength (MSB)

    /******************** Descriptor HID IN endpoint ********************/
    0x07,                   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
    HID_IN_EP,              /* bEndpointAddress: Endpoint Address (IN) */
    0x03,                   /* bmAttributes: Interrupt endpoint */
    HID_IN_PACKET_SIZE,     /* wMaxPacketSize */
    0x00,                   /* wMaxPacketSize */
    HID_INTERVAL,           /* bInterval: Polling Interval (10 ms) */

#ifdef HID_OUT_EP
    /******************** Descriptor HID OUT endpoint ********************/
    0x07,                   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
    HID_OUT_EP,             /* bEndpointAddress: Endpoint Address (OUT) */
    0x03,                   /* bmAttributes: Interrupt endpoint */
    HID_OUT_PACKET_SIZE,    /* wMaxPacketSize */
    0x00,                   /* wMaxPacketSize */
    HID_INTERVAL,           /* bInterval: Polling Interval (10 ms) */
#endif
};

static uint8_t HIDDefaultReportDescriptor[] = {
    USAGE_PAGE(2),      LSB(0xFF00), MSB(0xFF00), //
    USAGE(2),           LSB(0x0200), MSB(0x0200), //
    COLLECTION(1),      0x01,                     // Collection (Application)

    REPORT_SIZE(1),     0x08, // 8 bits
    LOGICAL_MINIMUM(1), 0x80, //
    LOGICAL_MAXIMUM(1), 0x7F, //

    REPORT_COUNT(1),    64,   //
    USAGE(1),           0x01, //
    INPUT(1),           0x02, // Data, Var, Abs

    REPORT_COUNT(1),    64,   //
    USAGE(1),           0x02, //
    OUTPUT(1),          0x02, // Data, Var, Abs

    END_COLLECTION(0),
};

#define USBD_HID_Report HIDDefaultReportDescriptor

USB_OTG_CORE_HANDLE* USBDevHandle;

void (*USBD_HID_OutputReport_Event)(const uint8_t* reportBuffer) = NULL;

/**
 * @brief  USBD_HID_Init
 *         Initialize the HID interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_HID_Init(void* pdev, uint8_t cfgidx)
{
    printf("USBD_HID_Init\n");
    USBDevHandle = pdev;

    CircularBufferConstractor(&SendCircularBuffer, 0);
    SendCircularBuffer.Init(&SendCircularBuffer, SendByteBuffer, sizeof(SendByteBuffer));
    InputBuffer = &SendCircularBuffer;

#if HID_OUT_BUFFER_COUNT
    CircularBufferConstractor(&ReadCircularBuffer, 0);
    ReadCircularBuffer.Init(&ReadCircularBuffer, ReadByteBuffer, sizeof(ReadByteBuffer));
    OutputBuffer = &ReadCircularBuffer;
#endif

    /* Open EP IN */
    DCD_EP_Open(pdev, HID_IN_EP, HID_IN_PACKET_SIZE, USB_OTG_EP_INT);

#ifdef HID_OUT_EP
    /* Open EP OUT */
    DCD_EP_Open(pdev, HID_OUT_EP, HID_OUT_PACKET_SIZE, USB_OTG_EP_INT);
    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(pdev, HID_OUT_EP, RxBuffer, HID_OUT_PACKET_SIZE);
#endif
    return USBD_OK;
}

/**
 * @brief  USBD_HID_Init
 *         DeInitialize the HID layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_HID_DeInit(void* pdev, uint8_t cfgidx)
{
    /* Close EP IN */
    DCD_EP_Close(pdev, HID_IN_EP);

#ifdef HID_OUT_EP
    /* Close EP OUT */
    DCD_EP_Close(pdev, HID_OUT_EP);
#endif
    return USBD_OK;
}

/**
 * @brief  USBD_HID_Setup
 *         Handle the HID specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_HID_Setup(void* pdev, USB_SETUP_REQ* req)
{
    uint16_t len            = 0;
    uint8_t* pbuf           = NULL;
    uint16_t descriptorSize = 0;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            switch (req->bRequest)
            {
                case SET_REPORT:
                    USBD_CtlPrepareRx(pdev, RxBuffer, HID_OUT_PACKET_SIZE);
                    break;
                case SET_PROTOCOL:
                    USBD_HID_Protocol = (uint8_t)(req->wValue);
                    break;

                case GET_PROTOCOL:
                    USBD_CtlSendData(pdev, (uint8_t*)&USBD_HID_Protocol, 1);
                    break;

                case SET_IDLE:
                    USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
                    break;

                case GET_IDLE:
                    USBD_CtlSendData(pdev, (uint8_t*)&USBD_HID_IdleState, 1);
                    break;

                default:
                    return USBD_FAIL;
            }
            break;

        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest)
            {
                case USB_REQ_GET_DESCRIPTOR:
                    if (req->wValue >> 8 == REPORT_DESCRIPTOR)
                    {
                        descriptorSize = sizeof(USBD_HID_Report);

                        len  = MIN(descriptorSize, req->wLength);
                        pbuf = USBD_HID_Report;
                    }
                    else if (req->wValue >> 8 == HID_DESCRIPTOR)
                    {
                        descriptorSize = USBD_HID_Config[18];

                        USBD_HID_Config[25] = LSB(sizeof(USBD_HID_Report));
                        USBD_HID_Config[26] = MSB(sizeof(USBD_HID_Report));

                        len  = MIN(descriptorSize, req->wLength);
                        pbuf = &USBD_HID_Config[18];
                    }

                    USBD_CtlSendData(pdev, pbuf, len);
                    break;

                case USB_REQ_GET_INTERFACE:
                    USBD_CtlSendData(pdev, (uint8_t*)&USBD_HID_AltSet, 1);
                    break;

                case USB_REQ_SET_INTERFACE:
                    USBD_HID_AltSet = (uint8_t)(req->wValue);
                    break;
            }
            break;
        default:
            return USBD_FAIL;
    }
    return USBD_OK;
}

uint32_t USBD_HID_PutInputReport(uint8_t* report, uint16_t len)
{
    if (!USBDevHandle)
        return 0;

    if (USBDevHandle->dev.device_status != USB_OTG_CONFIGURED)
        return 0;

    uint16_t reportCount = (len + HID_IN_PACKET_SIZE - 1) / HID_IN_PACKET_SIZE;

    uint8_t* reportPoint = report;
    for (int reportIndex = 0; reportIndex < reportCount; reportIndex++)
    {
        if (len < HID_IN_PACKET_SIZE)
        {
            uint8_t reportAlignBuffer[HID_IN_PACKET_SIZE];
            memcpy(reportAlignBuffer, reportPoint, len);
            reportPoint = reportAlignBuffer;
        }
        uint32_t pushSize = InputBuffer->Push(InputBuffer, reportPoint, HID_IN_PACKET_SIZE, false);
        if (pushSize != HID_IN_PACKET_SIZE)
            return len;
        len -= MIN(len, HID_IN_PACKET_SIZE);
    }

    return len ? len : USBD_OK;
}

#if HID_OUT_BUFFER_COUNT
uint32_t USBD_HID_GetOutputReport(uint8_t* report, uint32_t len)
{
    if (!USBDevHandle)
        return 0;

    if (USBDevHandle->dev.device_status != USB_OTG_CONFIGURED)
        return 0;

    len = (len / HID_IN_PACKET_SIZE) * HID_IN_PACKET_SIZE;

    return OutputBuffer->Pop(OutputBuffer, report, len, false);
}
#endif

static uint8_t* USBD_HID_GetCfgDesc(uint8_t speed, uint16_t* length)
{
    USBD_HID_Config[2] = LSB(sizeof(USBD_HID_Config));
    USBD_HID_Config[3] = MSB(sizeof(USBD_HID_Config));

    USBD_HID_Config[25] = LSB(sizeof(USBD_HID_Report));
    USBD_HID_Config[26] = MSB(sizeof(USBD_HID_Report));

    *length = sizeof(USBD_HID_Config);

    return USBD_HID_Config;
}

static uint8_t USBD_HID_DataIn(void* pdev, uint8_t epnum)
{
    if (TxPending == 2)
    {
        TxPending = 0;
        return USBD_OK;
    }

    USB_OTG_EP* ep      = &((USB_OTG_CORE_HANDLE*)pdev)->dev.in_ep[epnum];
    uint16_t    txCount = ep->total_data_len;

    SendCircularBuffer.EndPop(&SendCircularBuffer, txCount);
    if (!SendCircularBuffer.PopSize && !SendCircularBuffer.StartPop(&SendCircularBuffer, HID_IN_PACKET_SIZE, true, false))
    {
        TxPending = 2;
    }

    /* Prepare the available data buffer to be sent on IN endpoint */
    DCD_EP_Tx(pdev, HID_IN_EP, SendCircularBuffer.Buffer + SendCircularBuffer.PopOffset, SendCircularBuffer.PopSize);
    return USBD_OK;
}

static uint8_t USBD_HID_DataOut(void* pdev, uint8_t epnum)
{
#if HID_OUT_BUFFER_COUNT
    /* Get the received data buffer and update the counter */
    uint16_t rxCount  = HID_OUT_PACKET_SIZE;
    uint16_t rxOffset = 0;

    if (RxPending)
    {
        rxOffset = rxCount - RxPending;
        rxCount  = RxPending;
    }

    uint16_t pushedCount = ReadCircularBuffer.Push(&ReadCircularBuffer, RxBuffer + rxOffset, rxCount, true);
    printf("pushedCount: %d, rxCount: %d.\n", pushedCount, rxCount);
    // pushedCount will little then rxCount if CircularBuffer is Full, Set RxPending for Async Data Out Stage
    if (pushedCount < rxCount)
    {
        RxPending = rxCount - pushedCount;
        return USBD_BUSY;
    }
    else
        RxPending = 0;
#endif

    if (USBD_HID_OutputReport_Event)
        USBD_HID_OutputReport_Event(RxBuffer);

#ifdef HID_OUT_EP
    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(pdev, HID_OUT_EP, (uint8_t*)(RxBuffer), HID_OUT_PACKET_SIZE);
#endif
    return USBD_OK;
}

static uint8_t USBD_HID_EP0_RxReady(void* pdev)
{
    USB_OTG_EP* ep = &((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[0];
    if (ep->xfer_buff != RxBuffer)
        return USBD_OK;
    USBD_HID_DataOut(pdev, NULL);
    return USBD_OK;
}

static void StartAsyncSend(void* pdev)
{
    if (TxPending || SendCircularBuffer.PopSize || !SendCircularBuffer.StartPop(&SendCircularBuffer, HID_IN_PACKET_SIZE, true, true))
        return;

    TxPending = 1;
    DCD_EP_Tx(pdev, HID_IN_EP, SendCircularBuffer.Buffer + SendCircularBuffer.PopOffset, HID_IN_PACKET_SIZE);
}

static uint32_t sofCount = 0;

static uint8_t USBD_HID_SOF(void* pdev)
{
    if (((USB_OTG_CORE_HANDLE*)pdev)->dev.device_status != USB_OTG_CONFIGURED)
        return USBD_OK;

    if (sofCount++ < HID_FRAME_INTERVAL)
        return USBD_OK;

    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef txcsrl;

    uint8_t isBusy = 0;

#if HID_OUT_BUFFER_COUNT
    if (RxPending && USBD_HID_DataOut(pdev, NULL))
        isBusy = 1;
#endif

    txcsrl.d8 = ((USB_OTG_CORE_HANDLE*)pdev)->regs.CSRREGS[HID_IN_EP & 0x7f]->TXCSRL;
    if (txcsrl.b.fifo_not_empty)
        isBusy = 1;
    else
        StartAsyncSend(pdev);

    if (isBusy)
        return USBD_OK;

    sofCount = 0;

    return USBD_OK;
}

USBD_Class_cb_TypeDef USBD_HID_cb = {
    USBD_HID_Init,
    USBD_HID_DeInit,
    USBD_HID_Setup,
    NULL,                 /* EP0_TxSent*/
    USBD_HID_EP0_RxReady, /* EP0_RxReady*/
    USBD_HID_DataIn,      /* DataIn*/
    USBD_HID_DataOut,     /* DataOut*/
    USBD_HID_SOF,         /* SOF */
    NULL,
    NULL,
    USBD_HID_GetCfgDesc,
};
