/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_dcd_int.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Peripheral Device interrupt subroutines.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "usb_dcd_int.h"


/** @defgroup USB_DCD_INT_Private_FunctionPrototypes
* @{
*/
/* static functions */
//static uint16_t DCD_ReadDevInEP(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum);

/* Interrupt Handlers */
static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr);
static uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr);
static uint32_t DCD_HandleSof_ISR(USB_OTG_CORE_HANDLE *pdev);

//static uint32_t DCD_HandleRxStatusQueueLevel_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_WriteEmptyTxFifo(USB_OTG_CORE_HANDLE *pdev, uint32_t epnum);

static uint32_t DCD_HandleUsbReset_ISR(USB_OTG_CORE_HANDLE *pdev);
//static uint32_t DCD_HandleEnumDone_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_HandleResume_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_HandleUSBSuspend_ISR(USB_OTG_CORE_HANDLE *pdev);

//static uint32_t DCD_IsoINIncomplete_ISR(USB_OTG_CORE_HANDLE *pdev);
//static uint32_t DCD_IsoOUTIncomplete_ISR(USB_OTG_CORE_HANDLE *pdev);
#ifdef VBUS_SENSING_ENABLED
static uint32_t DCD_SessionRequest_ISR(USB_OTG_CORE_HANDLE *pdev);
//static uint32_t DCD_OTG_ISR(USB_OTG_CORE_HANDLE *pdev);
#endif

/**
* @}
*/

/** @defgroup USB_DCD_INT_Private_Functions
* @{
*/

/**
* @brief  USBD_OTG_ISR_Handler
*         handles all USB Interrupts
* @param  pdev: device instance
* @retval status
*/
uint32_t USBD_OTG_ISR_Handler(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_INTRUSB_TypeDef  gintr_status;
    USB_OTG_INTRTX_TypeDef   intr_tx;
    USB_OTG_INTRRX_TypeDef   intr_rx;
    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef  csr0l;
    uint32_t retval = 0;
    USB_OTG_EP* ep;
    uint32_t rxLength = 0;

    if (USB_OTG_IsDeviceMode(pdev)) { /* ensure that we are in device mode */
        gintr_status.d8 = USB_OTG_ReadCoreItr(pdev);
        intr_rx.d16 = USB_OTG_ReadDevAllOutEp_itr(pdev);
        intr_tx.d16 = USB_OTG_ReadDevAllInEPItr(pdev);
        if ((!gintr_status.d8) && (!intr_rx.d16) && (!intr_tx.d16)) { /* avoid spurious interrupt */
            return 0;
        }

        // Prevent suspend state conflicts
        if (gintr_status.b.resume) {
            retval |= DCD_HandleResume_ISR(pdev);
        } else if (gintr_status.b.suspend) {
            retval |= DCD_HandleUSBSuspend_ISR(pdev);
        }

        if (gintr_status.b.sof) {
            retval |= DCD_HandleSof_ISR(pdev);
        }
        if (gintr_status.b.reset_babble) {
            retval |= DCD_HandleUsbReset_ISR(pdev);
        }
        if (gintr_status.b.sess_req) {
            retval |= DCD_SessionRequest_ISR(pdev);
        }
        if (gintr_status.b.VBus_error) {
        }
        if (intr_tx.d16 & 0x01) {
            csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
            if (csr0l.b.sent_stall) {
                DCD_EP_ClrStall(pdev, 0x80);
            }

            if (csr0l.b.setup_end) {
                csr0l.b.serviced_setupend = 1;
                USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
            }
            if (pdev->dev.addr_param.SetAddress_Flag) {
                pdev->regs.COMMREGS->FADDR = pdev->dev.addr_param.Address_Value;
                pdev->dev.addr_param.SetAddress_Flag = 0;
            }
            if (csr0l.b.rx_pkt_rdy) {
                ep = &pdev->dev.out_ep[0];
                /* Read Packet */
                rxLength = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->COUNT.COUNT0);
                /* Copy the setup packet received in FIFO into the setup buffer in RAM */
                USB_OTG_ReadPacket(pdev, ep->xfer_buff + ep->xfer_count, 0, rxLength);
                ep->xfer_count += rxLength;
                if ((rxLength < ep->maxpacket) || (ep->xfer_count >= ep->xfer_len)) {
                    if (ep->xfer_buff == pdev->dev.setup_packet) {
                        /* deal with receive setup packet */
                        USBD_DCD_INT_fops->SetupStage(pdev);
                    } else {
                        /* RX COMPLETE out stage end*/
                        USBD_DCD_INT_fops->DataOutStage(pdev, 0);
                        csr0l.b.data_end = 1;
                        csr0l.b.serviced_rxpktrdy = 1;
                        USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
                    }
                    ep->xfer_count = 0;
                } else {
                    csr0l.b.serviced_rxpktrdy = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
                }
            } else if (!csr0l.b.tx_pkt_rdy) {
                USBD_DCD_INT_fops->DataInStage(pdev, 0);
            }
        }
        if ((intr_rx.d16 & 0xFE)) {
            retval |= DCD_HandleOutEP_ISR(pdev, intr_rx.d16 & 0xFE);
        }

        if (intr_tx.d16 & 0xFE) {
            retval |= DCD_HandleInEP_ISR(pdev, intr_tx.d16 & 0xFE);
        }
    }
    return retval;
}

