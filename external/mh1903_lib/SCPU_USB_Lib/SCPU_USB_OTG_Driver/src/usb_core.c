/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_core.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : USB-OTG Core layer.
 *****************************************************************************/

/* Include ------------------------------------------------------------------*/
#include "mhscpu.h"
#include "usb_core.h"
#include "usb_bsp.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Ptivate function prototypes ----------------------------------------------*/

/******************************************************************************
* Function Name  :
* Description    :
* Input          :
* Output         :
* Return         :
******************************************************************************/
/** @addtogroup USB_OTG_DRIVER
* @{
*/

/** @defgroup USB_CORE
* @brief This file includes the USB-OTG Core Layer
* @{
*/


/** @defgroup USB_CORE_Private_Defines
* @{
*/

/**
* @}
*/


/** @defgroup USB_CORE_Private_TypesDefinitions
* @{
*/
/**
* @}
*/



/** @defgroup USB_CORE_Private_Macros
* @{
*/
/**
* @}
*/


/** @defgroup USB_CORE_Private_Variables
* @{
*/
/**
* @}
*/


/** @defgroup USB_CORE_Private_FunctionPrototypes
* @{
*/
/**
* @}
*/


/** @defgroup USB_CORE_Private_Functions
* @{
*/
/**
* @brief  USB_OTG_FifosizeReg
*         Turn Fifo size to Set register value
* @param  fifosiz : Set Fifo size
* @retval None
*/
#ifdef USE_DEVICE_MODE
static uint8_t USB_OTG_FifosizeReg(uint16_t fifosiz)
{
    uint8_t register_value = 0;
    assert_param(IS_FIFO_SIZE(fifosiz));

    switch (fifosiz) {
    case 8:
        register_value = 0;
        break;
    case 16:
        register_value = 1;
        break;
    case 32:
        register_value = 2;
        break;
    case 64:
        register_value = 3;
        break;
    case 128:
        register_value = 4;
        break;
    case 256:
        register_value = 5;
        break;
    case 512:
        register_value = 6;
        break;
    default:
        break;
    }
    return register_value;
}
#endif

/**
* @brief  USB_OTG_FifoStartAddr
*
* @param  RxFifosize : Rx fifo size; TxFifosize: Tx fifo size
* @retval None
*/
//static uint16_t USB_OTG_FifoStartAddr(uint32_t RxFifosize, uint32_t TxFifosize)
//{
//    uint16_t uint16_start_addr = 0;
//    uint16_start_addr = ((RxFifosize > TxFifosize) ?  RxFifosize : TxFifosize) >> 3;
//    return uint16_start_addr;
//}

/**
* @brief  USB_OTG_InitDevSpeed :Initializes the DevSpd field of DCFG register
*         depending the PHY type and the enumeration speed of the device.
* @param  pdev : Selected device
* @retval : None
*/
void USB_OTG_InitDevSpeed(USB_OTG_CORE_HANDLE *pdev, uint8_t speed)
{
//    USB_OTG_POWER_TypeDef power;
//    power.d8 = USB_OTG_READ_REG8();
}

/**
* @brief  USB_OTG_EpFifoConfiguration : Configure Epn's FIFO Size
* @param  pdev : Selected device
* @retval None
*/
/*
static void USB_OTG_EpFifoConfiguration(USB_OTG_CORE_HANDLE *pdev,
                                        uint8_t ep_num,
                                        uint32_t Trx_fifo_startaddr,
                                        uint16_t rx_fifo_size,
                                        uint16_t tx_fifo_size)
{
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef    txcsrl;
    USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef    rxcsrl;
    txcsrl.d8 = 0;
    rxcsrl.d8 = 0;
    txcsrl.b.flush_fifo = 1;
    rxcsrl.b.flush_fifo = 1;
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INDEX, ep_num);

    // set Rx FIFO size
    USB_OTG_WRITE_REG16(&pdev->regs.CSRREGS[ep_num]->RXMAXP, pdev->dev.out_ep[ep_num].maxpacket);
    USB_OTG_WRITE_REG16(&pdev->regs.DYNFIFOREGS->RXFIFOADD,  \
                        Trx_fifo_startaddr >> 3);
    USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->RXFIFOSZ,USB_OTG_FifosizeReg(rx_fifo_size));
    // Flush the FIFOs
    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep_num]->RXCSRL, rxcsrl.d8);
    // set Tx FIFO size
    USB_OTG_WRITE_REG16(&pdev->regs.CSRREGS[ep_num]->TXMAXP, pdev->dev.in_ep[ep_num].maxpacket);
    USB_OTG_WRITE_REG16(&pdev->regs.DYNFIFOREGS->TXFIFOADD,  \
                        (Trx_fifo_startaddr + rx_fifo_size) >> 3);
    USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->TXFIFOSZ,USB_OTG_FifosizeReg(tx_fifo_size));
    // Flush the FIFOs
    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep_num]->TXCSRL, txcsrl.d8);

}
*/

/**
* @brief  USB_OTG_EnableCommonInt
*         Initializes the commmon interrupts, used in both device and host modes
* @param  pdev : Selected device
* @retval None
*/
static void USB_OTG_EnableCommonInt(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_INTRUSBE_TypeDef intr_usbe;
    intr_usbe.d8 = 0;
    /* Clear any pending interrupts */
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSB, 0xFF);
    /* Enable the interrupts in the INTMSK */
    intr_usbe.b.en_resume = 1;
    intr_usbe.b.en_suspend = 1;

    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSBE, intr_usbe.d8);
    /* Enable Epn_Rx interrupr(n = 1,2,3,4,5,6,7) */
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRRXE, 0x00);
    /* Enable Epn_Tx interrupr(n = 0,1,2,3,4,5,6,7) */
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRTXE, 0x00);
}

/**
* @brief  USB_OTG_CoreReset : Soft reset of the core
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
//static USB_OTG_STS USB_OTG_CoreReset(USB_OTG_CORE_HANDLE *pdev)
//{
//    USB_OTG_STS status = USB_OTG_OK;
//    __IO USB_OTG_POWER_TypeDef  power;

//    power.d8 = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->POWER);
//    /* Core Soft Reset */
//    power.b.en_suspendM = 1;
//    power.b.reset = 1;
//    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
//    /* The CPU should keep the Reset bit set for at least 20 ms to ensure correct
//     * resetting of the target device.
//     */
//    USB_OTG_BSP_mDelay(25);
//    power.b.reset = 0;
//    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
//    /* Wait for 3 PHY Clocks*/
//    USB_OTG_BSP_uDelay(3);
//    return status;
//}

