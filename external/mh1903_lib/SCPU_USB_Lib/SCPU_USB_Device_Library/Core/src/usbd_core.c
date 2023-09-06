/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_core.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Peripheral Device Interface low layer.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "usbd_core.h"
#include "usb_bsp.h"
#include "usb_dcd_int.h"
#include "usbd_ioreq.h"
#include "usbd_req.h"

/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Ptivate function prototypes ----------------------------------------------*/

/** @defgroup USBD_CORE
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_CORE_Private_FunctionPrototypes
 * @{
 */
static uint8_t USBD_SetupStage(USB_OTG_CORE_HANDLE* pdev);
static uint8_t USBD_DataOutStage(USB_OTG_CORE_HANDLE* pdev, uint8_t epnum);
static uint8_t USBD_DataInStage(USB_OTG_CORE_HANDLE* pdev, uint8_t epnum);
static uint8_t USBD_SOF(USB_OTG_CORE_HANDLE* pdev);
static uint8_t USBD_Reset(USB_OTG_CORE_HANDLE* pdev);
static uint8_t USBD_Suspend(USB_OTG_CORE_HANDLE* pdev);
static uint8_t USBD_Resume(USB_OTG_CORE_HANDLE* pdev);

#ifdef VBUS_SENSING_ENABLED
static uint8_t USBD_DevConnected(USB_OTG_CORE_HANDLE* pdev);
static uint8_t USBD_DevDisconnected(USB_OTG_CORE_HANDLE* pdev);
#endif

static uint8_t USBD_IsoINIncomplete(USB_OTG_CORE_HANDLE* pdev);
static uint8_t USBD_IsoOUTIncomplete(USB_OTG_CORE_HANDLE* pdev);
static uint8_t USBD_RunTestMode(USB_OTG_CORE_HANDLE* pdev);
/**
 * @}
 */

/** @defgroup USBD_CORE_Private_Variables
 * @{
 */

__IO USB_OTG_TESTMODE_TypeDef SET_TEST_MODE;

USBD_DCD_INT_cb_TypeDef USBD_DCD_INT_cb = {
    USBD_DataOutStage,
    USBD_DataInStage,
    USBD_SetupStage,
    USBD_SOF,

    USBD_Reset,
    USBD_Suspend,
    USBD_Resume,
    USBD_IsoINIncomplete,
    USBD_IsoOUTIncomplete,
#ifdef VBUS_SENSING_ENABLED
    USBD_DevConnected,
    USBD_DevDisconnected,
#endif
};

USBD_DCD_INT_cb_TypeDef* USBD_DCD_INT_fops = &USBD_DCD_INT_cb;
/**
 * @}
 */

/** @defgroup USBD_CORE_Private_Functions
 * @{
 */

/**
 * @brief  USBD_Init
 *         Initailizes the device stack and load the class driver
 * @param  pdev: device instance
 * @param  core_address: USB OTG core ID
 * @param  class_cb: Class callback structure address
 * @param  usr_cb: User callback structure address
 * @retval None
 */
