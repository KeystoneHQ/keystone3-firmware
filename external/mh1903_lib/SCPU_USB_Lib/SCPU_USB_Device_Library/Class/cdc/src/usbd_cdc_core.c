/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_cdc_core.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : This file provides the high layer firmware functions
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "log_print.h"
#include "protocol_parse.h"
#include "assert.h"
#include "user_msg.h"

/** @defgroup usbd_cdc
 * @brief usbd core module
 * @{
 */

/** @defgroup usbd_cdc_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup usbd_cdc_Private_Defines
 * @{
 */
/**
 * @}
 */

/** @defgroup usbd_cdc_Private_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup usbd_cdc_Private_FunctionPrototypes
 * @{
 */

/*********************************************
   CDC Device library callbacks
 *********************************************/
static uint8_t usbd_cdc_Init(void* pdev, uint8_t cfgidx);
static uint8_t usbd_cdc_DeInit(void* pdev, uint8_t cfgidx);
static uint8_t usbd_cdc_Setup(void* pdev, USB_SETUP_REQ* req);
static uint8_t usbd_cdc_EP0_RxReady(void* pdev);
static uint8_t usbd_cdc_DataIn(void* pdev, uint8_t epnum);
static uint8_t usbd_cdc_DataOut(void* pdev, uint8_t epnum);
static uint8_t usbd_cdc_SOF(void* pdev);


static uint8_t* USBD_cdc_GetCfgDesc(uint8_t speed, uint16_t* length);

static void USBD_cdc_SendBuffer(const uint8_t *data, uint32_t len);
static void USBD_cdc_SendCallback(void);

extern CDC_IF_Prop_TypeDef APP_FOPS;

__ALIGN_BEGIN uint8_t usbd_cdc_CfgDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END;

__ALIGN_BEGIN static __IO uint32_t usbd_cdc_AltSet __ALIGN_END = 0;

__ALIGN_BEGIN uint8_t USB_Rx_Buffer[CDC_DATA_OUT_PACKET_SIZE] __ALIGN_END;

__ALIGN_BEGIN uint8_t CmdBuff[CDC_CMD_PACKET_SIZE] __ALIGN_END;

static CircularBufferStruct SendCircularBuffer;
static CircularBufferStruct ReadCircularBuffer;

//static uint8_t SendByteBuffer[CDC_APP_TX_DATA_SIZE];
//static uint8_t ReadByteBuffer[CDC_APP_RX_DATA_SIZE];

//static volatile uint32_t TxPending = 0;
//static volatile uint32_t RxPending = 0;
//static volatile uint32_t sofCount  = 0;

extern USB_OTG_CORE_HANDLE g_usbDev;

#define CDC_TX_MAX_LENGTH               1024
#define CDC_PACKET_SIZE                 64

static uint8_t g_cdcSendBuffer[CDC_TX_MAX_LENGTH];
static uint32_t g_cdcSendIndex = 0;

static uint32_t cdcCmd = 0xFF;
static uint32_t cdcLen = 0;

CDC_Data_TypeDef CDCData = {
    0,                   //
    &SendCircularBuffer, //
    &ReadCircularBuffer  //
};

/* CDC interface class callbacks structure */
USBD_Class_cb_TypeDef USBD_CDC_cb = {
    usbd_cdc_Init,
    usbd_cdc_DeInit,

    usbd_cdc_Setup,
    NULL, /* EP0_TxSent, */
    usbd_cdc_EP0_RxReady,

    usbd_cdc_DataIn,
    usbd_cdc_DataOut,
    usbd_cdc_SOF,

    NULL,
    NULL,
    USBD_cdc_GetCfgDesc,

    USBD_WinUSBOSStrDescriptor,
};