/**
* @brief  USB_OTG_WritePacket : Writes a packet into the Tx FIFO associated
*         with the EP
* @param  pdev : Selected device
* @param  src : source pointer
* @param  ch_ep_num : end point number
* @param  bytes : No. of bytes
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_WritePacket(USB_OTG_CORE_HANDLE *pdev,
                                uint8_t             *src,
                                uint8_t             ch_ep_num,
                                uint16_t            len)
{
    USB_OTG_STS status = USB_OTG_OK;
    uint32_t i = 0;

#ifdef USE_DEVICE_MODE
    for (i = 0; i < len; i++) {
        if ((int)src >= (MHSCPU_SRAM_BASE + MHSCPU_SRAM_SIZE) || (int)src < MHSCPU_SRAM_BASE) {
            while (1) {}
        }
        pdev->regs.FIFO[ch_ep_num]->byte = *src++;
    }
#endif
#ifdef USE_HOST_MODE
    for (i = 0; i < len; i++) {
        pdev->regs.FIFO[ch_ep_num]->byte = *src++;
    }
#endif
    return status;
}

/**
* @brief  USB_OTG_ReadPacket : Reads a packet from the Rx FIFO
* @param  pdev : Selected device
* @param  dest : Destination Pointer
* @param  bytes : No. of bytes
* @retval None
*/
void *USB_OTG_ReadPacket(USB_OTG_CORE_HANDLE *pdev,
                         uint8_t *dest,
                         uint8_t ch_ep_num,
                         uint16_t len)
{
    uint32_t i = 0;
    uint32_t count32b = len / 4;
    uint32_t *data_buff = (uint32_t *)dest;

    for (i = 0; i < count32b; i++, data_buff++) {
        *data_buff = pdev->regs.FIFO[ch_ep_num]->word;
    }
    for (i = count32b * 4; i < len; i++) {
        dest[i] = pdev->regs.FIFO[ch_ep_num]->byte;
    }
    /* Return the buffer pointer because if the transfer is composed of several
     packets, the data of the next packet must be stored following the
     previous packet's data */
    return ((void *)dest);
}

/**
* @brief  USB_OTG_SelectCore
*         Initialize core registers address.
* @param  pdev : Selected device
* @param  coreID : USB OTG Core ID
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_SelectCore(USB_OTG_CORE_HANDLE *pdev,
                               USB_OTG_CORE_ID_TypeDef coreID)
{
    uint32_t i, baseAddress = 0;
    USB_OTG_STS status = USB_OTG_OK;

    /* at startup the core is in FS mode */
    pdev->cfg.speed = USB_OTG_SPEED_FULL;
    pdev->cfg.mps = USB_OTG_FS_MAX_PACKET_SIZE;

    /* initialize device cfg following its address */
    if (coreID == USB_OTG_FS_CORE_ID) {
        baseAddress             = USB_OTG_FS_BASE_ADDR;
        pdev->cfg.coreID        = USB_OTG_FS_CORE_ID;
        pdev->cfg.host_channels = USB_OTG_MAX_EP_COUNT;
        pdev->cfg.dev_endpoints = USB_OTG_MAX_EP_COUNT;
        pdev->cfg.TotalFifoSize = USB_OTG_MAX_FIFO_SIZE; /* in 8-bits */
        pdev->cfg.UsedFifoSize  = 0;
    }

    /* Common USB Registers */
    pdev->regs.COMMREGS = (USB_OTG_COMMREGS *)(baseAddress + USB_OTG_COMMON_GLOBAL_REG_OFFSET);
    /* Indexed CSR */
    pdev->regs.INDEXREGS = (USB_OTG_INDEXREGS *)(baseAddress + USB_OTG_INDEXED_CSR_REG_OFFSET);
    /* FIFOS */
    for (i = 0; i < NUM_EP_FIFO; i++) {
        pdev->regs.FIFO[i] = (USB_OTG_FIFO *)(baseAddress + \
                                              USB_OTG_EP_FIFO_GLOBAL_REG_OFFSET + i * USB_OTG_EP_FIFO_REG_OFFSET);
    }
    /* DynFIFO + Version */
    pdev->regs.DYNFIFOREGS = (USB_OTG_DYNFIFOREGS *)(baseAddress + USB_OTG_DYNFIFO_REG_OFFSET);
    /* ULPI & Addnl. Config. registers */
    pdev->regs.ULPIREGS = (USB_OTG_ULPIREGS *)(baseAddress + USB_OTG_ULPI_REG_OFFSET);
    /* TADDR Epn (n = 0 - 15). 0x80 - 0xFF */
    for (i = 0; i < NUM_EP_FIFO; i++) {
        pdev->regs.TADDRREGS[i] = (USB_OTG_TADDRREGS *)(baseAddress + \
                                  USB_OTG_TADDR_GLOBAL_REG_OFFSET + i * USB_OTG_TADDR_REG_OFFSET);
    }
    /* CSR EPn (n = 0 - 15). 0x100 - 0x1FF */
    for (i = 0; i < NUM_EP_FIFO; i++) {
        pdev->regs.CSRREGS[i] = (USB_OTG_CSRREGS *)(baseAddress + \
                                USB_OTG_EP_CSR_GLOBAL_REG_OFFSET + i * USB_OTG_EP_CSR_REG_OFFSET);
    }
    /* Optional DMA Registers */
    for (i = 0; i < NUM_DMA_CHANNEL; i++) {
        pdev->regs.DMAREGS[i] = (USB_OTG_DMAREGS *)(baseAddress + \
                                USB_OTG_DMA_GLOBAL_REG_OFFSET + i * USB_OTG_DMA_REG_OFFSET);
    }
    /* Extended Registers */
    pdev->regs.EXTREGS = (USB_OTG_EXTREGS *)(baseAddress + USB_OTG_EXTENED_REG_OFFSET);
    /* LPM Registers */
    pdev->regs.LPMREGS = (USB_OTG_LPMREGS *)(baseAddress + USB_OTG_LPM_REG_OFFSET);

    return status;
}


/**
* @brief  USB_OTG_CoreInit
*         Initializes the USB_OTG controller registers and prepares the core
*         device mode or host mode operation.
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_CoreInit(USB_OTG_CORE_HANDLE* pdev)
{
    return USB_OTG_OK;
}

/**
* @brief  USB_OTG_EnableGlobalInt
*         Enables the controller's Global Int in the ISER reg
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EnableGlobalInt(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_STS status = USB_OTG_OK;
    /* USB -- INT_ISER[1] */
    NVIC->ISER[0] = 0x02;  /* Set Bit1 */
    return status;
}

/**
* @brief  USB_OTG_DisableGlobalInt
*         Disables the controller's Global Int in the ICER reg
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_DisableGlobalInt(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_STS status = USB_OTG_OK;
    /* USB -- INT_ICER[1] */
    NVIC->ICER[0] = 0x00000002;  /* Clear Bit1 */
    return status;
}

/**
 * @brief  USB_OTG_FlushTxFifo : Flush a Tx FIFO
 * @param  pdev : Selected device
 * @param  num : FO num
 * @retval USB_OTG_STS : status
 */
USB_OTG_STS USB_OTG_FlushTxFifo(USB_OTG_CORE_HANDLE* pdev, uint32_t num)
{
    USB_OTG_STS status = USB_OTG_OK;

    USB_OTG_TXCSRL_IN_HOST_TypeDef      txcsrl;
    USB_OTG_CSR0H_IN_PERIPHERAL_TypeDef csr0h;
    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef csr0l;

    if (num == 0) {
        csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
        if (!csr0l.b.rx_pkt_rdy && !csr0l.b.tx_pkt_rdy)
            return status;

        // FlushFIFO should only be used when TxPktRdy/RxPktRdy is set.
        // The FIFO pointer is reset and the TxPktRdy/RxPktRdy bit (below) is cleared
        csr0h.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRH.CSR0H);
        csr0h.b.flush_fifo = 1;

        USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRH.CSR0H, csr0h.d8);
    } else {
        txcsrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[num]->TXCSRL);
        if (!txcsrl.b.tx_pkt_rdy)
            return status;

        // FlushFIFO should only be used when RxPktRdy is set.
        // The FIFO pointer is reset, the TxPktRdy bit is cleared and an interrupt is generated
        txcsrl.b.flush_fifo = 1;
        txcsrl.b.tx_pkt_rdy = 0;
        USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[num]->TXCSRL, txcsrl.d8);
    }

    /* Wait for 3 PHY Clocks*/
    USB_OTG_BSP_uDelay(3);

    return status;
}