#ifdef VBUS_SENSING_ENABLED
/**
* @brief  DCD_SessionRequest_ISR
*         Indicates that the USB_OTG controller has detected a connection
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_SessionRequest_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    return 1;
}

/**
* @brief  DCD_OTG_ISR
*         Indicates that the USB_OTG controller has detected an OTG event:
*                 used to detect the end of session i.e. disconnection
* @param  pdev: device instance
* @retval status
*/
#if 0
static uint32_t DCD_OTG_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    return 1;
}
#endif
#endif

/**
* @brief  DCD_HandleResume_ISR
*         Indicates that the USB_OTG controller has detected a resume or
*                 remote Wake-up sequence
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleResume_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    /* Inform upper layer by the Resume Event */
    USBD_DCD_INT_fops->Resume(pdev);

    return 1;
}

/**
* @brief  USB_OTG_HandleUSBSuspend_ISR
*         Indicates that SUSPEND state has been detected on the USB
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleUSBSuspend_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_INTRUSB_TypeDef  intr_usb;
    __IO uint8_t prev_status = 0;
    /* Clear session */

    prev_status = pdev->dev.device_status;
    USBD_DCD_INT_fops->Suspend(pdev);

    intr_usb.d8 = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->INTRUSB);
    if ((intr_usb.b.suspend == 1) &&
            (pdev->dev.connection_status == 1) &&
            (prev_status == USB_OTG_CONFIGURED)) {
        /*  switch-off the clocks */

        /* Request to enter Sleep mode after exit from current ISR */
        SCB->SCR |= (SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk);
    }
    return 1;
}

/**
* @brief  DCD_HandleInEP_ISR
*         Indicates that an IN EP has a pending Interrupt
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr)
{
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef txcsrl;
    uint16_t epnum = 0;
    while (ep_intr) {
        if (ep_intr & 0x01) { /* In ITR */
            txcsrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[epnum]->TXCSRL);
            if (!txcsrl.b.tx_pkt_rdy) {
                USBD_DCD_INT_fops->DataInStage(pdev, epnum);
            }
            if (txcsrl.b.sent_stall) {
                CLEAR_IN_EP_INTR(epnum, sent_stall);
            }
            if (txcsrl.b.send_stall) {
                CLEAR_IN_EP_INTR(epnum, send_stall);
            }
            if (txcsrl.b.under_run) {
                CLEAR_IN_EP_INTR(epnum, under_run);
            }
            if (!txcsrl.b.fifo_not_empty) {
                DCD_WriteEmptyTxFifo(pdev, epnum);
            }
        }
        epnum++;
        ep_intr >>= 1;
    }

    return 1;
}

