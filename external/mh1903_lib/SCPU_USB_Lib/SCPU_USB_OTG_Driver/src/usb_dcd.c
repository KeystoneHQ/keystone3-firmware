/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_dcd.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Peripheral Device Interface layer.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "usb_dcd.h"
#include "usb_bsp.h"

void DCD_Init(USB_OTG_CORE_HANDLE* pdev, USB_OTG_CORE_ID_TypeDef coreID)
{
    /* Set Register Address */
    USB_OTG_SelectCore(pdev, coreID);

    pdev->dev.device_status  = USB_OTG_DEFAULT;
    pdev->dev.device_address = 0;

    USB_OTG_DisableGlobalInt(pdev);

    /* Init the Core (common init.) */
    USB_OTG_CoreInit(pdev);

    /* Force Device Mode*/
    USB_OTG_SetCurrentMode(pdev, DEVICE_MODE);

    /* Init Device */
    USB_OTG_CoreInitDev(pdev);

    /* Enable USB Global interrupt */
    USB_OTG_EnableGlobalInt(pdev);
}

/**
* @brief  Configure an EP
* @param pdev : Device instance
* @param epdesc : Endpoint Descriptor
* @retval : status
*/
uint32_t DCD_EP_Open(USB_OTG_CORE_HANDLE *pdev,
                     uint8_t ep_addr,
                     uint16_t ep_mps,
                     uint8_t ep_type)
{
    USB_OTG_EP *ep;

    if ((ep_addr & 0x80) == 0x80) {
        ep = &pdev->dev.in_ep[ep_addr & 0x7F];
    } else {
        ep = &pdev->dev.out_ep[ep_addr & 0x7F];
    }
    // ep->num = ep_addr & 0x7F;
    // ep->is_in = (0x80 & ep_addr) != 0;
    ep->maxpacket = ep_mps;
    ep->type = ep_type;
    /* Never used
    if (ep->is_in)
    {
        // Assign a Tx FIFO
        ep->tx_fifo_num = ep->num;
    }
    // Set initial data PID.
    if (ep_type == USB_OTG_EP_BULK )
    {
        ep->data_pid_start = 0;
    }
    */
    USB_OTG_EPActivate(pdev, ep);
    return 0;
}

/**
* @brief  called when an EP is disabled
* @param pdev: device instance
* @param ep_addr: endpoint address
* @retval : status
*/
uint32_t DCD_EP_Close(USB_OTG_CORE_HANDLE *pdev, uint8_t  ep_addr)
{
    USB_OTG_EP *ep;

    if ((ep_addr & 0x80) == 0x80) {
        ep = &pdev->dev.in_ep[ep_addr & 0x7F];
    } else {
        ep = &pdev->dev.out_ep[ep_addr & 0x7F];
    }
    // ep->num   = ep_addr & 0x7F;
    // ep->is_in = (0x80 & ep_addr) != 0;
    USB_OTG_EPDeactivate(pdev, ep);
    return 0;
}

/**
* @brief  DCD_EP_PrepareRx
* @param pdev: device instance
* @param ep_addr: endpoint address
* @param pbuf: pointer to Rx buffer
* @param buf_len: data length
* @retval : status
*/
uint32_t   DCD_EP_PrepareRx(USB_OTG_CORE_HANDLE *pdev,
                            uint8_t   ep_addr,
                            uint8_t *pbuf,
                            uint16_t  buf_len)
{
    USB_OTG_EP *ep;

    ep = &pdev->dev.out_ep[ep_addr & 0x7F];

    /*setup and start the Xfer */
    ep->xfer_buff = pbuf;
    ep->xfer_len = buf_len;
    ep->xfer_count = 0;
    ep->is_in = 0;
    ep->num = ep_addr & 0x7F;

    if (ep->num == 0) {
        USB_OTG_EP0StartXfer(pdev, ep);
    } else {
        USB_OTG_EPStartXfer(pdev, ep);
    }
    return 0;
}

/**
* @brief  Transmit data over USB
* @param pdev: device instance
* @param ep_addr: endpoint address
* @param pbuf: pointer to Tx buffer
* @param buf_len: data length
* @retval : status
*/
uint32_t  DCD_EP_Tx(USB_OTG_CORE_HANDLE *pdev,
                    uint8_t   ep_addr,
                    uint8_t   *pbuf,
                    uint32_t   buf_len)
{
    USB_OTG_EP *ep;

    ep = &pdev->dev.in_ep[ep_addr & 0x7F];

    /* Setup and start the Transfer */
    ep->is_in = 1;
    ep->num = ep_addr & 0x7F;
    ep->xfer_buff = pbuf;

    ep->total_data_len = buf_len;
    ep->rem_data_len = buf_len;
    ep->xfer_len  = 0;
    ep->xfer_count = 1;

    if (ep->num == 0) {
        ep->xfer_count += buf_len / ep->maxpacket + 1;
        USB_OTG_EP0StartXfer(pdev, ep);
    } else {
        ep->xfer_count += (buf_len + ep->maxpacket - 1) / ep->maxpacket;
        USB_OTG_EPStartXfer(pdev, ep);
    }
    return 0;
}