/**
* @brief  USB_OTG_FlushRxFifo : Flush a Rx FIFO
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_FlushRxFifo(USB_OTG_CORE_HANDLE *pdev, uint32_t num)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_RXCSRL_IN_HOST_TypeDef rxcsrl;

    if (!num) {
        return status;
    }

    rxcsrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[num]->RXCSRL);
    if (!rxcsrl.b.rx_pkt_rdy)
        return status;

    // FlushFIFO should only be used when RxPktRdy is set.
    // The FIFO pointer is reset and the RxPktRdy bit is cleared.
    rxcsrl.b.flush_fifo = 1;
    rxcsrl.b.rx_pkt_rdy = 0;
    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[num]->RXCSRL, rxcsrl.d8);

    /* Wait for 3 PHY Clocks*/
    USB_OTG_BSP_uDelay(3);

    return status;
}

/**
* @brief  USB_OTG_SetCurrentMode : Set ID line
* @param  pdev : Selected device
* @param  mode :  (Host/device)
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_SetCurrentMode(USB_OTG_CORE_HANDLE *pdev, uint8_t mode)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_POWER_TypeDef  power;
    USB_OTG_DEVCTL_TypeDef devctl;

    power.d8 = 0;
    devctl.d8 = USB_OTG_READ_REG8(&pdev->regs.DYNFIFOREGS->DEVCTL);

    if (mode == HOST_MODE) {
        // power.b.HS_enab = 1;
        power.b.en_suspendM = 1;
        USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
        devctl.b.host_mode = 1;
        devctl.b.session = 1;
        USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->DEVCTL, devctl.d8);
    } else if (mode == DEVICE_MODE) {
        devctl.b.host_mode = 0;
        power.b.en_suspendM = 1;
        devctl.b.session = 0;
        power.b.soft_conn = 1;
        // power.b.HS_enab = 1;
        USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->DEVCTL, devctl.d8);
        USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
    }

    USB_OTG_BSP_mDelay(50);

    return status;
}

/**
* @brief  USB_OTG_GetMode : Get current mode
* @param  pdev : Selected device
* @retval current mode
*/
uint8_t USB_OTG_GetMode(USB_OTG_CORE_HANDLE *pdev)
{
    return ((USB_OTG_READ_REG8(&pdev->regs.DYNFIFOREGS->DEVCTL) & 0x4) >> 2);
}

/**
* @brief  USB_OTG_IsDeviceMode : Check if it is device mode
* @param  pdev : Selected device
* @retval num_in_ep
*/
uint8_t USB_OTG_IsDeviceMode(USB_OTG_CORE_HANDLE *pdev)
{
    return (USB_OTG_GetMode(pdev) != HOST_MODE);
}

/**
* @brief  USB_OTG_IsHostMode : Check if it is host mode
* @param  pdev : Selected device
* @retval num_in_ep
*/
uint8_t USB_OTG_IsHostMode(USB_OTG_CORE_HANDLE *pdev)
{
    return (USB_OTG_GetMode(pdev) == HOST_MODE);
}

/**
* @brief  USB_OTG_ReadCoreItr : returns the Core Interrupt register
* @param  pdev : Selected device
* @retval Status
*/
uint8_t USB_OTG_ReadCoreItr(USB_OTG_CORE_HANDLE *pdev)
{
    uint8_t v = 0;
    v = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->INTRUSB);
    v &= USB_OTG_READ_REG8(&pdev->regs.COMMREGS->INTRUSBE);
    return v;
}

/**
* @brief  USB_OTG_ReadOtgItr : returns the USB_OTG Interrupt register
* @param  pdev : Selected device
* @retval Status
*/
uint8_t USB_OTG_ReadOtgItr(USB_OTG_CORE_HANDLE *pdev)
{
    return (USB_OTG_READ_REG8(&pdev->regs.COMMREGS->INTRUSB));
}

#ifdef USE_HOST_MODE
/**
* @brief  USB_OTG_CoreInitHost : Initializes USB_OTG controller+ for host mode
* @param  pdev : Selected device
* @retval status
*/
USB_OTG_STS USB_OTG_CoreInitHost(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_STS                     status = USB_OTG_OK;
    uint32_t Trx_fifo_size = 0, Ttx_fifo_size = 0;
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef    txcsrl;
    USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef    rxcsrl;

    txcsrl.d8 = 0;
    rxcsrl.d8 = 0;
    txcsrl.b.flush_fifo = 1;
    rxcsrl.b.flush_fifo = 1;
    /* Test */
    pdev->regs.CSRREGS[1]->TXMAXP = 0x0040;
    pdev->regs.CSRREGS[1]->TXCSRL = 0x48; // ClrDataTog, FlushFIFO.
    pdev->regs.CSRREGS[1]->TXCSRH = 0x28; // TxMode, FrcDataTog.

    // FIFO tot size = 512.
    // 0-63, TP0, 64-127, TP1 Txd, 128-191, TP1, RXD.
    pdev->regs.COMMREGS->INDEX = 1;
    pdev->regs.DYNFIFOREGS->TXFIFOSZ = 3; // 64
    pdev->regs.DYNFIFOREGS->TXFIFOADD = 8;
    pdev->regs.DYNFIFOREGS->RXFIFOSZ = 3; // 64
    pdev->regs.DYNFIFOREGS->RXFIFOADD = 16;
    pdev->regs.COMMREGS->INDEX = 0;

    pdev->regs.COMMREGS->INDEX = 2;
    pdev->regs.DYNFIFOREGS->TXFIFOSZ = 3; // 64
    pdev->regs.DYNFIFOREGS->TXFIFOADD = 24;
    pdev->regs.DYNFIFOREGS->RXFIFOSZ = 3; // 64
    pdev->regs.DYNFIFOREGS->RXFIFOADD = 32;
    pdev->regs.COMMREGS->INDEX = 0;
    if (pdev->cfg.coreID == USB_OTG_FS_CORE_ID) {
//      /* Set Full speed phy */
//        USB_OTG_InitDevSpeed (pdev , USB_OTG_SPEED_PARAM_FULL);
//        /* FIFO total size = 512 bytes */
//        /* EP0 */
//        USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INDEX, 0);
//        /* set Rx FIFO size */
//        USB_OTG_WRITE_REG16(&pdev->regs.DYNFIFOREGS->RXFIFOADD, 0);
//        USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->RXFIFOSZ,USB_OTG_FifosizeReg(RX0_FIFO_FS_SIZE));
//        /* set Tx FIFO size */
//        USB_OTG_WRITE_REG16(&pdev->regs.DYNFIFOREGS->TXFIFOADD,0);
//        USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->TXFIFOSZ,USB_OTG_FifosizeReg(TX0_FIFO_FS_SIZE));
//        /* EP1 */
//        Trx_fifo_size = ((RX0_FIFO_FS_SIZE > TX0_FIFO_FS_SIZE) ?  RX0_FIFO_FS_SIZE : TX0_FIFO_FS_SIZE);
//        USB_OTG_EpFifoConfiguration(pdev,
//                                    1,
//                                    Trx_fifo_size,
//                                    RX1_FIFO_FS_SIZE,
//                                    TX1_FIFO_FS_SIZE);
//        /* EP2 */
//        Trx_fifo_size += RX1_FIFO_FS_SIZE + TX1_FIFO_FS_SIZE;
//        USB_OTG_EpFifoConfiguration(pdev,
//                                    2,
//                                    Trx_fifo_size,
//                                    RX2_FIFO_FS_SIZE,
//                                    TX2_FIFO_FS_SIZE);
//
//        /* EP3 */
//        Trx_fifo_size += RX2_FIFO_FS_SIZE + TX2_FIFO_FS_SIZE;
//        USB_OTG_EpFifoConfiguration(pdev,
//                                    3,
//                                    Trx_fifo_size,
//                                    RX3_FIFO_FS_SIZE,
//                                    TX3_FIFO_FS_SIZE);
//        /* EP4 */
//        Trx_fifo_size += RX3_FIFO_FS_SIZE + TX3_FIFO_FS_SIZE;
//        USB_OTG_EpFifoConfiguration(pdev,
//                                    4,
//                                    Trx_fifo_size,
//                                    RX4_FIFO_FS_SIZE,
//                                    TX4_FIFO_FS_SIZE);
//        /* EP5 */
//        Trx_fifo_size += RX4_FIFO_FS_SIZE + TX4_FIFO_FS_SIZE;
//        USB_OTG_EpFifoConfiguration(pdev,
//                                    5,
//                                    Trx_fifo_size,
//                                    RX5_FIFO_FS_SIZE,
//                                    TX5_FIFO_FS_SIZE);
//        /* EP6 */
//        Trx_fifo_size += RX5_FIFO_FS_SIZE + TX5_FIFO_FS_SIZE;
//        USB_OTG_EpFifoConfiguration(pdev,
//                                    6,
//                                    Trx_fifo_size,
//                                    RX6_FIFO_FS_SIZE,
//                                    TX6_FIFO_FS_SIZE);
//        /* EP7 */
//        Trx_fifo_size += RX6_FIFO_FS_SIZE + TX6_FIFO_FS_SIZE;
//        USB_OTG_EpFifoConfiguration(pdev,
//                                    7,
//                                    Trx_fifo_size,
//                                    RX7_FIFO_FS_SIZE,
//                                    TX7_FIFO_FS_SIZE);
    }
    /* Clear all pending Device Interrupts */
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSBE, 0x00);
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSB, 0x00);
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRRXE, 0x00);
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRRX, 0x00);
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRTXE, 0x00);
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRTX, 0x00);
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INDEX, 0);

    USB_OTG_EnableHostInt(pdev);
    return status;
}