#ifdef USBD_ENABLE_MSC
/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t usbd_cdc_CfgDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END = {
    /*Configuration Descriptor*/
    0x09,                              /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
    LOBYTE(USB_CDC_CONFIG_DESC_SIZ),   /* wTotalLength:no of returned bytes */
    HIBYTE(USB_CDC_CONFIG_DESC_SIZ),   /* */
    0x01,                              /* bNumInterfaces: 2 interface */
    0x01,                              /* bConfigurationValue: Configuration value */
    0x00,                              /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,                              /* bmAttributes: self powered */
    0x32,                              /* MaxPower 0 mA */

    /*---------------------------------------------------------------------------*/
    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface, zero based index of this interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: 2 endpoints used */
    0xFF,   /* bInterfaceClass: vendor */
    0xFF,   /* bInterfaceSubClass: */
    0xFF,   /* bInterfaceProtocol: */
    0x02,   /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,                                       /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
    CDC_IN_EP,                                  /* bEndpointAddress */
    0x03,                                       /* bmAttributes: interrupt */
    LOBYTE(64),        /* wMaxPacketSize: */
    HIBYTE(64),
    0x01,                                       /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,                                       /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                                 /* bEndpointAddress */
    0x03,                                       /* bmAttributes: interrupt */
    LOBYTE(64),        /* wMaxPacketSize: */
    HIBYTE(64),
    0x01,                                       /* bInterval: ignore for Bulk transfer */
};
#else
#define WINUSB_CONFIG_DESC_SIZE 32
#define CDC_DATA_HS_MAX_PACKET_SIZE                 64U  /* Endpoint IN & OUT Packet size */
static __ALIGN_BEGIN uint8_t USBD_CDC_CfgHSDesc[WINUSB_CONFIG_DESC_SIZE] __ALIGN_END = {
    /*Configuration Descriptor*/
    0x09,   /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
    WINUSB_CONFIG_DESC_SIZE,                /* wTotalLength:no of returned bytes */
    0x00,
    0x01,   /* bNumInterfaces: 1 interface for WINUSB */
    0x01,   /* bConfigurationValue: Configuration value */
    USBD_IDX_CONFIG_STR,   /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,   /* bmAttributes: self powered */
    0x32,   /* MaxPower 50*2 mA */

    /*---------------------------------------------------------------------------*/
    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
    0x00,   /* bInterfaceNumber: Number of Interface, zero based index of this interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0xFF,   /* bInterfaceClass: vendor */
//  0x00,   /* bInterfaceClass: vendor */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                        /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
    0x00,                              /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
    CDC_IN_EP,                         /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
    0x00                               /* bInterval: ignore for Bulk transfer */
};

#endif


/** @defgroup usbd_cdc_Private_Functions
 * @{
 */

/**
 * @brief  usbd_cdc_Init
 *         Initilaize the CDC interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t usbd_cdc_Init(void* pdev, uint8_t cfgidx)
{
    //CircularBufferConstractor(&SendCircularBuffer, 0);
    //SendCircularBuffer.Init(&SendCircularBuffer, SendByteBuffer, sizeof(SendByteBuffer));
    //CircularBufferConstractor(&ReadCircularBuffer, 0);
    //ReadCircularBuffer.Init(&ReadCircularBuffer, ReadByteBuffer, sizeof(ReadByteBuffer));
    //
    //RxPending = 0;
    //TxPending = 0;
    //sofCount  = 0;

    /* Open EP IN */
    DCD_EP_Open(pdev, CDC_IN_EP, CDC_DATA_IN_PACKET_SIZE, USB_OTG_EP_INT);

    /* Open EP OUT */
    DCD_EP_Open(pdev, CDC_OUT_EP, CDC_DATA_OUT_PACKET_SIZE, USB_OTG_EP_INT);

    //DCD_EP_Open(pdev, 0x81, 8, USB_OTG_EP_INT);

    /* Initialize the Interface physical components */
    APP_FOPS.pIf_Init();

    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(pdev, CDC_OUT_EP, (uint8_t*)(USB_Rx_Buffer), CDC_DATA_OUT_PACKET_SIZE);
    printf("cdc init\r\n");
    return USBD_OK;
}

/**
 * @brief  usbd_cdc_Init
 *         DeInitialize the CDC layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t usbd_cdc_DeInit(void* pdev, uint8_t cfgidx)
{
    /* Open EP IN */
    DCD_EP_Close(pdev, CDC_IN_EP);

    /* Open EP OUT */
    DCD_EP_Close(pdev, CDC_OUT_EP);

    /* Restore default state of the Interface physical components */
    APP_FOPS.pIf_DeInit();

    return USBD_OK;
}

/**
 * @brief  usbd_cdc_Setup
 *         Handle the CDC specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t usbd_cdc_Setup(void* pdev, USB_SETUP_REQ* req)
{
    uint16_t len  = USB_CDC_DESC_SIZ;
    uint8_t* pbuf = usbd_cdc_CfgDesc + 9;

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    /* CDC Class Requests -------------------------------*/
    case USB_REQ_TYPE_CLASS:
        /* Check if the request is a data setup packet */
        if (req->wLength) {
            /* Check if the request is Device-to-Host */
            if (req->bmRequest & 0x80) {
                /* Get the data to be sent to Host from interface layer */
                APP_FOPS.pIf_Ctrl(req->bRequest, CmdBuff, req->wLength);

                /* Send the data to the host */
                USBD_CtlSendData(pdev, CmdBuff, req->wLength);
            } else { /* Host-to-Device requeset */
                /* Set the value of the current command to be processed */
                cdcCmd = req->bRequest;
                cdcLen = req->wLength;

                /* Prepare the reception of the buffer over EP0
                Next step: the received data will be managed in usbd_cdc_EP0_TxSent()
                function. */
                USBD_CtlPrepareRx(pdev, CmdBuff, req->wLength);
            }
        } else { /* No Data request */
            /* Transfer the command to the interface layer */
            APP_FOPS.pIf_Ctrl(req->bRequest, NULL, 0);
        }
        return USBD_OK;

    /* Standard Requests -------------------------------*/
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest) {
        case USB_REQ_GET_DESCRIPTOR:
            if ((req->wValue >> 8) == CDC_DESCRIPTOR_TYPE) {
                pbuf = usbd_cdc_CfgDesc + 9 + (9 * USBD_ITF_MAX_NUM);
                len  = MIN(USB_CDC_DESC_SIZ, req->wLength);
            }

            USBD_CtlSendData(pdev, pbuf, len);
            break;

        case USB_REQ_GET_INTERFACE:
            USBD_CtlSendData(pdev, (uint8_t*)&usbd_cdc_AltSet, 1);
            break;

        case USB_REQ_SET_INTERFACE:
            if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM) {
                usbd_cdc_AltSet = (uint8_t)(req->wValue);
            } else {
                /* Call the error management function (command will be nacked */
                USBD_CtlError(pdev, req);
            }
            break;
        }
    default:
        USBD_CtlError(pdev, req);
        return USBD_FAIL;
    }
}