void USBD_Init(USB_OTG_CORE_HANDLE* pdev, USB_OTG_CORE_ID_TypeDef coreID, USBD_DEVICE* pDevice, USBD_Class_cb_TypeDef* class_cb, USBD_Usr_cb_TypeDef* usr_cb)
{
    static bool nvicSet = false;

    pdev->regs.COMMREGS->POWER    = 0x00;
    pdev->regs.ULPIREGS->LINKINFO = 0x51;

    /* Hardware Init */
    USB_OTG_BSP_Init(pdev);

    /*Register class and user callbacks */
    pdev->dev.class_cb   = class_cb;
    pdev->dev.usr_cb     = usr_cb;
    pdev->dev.usr_device = pDevice;

    /* set USB OTG core params */
    DCD_Init(pdev, coreID);

    /* Upon Init call usr callback */
    pdev->dev.usr_cb->Init();

    /* Enable Interrupts */
    USB_OTG_BSP_EnableInterrupt(pdev);

    if (nvicSet == false) {
        nvicSet = true;
        /* Enable USB Interrupt */
        NVIC_InitTypeDef   NVIC_InitStructure;
        NVIC_SetPriorityGrouping(NVIC_PriorityGroup_0);
        NVIC_InitStructure.NVIC_IRQChannel                   = USB_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
}

/**
 * @brief  USBD_DeInit
 *         Re-Initialize th device library
 * @param  pdev: device instance
 * @retval status: status
 */
USBD_Status USBD_DeInit(USB_OTG_CORE_HANDLE* pdev)
{
    // DisConnect device
    DCD_DevDisconnect(pdev);
    USBD_DevDisconnected(pdev);
    return USBD_OK;
}

/**
 * @brief  USBD_SetupStage
 *         Handle the setup stage
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_SetupStage(USB_OTG_CORE_HANDLE* pdev)
{
    USB_SETUP_REQ req;

    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef csr0l;

    USBD_ParseSetupRequest(pdev, &req);
    //printf("bm=%X,b=%X\r\n", req.bmRequest, req.bRequest);

    if (req.wLength) {
        csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);

        csr0l.b.serviced_rxpktrdy = 1;
        USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
    }

    switch (req.bmRequest & 0x1F) {
    case USB_REQ_RECIPIENT_DEVICE:
        USBD_StdDevReq(pdev, &req);
        break;

    case USB_REQ_RECIPIENT_INTERFACE:
        USBD_StdItfReq(pdev, &req);
        break;

    case USB_REQ_RECIPIENT_ENDPOINT:
        USBD_StdEPReq(pdev, &req);
        break;

    default:
        DCD_EP_Stall(pdev, req.bmRequest & 0x80);
        break;
    }

    if (!req.wLength) {
        csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
        // Protect zero length packet.
        csr0l.b.data_end = 1;
        csr0l.b.serviced_rxpktrdy = 1;
        USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
    }

    return USBD_OK;
}

/**
 * @brief  USBD_DataOutStage
 *         Handle data out stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t USBD_DataOutStage(USB_OTG_CORE_HANDLE* pdev, uint8_t epnum)
{
    USB_OTG_EP* ep;
    ep = &pdev->dev.out_ep[epnum & 0x7F];
    if (epnum == 0) {
        if (pdev->dev.device_state == USB_OTG_EP0_DATA_OUT) {
            if ((pdev->dev.class_cb->EP0_RxReady != NULL) && (pdev->dev.device_status == USB_OTG_CONFIGURED)) {
                pdev->dev.class_cb->EP0_RxReady(pdev);
            }
            ep->xfer_buff  = pdev->dev.setup_packet;
            ep->xfer_count = 0;
            ep->xfer_len   = 8;
        }
    } else {
        if (ep->rem_data_len > ep->maxpacket) {
            ep->rem_data_len -= ep->maxpacket;
            USB_OTG_EPStartXfer(pdev, ep);
        } else {
            if ((pdev->dev.class_cb->DataOut != NULL) && (pdev->dev.device_status == USB_OTG_CONFIGURED)) {
                pdev->dev.class_cb->DataOut(pdev, epnum);
            }
        }
    }
    return USBD_OK;
}

/**
 * @brief  USBD_DataInStage
 *         Handle data in stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t USBD_DataInStage(USB_OTG_CORE_HANDLE* pdev, uint8_t epnum)
{
    USB_OTG_EP* ep;
    ep = &pdev->dev.in_ep[epnum];
    if (epnum == 0) {
        if (ep->xfer_count > 1) {
            USB_OTG_EP0StartXfer(pdev, ep);
        } else if (ep->xfer_count == 1) {
            ep->xfer_count = 0;
            if (pdev->dev.device_state == USB_OTG_EP0_DATA_IN) {
                if ((pdev->dev.class_cb->EP0_TxSent != NULL) && (pdev->dev.device_status == USB_OTG_CONFIGURED)) {
                    pdev->dev.class_cb->EP0_TxSent(pdev);
                }
                USBD_CtlReceiveStatus(pdev);
            } else if (pdev->dev.test_mode == 1) {
                USBD_RunTestMode(pdev);
                pdev->dev.test_mode = 0;
            }
        }
    } else {
        if (ep->xfer_count > 1) {
            USB_OTG_EPStartXfer(pdev, ep);
        } else if (ep->xfer_count == 1 || ep->total_data_len == 0) {
            ep->xfer_count = 0;
            if ((pdev->dev.class_cb->DataIn != NULL) && (pdev->dev.device_status == USB_OTG_CONFIGURED)) {
                pdev->dev.class_cb->DataIn(pdev, epnum);
            }
        }
    }

    return USBD_OK;
}

/**
 * @brief  USBD_RunTestMode
 *         Launch test mode process
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_RunTestMode(USB_OTG_CORE_HANDLE* pdev)
{
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->TESTMODE, SET_TEST_MODE.d8);
    return USBD_OK;
}

/**
 * @brief  USBD_Reset
 *         Handle Reset event
 * @param  pdev: device instance
 * @retval status
 */

static uint8_t USBD_Reset(USB_OTG_CORE_HANDLE* pdev)
{
    USB_OTG_EP *ep;

    // Reset all ep config to default
    for (int i = 0; i < pdev->cfg.dev_endpoints; i++) {
        ep           = &pdev->dev.in_ep[i];
        ep->num      = i;
        ep->is_in    = 1;
        ep->is_stall = 0;
        ep->type     = EP_TYPE_CTRL;

        ep->is_fifo_allocated = 0;

        ep->maxpacket      = 0;
        ep->xfer_buff      = 0;
        ep->xfer_len       = 0;
        ep->rem_data_len   = 0;
        ep->total_data_len = 0;

        ep           = &pdev->dev.out_ep[i];
        ep->num      = i;
        ep->is_in    = 0;
        ep->is_stall = 0;
        ep->type     = EP_TYPE_CTRL;

        ep->is_fifo_allocated = 0;

        ep->maxpacket      = 0;
        ep->xfer_buff      = 0;
        ep->xfer_len       = 0;
        ep->rem_data_len   = 0;
        ep->total_data_len = 0;
    }

    /* Open EP0 OUT */
    DCD_EP_Open(pdev, 0x00, USB_OTG_MAX_EP0_SIZE, EP_TYPE_CTRL);

    /* Open EP0 IN */
    DCD_EP_Open(pdev, 0x80, USB_OTG_MAX_EP0_SIZE, EP_TYPE_CTRL);

    // Prepare EP0 Rx
    DCD_EP_PrepareRx(pdev, 0x00, pdev->dev.setup_packet, 8);

    /* Upon Reset call usr call back */
    pdev->dev.device_status = USB_OTG_DEFAULT;
    pdev->dev.usr_cb->DeviceReset(pdev->cfg.speed);

    return USBD_OK;
}

/**
 * @brief  USBD_Resume
 *         Handle Resume event
 * @param  pdev: device instance
 * @retval status
 */

static uint8_t USBD_Resume(USB_OTG_CORE_HANDLE* pdev)
{
    /* Upon Resume call usr call back */
    pdev->dev.usr_cb->DeviceResumed();
    pdev->dev.device_status = pdev->dev.device_old_status;
    pdev->dev.device_status = USB_OTG_CONFIGURED;
    return USBD_OK;
}

/**
 * @brief  USBD_Suspend
 *         Handle Suspend event
 * @param  pdev: device instance
 * @retval status
 */

static uint8_t USBD_Suspend(USB_OTG_CORE_HANDLE* pdev)
{
    pdev->dev.device_old_status = pdev->dev.device_status;
    pdev->dev.device_status     = USB_OTG_SUSPENDED;
    /* Upon Resume call usr call back */
    pdev->dev.usr_cb->DeviceSuspended();
    return USBD_OK;
}

/**
 * @brief  USBD_SOF
 *         Handle SOF event
 * @param  pdev: device instance
 * @retval status
 */

static uint8_t USBD_SOF(USB_OTG_CORE_HANDLE* pdev)
{
    if (pdev->dev.class_cb->SOF) {
        pdev->dev.class_cb->SOF(pdev);
    }
    return USBD_OK;
}
/**
 * @brief  USBD_SetCfg
 *        Configure device and start the interface
 * @param  pdev: device instance
 * @param  cfgidx: configuration index
 * @retval status
 */

USBD_Status USBD_SetCfg(USB_OTG_CORE_HANDLE* pdev, uint8_t cfgidx)
{
    pdev->dev.class_cb->Init(pdev, cfgidx);

    /* Upon set config call usr call back */
    pdev->dev.usr_cb->DeviceConfigured();
    return USBD_OK;
}

/**
 * @brief  USBD_ClrCfg
 *         Clear current configuration
 * @param  pdev: device instance
 * @param  cfgidx: configuration index
 * @retval status: USBD_Status
 */
USBD_Status USBD_ClrCfg(USB_OTG_CORE_HANDLE* pdev, uint8_t cfgidx)
{
    pdev->dev.class_cb->DeInit(pdev, cfgidx);
    return USBD_OK;
}

/**
 * @brief  USBD_IsoINIncomplete
 *         Handle iso in incomplete event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_IsoINIncomplete(USB_OTG_CORE_HANDLE* pdev)
{
    pdev->dev.class_cb->IsoINIncomplete(pdev);
    return USBD_OK;
}

/**
 * @brief  USBD_IsoOUTIncomplete
 *         Handle iso out incomplete event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_IsoOUTIncomplete(USB_OTG_CORE_HANDLE* pdev)
{
    pdev->dev.class_cb->IsoOUTIncomplete(pdev);
    return USBD_OK;
}

#ifdef VBUS_SENSING_ENABLED
/**
 * @brief  USBD_DevConnected
 *         Handle device connection event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_DevConnected(USB_OTG_CORE_HANDLE* pdev)
{
    pdev->dev.usr_cb->DeviceConnected();
    pdev->dev.connection_status = 1;
    return USBD_OK;
}

/**
 * @brief  USBD_DevDisconnected
 *         Handle device disconnection event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_DevDisconnected(USB_OTG_CORE_HANDLE* pdev)
{
    pdev->dev.usr_cb->DeviceDisconnected();
    pdev->dev.class_cb->DeInit(pdev, 0);
    pdev->dev.connection_status = 0;
    return USBD_OK;
}
#endif
/**
 * @}
 */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