/**
* @brief  USB_OTG_IsEvenFrame
*         This function returns the frame number for sof packet
* @param  pdev : Selected device
* @retval Frame number
*/
uint8_t USB_OTG_IsEvenFrame(USB_OTG_CORE_HANDLE *pdev)
{
    return !(USB_OTG_READ_REG16(&pdev->regs.COMMREGS->FRAME) & 0x1);
}

/**
* @brief  USB_OTG_DriveVbus : set/reset vbus
* @param  pdev : Selected device
* @param  state : VBUS state
* @retval None
*/
void USB_OTG_DriveVbus(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{

}
/**
* @brief  USB_OTG_EnableHostInt: Enables the Host mode interrupts
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EnableHostInt(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_STS       status = USB_OTG_OK;
    USB_OTG_INTRUSBE_TypeDef intr_usbe;
    USB_OTG_DMA_INTR_TypeDef dma_intr;

    /* Disable all interrupts. */
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSBE, 0);
    /* Clear any pending interrupts */
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSB, 0);
    /* Enable the common interrupts */
    USB_OTG_EnableCommonInt(pdev);

    dma_intr.d8 = 0;
    /* Only One DMA Intr register */
    USB_OTG_WRITE_REG8(&pdev->regs.DMAREGS[0]->DMA_INTR, dma_intr.d8);

    /* Enable interrupts matching to the Host mode ONLY */
    intr_usbe.d8 = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->INTRUSBE);
    intr_usbe.b.en_VBus_error = 1;
    intr_usbe.b.en_sess_req = 1;
    intr_usbe.b.en_discon = 1;
    intr_usbe.b.en_conn = 1;
    intr_usbe.b.en_sof = 1;
    intr_usbe.b.en_reset_babble = 1;
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSBE, intr_usbe.d8);

    return status;
}

/**
* @brief  USB_OTG_InitFSLSPClkSel : Initializes the FSLSPClkSel field of the
*         HCFG register on the PHY type
* @param  pdev : Selected device
* @param  freq : clock frequency
* @retval None
*/
void USB_OTG_InitFSLSPClkSel(USB_OTG_CORE_HANDLE *pdev, uint8_t freq)
{

}

/**
* @brief  USB_OTG_ReadHPRT0 : Reads HPRT0 to modify later
* @param  pdev : Selected device
* @retval HPRT0 value
*/
uint32_t USB_OTG_ReadHPRT0(USB_OTG_CORE_HANDLE *pdev)
{

}

/**
* @brief  USB_OTG_ReadHostAllChannels_intr : Register PCD Callbacks
* @param  pdev : Selected device
* @retval Status
*/
uint8_t USB_OTG_ReadHostAllChannels_intr(USB_OTG_CORE_HANDLE *pdev)
{
    return (USB_OTG_READ_REG8(&pdev->regs.COMMREGS->INTRUSB));
}

/**
* @brief  USB_OTG_ResetPort : Reset Host Port
* @param  pdev : Selected device
* @retval status
* @note : (1)The application must wait at least 10 ms (+ 10 ms security)
*   before clearing the reset bit.
*/
uint32_t USB_OTG_ResetPort(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_POWER_TypeDef power;

    power.d8 = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->POWER);
    power.b.en_suspendM = 1;
    power.b.reset = 1;
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
    USB_OTG_BSP_mDelay(100);
    power.b.en_suspendM = 1;
    power.b.reset = 0;
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
    USB_OTG_BSP_mDelay(200);

    return 1;
}

