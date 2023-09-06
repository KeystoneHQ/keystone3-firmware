/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_req.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : This file provides the standard USB requests following chapter 9.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "usbd_req.h"
#include "usbd_ioreq.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Ptivate function prototypes ----------------------------------------------*/

extern __IO USB_OTG_TESTMODE_TypeDef SET_TEST_MODE;

__ALIGN_BEGIN uint32_t USBD_ep_status   __ALIGN_END = 0;
__ALIGN_BEGIN uint32_t USBD_default_cfg __ALIGN_END = 0;
__ALIGN_BEGIN uint32_t USBD_cfg_status  __ALIGN_END = 0;
__ALIGN_BEGIN uint8_t                   USBD_StrDesc[USB_MAX_STR_DESC_SIZ] __ALIGN_END;
/**
 * @}
 */

/** @defgroup USBD_REQ_Private_FunctionPrototypes
 * @{
 */
static void USBD_WinUSBGetDescriptor(USB_OTG_CORE_HANDLE *pdev, USB_SETUP_REQ *req);

static void USBD_GetDescriptor(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req);

static void USBD_SetAddress(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req);

static void USBD_SetConfig(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req);

static void USBD_GetConfig(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req);

static void USBD_GetStatus(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req);

static void USBD_SetFeature(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req);

static void USBD_ClrFeature(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req);

static uint8_t USBD_GetLen(uint8_t* buf);
/**
 * @}
 */

/**
 * @brief  USBD_StdDevReq
 *         Handle standard usb device requests
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval status
 */
USBD_Status USBD_StdDevReq(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    USBD_Status ret = USBD_OK;

    //printf("bRequest=%X\r\n", req->bRequest);
    switch (req->bRequest) {
    case USB_REQ_GET_DESCRIPTOR:
        USBD_GetDescriptor(pdev, req);
        break;
    //case USB_REQ_MS_VENDOR_CODE:
    case 0xA0:
        USBD_WinUSBGetDescriptor(pdev, req);
        break;
    case USB_REQ_SET_ADDRESS:
        pdev->dev.addr_param.SetAddress_Flag = 1;
        pdev->dev.addr_param.Address_Value   = (uint8_t)(req->wValue) & 0x7F;
        USBD_SetAddress(pdev, req);
        break;

    case USB_REQ_SET_CONFIGURATION:
        USBD_SetConfig(pdev, req);
        break;

    case USB_REQ_GET_CONFIGURATION:
        USBD_GetConfig(pdev, req);
        break;

    case USB_REQ_GET_STATUS:
        USBD_GetStatus(pdev, req);
        break;

    case USB_REQ_SET_FEATURE:
        USBD_SetFeature(pdev, req);
        break;

    case USB_REQ_CLEAR_FEATURE:
        USBD_ClrFeature(pdev, req);
        break;
    case USB_REQ_SET_DESCRIPTOR:
        break;
    default:
        USBD_CtlError(pdev, req);
        break;
    }

    return ret;
}

/**
 * @brief  USBD_StdItfReq
 *         Handle standard usb interface requests
 * @param  pdev: USB OTG device instance
 * @param  req: usb request
 * @retval status
 */
USBD_Status USBD_StdItfReq(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    USBD_Status ret = USBD_OK;
    switch (pdev->dev.device_status) {
    case USB_OTG_CONFIGURED:
        if (LOBYTE(req->wIndex) > USBD_ITF_MAX_NUM || pdev->dev.class_cb->Setup(pdev, req) != USBD_OK) {
            USBD_CtlError(pdev, req);
        }
        break;

    default:
        USBD_CtlError(pdev, req);
        break;
    }
    return ret;
}

/**
 * @brief  USBD_StdEPReq
 *         Handle standard usb endpoint requests
 * @param  pdev: USB OTG device instance
 * @param  req: usb request
 * @retval status
 */