/**
* @brief  Stall an endpoint.
* @param pdev: device instance
* @param epnum: endpoint address
* @retval : status
*/

uint32_t  DCD_EP_Stall(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    USB_OTG_EP *ep;
    if ((0x80 & epnum) == 0x80) {
        ep = &pdev->dev.in_ep[epnum & 0x7F];
    } else {
        ep = &pdev->dev.out_ep[epnum];
    }

    ep->is_stall = 1;
    ep->num   = epnum & 0x7F;
    ep->is_in = ((epnum & 0x80) == 0x80);

    USB_OTG_EPSetStall(pdev, ep);
    return (0);
}

/**
* @brief  Clear stall condition on endpoints.
* @param pdev: device instance
* @param epnum: endpoint address
* @retval : status
*/
uint32_t  DCD_EP_ClrStall(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    USB_OTG_EP *ep;
    if ((0x80 & epnum) == 0x80) {
        ep = &pdev->dev.in_ep[epnum & 0x7F];
    } else {
        ep = &pdev->dev.out_ep[epnum];
    }

    ep->is_stall = 0;
    ep->num   = epnum & 0x7F;
    ep->is_in = ((epnum & 0x80) == 0x80);

    USB_OTG_EPClearStall(pdev, ep);
    return (0);
}

/**
* @brief  This Function flushes the FIFOs.
* @param pdev: device instance
* @param epnum: endpoint address
* @retval : status
*/
uint32_t DCD_EP_Flush(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    if ((epnum & 0x80) == 0x80) {
        USB_OTG_FlushTxFifo(pdev, epnum & 0x7F);
    } else {
        USB_OTG_FlushRxFifo(pdev, epnum);
    }

    return (0);
}

/**
* @brief  This Function set USB device address
* @param pdev: device instance
* @param address: new device address
* @retval : status
*/

void  DCD_EP_SetAddress(USB_OTG_CORE_HANDLE *pdev, uint8_t address)
{
    USB_OTG_FADDR_TypeDef  faddr;
    faddr.d8 = 0;
    faddr.b.func_addr = address;
    USB_OTG_MODIFY_REG8(&pdev->regs.COMMREGS->FADDR, 0, faddr.d8);
}

/**
* @brief  Connect device (enable internal pull-up)
* @param pdev: device instance
* @retval : None
*/
void  DCD_DevConnect(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_POWER_TypeDef  power;
    power.d8 = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->POWER);
    /* Connect device */
    power.b.soft_conn  = 1;
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
    USB_OTG_BSP_mDelay(3);
}

/**
* @brief  Disconnect device (disable internal pull-up)
* @param pdev: device instance
* @retval : None
*/
void DCD_DevDisconnect(USB_OTG_CORE_HANDLE* pdev)
{
    USB_OTG_POWER_TypeDef power;

    power.d8 = pdev->regs.COMMREGS->POWER;

    // If bus in suspend mode resume bus at first
    if (pdev->dev.device_status == USB_OTG_SUSPENDED) {
        power.b.resume             = 1;
        pdev->regs.COMMREGS->POWER = power.d8;
        USB_OTG_BSP_mDelay(10);
    }

    // Software disconnect device
    power.b.soft_conn = 0;

    pdev->regs.COMMREGS->POWER = power.d8;
    USB_OTG_BSP_mDelay(3);
}

/**
* @brief  returns the EP Status
* @param  pdev : Selected device
*         epnum : endpoint address
* @retval : EP status
*/
uint32_t DCD_GetEPStatus(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    USB_OTG_EP *ep;
    uint32_t Status = 0;

    if ((0x80 & epnum) == 0x80) {
        ep = &pdev->dev.in_ep[epnum & 0x7F];
    } else {
        ep = &pdev->dev.out_ep[epnum];
    }

    Status = USB_OTG_GetEPStatus(pdev, ep);

    /* Return the current status */
    return Status;
}

/**
* @brief  Set the EP Status
* @param  pdev : Selected device
*         Status : new Status
*         epnum : EP address
* @retval : None
*/
void DCD_SetEPStatus(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum, uint32_t Status)
{
    USB_OTG_EP *ep;

    if ((0x80 & epnum) == 0x80) {
        ep = &pdev->dev.in_ep[epnum & 0x7F];
    } else {
        ep = &pdev->dev.out_ep[epnum];
    }

    USB_OTG_SetEPStatus(pdev, ep, Status);
}


/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