/**
* @brief  USB_OTG_HC_Init : Prepares a host channel for transferring packets
* @param  pdev : Selected device
* @param  hc_num : channel number
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_HC_Init(USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num)
{
    USB_OTG_STS status = USB_OTG_OK;
    uint32_t intr_enable = 0;
    USB_OTG_TYPE0_TypeDef  type0;
    USB_OTG_TXTYPE_TypeDef tx_type;
    USB_OTG_RXTYPE_TypeDef rx_type;
    USB_OTG_RXCSRH_IN_HOST_TypeDef  rx_csrh;
    USB_OTG_RXCSRL_IN_HOST_TypeDef  rx_csrl;
    USB_OTG_TXCSRH_IN_HOST_TypeDef  tx_csrh;
    USB_OTG_TXCSRL_IN_HOST_TypeDef  tx_csrl;
    type0.d8 = 0x00;
    tx_type.d8 = 0x00;
    rx_type.d8 = 0x00;
    rx_csrh.d8 = 0x00;
    rx_csrl.d8 = 0x00;
    tx_csrh.d8 = 0x00;
    tx_csrl.d8 = 0x00;

    switch (pdev->host.hc[hc_num].ep_type) {
    case EP_TYPE_CTRL:
        type0.b.speed = pdev->host.hc[hc_num].speed;
        /* Speed */
        USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->TXPE.TYPE0, type0.d8);
        break;
    case EP_TYPE_BULK:
        if (pdev->host.hc[hc_num].ep_is_in) {
            rx_type.b.speed = pdev->host.hc[hc_num].speed;
            rx_type.b.protocol = EP_TYPE_BULK;
            rx_type.b.target_EP_number = pdev->host.hc[hc_num].ep_num;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXTYPE, rx_type.d8);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXMAXP, \
                               pdev->host.hc[hc_num].max_packet);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXINTERVAL, 3);
            /* If the CPU sets this bit, the ReqPkt bit will be automatically set when the RxPktRdy bit is cleared.
             * Note: This bit is automatically cleared when a short packet is received.
             */
            rx_csrh.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRH);
            rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
            rx_csrh.b.auto_req = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRH, rx_csrh.d8);
            /* When the endpoint is first configured, the endpoint data toggle should be set to 0 */
            rx_csrl.b.clr_data_tog = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
            /* Clear FIFO */
            rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
            if (rx_csrl.b.rx_pkt_rdy) {
                rx_csrl.b.flush_fifo = 1;
                USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
                rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
                if (rx_csrl.b.rx_pkt_rdy) {
                    rx_csrl.b.flush_fifo = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
                }
            }
        } else {
            tx_type.b.speed = pdev->host.hc[hc_num].speed;
            tx_type.b.protocol = EP_TYPE_BULK;
            tx_type.b.target_EP_number = pdev->host.hc[hc_num].ep_num;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXTYPE, tx_type.d8);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXMAXP, \
                               pdev->host.hc[hc_num].max_packet);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXINTERVAL, 3);
        }
        break;
    case EP_TYPE_INTR:
        if (pdev->host.hc[hc_num].ep_is_in) {
            rx_type.b.speed = pdev->host.hc[hc_num].speed;
            rx_type.b.protocol = EP_TYPE_INTR;
            rx_type.b.target_EP_number = pdev->host.hc[hc_num].ep_num;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXTYPE, rx_type.d8);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXMAXP, \
                               pdev->host.hc[hc_num].max_packet);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXINTERVAL, 1);
            /* If the CPU sets this bit, the ReqPkt bit will be automatically set when the RxPktRdy bit is cleared.
             * Note: This bit is automatically cleared when a short packet is received.
             */
            rx_csrh.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRH);
            rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
//                rx_csrh.b.auto_req = 1;
//                USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRH, rx_csrh.d8);
            /* When the endpoint is first configured, the endpoint data toggle should be set to 0 */
            rx_csrl.b.clr_data_tog = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
            /* Clear FIFO */
            rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
            if (rx_csrl.b.rx_pkt_rdy) {
                rx_csrl.b.flush_fifo = 1;
                USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
                rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
                if (rx_csrl.b.rx_pkt_rdy) {
                    rx_csrl.b.flush_fifo = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
                }
            }
        } else {
            tx_type.b.speed = pdev->host.hc[hc_num].speed;
            tx_type.b.protocol = EP_TYPE_INTR;
            tx_type.b.target_EP_number = pdev->host.hc[hc_num].ep_num;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXTYPE, tx_type.d8);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXMAXP, \
                               pdev->host.hc[hc_num].max_packet);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXINTERVAL, 1);
        }
        break;
    case EP_TYPE_ISOC:
        if (pdev->host.hc[hc_num].ep_is_in) {
            rx_type.b.speed = pdev->host.hc[hc_num].speed;
            rx_type.b.protocol = EP_TYPE_ISOC;
            rx_type.b.target_EP_number = pdev->host.hc[hc_num].ep_num;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXTYPE, rx_type.d8);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXMAXP, \
                               pdev->host.hc[hc_num].max_packet);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXINTERVAL, 1);
            /* If the CPU sets this bit, the ReqPkt bit will be automatically set when the RxPktRdy bit is cleared.
             * Note: This bit is automatically cleared when a short packet is received.
             */
            rx_csrh.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRH);
            rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
            rx_csrh.b.auto_req = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRH, rx_csrh.d8);
            /* When the endpoint is first configured, the endpoint data toggle should be set to 0 */
            rx_csrl.b.clr_data_tog = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
            /* Clear FIFO */
            rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
            if (rx_csrl.b.rx_pkt_rdy) {
                rx_csrl.b.flush_fifo = 1;
                USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
                rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
                if (rx_csrl.b.rx_pkt_rdy) {
                    rx_csrl.b.flush_fifo = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
                }
            }
        } else {
            tx_type.b.speed = pdev->host.hc[hc_num].speed;
            tx_type.b.protocol = EP_TYPE_ISOC;
            tx_type.b.target_EP_number = pdev->host.hc[hc_num].ep_num;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXTYPE, tx_type.d8);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXMAXP, \
                               pdev->host.hc[hc_num].max_packet);
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXINTERVAL, 1);
        }
        break;
    }
    return status;
}

/**
* @brief  USB_OTG_HC_StartXfer : Start transfer
* @param  pdev : Selected device
* @param  hc_num : channel number
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_HC_StartXfer(USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num)
{
    USB_OTG_STS status = USB_OTG_OK;
    uint16_t len_words = 0;
    uint16_t num_packets;
    uint16_t max_hc_pkt_count;
    USB_OTG_CSR0L_IN_HOST_TypeDef csr0l;
    USB_OTG_CSR0H_IN_HOST_TypeDef csr0h;
    USB_OTG_TXCSRL_IN_HOST_TypeDef tx_csrl;
    USB_OTG_RXCSRL_IN_HOST_TypeDef rx_csrl;
    max_hc_pkt_count = 256;

    /* Compute the expected number of packets associated to the transfer */
    if (pdev->host.hc[hc_num].xfer_len > 0) {
        num_packets = (pdev->host.hc[hc_num].xfer_len + \
                       pdev->host.hc[hc_num].max_packet - 1) / pdev->host.hc[hc_num].max_packet;

        if (num_packets > max_hc_pkt_count) {
            num_packets = max_hc_pkt_count;
            pdev->host.hc[hc_num].xfer_len = num_packets * \
                                             pdev->host.hc[hc_num].max_packet;
        }
    } else { /* Data-length is zero */
        num_packets = 1;
    }

    if (pdev->host.hc[hc_num].ep_is_in) {
        pdev->host.hc[hc_num].xfer_len = num_packets * \
                                         pdev->host.hc[hc_num].max_packet;
    }