USBD_Status USBD_StdEPReq(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{

    uint8_t     ep_addr;
    USBD_Status ret = USBD_OK;
    ep_addr         = LOBYTE(req->wIndex);

    switch (req->bRequest) {
    case USB_REQ_SET_FEATURE:
        switch (pdev->dev.device_status) {
        case USB_OTG_ADDRESSED:
            if ((ep_addr != 0x00) && (ep_addr != 0x80)) {
                DCD_EP_Stall(pdev, ep_addr);
            }
            break;

        case USB_OTG_CONFIGURED:
            if (req->wValue == USB_FEATURE_EP_HALT) {
                if ((ep_addr != 0x00) && (ep_addr != 0x80)) {
                    DCD_EP_Stall(pdev, ep_addr);
                }
            }
            pdev->dev.class_cb->Setup(pdev, req);
            USBD_CtlSendStatus(pdev);

            break;

        default:
            USBD_CtlError(pdev, req);
            break;
        }
        break;

    case USB_REQ_CLEAR_FEATURE:
        switch (pdev->dev.device_status) {
        case USB_OTG_ADDRESSED:
            if ((ep_addr != 0x00) && (ep_addr != 0x80)) {
                DCD_EP_Stall(pdev, ep_addr);
            }
            break;

        case USB_OTG_CONFIGURED:
            if (req->wValue == USB_FEATURE_EP_HALT) {
                if ((ep_addr != 0x00) && (ep_addr != 0x80)) {
                    DCD_EP_ClrStall(pdev, ep_addr);
                    pdev->dev.class_cb->Setup(pdev, req);
                }
                USBD_CtlSendStatus(pdev);
            }
            break;

        default:
            USBD_CtlError(pdev, req);
            break;
        }
        break;

    case USB_REQ_GET_STATUS:
        switch (pdev->dev.device_status) {
        case USB_OTG_ADDRESSED:
            if ((ep_addr != 0x00) && (ep_addr != 0x80)) {
                DCD_EP_Stall(pdev, ep_addr);
            }
            break;

        case USB_OTG_CONFIGURED:

            if ((ep_addr & 0x80) == 0x80) {
                if (pdev->dev.in_ep[ep_addr & 0x7F].is_stall) {
                    USBD_ep_status = 0x0001;
                } else {
                    USBD_ep_status = 0x0000;
                }
            } else if ((ep_addr & 0x80) == 0x00) {
                if (pdev->dev.out_ep[ep_addr].is_stall) {
                    USBD_ep_status = 0x0001;
                }

                else {
                    USBD_ep_status = 0x0000;
                }
            }
            USBD_CtlSendData(pdev, (uint8_t*)&USBD_ep_status, 2);
            break;

        default:
            USBD_CtlError(pdev, req);
            break;
        }
        break;

    default:
        break;
    }
    return ret;
}



static void USBD_WinUSBGetDescriptor(USB_OTG_CORE_HANDLE *pdev, USB_SETUP_REQ *req)
{
    uint16_t len;
    uint8_t *pbuf;

    switch (req->wIndex) {
    case 0x04: // compat ID
        pbuf = pdev->dev.usr_device->GetWinUSBOSFeatureDescriptor(&len);
        break;
    case 0x05:
        pbuf = pdev->dev.usr_device->GetWinUSBOSPropertyDescriptor(&len);
        break;

    default:
        USBD_CtlError(pdev, req);
        return;
    }

    if ((len != 0) && (req->wLength != 0)) {
        len = MIN(len, req->wLength);
        USBD_CtlSendData(pdev, pbuf, len);
    }
}


/**
 * @brief  USBD_GetDescriptor
 *         Handle Get Descriptor requests
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval status
 */
static void USBD_GetDescriptor(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    uint16_t len;
    uint8_t* pbuf = NULL;
    //    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef csr0l;
    //    csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);

    //printf("wValue=%X\r\n", req->wValue);
    switch (req->wValue >> 8) {
    case USB_DESC_TYPE_DEVICE:
        pbuf = pdev->dev.usr_device->GetDeviceDescriptor(pdev->cfg.speed, &len);
        len  = MIN(len, req->wLength);
        break;

    case USB_DESC_TYPE_CONFIGURATION:
        pbuf    = (uint8_t*)pdev->dev.class_cb->GetConfigDescriptor(pdev->cfg.speed, &len);
        pbuf[1] = USB_DESC_TYPE_CONFIGURATION;

        pdev->dev.pConfig_descriptor = pbuf;
        break;

    case USB_DESC_TYPE_STRING:
        switch ((uint8_t)(req->wValue)) {
        case USBD_IDX_LANGID_STR:
            pbuf = pdev->dev.usr_device->GetLangIDStrDescriptor(pdev->cfg.speed, &len);
            break;

        case USBD_IDX_MFC_STR:
            pbuf = pdev->dev.usr_device->GetManufacturerStrDescriptor(pdev->cfg.speed, &len);
            break;

        case USBD_IDX_PRODUCT_STR:
            pbuf = pdev->dev.usr_device->GetProductStrDescriptor(pdev->cfg.speed, &len);
            break;

        case USBD_IDX_SERIAL_STR:
            pbuf = pdev->dev.usr_device->GetSerialStrDescriptor(pdev->cfg.speed, &len);
            break;

        case USBD_IDX_CONFIG_STR:
            pbuf = pdev->dev.usr_device->GetConfigurationStrDescriptor(pdev->cfg.speed, &len);
            break;

        case USBD_IDX_INTERFACE_STR:
            pbuf = pdev->dev.usr_device->GetInterfaceStrDescriptor(pdev->cfg.speed, &len);
            break;

        case 0xEE: // OS String
            if (pdev->dev.usr_device->GetWinUSBOSStrDescriptor != NULL) {
                pbuf = pdev->dev.usr_device->GetWinUSBOSStrDescriptor(&len);
            }
            break;

        default:
#ifdef USB_SUPPORT_USER_STRING_DESC
            pbuf = pdev->dev.class_cb->GetUsrStrDescriptor(pdev->cfg.speed, (req->wValue), &len);
            break;
#else
            USBD_CtlError(pdev, req);
            return;
#endif /* USBD_CtlError(pdev , req); */
        }
        break;
    case USB_DESC_TYPE_DEVICE_QUALIFIER:
        pbuf = pdev->dev.usr_device->GetQualiferDescriptor(pdev->cfg.speed, &len);
        len  = MIN(len, req->wLength);
        break;

    case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
        USBD_CtlError(pdev, req);
        return;

    default:
        USBD_CtlError(pdev, req);
        return;
    }

    if ((len != 0) && (req->wLength != 0)) {

        len = MIN(len, req->wLength);

        USBD_CtlSendData(pdev, pbuf, len);
    }
}

/**
 * @brief  USBD_SetAddress
 *         Set device address
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval status
 */
static void USBD_SetAddress(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    uint8_t dev_addr;

    if ((req->wIndex == 0) && (req->wLength == 0)) {
        dev_addr = (uint8_t)(req->wValue) & 0x7F;

        if (pdev->dev.device_status == USB_OTG_CONFIGURED) {
            USBD_CtlError(pdev, req);
        } else {
            pdev->dev.device_address = dev_addr;
            //      DCD_EP_SetAddress(pdev, dev_addr);
            //      USBD_CtlSendStatus(pdev);

            if (dev_addr != 0) {
                pdev->dev.device_status = USB_OTG_ADDRESSED;
            } else {
                pdev->dev.device_status = USB_OTG_DEFAULT;
            }
        }
    } else {
        USBD_CtlError(pdev, req);
    }
}

/**
 * @brief  USBD_SetConfig
 *         Handle Set device configuration request
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval status
 */
static void USBD_SetConfig(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{

    static uint8_t cfgidx;

    cfgidx = (uint8_t)(req->wValue);

    if (cfgidx > USBD_CFG_MAX_NUM) {
        USBD_CtlError(pdev, req);
    } else {
        switch (pdev->dev.device_status) {
        case USB_OTG_ADDRESSED:
            if (cfgidx) {
                pdev->dev.device_config = cfgidx;
                pdev->dev.device_status = USB_OTG_CONFIGURED;
                USBD_SetCfg(pdev, cfgidx);
                USBD_CtlSendStatus(pdev);
            } else {
                USBD_CtlSendStatus(pdev);
            }
            break;

        case USB_OTG_CONFIGURED:
            if (cfgidx == 0) {
                pdev->dev.device_status = USB_OTG_ADDRESSED;
                pdev->dev.device_config = cfgidx;
                USBD_ClrCfg(pdev, cfgidx);
                USBD_CtlSendStatus(pdev);
            } else if (cfgidx != pdev->dev.device_config) {
                /* Clear old configuration */
                USBD_ClrCfg(pdev, pdev->dev.device_config);

                /* set new configuration */
                pdev->dev.device_config = cfgidx;
                USBD_SetCfg(pdev, cfgidx);
                USBD_CtlSendStatus(pdev);
            } else {
                USBD_CtlSendStatus(pdev);
            }
            break;

        default:
            USBD_CtlError(pdev, req);
            break;
        }
    }
}

/**
 * @brief  USBD_GetConfig
 *         Handle Get device configuration request
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval status
 */
static void USBD_GetConfig(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    if (req->wLength != 1) {
        USBD_CtlError(pdev, req);
    } else {
        switch (pdev->dev.device_status) {
        case USB_OTG_ADDRESSED:

            USBD_CtlSendData(pdev, (uint8_t*)&USBD_default_cfg, 1);
            break;

        case USB_OTG_CONFIGURED:

            USBD_CtlSendData(pdev, &pdev->dev.device_config, 1);
            break;

        default:
            USBD_CtlError(pdev, req);
            break;
        }
    }
}

/**
 * @brief  USBD_GetStatus
 *         Handle Get Status request
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval status
 */
static void USBD_GetStatus(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    switch (pdev->dev.device_status) {
    case USB_OTG_ADDRESSED:
    case USB_OTG_CONFIGURED:

#ifdef USBD_SELF_POWERED
        USBD_cfg_status = USB_CONFIG_SELF_POWERED;
#else
        USBD_cfg_status = 0x00;
#endif

        if (pdev->dev.DevRemoteWakeup) {
            USBD_cfg_status |= USB_CONFIG_REMOTE_WAKEUP;
        }

        USBD_CtlSendData(pdev, (uint8_t*)&USBD_cfg_status, 2);
        break;

    default:
        USBD_CtlError(pdev, req);
        break;
    }
}

/**
 * @brief  USBD_SetFeature
 *         Handle Set device feature request
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval status
 */
static void USBD_SetFeature(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    USB_OTG_TESTMODE_TypeDef dctl;

    uint8_t test_mode = 0;

    if (req->wValue == USB_FEATURE_REMOTE_WAKEUP) {
        pdev->dev.DevRemoteWakeup = 1;
        // if (pdev->dev.class_cb->Setup(pdev, req) != USBD_OK) {
        USBD_CtlSendStatus(pdev);
        //}
        // USB_OTG_ActiveRemoteWakeup(pdev);
    } else if ((req->wValue == USB_FEATURE_TEST_MODE) && ((req->wIndex & 0xFF) == 0)) {
        // dctl.d8 = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->TESTMODE);
        dctl.d8   = 0;
        test_mode = req->wIndex >> 8;
        switch (test_mode) {
        case 1: // TEST_SE0_NAK
            dctl.b.test_SE0_NAK = 1;
            break;

        case 2: // TEST_J
            dctl.b.test_J = 1;
            break;

        case 3: // TEST_K
            dctl.b.test_K = 1;
            break;

        case 4: // TEST_PACKET
            dctl.b.test_packet = 1;
            break;

        case 5: // TEST_FORCE_ENABLE
            dctl.b.force_FS   = 1;
            dctl.b.force_host = 1; /* Full-speed Host */
            break;
        }
        SET_TEST_MODE       = dctl;
        pdev->dev.test_mode = 1;
        USBD_CtlSendStatus(pdev);
    }
}

/**
 * @brief  USBD_ClrFeature
 *         Handle clear device feature request
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval status
 */
static void USBD_ClrFeature(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    switch (pdev->dev.device_status) {
    case USB_OTG_ADDRESSED:
    case USB_OTG_CONFIGURED:
    case USB_OTG_SUSPENDED:
        if (req->wValue == USB_FEATURE_REMOTE_WAKEUP) {
            pdev->dev.DevRemoteWakeup = 0;
            pdev->dev.device_status   = USB_OTG_CONFIGURED;
            // if (pdev->dev.class_cb->Setup(pdev, req) != USBD_OK) {
            USBD_CtlSendStatus(pdev);
            //}
        }
        break;

    default:
        USBD_CtlError(pdev, req);
        break;
    }
}

/**
 * @brief  USBD_ParseSetupRequest
 *         Copy buffer into setup structure
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval None
 */

void USBD_ParseSetupRequest(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    req->bmRequest = *(uint8_t*)(pdev->dev.setup_packet);
    req->bRequest  = *(uint8_t*)(pdev->dev.setup_packet + 1);
    req->wValue    = SWAPBYTE(pdev->dev.setup_packet + 2);
    req->wIndex    = SWAPBYTE(pdev->dev.setup_packet + 4);
    req->wLength   = SWAPBYTE(pdev->dev.setup_packet + 6);

    // pdev->dev.in_ep[0].ctl_data_len = req->wLength;
    pdev->dev.device_state          = USB_OTG_EP0_SETUP;
}

/**
 * @brief  USBD_CtlError
 *         Handle USB low level Error
 * @param  pdev: device instance
 * @param  req: usb request
 * @retval None
 */

void USBD_CtlError(USB_OTG_CORE_HANDLE* pdev, USB_SETUP_REQ* req)
{
    DCD_EP_Stall(pdev, 0x80);
    DCD_EP_Stall(pdev, 0);
    USB_OTG_EP0_OutStart(pdev);
}

/**
 * @brief  USBD_GetString
 *         Convert Ascii string into unicode one
 * @param  desc : descriptor buffer
 * @param  unicode : Formatted string buffer (unicode)
 * @param  len : descriptor length
 * @retval None
 */
void USBD_GetString(uint8_t* desc, uint8_t* unicode, uint16_t* len)
{
    uint8_t idx = 0;

    if (desc != NULL) {
        *len           = USBD_GetLen(desc) * 2 + 2;
        unicode[idx++] = *len;
        unicode[idx++] = USB_DESC_TYPE_STRING;

        while (*desc != '\0') {
            unicode[idx++] = *desc++;
            unicode[idx++] = 0x00;
        }
    }
}

/**
 * @brief  USBD_GetLen
 *         return the string length
 * @param  buf : pointer to the ascii string buffer
 * @retval string length
 */
static uint8_t USBD_GetLen(uint8_t* buf)
{
    uint8_t len = 0;

    while (*buf != '\0') {
        len++;
        buf++;
    }

    return len;
}

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