/**
 * @brief  usbd_cdc_EP0_RxReady
 *         Data received on control endpoint
 * @param  pdev: device device instance
 * @retval status
 */
static uint8_t usbd_cdc_EP0_RxReady(void* pdev)
{
    USB_OTG_EP* ep = &((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[0];
    if (ep->xfer_buff != CmdBuff)
        return USBD_OK;

    // Will fired when CDC Set Cmd request callback
    if (cdcCmd != NO_CMD) {
        /* Process the data */
        APP_FOPS.pIf_Ctrl(cdcCmd, CmdBuff, cdcLen);

        /* Reset the command variable to default value */
        cdcCmd = NO_CMD;
    }

    return USBD_OK;
}

/**
 * @brief  usbd_audio_DataIn
 *         Data sent on non-control IN endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t usbd_cdc_DataIn(void* pdev, uint8_t epnum)
{
    ///* Prepare the available data buffer to be sent on IN endpoint */
    USBD_cdc_SendCallback();
    return USBD_OK;
}

/**
 * @brief  usbd_cdc_DataOut
 *         Data received on non-control Out endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t usbd_cdc_DataOut(void* pdev, uint8_t epnum)
{
void PushDataToField(uint8_t *data, uint16_t len);
    USB_OTG_EP* ep = &((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum];
    uint16_t rxCount  = ep->xfer_count;
    //PrintArray("WEBUSB rx", USB_Rx_Buffer, rxCount);
    PushDataToField(USB_Rx_Buffer, rxCount);
    PubValueMsg(SPRING_MSG_GET, rxCount);
    DCD_EP_PrepareRx(pdev, CDC_OUT_EP, (uint8_t*)(USB_Rx_Buffer), CDC_DATA_OUT_PACKET_SIZE);
    return USBD_OK;
}

static uint8_t usbd_cdc_SOF(void* pdev)
{
    return USBD_OK;
}

/**
 * @brief  Handle_USBAsynchXfer
 *         Send data to USB
 * @param  pdev: instance
 * @retval None
 */
//static void Handle_USBAsynchXfer(void* pdev)
//{
//if (TxPending || SendCircularBuffer.PopSize || !SendCircularBuffer.StartPop(&SendCircularBuffer, SendCircularBuffer.Length, true, true))
//    return;
//
//TxPending = 1;
//DCD_EP_Tx(pdev, CDC_IN_EP, SendCircularBuffer.Buffer + SendCircularBuffer.PopOffset, SendCircularBuffer.PopSize);
//}

/**
 * @brief  USBD_cdc_GetCfgDesc
 *         Return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_cdc_GetCfgDesc(uint8_t speed, uint16_t* length)
{
#ifdef USBD_ENABLE_MSC
    *length = sizeof(usbd_cdc_CfgDesc);
    return usbd_cdc_CfgDesc;
#else
    *length = sizeof(USBD_CDC_CfgHSDesc);
    return USBD_CDC_CfgHSDesc;
#endif
}

void USBD_cdc_SendBuffer_Cb(const uint8_t *data, uint32_t len)
{
    USBD_cdc_SendBuffer(data, len);
}

static void USBD_cdc_SendBuffer(const uint8_t *data, uint32_t len)
{
    uint32_t sendLen;
    uint32_t remaining;
    g_cdcSendIndex = 0;

    ASSERT(len <= CDC_TX_MAX_LENGTH);
    memcpy(g_cdcSendBuffer, data, len);

    while (g_cdcSendIndex < len) {
        remaining = len - g_cdcSendIndex;
        sendLen = remaining > CDC_PACKET_SIZE ? CDC_PACKET_SIZE : remaining;

        PrintArray("sendBuf USBD_cdc_SendBuffer", g_cdcSendBuffer + g_cdcSendIndex, sendLen);
        DCD_EP_Tx(&g_usbDev, CDC_IN_EP, g_cdcSendBuffer + g_cdcSendIndex, sendLen);

        g_cdcSendIndex += sendLen;
    }

    g_cdcSendIndex = 0;
    printf("usb send over\n");
}

static void USBD_cdc_SendCallback(void)
{
    printf("USBD_cdc_SendCallback usb send over\n");
}