//  if ((pdev->host.hc[hc_num].ep_is_in == 0) &&
//      (pdev->host.hc[hc_num].xfer_len > 0))
    if (pdev->host.hc[hc_num].ep_is_in == 0) {
        switch (pdev->host.hc[hc_num].ep_type) {
        /* Non periodic transfer */
        case EP_TYPE_CTRL:
        case EP_TYPE_BULK:
            if (pdev->host.hc[hc_num].ep_num) {
                tx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXCSRL);
                if (pdev->host.hc[hc_num].xfer_len == 0) {
                    USB_OTG_WritePacket(pdev,
                                        pdev->host.hc[hc_num].xfer_buff,
                                        pdev->host.hc[hc_num].ep_num,
                                        0);
                    pdev->host.hc[hc_num].xfer_count = 0;
                    tx_csrl.b.tx_pkt_rdy = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXCSRL, tx_csrl.d8);
                } else if (pdev->host.hc[hc_num].xfer_len >= 64) {
                    USB_OTG_WritePacket(pdev,
                                        pdev->host.hc[hc_num].xfer_buff,
                                        pdev->host.hc[hc_num].ep_num,
                                        64);
                    pdev->host.hc[hc_num].xfer_count += 64;
                    tx_csrl.b.tx_pkt_rdy = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXCSRL, tx_csrl.d8);
                } else {
                    /* Write packet into the Tx FIFO. */
                    USB_OTG_WritePacket(pdev,
                                        pdev->host.hc[hc_num].xfer_buff,
                                        pdev->host.hc[hc_num].ep_num,
                                        pdev->host.hc[hc_num].xfer_len);
                    pdev->host.hc[hc_num].xfer_count = pdev->host.hc[hc_num].xfer_len;
                    tx_csrl.b.tx_pkt_rdy = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXCSRL, tx_csrl.d8);
                }
            } else {
                csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
                if (pdev->host.hc[hc_num].xfer_len == 0) {
                    /* Write packet into the Tx FIFO. */
                    USB_OTG_WritePacket(pdev,
                                        pdev->host.hc[hc_num].xfer_buff,
                                        pdev->host.hc[hc_num].ep_num,
                                        0);
                    pdev->host.hc[hc_num].xfer_count += pdev->host.hc[hc_num].xfer_len;
                    csr0l.b.tx_pkt_rdy = 1;
                    csr0l.b.status_pkt = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
                } else if (pdev->host.hc[hc_num].xfer_len >= 64) {
                    USB_OTG_WritePacket(pdev,
                                        pdev->host.hc[hc_num].xfer_buff,
                                        pdev->host.hc[hc_num].ep_num,
                                        64);
                    pdev->host.hc[hc_num].xfer_count += 64;
                    csr0l.b.tx_pkt_rdy = 1;
                    csr0l.b.setup_pkt = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
                } else {
                    /* Write packet into the Tx FIFO. */
                    USB_OTG_WritePacket(pdev,
                                        pdev->host.hc[hc_num].xfer_buff,
                                        pdev->host.hc[hc_num].ep_num,
                                        pdev->host.hc[hc_num].xfer_len);
                    pdev->host.hc[hc_num].xfer_count += pdev->host.hc[hc_num].xfer_len;
                    csr0l.b.tx_pkt_rdy = 1;
                    csr0l.b.setup_pkt = 1;
                    USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
                }
            }
            break;
        /* Periodic transfer */
        case EP_TYPE_INTR:
        case EP_TYPE_ISOC:
            tx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXCSRL);
            if (pdev->host.hc[hc_num].xfer_len == 0) {
                /* Write packet into the Tx FIFO. */
                USB_OTG_WritePacket(pdev,
                                    pdev->host.hc[hc_num].xfer_buff,
                                    pdev->host.hc[hc_num].ep_num,
                                    0);
                pdev->host.hc[hc_num].xfer_count += pdev->host.hc[hc_num].xfer_len;
                tx_csrl.b.tx_pkt_rdy = 1;
                USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXCSRL, tx_csrl.d8);
            } else if (pdev->host.hc[hc_num].xfer_len >= 64) {
                USB_OTG_WritePacket(pdev,
                                    pdev->host.hc[hc_num].xfer_buff,
                                    pdev->host.hc[hc_num].ep_num,
                                    64);
                pdev->host.hc[hc_num].xfer_count += 64;
                tx_csrl.b.tx_pkt_rdy = 1;
                USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXCSRL, tx_csrl.d8);
            } else {
                /* Write packet into the Tx FIFO. */
                USB_OTG_WritePacket(pdev,
                                    pdev->host.hc[hc_num].xfer_buff,
                                    pdev->host.hc[hc_num].ep_num,
                                    pdev->host.hc[hc_num].xfer_len);
                pdev->host.hc[hc_num].xfer_count = pdev->host.hc[hc_num].xfer_len;
                tx_csrl.b.tx_pkt_rdy = 1;
                USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->TXCSRL, tx_csrl.d8);
            }
            break;

        default:
            break;
        }
    } else {
        switch (pdev->host.hc[hc_num].ep_type) {
        /* Non periodic transfer */
        case EP_TYPE_CTRL:
            csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
            csr0h.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRH.CSR0H);
            csr0l.b.req_pkt = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
            break;
        case EP_TYPE_BULK:
        /* Periodic transfer */
        case EP_TYPE_INTR:
        case EP_TYPE_ISOC:
            rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL);
            rx_csrl.b.ReqPkt = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[pdev->host.hc[hc_num].ep_num]->RXCSRL, rx_csrl.d8);
            break;
        default:
            break;
        }
    }
    return status;
}


/**
* @brief  USB_OTG_HC_Halt : Halt channel
* @param  pdev : Selected device
* @param  hc_num : channel number
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_HC_Halt(USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num)
{
    return USB_OTG_OK;
}

/**
* @brief  Issue a ping token
* @param  None
* @retval : None
*/
USB_OTG_STS USB_OTG_HC_DoPing(USB_OTG_CORE_HANDLE *pdev, uint8_t hc_num)
{
    return USB_OTG_OK;
}

/**
* @brief  Stop the device and clean up fifo's
* @param  None
* @retval : None
*/
void USB_OTG_StopHost(USB_OTG_CORE_HANDLE *pdev)
{

}
#endif

#ifdef USE_DEVICE_MODE
/*         PCD Core Layer       */
/**
* @brief  USB_OTG_CoreInitDev : Initializes the USB_OTG controller registers
*         for device mode
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_CoreInitDev(USB_OTG_CORE_HANDLE* pdev)
{
    USB_OTG_STS status = USB_OTG_OK;

    if (pdev->cfg.coreID == USB_OTG_FS_CORE_ID) {
        /* Set Full speed phy */
        USB_OTG_InitDevSpeed(pdev, USB_OTG_SPEED_PARAM_FULL);
    }
    /* Clear all pending Device Interrupts */
    USB_OTG_READ_REG8(&pdev->regs.COMMREGS->INTRUSB);
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSBE, 0x00);
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSB, 0x00);
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRRXE, 0x00);
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRRX, 0x00);
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRTXE, 0x00);
    USB_OTG_WRITE_REG16(&pdev->regs.COMMREGS->INTRTX, 0x00);
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INDEX, 0);

    USB_OTG_EnableDevInt(pdev);
    return status;
}

/**
* @brief  USB_OTG_EnableDevInt : Enables the Device mode interrupts
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EnableDevInt(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_INTRUSBE_TypeDef intr_usbe;

    /* Disable all interrupts. */
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSBE, 0);
    /* Clear any pending interrupts */
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSB, 0);
    /* Enable the common interrupts */
    USB_OTG_EnableCommonInt(pdev);

    intr_usbe.d8 = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->INTRUSBE);
    intr_usbe.b.en_discon = 1;
    intr_usbe.b.en_sof = 1;
    intr_usbe.b.en_reset_babble = 1;
//  intr_usbe.b.en_suspend = 1;
//  intr_usbe.b.en_resume = 1;
    USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INTRUSBE, intr_usbe.d8);

    return status;
}

/**
* @brief  USB_OTG_GetDeviceSpeed
*         Get the device speed from the device status register
* @param  None
* @retval status
*/
enum USB_OTG_SPEED USB_OTG_GetDeviceSpeed(USB_OTG_CORE_HANDLE *pdev)
{
    return USB_SPEED_FULL;
}

/**
* @brief  enables EP0 OUT to receive SETUP packets and configures EP0
*   for transmitting packets
* @param  None
* @retval USB_OTG_STS : status
*/
USB_OTG_STS  USB_OTG_EP0Activate(USB_OTG_CORE_HANDLE *pdev)
{
    return USB_OTG_OK;
}