/**
* @brief  DCD_HandleOutEP_ISR
*         Indicates that an OUT EP has a pending Interrupt
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr)
{
    USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef  rxcsrl;
    USB_OTG_RXCOUNT_TypeDef  rx_count;
    uint32_t epnum = 1;
    uint32_t rx_fifo_len = 0;

    ep_intr >>= 1;
    while (ep_intr) {
        if (ep_intr & 0x1) {
            rxcsrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[epnum]->RXCSRL);
            /* Transfer complete */
            if (rxcsrl.b.rx_pkt_rdy) {
                /* Inform upper layer: data ready */
                rx_count.d16 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[epnum]->RXCOUNT);
                rx_fifo_len = MIN(rx_count.d16, pdev->dev.out_ep[epnum].maxpacket);
                USB_OTG_ReadPacket(pdev,
                                   pdev->dev.out_ep[epnum].xfer_buff + pdev->dev.out_ep[epnum].xfer_count,
                                   epnum,
                                   rx_count.d16);
                pdev->dev.out_ep[epnum].xfer_count += rx_fifo_len;
                /* RX COMPLETE */
                USBD_DCD_INT_fops->DataOutStage(pdev, epnum);
            }
            /* Endpoint disable  */
            if (rxcsrl.b.sent_stall) {
                /* Clear the bit in RXCSRL for this interrupt */
                CLEAR_OUT_EP_INTR(epnum, sent_stall);
            }
            if (rxcsrl.b.send_stall) {
                /* Clear the bit in RXCSRL for this interrupt */
                CLEAR_OUT_EP_INTR(epnum, send_stall);
            }
            if (rxcsrl.b.over_run) {
                /* Clear the bit in RXCSRL for this interrupt */
                CLEAR_OUT_EP_INTR(epnum, over_run);
            }
            if (rxcsrl.b.data_error) {
                /* Clear the bit in RXCSRL for this interrupt */
                CLEAR_OUT_EP_INTR(epnum, data_error);
            }
        }
        epnum++;
        ep_intr >>= 1;
    }
    return 1;
}

/**
* @brief  DCD_HandleSof_ISR
*         Handles the SOF Interrupts
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleSof_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    USBD_DCD_INT_fops->SOF(pdev);
    return 1;
}

/**
* @brief  DCD_HandleRxStatusQueueLevel_ISR
*         Handles the Rx Status Queue Level Interrupt
* @param  pdev: device instance
* @retval status
*/
#if 0
static uint32_t DCD_HandleRxStatusQueueLevel_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    return 1;
}
#endif

/**
* @brief  DCD_WriteEmptyTxFifo
*         check FIFO for the next packet to be loaded
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_WriteEmptyTxFifo(USB_OTG_CORE_HANDLE *pdev, uint32_t epnum)
{
    return 1;
}

/**
* @brief  DCD_HandleUsbReset_ISR
*         This interrupt occurs when a USB Reset is detected
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleUsbReset_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_FADDR_TypeDef faddr;
    USB_OTG_INTRUSB_TypeDef intr_usb;
    USB_OTG_INTRRX_TypeDef  intr_rx;
    USB_OTG_INTRTX_TypeDef  intr_tx;
    /* Flush the Tx FIFO */
    USB_OTG_FlushTxFifo(pdev,  0);

    /* Reset Device Address */
    faddr.d8 = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->FADDR);
    faddr.b.func_addr = 0;
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->FADDR, faddr.d8);


    /* setup EP0 to receive SETUP packets */
    USB_OTG_EP0_OutStart(pdev);

    /* Clear interrupt */
    intr_usb.d8 = 0;
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSB, intr_usb.d8);
    intr_rx.d16 = 0;
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRRX, intr_rx.d16);
    intr_tx.d16 = 0;
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRTX, intr_tx.d16);

    /*Reset internal state machine */
    USBD_DCD_INT_fops->Reset(pdev);
    return 1;
}

/**
* @brief  DCD_HandleEnumDone_ISR
*         Read the device status register and set the device speed
* @param  pdev: device instance
* @retval status
*/
#if 0
static uint32_t DCD_HandleEnumDone_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    return 1;
}
#endif


/**
* @brief  DCD_IsoINIncomplete_ISR
*         handle the ISO IN incomplete interrupt
* @param  pdev: device instance
* @retval status
*/
#if 0
static uint32_t DCD_IsoINIncomplete_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    return 1;
}
#endif

/**
* @brief  DCD_IsoOUTIncomplete_ISR
*         handle the ISO OUT incomplete interrupt
* @param  pdev: device instance
* @retval status
*/
#if 0
static uint32_t DCD_IsoOUTIncomplete_ISR(USB_OTG_CORE_HANDLE *pdev)
{
    return 1;
}
#endif
/**
* @brief  DCD_ReadDevInEP
*         Reads ep flags
* @param  pdev: device instance
* @retval status
*/
#if 0
static uint16_t DCD_ReadDevInEP(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    uint16_t v;
    v = ((USB_OTG_READ_REG8(&pdev->regs.CSRREGS[epnum]->TXCSRH) << 8) | \
         USB_OTG_READ_REG8(&pdev->regs.CSRREGS[epnum]->TXCSRL));
    return v;
}
#endif

/**
* @}
*/


/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