/**
* @brief  USB_OTG_EPActivate : Activates an EP
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPActivate(USB_OTG_CORE_HANDLE* pdev, USB_OTG_EP* ep)
{
    USB_OTG_STS             status = USB_OTG_OK;
    USB_OTG_INTRRXE_TypeDef intr_rxe;
    USB_OTG_INTRTXE_TypeDef intr_txe;

    // Setup the fifo addr and size of endpoint
    if (!ep->is_fifo_allocated) {
        if (ep->num) {
            // assert if the UsedFifoSize overflow
            assert_param((pdev->cfg.UsedFifoSize + ep->maxpacket) <= pdev->cfg.TotalFifoSize);

            USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INDEX, ep->num);
            if (ep->is_in) {
                USB_OTG_WRITE_REG16(&pdev->regs.INDEXREGS->TXMAXP, ep->maxpacket);
                USB_OTG_WRITE_REG16(&pdev->regs.DYNFIFOREGS->TXFIFOADD, pdev->cfg.UsedFifoSize >> 3);
                USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->TXFIFOSZ, USB_OTG_FifosizeReg(ep->maxpacket));
            } else {
                USB_OTG_WRITE_REG16(&pdev->regs.INDEXREGS->RXMAXP, ep->maxpacket);
                USB_OTG_WRITE_REG16(&pdev->regs.DYNFIFOREGS->RXFIFOADD, pdev->cfg.UsedFifoSize >> 3);
                USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->RXFIFOSZ, USB_OTG_FifosizeReg(ep->maxpacket));
            }
            pdev->cfg.UsedFifoSize += ep->maxpacket;
            USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->INDEX, 0);
        } else {
            // Reset EP0 UsedFifoSize to default fix value 64
            pdev->cfg.UsedFifoSize = 64;
        }

        ep->is_fifo_allocated = 1;
    }

    if (ep->is_in) {
        intr_txe.d16 = 1 << ep->num;
        USB_OTG_MODIFY_REG16(&pdev->regs.COMMREGS->INTRTXE, 0, intr_txe.d16);
    } else if (ep->num) {
        intr_rxe.d16 = 1 << ep->num;
        USB_OTG_MODIFY_REG16(&pdev->regs.COMMREGS->INTRRXE, 0, intr_rxe.d16);
    }

    return status;
}

/**
* @brief  USB_OTG_EPDeactivate : Deactivates an EP
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPDeactivate(USB_OTG_CORE_HANDLE* pdev, USB_OTG_EP* ep)
{
    USB_OTG_STS status = USB_OTG_OK;

    USB_OTG_INTRRXE_TypeDef intr_rxe;
    USB_OTG_INTRTXE_TypeDef intr_txe;
    if (ep->is_in == 1) {
        intr_txe.d16 = 1 << ep->num;
        USB_OTG_MODIFY_REG16(&pdev->regs.COMMREGS->INTRTXE, intr_txe.d16, 0);
    } else {
        intr_rxe.d16 = 1 << ep->num;
        USB_OTG_MODIFY_REG16(&pdev->regs.COMMREGS->INTRRXE, intr_rxe.d16, 0);
    }
    return status;
}

/**
 * @brief  USB_OTG_EPStartXfer : Handle the setup for data xfer for an EP and
 *         starts the xfer
 * @param  pdev : Selected device
 * @retval USB_OTG_STS : status
 */
USB_OTG_STS USB_OTG_EPStartXfer(USB_OTG_CORE_HANDLE* pdev, USB_OTG_EP* ep)
{
    USB_OTG_STS                          status = USB_OTG_OK;
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef tx_csrl;
    USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef rx_csrl;
    USB_OTG_RXCOUNT_TypeDef              rx_count;

    /* IN endpoint */
    if (ep->is_in) {
        ep->xfer_len = MIN(ep->rem_data_len, ep->maxpacket);
        if (ep->xfer_len) {
            USB_OTG_WritePacket(pdev, ep->xfer_buff + ep->total_data_len - ep->rem_data_len, ep->num, ep->xfer_len);
            ep->rem_data_len -= ep->xfer_len;
        }
        if (ep->xfer_count)
            ep->xfer_count--;

        tx_csrl.d8           = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL);
        tx_csrl.b.tx_pkt_rdy = 1;
        USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL, tx_csrl.d8);
    } else {
        /* OUT endpoint */
        rx_csrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[ep->num]->RXCSRL);
        if (ep->xfer_len == 0) {
            ep->rem_data_len     = 0;
            ep->xfer_count       = ep->xfer_len;
            rx_csrl.b.rx_pkt_rdy = 0;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->RXCSRL, rx_csrl.d8);
        } else {
            rx_count.d16 = USB_OTG_READ_REG16(&pdev->regs.CSRREGS[ep->num]->RXCOUNT);
            USB_OTG_ReadPacket(pdev, ep->xfer_buff + ep->xfer_count, ep->num, rx_count.d16);
            ep->xfer_count += rx_count.d16;
            if (ep->xfer_len >= ep->xfer_count) {
                ep->rem_data_len = ep->xfer_len - ep->xfer_count;
            } else {
                ep->rem_data_len = 0;
                ep->xfer_count   = ep->xfer_len;
            }
            rx_csrl.b.rx_pkt_rdy = 0;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->RXCSRL, rx_csrl.d8);
        }
    }
    return status;
}

/**
* @brief  USB_OTG_EP0StartXfer : Handle the setup for a data xfer for EP0 and
*         starts the xfer
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EP0StartXfer(USB_OTG_CORE_HANDLE* pdev, USB_OTG_EP* ep)
{
    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef csr0l;
    /* IN endpoint */
    if (ep->is_in && ep->xfer_count) {
        ep->xfer_len = MIN(ep->rem_data_len, ep->maxpacket);
        if (ep->xfer_len) {
            USB_OTG_WritePacket(pdev, ep->xfer_buff + ep->total_data_len - ep->rem_data_len, 0, ep->xfer_len);
            ep->rem_data_len -= ep->xfer_len;
        }
        if (ep->xfer_count)
            ep->xfer_count--;

        csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
        if (ep->xfer_count == 0)
            csr0l.b.data_end = 1;
        csr0l.b.tx_pkt_rdy = 1;
        USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
    }
    return USB_OTG_OK;
}

/**
* @brief  USB_OTG_EPSetStall : Set the EP STALL
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPSetStall(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef csr0l;
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef txcsrl;
    USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef rxcsrl;

    if (ep->is_in == 1) {
        if (ep->num) {
            txcsrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL);
            /* set the stall bit */
            txcsrl.b.send_stall = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL, txcsrl.d8);
        } else {
            csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
            /* set the stall bit */
            csr0l.b.send_stall = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
        }
    } else { /* OUT Endpoint */
        if (ep->num) {
            rxcsrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[ep->num]->RXCSRL);
            /* set the stall bit */
            rxcsrl.b.send_stall = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->RXCSRL, rxcsrl.d8);
        } else {
            csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
            /* set the stall bit */
            csr0l.b.send_stall = 1;
            USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
        }
    }
    return status;
}

/**
* @brief  Clear the EP STALL
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPClearStall(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;

    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef csr0l;
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef txcsrl;
    USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef rxcsrl;

    if (ep->is_in == 1) {
        if (ep->num) {
            txcsrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL);
            /* set the stall bit */
            txcsrl.b.sent_stall = 0;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL, txcsrl.d8);
        } else {
            csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
            /* set the stall bit */
            csr0l.b.sent_stall = 0;
            USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
        }
    } else { /* OUT Endpoint */
        if (ep->num) {
            rxcsrl.d8 = USB_OTG_READ_REG8(&pdev->regs.CSRREGS[ep->num]->RXCSRL);
            /* set the stall bit */
            rxcsrl.b.sent_stall = 0;
            USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->RXCSRL, rxcsrl.d8);
        } else {
            csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
            /* set the stall bit */
            csr0l.b.sent_stall = 0;
            USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
        }
    }
    return status;
}

/**
* @brief  USB_OTG_ReadDevAllOutEp_itr : returns OUT endpoint interrupt bits
* @param  pdev : Selected device
* @retval OUT endpoint interrupt bits
*/
uint16_t USB_OTG_ReadDevAllOutEp_itr(USB_OTG_CORE_HANDLE *pdev)
{
    uint16_t v;
    v  = USB_OTG_READ_REG16(&pdev->regs.COMMREGS->INTRRX);
    v &= USB_OTG_READ_REG16(&pdev->regs.COMMREGS->INTRRXE);
    return ((v & 0xFE));
}

/**
* @brief  USB_OTG_ReadDevOutEP_itr : returns Device OUT EP Interrupt register
* @param  pdev : Selected device
* @param  ep : end point number
* @retval Device OUT EP Interrupt register
*/
uint16_t USB_OTG_ReadDevOutEP_itr(USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
    uint16_t v;
    v  = USB_OTG_READ_REG16(&pdev->regs.COMMREGS->INTRRX & (0x01 << epnum));
    v &= USB_OTG_READ_REG16(&pdev->regs.COMMREGS->INTRRXE);
    return v;
}

/**
* @brief  USB_OTG_ReadDevAllInEPItr : Get int status register
* @param  pdev : Selected device
* @retval int status register
*/
uint16_t USB_OTG_ReadDevAllInEPItr(USB_OTG_CORE_HANDLE *pdev)
{
    uint16_t v;
    v = USB_OTG_READ_REG16(&pdev->regs.COMMREGS->INTRTX);
    v &= USB_OTG_READ_REG16(&pdev->regs.COMMREGS->INTRTXE);
    return (v & 0xFF);
}

/**
* @brief  configures EPO to receive SETUP packets
* @param  None
* @retval : None
*/
void USB_OTG_EP0_OutStart(USB_OTG_CORE_HANDLE *pdev)
{

}

/**
 * @brief  USB_OTG_RemoteWakeup : active remote wakeup signalling
 * @param  None
 * @retval : None
 */
void USB_OTG_ActiveRemoteWakeup(USB_OTG_CORE_HANDLE* pdev)
{
    USB_OTG_POWER_TypeDef power;
    /* Note: If CLK has been stopped,it will need be restarted before
     * this write can occur.
     */
    if (pdev->dev.DevRemoteWakeup) {
        power.d8       = USB_OTG_READ_REG8(&pdev->regs.COMMREGS->POWER);
        power.b.resume = 1;
        USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
        /* The software should leave then this bit set for approximately 10ms
         * (minimum of 2ms, a maximum of 15ms) before resetting it to 0.
         */
        USB_OTG_BSP_mDelay(10);
        power.b.resume = 0;
        USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
    }
}

/**
* @brief  USB_OTG_UngateClock : active USB Core clock
* @param  None
* @retval : None
*/
void USB_OTG_UngateClock(USB_OTG_CORE_HANDLE *pdev)
{

}

/**
* @brief  Stop the device and clean up fifo's
* @param  None
* @retval : None
*/
void USB_OTG_StopDevice(USB_OTG_CORE_HANDLE *pdev)
{

}

/**
* @brief  returns the EP Status
* @param  pdev : Selected device
*         ep : endpoint structure
* @retval : EP status
*/

uint32_t USB_OTG_GetEPStatus(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep)
{
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef  txcsrl;
    USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef  rxcsrl;
    __IO uint8_t *depctl_addr;
    uint32_t Status = 0;

    txcsrl.d8 = 0;
    rxcsrl.d8 = 0;
    if (ep->is_in == 1) {
        depctl_addr = &(pdev->regs.CSRREGS[ep->num]->TXCSRL);
        txcsrl.d8 = USB_OTG_READ_REG8(depctl_addr);

        if (txcsrl.b.sent_stall == 1)
            Status = USB_OTG_EP_TX_STALL;
        else if (txcsrl.b.under_run == 1 || txcsrl.b.tx_pkt_rdy == 1)
            Status = USB_OTG_EP_TX_NAK;
        else
            Status = USB_OTG_EP_TX_VALID;
    } else {
        depctl_addr = &(pdev->regs.CSRREGS[ep->num]->RXCSRL);
        rxcsrl.d8 = USB_OTG_READ_REG8(depctl_addr);
        if (rxcsrl.b.sent_stall == 1)
            Status = USB_OTG_EP_RX_STALL;
        else if ((rxcsrl.b.data_error == 1) || (rxcsrl.b.over_run == 1))
            Status = USB_OTG_EP_RX_NAK;
        else
            Status = USB_OTG_EP_RX_VALID;
    }

    /* Return the current status */
    return Status;
}

/**
* @brief  Set the EP Status
* @param  pdev : Selected device
*         Status : new Status
*         ep : EP structure
* @retval : None
*/
void USB_OTG_SetEPStatus(USB_OTG_CORE_HANDLE *pdev, USB_OTG_EP *ep, uint32_t Status)
{
    USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef  txcsrl;
    USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef  rxcsrl;
    __IO uint8_t *depctl_addr;

    txcsrl.d8 = 0;
    rxcsrl.d8 = 0;
    /* Process for IN endpoint */
    if (ep->is_in == 1) {
        depctl_addr = &(pdev->regs.CSRREGS[ep->num]->TXCSRL);
        txcsrl.d8 = USB_OTG_READ_REG8(depctl_addr);

        if (Status == USB_OTG_EP_TX_STALL) {
            USB_OTG_EPSetStall(pdev, ep);
            return;
        } else if (Status == USB_OTG_EP_TX_NAK) {
//            txcsrl.b.snak = 1;
        } else if (Status == USB_OTG_EP_TX_VALID) {
            if ((txcsrl.b.sent_stall == 1) || (txcsrl.b.send_stall == 1)) {
                // ep->even_odd_frame = 0;
                USB_OTG_EPClearStall(pdev, ep);
                return;
            }
            txcsrl.b.send_stall = 0;
            txcsrl.b.sent_stall = 0;
            txcsrl.b.under_run = 0;
        } else if (Status == USB_OTG_EP_TX_DIS) {
//            txcsrl.b.usbactep = 0;
        }
        USB_OTG_WRITE_REG8(depctl_addr, txcsrl.d8);
    } else { /* Process for OUT endpoint */
        depctl_addr = &(pdev->regs.CSRREGS[ep->num]->RXCSRL);
        rxcsrl.d8 = USB_OTG_READ_REG8(depctl_addr);

        if (Status == USB_OTG_EP_RX_STALL)  {
            rxcsrl.b.send_stall = 1;
        } else if (Status == USB_OTG_EP_RX_NAK) {
//            rxcsrl.b.over_run = 1;
//            rxcsrl.b.data_error = 1;
        } else if (Status == USB_OTG_EP_RX_VALID) {
            if ((rxcsrl.b.send_stall == 1) || (rxcsrl.b.sent_stall == 1)) {
                // ep->even_odd_frame = 0;
                USB_OTG_EPClearStall(pdev, ep);
                return;
            }
            rxcsrl.b.send_stall = 0;
            rxcsrl.b.sent_stall = 0;
            rxcsrl.b.data_error = 0;
            rxcsrl.b.over_run = 0;
        } else if (Status == USB_OTG_EP_RX_DIS) {
//            txcsrl.b.usbactep = 0;
        }
        USB_OTG_WRITE_REG8(depctl_addr, rxcsrl.d8);
    }
}
#endif

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
