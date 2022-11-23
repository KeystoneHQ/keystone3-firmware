/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_regs.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : USB OTG IP hardware registers.
 *****************************************************************************/


#ifndef __USB_OTG_REGS_H__
#define __USB_OTG_REGS_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "mhscpu.h"
#include "usb_conf.h"
/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/* Exported variables -------------------------------------------------------*/

#ifndef USB_OTG_MAX_EP_COUNT
#define USB_OTG_MAX_EP_COUNT 4
#endif

#ifndef USB_OTG_MAX_FIFO_SIZE
#define USB_OTG_MAX_FIFO_SIZE (512)
#endif

#define USB_OTG_FS_BASE_ADDR                                USB_BASE
#define NUM_EP_FIFO                                         USB_OTG_MAX_EP_COUNT
#define NUM_DMA_CHANNEL                                     1
#define NUM_HOSTRQPK                                        1

#define USB_OTG_COMMON_GLOBAL_REG_OFFSET                    0x000
#define USB_OTG_INDEXED_CSR_REG_OFFSET                      0x010
#define USB_OTG_EP_FIFO_GLOBAL_REG_OFFSET                   0x020
#define USB_OTG_EP_FIFO_REG_OFFSET                          0x004
#define USB_OTG_DYNFIFO_REG_OFFSET                          0x060
#define USB_OTG_ULPI_REG_OFFSET                             0x070
#define USB_OTG_TADDR_GLOBAL_REG_OFFSET                     0x080
#define USB_OTG_TADDR_REG_OFFSET                            0x008
#define USB_OTG_EP_CSR_GLOBAL_REG_OFFSET                    0x100
#define USB_OTG_EP_CSR_REG_OFFSET                           0x010
#define USB_OTG_DMA_GLOBAL_REG_OFFSET                       0x200
#define USB_OTG_DMA_REG_OFFSET                              0x010
#define USB_OTG_EXTENED_REG_OFFSET                          0x300
#define USB_OTG_LPM_REG_OFFSET                              0x360

#define USB_OTG_HS_MAX_PACKET_SIZE           512
#define USB_OTG_FS_MAX_PACKET_SIZE           64
#define USB_OTG_MAX_EP0_SIZE                 64

/* USB OTG DMA Burst Mode Select
 *          00 = Burst Mode 0: Burst of unspecified length
 *          01 = Burst Mode 1: INCR4 or unspecified length
 *          10 = Burst Mode 2: INCR8,INCR4 or unspecified length
 *          11 = Burst Mode 3: INCR16,INCR8,INCR4 or unspecified length
 */
typedef enum _USB_OTG_DMA_BRSTN_Typedef {
    USB_OTG_DMA_BRSTN_MODE0 = 0x00,
    USB_OTG_DMA_BRSTN_MODE1 = 0x01,
    USB_OTG_DMA_BRSTN_MODE2 = 0x02,
    USB_OTG_DMA_BRSTN_MODE3 =  0x03
} USB_OTG_DMA_BRSTN_Typedef;
/* USB OTG DMA Mode Select
 * When operating in DMA Mode 0, the DMA controller can be only load/unload one packet,
 * so processor intervention is required for each packet transferred over the USB. This
 * mode can be used with any endpoint,whether it uses Control,Bulk,Isochronous,or Interrupt
 * transaction(i.e.including Endpoint 0).
 *
 * When operating in DMA Mode 1, the DMA controller can be programmed to load/unload a
 * complete bulk transfer(which can be many packets). Once set up,the DMA controller will
 * load/unload all packets of the transfer,interrupting the processor only when the transfer
 * has completed. DMA Mode 1 can only be used with endpoints that use Bulk transaction.
 */
typedef enum _USB_OTG_DMA_MODE_TRANSFER_Typedef {
    USB_OTG_DMA_MODE0_TRANSFER = 0x00,
    USB_OTG_DMA_MODE1_TRANSFER = 0x01
} USB_OTG_DMA_MODE_TRANSFER_Typedef;
/* USB OTG DMA DIR Select */
typedef enum _USB_OTG_DMA_Endpoint_DIR_Typedef {
    USB_OTG_DMA_WRITE_Endpoint = 0x00,
    USB_OTG_DMA_READ_Endpoint = 0x01
} USB_OTG_DMA_Endpoint_DIR_Typedef;
/* USB OTG DMA Mode Enable */
typedef enum _USB_OTG_DMA_MODE_Typedef {
    USB_OTG_DMA_MODE_DISABLE = 0x00,
    USB_OTG_DMA_MODE_ENABLE = 0x01
} USB_OTG_DMA_MODE_Typedef;
/* USB OTG DMA INTR Enable */
typedef enum _USB_OTG_DMA_INTR_EN_Typedef {
    USB_OTG_DMA_INTR_DISABLE = 0x00,
    USB_OTG_DMA_INTR_ENABLE = 0x01
} USB_OTG_DMA_INTR_EN_Typedef;
/* USB OTG DMA assigned */
typedef enum _USB_OTG_DMA_EP_Typedef {
    USB_OTG_DMA_EP0 = 0x00,
    USB_OTG_DMA_EP1 = 0x01,
    USB_OTG_DMA_EP2 = 0x02,
    USB_OTG_DMA_EP3 = 0x03,
    USB_OTG_DMA_EP4 = 0x04,
    USB_OTG_DMA_EP5 = 0x05,
    USB_OTG_DMA_EP6 = 0x06,
    USB_OTG_DMA_EP7 = 0x07
} USB_OTG_DMA_EP_Typedef;
/* USB OTG DMA Function structure */
typedef struct _USB_OTG_DMA_Typedef {
    USB_OTG_DMA_BRSTN_Typedef brstn;
    USB_OTG_DMA_MODE_TRANSFER_Typedef transfer_mode;
    USB_OTG_DMA_Endpoint_DIR_Typedef dir;
//    USB_OTG_DMA_MODE_Typedef mode_en;
//    USB_OTG_DMA_INTR_Typedef intr_en;
    USB_OTG_DMA_EP_Typedef dma_ep_num;
    uint32_t addr;
    uint32_t count;
} USB_OTG_DMA_Typedef;

/**
  * These registers provide control and status for the complete core.
  */
typedef struct _USB_OTG_COMMREGS {
    // Common USB Registers. 0x00 - 0x0F.
    __IO    uint8_t     FADDR;          // Peripheral Mode Only.
    __IO    uint8_t     POWER;
    __I     uint16_t    INTRTX;
    __I     uint16_t    INTRRX;
    __IO    uint16_t    INTRTXE;
    __IO    uint16_t    INTRRXE;
    __I     uint8_t     INTRUSB;
    __IO    uint8_t     INTRUSBE;
    __I     uint16_t    FRAME;
    __IO    uint8_t     INDEX;
    __IO    uint8_t     TESTMODE;
} USB_OTG_COMMREGS;
/**
  * These registers provide control and status for the currently selected
  * endpoint. The registers mapped into this section depend on whether the
  * core is in Peripheral mode(DevCtr.D2 = 0) or in Host mode(DevCtr.D2 = 1)
  * and on the value of the Index register.
  */
typedef struct _USB_OTG_INDEXREGS {
    // Indexed CSR. 0x10 - 0x1F.
    __IO    uint16_t    TXMAXP;
    union {
        __IO    uint8_t     CSR0L;
        __IO    uint8_t     TXCSRL;
    } CSRL;
    union {
        __IO    uint8_t     CSR0H;
        __IO    uint8_t     TXCSRH;
    } CSRH;
    __IO    uint16_t    RXMAXP;
    __IO    uint8_t     RXCSRL;
    __IO    uint8_t     RXCSRH;
    union {
        __I     uint8_t     COUNT0;
        __I     uint16_t    RXCOUNT;
    } COUNT;
    union {
        __IO    uint8_t     TYPE0;      // Host Mode Only.
        __IO    uint8_t     TXTYPE;     // Host Mode Only.
    } TXPE;
    union {
        __IO    uint8_t     NAKLIMIT0;  // Host Mode Only.
        __IO    uint8_t     TXINTERVAL; // Host Mode Only.
    } NAKLIMIT0_TXINTERVAL;
    __IO    uint8_t     RXTYPE;         // Host Mode Only.
    __IO    uint8_t     RXINTERVAL;     // Host Mode Only.
    uint8_t     UNUSED0;
    union {
        __I     uint8_t     CONFIGDATA;
        __I     uint8_t     FIFOSIZE;
    } CONFIGDATA_FIFOSIZE;
} USB_OTG_INDEXREGS;
/**
  * This address range provides access to the endpoint FIFOs.
  */
typedef union _USB_OTG_FIFOREG {
    // FIFOS, EP15 - EP0. 0x20 - 0x5F.
    __IO    uint8_t     byte;
    __IO    uint16_t    word;
    __IO    uint32_t    dword;
} USB_OTG_FIFOREG;
/**
  * The dynamio FIFO registers are only available if the USB is configured to use Dynamic
  * FIFO Sizing.There is one set of register per Endpoint excluding EP0.These are indexed
  * registers therefore to access them the INDEX register at address 0Eh must be set to the
  * appropriate endpoint.
  */
typedef struct _USB_OTG_DYNFIFOREGS {
    // OTG, DynFIFO + Version. 0x60 - 0x6F.
    __IO    uint8_t     DEVCTL;
    __IO    uint8_t     MISC;
    __IO    uint8_t     TXFIFOSZ;
    __IO    uint8_t     RXFIFOSZ;
    __IO    uint16_t    TXFIFOADD;
    __IO    uint16_t    RXFIFOADD;
    union {
        __O     uint32_t    VCONTROL;
        __I     uint32_t    VSTATUS;
    } VCONTROL_VSTATUS;
    __I     uint16_t    HWVERS;
    uint8_t     UNUSED1[2];
} USB_OTG_DYNFIFOREGS;
/**
  *
  */
typedef struct _USB_OTG_ULPIREGS {
    // ULPI & Addnl. Config. registers. 0x70 - 0x7F.
    __IO    uint8_t     ULPIVBUSCONTROL;
    __IO    uint8_t     ULPICARKITCONTROL;
    __IO    uint8_t     ULPIINTMASK;
    __I     uint8_t     ULPIINTSRC;
    __IO    uint8_t     ULPIREGDATA;
    __IO    uint8_t     ULPIREGADDR;
    __IO    uint8_t     ULPIREGCONTROL;
    __I     uint8_t     ULPIRAWDATA;
    __I     uint8_t     EPINFO;
    __I     uint8_t     RAMINFO;
    __IO    uint8_t     LINKINFO;
    __IO    uint8_t     VPLEN;
    __IO    uint8_t     HS_EOF1;
    __IO    uint8_t     FS_EOF1;
    __IO    uint8_t     LS_EOF1;
    __IO    uint8_t     SOFT_RST;
} USB_OTG_ULPIREGS;
/**
  * When the multipoint option is enabled in the configuration GUI,these registers provide
  * target function and hub address details foe each of the endpoint.These registers can
  * only be accessed when the multipoint option is enabled.
  */
typedef struct _USB_OTG_TADDRREGS {
    // TADDR Epn (n = 0 - 15). 0x80 - 0xFF.
    __IO    uint8_t     TXFUNCADDR;
    uint8_t     UNUSED0;
    __IO    uint8_t     TXHUBADDR;
    __IO    uint8_t     TXHUBPORT;
    __IO    uint8_t     RXFUNCADDR;
    uint8_t     UNUSED1;
    __IO    uint8_t     RXHUBADDR;
    __IO    uint8_t     RXHUBPORT;
} USB_OTG_TADDRREGS;
/**
  * The registers available at 10h-1Fh, accessible independently
  * of the setting of the Index register.
  * 100h-10Fh EP0 registers; 110h-11Fh EP1 registers;
  * 120h-120Fh EP2 registers;and so on.
  */
typedef struct _USB_OTG_CSRREGS {
    // CSR EPn (n = 0 - 15). 0x100 - 0x1FF;
    __IO    uint16_t    TXMAXP;
    __IO    uint8_t     TXCSRL;
    __IO    uint8_t     TXCSRH;
    __IO    uint16_t    RXMAXP;
    __IO    uint8_t     RXCSRL;
    __IO    uint8_t     RXCSRH;
    __IO    uint16_t    RXCOUNT;
    __IO    uint8_t     TXTYPE;     // Host Mode Only.
    __IO    uint8_t     TXINTERVAL; // Host Mode Only.
    __IO    uint8_t     RXTYPE;     // Host Mode Only.
    __IO    uint8_t     RXINTERVAL; // Host Mode Only.
    uint8_t     UNUSED0;
    __I     uint8_t     FIFOSIZE;
} USB_OTG_CSRREGS;
/**
  * These registers only appear if the design is synthesized to include optional
  * DMA controller.
  */
typedef struct _USB_OTG_DMAREGS {
    // Optional DMA Registers. 0x200 - 0x2N0.
    __IO    uint32_t    DMA_INTR;       /* Only one DMA INTR register */
    __IO    uint32_t    DMA_CNTL;
    __IO    uint32_t    DMA_ADDR;
    __IO    uint32_t    DMA_COUNT;
} USB_OTG_DMAREGS;
/**
  *
  */
typedef struct _USB_OTG_EXTREGS {
    // Extended Registers. 0x300 - 0x35F.
    __IO    uint32_t    RQPKTCOUNT[NUM_HOSTRQPK]; // Host Mode Only.
    uint32_t    UNUSEDRQPK[16 - NUM_HOSTRQPK];
    __IO    uint16_t    RXDPKTBUFDIS;   // Rx DPktBufDis.
    __IO    uint16_t    TXDPKTBUFDIS;   // Tx DPktBufDis.
    __IO    uint16_t    C_T_UCH;
    __IO    uint16_t    C_T_HSRTN;
    __IO    uint16_t    C_T_HSBT;
} USB_OTG_EXTREGS;
/**
  *
  */
typedef struct _USB_OTG_LPMREGS {
    // LPM Registers. 0x360 - 0x365.
    __I     uint16_t    LPM_ATTR;
    __IO    uint8_t     LPM_CNTRL;
    __IO    uint8_t     LPM_INTREN;
    __I     uint8_t     LPM_INTR;
    __IO    uint8_t     LPM_FADDR;      // Relavant in Host mode only.
} USB_OTG_LPMREGS;

typedef union  _USB_OTG_FIFO {
    __IO uint32_t word;
    __IO uint16_t halfword;
    __IO uint8_t  byte;
} USB_OTG_FIFO;
/**
  * OTG Core registers
  */
typedef struct {
    USB_OTG_COMMREGS        *COMMREGS;
    USB_OTG_INDEXREGS       *INDEXREGS;
    USB_OTG_FIFO            *FIFO[NUM_EP_FIFO];
    USB_OTG_DYNFIFOREGS     *DYNFIFOREGS;
    USB_OTG_ULPIREGS        *ULPIREGS;
    USB_OTG_TADDRREGS       *TADDRREGS[NUM_EP_FIFO];
    USB_OTG_CSRREGS         *CSRREGS[NUM_EP_FIFO];
    USB_OTG_DMAREGS         *DMAREGS[NUM_DMA_CHANNEL];
    USB_OTG_EXTREGS         *EXTREGS;
    USB_OTG_LPMREGS         *LPMREGS;
} USB_OTG_CORE_REGS, *PUSB_OTG_CORE_REGS;

//#define USB                           ((USB_OTG_CORE_REGS *) USB_BASE)

/**
  * FAddr is an 8-bit register that should be written with the 7-bit
  * address of the peripheral part of the transaction.
  * When the MUSBMHDRC is being used in Peripheral mode(DevCtl.D2=0), this register
  * should be written with the address received through a SET_ADDRESS command, which
  * will then be used for decoding the function address in subsequent token packets.
  * Little-endian mode
  */
typedef union _USB_OTG_FADDR_TypeDef {
    uint8_t d8;
    struct {
        uint8_t func_addr   : 7;       /* The function address */
        uint8_t reserved7   : 1;       /* Unused, always return 0 */
    } b;
} USB_OTG_FADDR_TypeDef;

/**
  * Power is an 8-bit register that is used for controlling Suspend and Resume
  * signaling, and some basic operational aspects of the MUSBMHDRC.
  */
typedef union _USB_OTG_POWER_TypeDef {
    uint8_t d8;
    struct {
        uint8_t en_suspendM    : 1;
        uint8_t suspend_mode   : 1;
        uint8_t resume         : 1;
        uint8_t reset          : 1;
        uint8_t HS_mode        : 1;
        uint8_t HS_enab        : 1;
        uint8_t soft_conn      : 1;     /* Periphera mode only */
        uint8_t ISO_update     : 1;     /* Periphera mode only */
    } b;
} USB_OTG_POWER_TypeDef;
/**
  * IntrTx is 16-bit read-only register that indicates which interrupts are
  * currently active foe Endpoint 0 and the TX Endpoints 1-15.
  * Note: Bits relating to endpoints that have not been configured will always
  *       return 0. Note also that all active interrupts are clear when this
  *       register is read.
  */
typedef union _USB_OTG_INTRTX_TypeDef {
    uint16_t d16;
    struct {
        uint16_t EP0_intp        : 1;
        uint16_t EP1_tx_intp     : 1;
        uint16_t EP2_tx_intp     : 1;
        uint16_t EP3_tx_intp     : 1;
        uint16_t EP4_tx_intp     : 1;
        uint16_t EP5_tx_intp     : 1;
        uint16_t EP6_tx_intp     : 1;
        uint16_t EP7_tx_intp     : 1;
        uint16_t EP8_tx_intp     : 1;
        uint16_t EP9_tx_intp     : 1;
        uint16_t EP10_tx_intp    : 1;
        uint16_t EP11_tx_intp    : 1;
        uint16_t EP12_tx_intp    : 1;
        uint16_t EP13_tx_intp    : 1;
        uint16_t EP14_tx_intp    : 1;
        uint16_t EP15_tx_intp    : 1;
    } b;
} USB_OTG_INTRTX_TypeDef;
/**
  * IntrRx is a 16-bit read-only register that indicates which of the interrupts
  * for Rx Endpoints 1-15 are currently active.
  * Note: Bits relating to endpoints that have not been configured will always
  *       return 0. Note also that all active interrupts are cleared when this
  *       register is read.
  */
typedef union _USB_OTG_INTRRX_TypeDef {
    uint16_t d16;
    struct {
        uint16_t reserved0               : 1;
        uint16_t EP1_rx_intp             : 1;
        uint16_t EP2_rx_intp             : 1;
        uint16_t EP3_rx_intp             : 1;
        uint16_t EP4_rx_intp             : 1;
        uint16_t EP5_rx_intp             : 1;
        uint16_t EP6_rx_intp             : 1;
        uint16_t EP7_rx_intp             : 1;
        uint16_t EP8_rx_intp             : 1;
        uint16_t EP9_rx_intp             : 1;
        uint16_t EP10_rx_intp            : 1;
        uint16_t EP11_rx_intp            : 1;
        uint16_t EP12_rx_intp            : 1;
        uint16_t EP13_rx_intp            : 1;
        uint16_t EP14_rx_intp            : 1;
        uint16_t EP15_rx_intp            : 1;
    } b;
} USB_OTG_INTRRX_TypeDef;
/**
  * IntrTxE is a 16-bit register that provides interrupt enable bits for the
  * interrupts in IntrTx.
  */
typedef union _USB_OTG_INTRTXE_TypeDef {
    uint16_t d16;
    struct {
        uint16_t en_EP0_intp        : 1;
        uint16_t en_EP1_tx_intp     : 1;
        uint16_t en_EP2_tx_intp     : 1;
        uint16_t en_EP3_tx_intp     : 1;
        uint16_t en_EP4_tx_intp     : 1;
        uint16_t en_EP5_tx_intp     : 1;
        uint16_t en_EP6_tx_intp     : 1;
        uint16_t en_EP7_tx_intp     : 1;
        uint16_t en_EP8_tx_intp     : 1;
        uint16_t en_EP9_tx_intp     : 1;
        uint16_t en_EP10_tx_intp    : 1;
        uint16_t en_EP11_tx_intp    : 1;
        uint16_t en_EP12_tx_intp    : 1;
        uint16_t en_EP13_tx_intp    : 1;
        uint16_t en_EP14_tx_intp    : 1;
        uint16_t en_EP15_tx_intp    : 1;
    } b;
} USB_OTG_INTRTXE_TypeDef;
/**
  * IntrRxE is a 16-bit register that provides interrupt enable bits for the
  * interrupts in IntrRx.
  */
typedef union _USB_OTG_INTRRXE_TypeDef {
    uint16_t d16;
    struct {
        uint16_t reserved0                  : 1;
        uint16_t en_EP1_rx_intp             : 1;
        uint16_t en_EP2_rx_intp             : 1;
        uint16_t en_EP3_rx_intp             : 1;
        uint16_t en_EP4_rx_intp             : 1;
        uint16_t en_EP5_rx_intp             : 1;
        uint16_t en_EP6_rx_intp             : 1;
        uint16_t en_EP7_rx_intp             : 1;
        uint16_t en_EP8_rx_intp             : 1;
        uint16_t en_EP9_rx_intp             : 1;
        uint16_t en_EP10_rx_intp            : 1;
        uint16_t en_EP11_rx_intp            : 1;
        uint16_t en_EP12_rx_intp            : 1;
        uint16_t en_EP13_rx_intp            : 1;
        uint16_t en_EP14_rx_intp            : 1;
        uint16_t en_EP15_rx_intp            : 1;
    } b;
} USB_OTG_INTRRXE_TypeDef;
/**
  * IntrUSB is an 8-bit read-only register that indicates which USB interrupts
  * are currently active. All active interrupts will be cleared when this
  * register is read.
  */
typedef union _USB_OTG_INTRUSB_TypeDef {
    uint8_t d8;
    struct {
        uint8_t suspend         : 1;    /* Set when Suspend signaling is detected on the bus. Only valid in Peripheral mode. */
        uint8_t resume          : 1;    /* Set when Resume signaling is detected on the bus while the MUSBMHDRC is in Suspend mode. */
        uint8_t reset_babble    : 1;    /* Reset: Set in Peripheral mode when Reset signaling is detected on the bus.
                                         * Babble: Set in Host mode when babble is detected. Note: Only active after first SOF has been sent.
                                         */
        uint8_t sof             : 1;    /* Set when a new frame starts.  */
        uint8_t conn            : 1;    /* Set when a device connection is detected. Only valid in Host mode. Valid at all transaction speeds. */
        uint8_t discon          : 1;    /* Set in Host mode when a device disconnect is detected. Set in Peripheral mode when a session ends. Valid at all transaction speeds. */
        uint8_t sess_req        : 1;    /* Set when Session Request signaling has been detected. Only valid when MUSBMHDRC is A device. */
        uint8_t VBus_error      : 1;    /* Set when VBus drops below the VBus Valid threshold during a session. Only valid when MUSBMHDRC is A device. */
    } b;
} USB_OTG_INTRUSB_TypeDef;
/**
  * IntrUSBE is an 8-bit register that provides interrupt enable bits for each
  * of the interrupts in IntrUSB.
  */
typedef union _USB_OTG_INTRUSBE_TypeDef {
    uint8_t d8;
    struct {
        uint8_t en_suspend         : 1;
        uint8_t en_resume          : 1;
        uint8_t en_reset_babble    : 1;
        uint8_t en_sof             : 1;
        uint8_t en_conn            : 1;
        uint8_t en_discon          : 1;
        uint8_t en_sess_req        : 1;
        uint8_t en_VBus_error      : 1;
    } b;
} USB_OTG_INTRUSBE_TypeDef;
/**
  * Frame is a 16-bit read-only register that holds the last received frame number.
  */
typedef union _USB_OTG_FRAME_TypeDef {
    uint16_t d16;
    struct {
        uint16_t frame_number       : 11;
        uint16_t reserved11_15      : 5;    /* Always return 0 */
    } b;
} USB_OTG_FRAME_TypeDef;
/**
  * Each TX endpoint and each Rx endpoint have their own set of control/status
  * registers located between 100h-1FFh. In addition one set of TX control/status
  * and one set of  Rx control/status registers appear at 10h-19h. Index is a 4-bit
  * register that determines which endpoint control/status registers are accessed.
  * Before accessing an endpoint's control/status registers at 10h-19h, the endpoint
  * number should be written to the Index register to ensure that the correct
  * control/status registers appear in the memory map.
  */
typedef union _USB_OTG_INDEX_TypeDef {
    uint8_t d8;
    struct {
        uint8_t selected_endpoint       : 4;
        uint8_t reserved4_7             : 4;
    } b;
} USB_OTG_INDEX_TypeDef;
/**
  * Testmode is an 8-bit register that is primarily used to put the MUSBMHDRC
  * into one of the four test modes for High-speed operation described in the
  * USB 2.0 specification - in response to a SET FEATURE: TESTMODE command.
  * It is not used in normal operation.
  */
typedef union _USB_OTG_TESTMODE_TypeDef {
    uint8_t d8;
    struct {
        uint8_t test_SE0_NAK            : 1;    /* High-speed mode */
        uint8_t test_J                  : 1;    /* High-speed mode */
        uint8_t test_K                  : 1;    /* High-speed mode */
        uint8_t test_packet             : 1;    /* High-speed mode */
        uint8_t force_HS                : 1;
        uint8_t force_FS                : 1;
        uint8_t FIFO_access             : 1;    /* The CPU sets this bit to transfer the packet in the Endpoint 0 TX FIFO
                                                 * to the Endpoint 0 Rx FIFO. It is cleared automatically.
                                                 */
        uint8_t force_host              : 1;    /* The CPU sets this bit to instruct the core to enter Host mode
                                                 * when the Session bit is set, regardless of whether it is
                                                 * connected to any peripheral.
                                                 */
    } b;
} USB_OTG_TESTMODE_TypeDef;
/**
  * DevCtl is an 8-bit register that is used to select whether the MUSBMHDRC is
  * operating in Peripheral mode or in Host mode, and for controlling and
  * monitoring the USB VBus line. If the PHY is suspended no PHY clock (XCLK)
  * is received and the VBus is not sampled.
  */
typedef union _USB_OTG_DEVCTL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t session             : 1;        /* When operation as an 'A' device,this bit is set or cleared by the CPU to
                                                 * start or end a session. When operating as a 'B' device,this bit is set/cleared
                                                 * by the MUSBMHDRC when a session starts/ends. It is also set by the CPU to initiate
                                                 * the Session Request Protocol.When the MUSBMHDRC is in Suspend mode,the bit may be
                                                 * cleared by the CPU to perform a software disconnect.Note:Clearing this bit when the
                                                 * core is not suspended will result in undefined behavior.
                                                 */
        uint8_t host_req            : 1;        /* When set,the MUSBMHDRC will initiate the Host Negotiation when Suspend
                                                 * mode is entered.It is cleared when Host Negotiation is completed.
                                                 */
        uint8_t host_mode           : 1;        /* This Read-only bit is set when the MUSBMHDRC is acting as a Host. */
        uint8_t VBus                : 2;        /* These Read-only bits encode the current VBus level as follows:
                                                 *            D4      D3       Meaning
                                                 *             0       0       Below SessionEnd
                                                 *             0       1       Above SessionEnd,below AValid
                                                 *             1       0       Above AValid,below VBus Valid
                                                 *             1       1       Above VBus Valid
                                                 */
        uint8_t LSDev               : 1;        /* This Read-only bit is set when a low-speed device has been detected being
                                                 * connected to the port. Only valid in Host mode.
                                                 */
        uint8_t FSDev               : 1;        /* This Read-only bit is set when a full-speed or high-speed device has been
                                                 * detected being connected to the port.(High-speed device are distinguished
                                                 * from full-speed by checking for high-speed chirps when the device is reset.)
                                                 * Only valid in Host mode.
                                                 */
        uint8_t B_Device            : 1;        /* This Read-only bit indicates whether the MUSBMHDRC is operating
                                                 * as the 'A' device or the 'B' device.
                                                 *      0 => 'A' device;         1 => 'B' device.
                                                 * Only valid while a session is in progress. To determine the role
                                                 * when no session is in progress, set the session bit and read this bit.
                                                 * NOTE: If the core is in Force_Host mode(i.e. a session has been started
                                                 * with Testmode.D7 = 1),this bit will indicate the state of the HOSTDISCON
                                                 * input signal from the PHY.
                                                 */
    } b;
} USB_OTG_DEVCTL_TypeDef;
/**
  * The MISC Register is an 8-bit register that contain various common
  * configuration bits.  These bits include the Rx/TX Early DMA enable bits.
  */
typedef union _USB_OTG_MISC_TypeDef {
    uint8_t d8;
    struct {
        uint8_t rx_edma             : 1;        /* 1'b0:DMA_REQ signal for all OUT Endpoints will be de-asserted when MAXP
                                                 *      bytes have been read to an endpoint.This is late mode.
                                                 * 1'b1:DMA_REQ signal for all OUT Endpoints will be de-asserted when MAXP-8
                                                 *      bytes have been read to an endpoint.This is early mode.
                                                 */
        uint8_t tx_edma             : 1;        /* 1'b0:DMA_REQ signal for all IN Endpoints will be de-asserted when MAXP
                                                 *      bytes have been written to an endpoint.This is late mode.
                                                 * 1'b1:DMA_REQ signal for all IN Endpoints will be de-asserted when MAXP-8
                                                 *      bytes have been written to an endpoint.This is early mode.
                                                 */
        uint8_t reserved2_7         : 6;        /* These bits are reserved. */
    } b;
} USB_OTG_MISC_TypeDef;
/**
  * CSR0L is an 8-bit register that provides control and status bits for Endpoint 0.
  * Note: The interpretation of the register depends on whether the MUSBMHDRC is acting
  * as a peripheral or as a host.User should also be aware that the value returned when
  * the register is read reflects the status attained e.g. as a result of writing to the register.
  */
typedef union _USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef {        /* in peripheral */
    uint8_t d8;
    struct {
        uint8_t rx_pkt_rdy                  : 1;    /* This bit is set when a data packet has been received. An interrupt is
                                                     * generated when this bit is set. The CPU clear this bit by setting the
                                                     * ServicedRxPktRdy bit.
                                                     */
        uint8_t tx_pkt_rdy                  : 1;    /* The CPU sets this bits after loading a data packet into the FIFO. It is clear
                                                     * automatically when a data packet has been transmitted. An interrupt is also
                                                     * generated at this point(if enabled).
                                                     */
        uint8_t sent_stall                  : 1;    /* This bit is set when a STALL handshake is transmitted.
                                                     * The CPU should clear this bit.
                                                     */
        uint8_t data_end                    : 1;    /* The CPU sets this bit:
                                                     * 1. When setting TxPktRdy for the last data packet.
                                                     * 2. When clearing RxPktRdy after unloading the last data packet.
                                                     * 3. When setting TxPktRdy for zero length data packet.
                                                     * It is cleared automatically.
                                                     */
        uint8_t setup_end                   : 1;    /* This bit will be set when a control transaction ends before the DataEnd
                                                     * bit has been set. An interrupt will be generated and the FIFO flushed at
                                                     * this time.The bit is cleared by the CPU writing a 1 to the ServicedSetupEnd bit.
                                                     */
        uint8_t send_stall                  : 1;    /* The CPU write a 1 to this bit to terminate the current transaction.
                                                     * The STALL handshake will be transmitted and then this bit will be
                                                     * cleared automatically.
                                                     */
        uint8_t serviced_rxpktrdy           : 1;    /* The CPU write a 1 to this bit to clear the RxPktRdy bit.
                                                     * It is Cleared automatically.
                                                     */
        uint8_t serviced_setupend           : 1;    /* The CPU write a 1 to this bit to clear the SetupEnd bit.
                                                     * It is Cleared automatically.
                                                     */
    } b;
} USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef;

typedef union _USB_OTG_CSR0L_IN_HOST_TypeDef {           /* in host */
    uint8_t d8;
    struct {
        uint8_t rx_pkt_rdy                  : 1;    /* This bit is set when a data packet has been received.An interrupt is generated
                                                     * (If enabled)when this bit set.The CPU should clear this bit when the packet has
                                                     * been read from the FIFO.
                                                     */
        uint8_t tx_pkt_rdy                  : 1;    /* The CPU sets this bit after loading a data packet into the FIFO.It is cleared
                                                     * automatically when a data packet has been transmitted. An interrupt is also
                                                     * generated at this point(If enabled).
                                                     */
        uint8_t rx_stall                    : 1;    /* This bit is set when a STALL handshake is received. The CPU should clear this bit. */
        uint8_t setup_pkt                   : 1;    /* The CPU sets this bit,at the TxPktRdy bit is set,to  send a SETUP token instead
                                                     * of an OUT token for the transaction. Note: Setting this bit also clear the Data
                                                     * Toggle.
                                                     */
        uint8_t error                       : 1;    /* This bit will be set when three attempts have been made to perform a transaction
                                                     * with no response from the peripheral.The CPU should clear this bit.An interrupt
                                                     * is generated when this bit is set.
                                                     */
        uint8_t req_pkt                     : 1;    /* The CPU sets this bit to request an IN transaction.
                                                     * It is cleared when RxPktRdy is set.
                                                     */
        uint8_t status_pkt                  : 1;    /* The CPU sets this bit at the same time as the TxPktRdy or ReqPkt bit is set,
                                                     * to perform a status stage transaction. Setting this bit ensures that the data
                                                     * toggle is set to 1 so that a DATA1 packet is used for the Status Stage transaction.
                                                     */
        uint8_t nak_timeout                 : 1;    /* This bit will be set when Endpoint 0 is halted following the receipt for longer
                                                     * than the time set by the NAKLimit0 register. The CPU should clear this bit to allow
                                                     * the endpoint to continue.
                                                     */
    } b;
} USB_OTG_CSR0L_IN_HOST_TypeDef;
/**
  * CSR0H is an 8-bit register that provides control and status bits for Endpoint 0.
  * Note: The interpretation of the register depends on whether the MUSBMHDRC is acting
  * as a peripheral or as a host.User should also be aware that the value returned when
  * the register is read reflects the status attained e.g. as a result of writing to the register.
  */
typedef union _USB_OTG_CSR0H_IN_PERIPHERAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t flush_fifo              : 1;    /* The CPU writes a 1 to this bit to flush the next packet to be transmitted/read from
                                                 * the Endpoint 0 FIFO. The FIFO pointer is reset and the TxPktRdy/RxPktRdy bit(below) is
                                                 * cleared. Note:FlushFIFO should only be used when TxPktRdy/RxPktRdy is set.At other
                                                 * times, it may cause data to be corrupted.
                                                 */
        uint8_t reserved1_7             : 7;    /* Unused. Return 0 when resd. */
    } b;
} USB_OTG_CSR0H_IN_PERIPHERAL_TypeDef;

typedef union _USB_OTG_CSR0H_IN_HOST_TypeDef {
    uint8_t d8;
    struct {
        uint8_t flush_fifo              : 1;    /* The CPU writes a 1 to this bit to flush the next packet to be transmitted/read from
                                                 * the Endpoint 0 FIFO. The FIFO pointer is reset and the TxPktRdy/RxPktRdy bit(below) is
                                                 * cleared. Note:FlushFIFO should only be used when TxPktRdy/RxPktRdy is set.At other
                                                 * times, it may cause data to be corrupted.
                                                 */
        uint8_t data_toggle             : 1;    /* When read,this bit indicates the current state of the Endpoint 0 data toggle. If D10
                                                 * is high,this bit may be written with the written with the required setting of the data
                                                 * toggle.If D10 is low,any value written to this bit is ignored.
                                                 */
        uint8_t data_toggle_wr_en       : 1;    /* The CPU written a 1 to this bit to enable the current state of Endpoint 0 data toggle
                                                 * to be written(see Data Toggle bit,below).This bit is automatically cleared once the new
                                                 * value is written.
                                                 */
        uint8_t dis_ping                : 1;    /* The CPU writes a 1 to this bit to instruct the core not to issue PING tokens in data
                                                 * and status phases of a high-speed Control transfer(for use with devices that do not
                                                 * respond to PING).
                                                 */
        uint8_t reserved1_7             : 4;    /* Unused. Return 0 when resd. */
    } b;
} USB_OTG_CSR0H_IN_HOST_TypeDef;
/**
  * Count0 is a 7-bit read-only register that indicates the number of
  * received data bytes in the Endpoint 0 FIFO.
  */
typedef union _USB_OTG_COUNT0_TypeDef {
    uint8_t d8;
    struct {
        uint8_t ep0_rx_count                    : 7;
        uint8_t reserved7                       : 1;
    } b;
} USB_OTG_COUNT0_TypeDef;

/**
  * These bit only apply when the Multipoint option is enabled in the
  * configuration GUI. When multipoint is enabled Type0 is an 8-bit register of
  * which only bits 6 and 7 are implemented.These bits should be written with
  * the operating speed of the targeted.
  * Note: HOST MODE ONLY!
  */
typedef union _USB_OTG_TYPE0_TypeDef {
    uint8_t d8;
    struct {
        uint8_t reserved0_5             : 6;    /* Reserved */
        uint8_t speed                   : 2;    /* Operating speed of the target device :
                                                 *      00:Unused(Note: If selected,the target will be using the same connectionn speed as the core.)
                                                 *      01:High
                                                 *      10:Full
                                                 *      11:Low
                                                 */
    } b;
} USB_OTG_TYPE0_TypeDef;
/**
  * ConfigData is an 8-bit Read-Only register that returns information about the
  * selected core configuration.
  */
typedef union _USB_OTG_CONFIGDATA_TypeDef {
    uint8_t d8;
    struct {
        uint8_t utmi_data_width             : 1;    /* Indicates selected UTMI+ data width. Always 0 indicating 8 bits. */
        uint8_t soft_cone                   : 1;    /* Always "1" . Indicates Soft Connect/Disconnect.  */
        uint8_t dyn_fifo_sizing             : 1;    /* When set to "1" indicates Dynamic FIFO Sizing option selected.  */
        uint8_t hb_txe                      : 1;    /* When set to "1" indicates High-bandwidth TX ISO Endpoint Support selected */
        uint8_t hb_rxe                      : 1;    /* When set to "1" indicates High-bandwidth Rx ISO Endpoint Support selected.  */
        uint8_t big_endian                  : 1;    /* Always "0". Indicates Little Endian ordering.  */
        uint8_t mp_txe                      : 1;    /* When set to "1" automatic splitting of bulk packets is selected */
        uint8_t mp_rxe                      : 1;    /* When set to "1" automatic amalgamation of bulk packets is selected  */
    } b;
} USB_OTG_CONFIGDATA_TypeDef;
/**
  * NAKLimit0 is a 5-bit register that sets the number of frames/microframes(High-Speed transfers)
  * after which Endpoint 0 should timeout on receiving a stream of NAK responses.(Equivalent setting
  * for other endpoints can br made through their TxInterval and RxInterval registers).
  * The number of frames/microframes selected is 2^(m - 1)(where m is the value set in the register,
  * Valid value 2-16).If the host receives NAK responses from the target for frames than the number
  * represented by the Limit set in this register,the endpoint will be halted.
  * Note: A value of 0 or 1 disable the NAK timeout function.HOST MODE ONLY!
  */
typedef union _USB_OTG_NAKLIMIT0_TypeDef {
    uint8_t d8;
    struct {
        uint8_t Endpoint0_NAK_Limit         : 5;    /* Endpoint 0 NAK limit (m) */
        uint8_t reserved5_7                 : 3;    /* Reserve */
    } b;
} USB_OTG_NAKLIMIT0_TypeDef;
/**
  * The TxMaxP register defines the maximum amount of data that can be transferred
  * through the selected TX endpoint in a single operation. There is a TxMaxP
  * register for each TX endpoint (except Endpoint 0).
  */
typedef union _USB_OTG_TXMAXP_TypeDef {
    uint16_t d16;
    struct {
        uint16_t max_payload_tran           : 11;   /* Bits 10:0 define(in bytes)the maximum payload transmitted in single transaction.
                                                     * The value set can be up to 1024 bytes but is subject to the constraints place by
                                                     * the USB Specification on packet sizes for Bulk,Interrupt and Isochronous transfers
                                                     * in Full-speed and High-speed operations.
                                                     */
        uint16_t ex_max                     : 5;
    } b;
} USB_OTG_TXMAXP_TypeDef;
/**
  * TxCSRL is an 8-bit register that provides control and status bits for transfers
  * through the currently-selected TX endpoint. There is a TxCSRL register for each
  * configured TX endpoint (not including Endpoint 0).
  * Note:The interpretation of the register depends on whether the MUSBMHDRC is acting
  * as a peripheral or as a host.Users should also be aware that the value returned
  * when the register is read reflects the status attsined e.g. as a result of writing
  * to the register.
  */
typedef union _USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t tx_pkt_rdy                  : 1;    /* The CPU sets this bit after loading a data packet into the FIFO. */
        uint8_t fifo_not_empty              : 1;    /* The USB sets this bit when there is at least 1 packet in the TX FIFO.  */
        uint8_t under_run                   : 1;    /* The USB sets this bit if an IN token is received when TxPktRdy is not set. The CPU should clear this bit.  */
        uint8_t flush_fifo                  : 1;    /* The CPU writes a 1 to this bit to flush the latest packet from the endpoint TX FIFO */
        uint8_t send_stall                  : 1;    /* The CPU writes a 1 to this bit to issue a STALL handshake to an IN token.The CPU
                                                     * clears this bit to terminate the stall condition.
                                                     * Note:This bit has no effect where the endpoint is being used for Isochronous transfers.
                                                     */
        uint8_t sent_stall                  : 1;    /* This bit is set when a STALL handshake is transmitted.The FIFO is flushed and
                                                     * the TxPktRdy bit is cleared(see below).The CPU should clear this bit.
                                                     */
        uint8_t clr_data_tog                : 1;    /* The CPU writes a 1 to this bit to reset the endpoint data toggle to 0. */
        uint8_t incomp_tx                   : 1;    /* When the endpoint is being used for high-bandwidth Isochronous,this bit is set to
                                                     * indicate where a large packet has been split into 2 or 3 packets for transmission
                                                     * but insufficient IN tokens have been received to send all the parts.
                                                     * Note:In anything other than isochronous transfers,this bit will always return 0.
                                                     */
    } b;
} USB_OTG_TXCSRL_IN_PERIPHERAL_TypeDef;

typedef union _USB_OTG_TXCSRL_IN_HOST_TypeDef {
    uint8_t d8;
    struct {
        uint8_t tx_pkt_rdy                  : 1;    /* The CPU sets this bit after loading a data packet into the FIFO. */
        uint8_t fifo_not_empty              : 1;    /* The USB sets this bit when there is at least 1 packet in the TX FIFO.  */
        uint8_t error                       : 1;
        uint8_t flush_fifo                  : 1;    /* The CPU writes a 1 to this bit to flush the latest packet from the endpoint TX FIFO */
        uint8_t setup_Pkt                   : 1;
        uint8_t Rx_stall                    : 1;
        uint8_t clr_data_tog                : 1;    /* The CPU writes a 1 to this bit to reset the endpoint data toggle to 0. */
        uint8_t NAK_timeout_incompTx        : 1;
    } b;
} USB_OTG_TXCSRL_IN_HOST_TypeDef;
/**
  * TxCSRH is an 8-bit register that provides additional control for transfers
  * through the currently-selected TX endpoint.
  */
typedef union _USB_OTG_TXCSRH_IN_PERIPHERAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t reserved0_1                 : 2;
        uint8_t dma_req_mode                : 1;
        uint8_t frc_data_tog                : 1;
        uint8_t dma_req_enab                : 1;
        uint8_t mode                        : 1;
        uint8_t iso                         : 1;
        uint8_t auto_set                    : 1;
    } b;
} USB_OTG_TXCSRH_IN_PERIPHERAL_TypeDef;

typedef union _USB_OTG_TXCSRH_IN_HOST_TypeDef {
    uint8_t d8;
    struct {
        uint8_t data_toggle                 : 1;
        uint8_t data_toggle_wren            : 1;
        uint8_t dma_req_mode                : 1;
        uint8_t frc_data_tog                : 1;
        uint8_t dma_req_enab                : 1;
        uint8_t mode                        : 1;
        uint8_t reserved6                   : 1;
        uint8_t auto_set                    : 1;
    } b;
} USB_OTG_TXCSRH_IN_HOST_TypeDef;
/**
  * The RxMaxP register defines the maximum amount of data that can be transferred
  * through the selected Rx endpoint in a single operation. There is a RxMaxP
  * register for each Rx endpoint (except Endpoint 0).
  */
typedef union _USB_OTG_RXMAXP_TypeDef {
    uint16_t d16;
    struct {
        uint16_t max_payload_tran               : 11;
        uint16_t ex_max                         : 5;
    } b;
} USB_OTG_RXMAXP_TypeDef;
/**
  * RxCSRL is an 8-bit register that provides control and status bits for transfers
  * through the currently-selected Rx endpoint. There is a RxCSRL register for each
  * configured Rx endpoint (not including Endpoint 0).
  */
typedef union _USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t rx_pkt_rdy                  : 1;    /* This bit is set when a data packet has been received */
        uint8_t fifo_full                   : 1;    /* */
        uint8_t over_run                    : 1;    /* */
        uint8_t data_error                  : 1;    /* */
        uint8_t flush_fifo                  : 1;
        uint8_t send_stall                  : 1;    /* The CPU writes a 1 to this bit to issue a STALL handshake. */
        uint8_t sent_stall                  : 1;    /* This bit is set when a STALL handshake is transmitted. The CPU should clear this bit.  */
        uint8_t clr_data_tog                : 1;
    } b;
} USB_OTG_RXCSRL_IN_PERIPHERAL_TypeDef;

typedef union _USB_OTG_RXCSRL_IN_HOST_TypeDef {
    uint8_t d8;
    struct {
        uint8_t rx_pkt_rdy                  : 1;    /* This bit is set when a data packet has been received */
        uint8_t fifo_full                   : 1;    /* */
        uint8_t error                       : 1;    /* */
        uint8_t data_error                  : 1;    /* */
        uint8_t flush_fifo                  : 1;
        uint8_t ReqPkt                      : 1;
        uint8_t RxStall                     : 1;
        uint8_t clr_data_tog                : 1;
    } b;
} USB_OTG_RXCSRL_IN_HOST_TypeDef;
/**
  * RxCSRH is an 8-bit register that provides additional control and status bits
  * for transfers through the currently-selected Rx endpoint. There is a RxCSRH
  * register for each configured Rx endpoint (not including Endpoint 0).
  */
typedef union _USB_OTG_RXCSRH_IN_PERIPHERAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t incomp_rx               : 1;
        uint8_t reserved1_2             : 2;
        uint8_t dma_req_mode            : 1;
        uint8_t dis_nyet_pid_error      : 1;
        uint8_t dma_req_enab            : 1;
        uint8_t iso                     : 1;
        uint8_t auto_clear              : 1;
    } b;
} USB_OTG_RXCSRH_IN_PERIPHERAL_TypeDef;

typedef union _USB_OTG_RXCSRH_IN_HOST_TypeDef {
    uint8_t d8;
    struct {
        uint8_t incomp_rx               : 1;
        uint8_t data_toggle             : 1;
        uint8_t data_toggle_wren        : 1;
        uint8_t dma_req_mode            : 1;
        uint8_t PID_error               : 1;
        uint8_t dma_req_enab            : 1;
        uint8_t auto_req                : 1;
        uint8_t auto_clear              : 1;
    } b;
} USB_OTG_RXCSRH_IN_HOST_TypeDef;
/**
  * RxCount is a 14-bit read-only register that holds the number of data bytes
  * in the packet currently in line to be read from the Rx FIFO. If the packet
  * was transmitted as multiple bulk packets, the number given will be for the
  * combined packet.
  */
typedef union _USB_OTG_RXCOUNT_TypeDef {
    uint16_t d16;
    struct {
        uint16_t ep_rx_count            : 14;
        uint16_t reserved14_15          : 2;
    } b;
} USB_OTG_RXCOUNT_TypeDef;
/**
  * TxType is an 8-bit register that should be written with the endpoint number to be targeted by the endpoint, the
  * transaction protocol to use for the currently-selected TX endpoint,and its operating speed.There is a TxType register
  * for each configured TX endpoint(except Endpoint 0,which has its own Type register at 1Ah).D6-D7 are only valid when
  * the core is configured with the Multipoint option.
  * Note: HOST MODE ONLY!
  */
typedef union _USB_OTG_TXTYPE_TypeDef {
    uint8_t d8;
    struct {
        uint8_t target_EP_number        : 4;    /* Operating speed of the target device when the core is configured with the
                                                 * multipoint option:
                                                 *      00: Unused                  01: High
                                                 *      10: Full                    11: Low
                                                 * When the core is not configured with the multipoint option these bits should
                                                 * not be accessed.
                                                 */
        uint8_t protocol                : 2;    /* The CPU should set this to select the required protocol for TX endpoint:
                                                 *       00: Control       01: Isochronous
                                                 *       10: Bulk          11: Interrupt
                                                 */
        uint8_t speed                   : 2;    /* Operating speed of the target device when the core is configured with
                                                 * the multipoint option:
                                                 *      00:Unused        01: High      10: Full      11:Low
                                                 * when the core is not configured with the multipoint option these bits
                                                 * should not be accessed.
                                                 */
    } b;
} USB_OTG_TXTYPE_TypeDef;
/**
  * TxInterval is an 8-bit register that,for Interrupt and Isochronous transfers,defines the polling interval
  * for the currently-selected Tx endpoint.For Bulk endpoints,this register sets the number of frames/microframes
  * after which the endpoint should timeout on receiving a stream of NAK responses.
  * Note: HOST MODE ONLY!
  */
typedef union _USB_OTG_TXINTERVAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t Tx_polling_interval      : 8;
    } b;
} USB_OTG_TXINTERVAL_TypeDef;
/**
  * RxType is an 8-bit register that should be written with the endpoint number to be targeted by the endpoint, the
  * transaction protocol to use for the currently-selected RX endpoint,and its operating speed.There is a RxType register
  * for each configured RX endpoint(except Endpoint 0,which has its own Type register at 1Ah).D6-D7 are only valid when
  * the core is configured with the Multipoint option.
  * Note: HOST MODE ONLY!
  */
typedef union _USB_OTG_RXTYPE_TypeDef {
    uint8_t d8;
    struct {
        uint8_t target_EP_number        : 4;    /* Operating speed of the target device when the core is configured with the
                                                 * multipoint option:
                                                 *      00: Unused                  01: High
                                                 *      10: Full                    11: Low
                                                 * When the core is not configured with the multipoint option these bits should
                                                 * not be accessed.
                                                 */
        uint8_t protocol                : 2;    /* The CPU should set this to select the required protocol for TX endpoint:
                                                 *       00: Control       01: Isochronous
                                                 *       10: Bulk          11: Interrupt
                                                 */
        uint8_t speed                   : 2;    /* Operating speed of the target device when the core is configured with
                                                 * the multipoint option:
                                                 *      00:Unused        01: High      10: Full      11:Low
                                                 * when the core is not configured with the multipoint option these bits
                                                 * should not be accessed.
                                                 */
    } b;
} USB_OTG_RXTYPE_TypeDef;
/**
  * RxInterval is an 8-bit register that,for Interrupt and Isochronous transfers,defines the polling interval
  * for the currently-selected Rx endpoint.For Bulk endpoints,this register sets the number of frames/microframes
  * after which the endpoint should timeout on receiving a stream of NAK responses.
  * Note: HOST MODE ONLY!
  */
typedef union _USB_OTG_RXINTERVAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t Tx_polling_interval      : 8;
    } b;
} USB_OTG_RXINTERVAL_TypeDef;

/**
  * FIFOSize is an 8-bit Read-Only register that returns the size of the FIFOs associated with the selected
  * additional Tx/Rx endpoints.The lower nibble encodes the size of the selected TX endpoint FIFO;the upper
  * nibble encodes the size of the selected Rx endpoint FIFO.Values of 3-13 correspond to a FIFO size of 2^n
  * bytes(8-8192bytes).If an endpoint has not been configured,a value of 0 will be displayed.Where the TX and
  * Rx endpoints share the same FIFO,the Rx FIFO size will be encoded as 0xF.
  * Note: The register only has this interpretation when the Index register is set to select one of
  * Endpoints 1-15 and Dynamic Sizing is not selected.It has a special interpretation when the Index register
  * is set to select Endpoint 0,while the result returned is not valid where Dynamic FIFO sizing is used.
  */
typedef union _USB_OTG_FIFOSIZE_TypeDef {
    uint8_t d8;
    struct {
        uint8_t tx_fifo_size            : 4;
        uint8_t rx_fifo_size            : 4;
    } b;
} USB_BOTG_FIFOSIZE_TypeDef;
/**
  * HWVers register is a 16-bit read-only register that returns information about
  * the version of the RTL from which the core hardware was generated, in
  * particular the RTL version number (vxx.yyy).
  */
typedef union _USB_OTG_HWNERS_TypeDef {
    uint16_t d16;
    struct {
        uint16_t minor_version_number           : 10;
        uint16_t major_version_number           : 5;
        uint16_t rc                             : 1;
    } b;
} USB_OTG_HWNERS_TypeDef;
/**
  * This 8-bit read-only register allows read-back of the number of TX and Rx
  * endpoints included in the design.
  */
typedef union _USB_OTG_EPINFO_TypeDef {
    uint8_t d8;
    struct {
        uint8_t tx_endpoint         : 4;
        uint8_t rx_endpoint         : 4;
    } b;
} USB_OTG_EPINFO_TypeDef;
/**
  * This 8-bit read-only register provides information about the width of the RAM.
  */
typedef union _USB_OTG_RAMINFO_TypeDef {
    uint8_t d8;
    struct {
        uint8_t ram_bits            : 4;
        uint8_t dma_chans           : 4;
    } b;
} USB_OTG_RAMINFO_TypeDef;
/**
  * This 8-bit register allows some delays to be specified.
  */
typedef union _USB_OTG_LINKINFO_TypeDef {
    uint8_t d8;
    struct {
        uint8_t wtid                : 4;
        uint8_t wtcon               : 4;
    } b;
} USB_OTG_LINKINFO_TypeDef;
/**
  * This 8-bit register sets the duration of the VBus pulsing charge.
  */
typedef union _USB_OTG_VPLEN_TypeDef {
    uint8_t d8;
    struct {
        uint8_t vplen               : 8;
    } b;
} USB_OTG_VPLEN_TypeDef;
/**
  * This 8-bit register sets the minimum time gap that is to be allowed between
  * the start of the last transaction and the EOF for Full-speed transactions.
  */
typedef union _USB_FS_EOF1_TypeDef {
    uint8_t d8;
    struct {
        uint8_t fs_eof1             : 8;
    } b;
} USB_FS_EOF1_TypeDef;
/**
  * This 8-bit register sets the minimum time gap that is to be allowed between
  * the start of the last transaction and the EOF for Low-speed transactions.
  */
typedef union _USB_LS_EOF1_TypeDef {
    uint8_t d8;
    struct {
        uint8_t ls_eof1             : 8;
    } b;
} USB_LS_EOF1_TypeDef;
/**
  * This 8-bit register will assert LOW the output reset signals NRSTO and NRSTOX.
  * This register is self clearing and will be reset by the input NRST.
  */
typedef union _USB_OTG_SOFT_RST_TypeDef {
    uint8_t d8;
    struct {
        uint8_t nrst                : 1;
        uint8_t nrstx               : 1;
        uint8_t reserved2_7         : 6;
    } b;
} USB_OTG_SOFT_RST_TypeDef;
/**
  * Rx DPktBufDis is a 16-bit register that indicates which of the Rx endpoints
  * have disabled the double packet buffer functionality
  */
typedef union _USB_OTG_RXDPKTBUFDIS_TypeDef {
    uint16_t d16;
    struct {
        uint16_t reserved0                  : 1;
        uint16_t ep1_rx_dis                 : 1;
        uint16_t ep2_rx_dis                 : 1;
        uint16_t ep3_rx_dis                 : 1;
        uint16_t ep4_rx_dis                 : 1;
        uint16_t ep5_rx_dis                 : 1;
        uint16_t ep6_rx_dis                 : 1;
        uint16_t ep7_rx_dis                 : 1;
        uint16_t ep8_rx_dis                 : 1;
        uint16_t ep9_rx_dis                 : 1;
        uint16_t ep10_rx_dis                : 1;
        uint16_t ep11_rx_dis                : 1;
        uint16_t ep12_rx_dis                : 1;
        uint16_t ep13_rx_dis                : 1;
        uint16_t ep14_rx_dis                : 1;
        uint16_t ep15_rx_dis                : 1;
    } b;
} USB_OTG_RXDPKTBUFDIS_TypeDef;
/**
  * Tx DPktBufDis is a 16-bit register that indicates which of the TX endpoints
  * have disabled the double packet buffer functionality
  */
typedef union _USB_OTG_TXDPKTBUFDIS_TypeDef {
    uint16_t d16;
    struct {
        uint16_t reserved0                  : 1;
        uint16_t ep1_tx_dis                 : 1;
        uint16_t ep2_tx_dis                 : 1;
        uint16_t ep3_tx_dis                 : 1;
        uint16_t ep4_tx_dis                 : 1;
        uint16_t ep5_tx_dis                 : 1;
        uint16_t ep6_tx_dis                 : 1;
        uint16_t ep7_tx_dis                 : 1;
        uint16_t ep8_tx_dis                 : 1;
        uint16_t ep9_tx_dis                 : 1;
        uint16_t ep10_tx_dis                : 1;
        uint16_t ep11_tx_dis                : 1;
        uint16_t ep12_tx_dis                : 1;
        uint16_t ep13_tx_dis                : 1;
        uint16_t ep14_tx_dis                : 1;
        uint16_t ep15_tx_dis                : 1;
    } b;
} USB_OTG_TXDPKTBUFDIS_TypeDef;
/**
  * This register sets the chirp timeout.
  */
typedef union _USB_OTG_C_T_UCH_TypeDef {
    uint16_t d16;
    struct {
        uint16_t c_t_uch                    : 16;
    } b;
} USB_OTG_C_T_UCH_TypeDef;
/**
  * This register sets the delay from the end of High Speed resume signaling
  * (acting as a Host) to enable the UTM normal operating mode.
  */
typedef union _USB_OTG_C_T_HHSRTN_TypeDef {
    uint16_t d16;
    struct {
        uint16_t c_t_hhsrtn                 : 16;
    } b;
} USB_OTG_C_T_HHSRTN_TypeDef;
/**
  * Use of this register will allow the high speed timeout to be set to values
  * that are greater the maximum specifed in USB 2.0 making the MUSBMHDRC non-complient.
  */
typedef union _USB_OTG_C_T_HSBT_TypeDef {
    uint8_t d8;
    struct {
        uint8_t hs_timeout_adder            : 4;
        uint8_t reserved4_7                 : 4;
    } b;
} USB_OTG_C_T_HSBT_TypeDef;
/**
  * This register provides an interrupt for each DMA channel.
  */
typedef union _USB_OTG_DMA_INTR_TypeDef {
    uint8_t d8;
    struct {
        uint8_t ch1_dma_intr                : 1;
        uint8_t ch2_dma_intr                : 1;
        uint8_t ch3_dma_intr                : 1;
        uint8_t ch4_dma_intr                : 1;
        uint8_t ch5_dma_intr                : 1;
        uint8_t ch6_dma_intr                : 1;
        uint8_t ch7_dma_intr                : 1;
        uint8_t ch8_dma_intr                : 1;
    } b;
} USB_OTG_DMA_INTR_TypeDef;
/**
  * This register is only available if the MUSBMHDRC is configured to use at
  * least one internal DMA channel.  This register provides provdes the DMA
  * transfer control for each channel.  The enabling, transfer direction,
  * transfer mode, the DMA burst modes are all controlled by this register.
  */
typedef union _USB_OTG_DMA_CNTL_TypeDef {
    uint16_t d16;
    struct {
        /* This bit enbale the DMA transfer and will cause the transfer to begin */
        uint16_t dma_en             : 1;
        /* This bie selects the DMA Transfer Direction.
         *          0 = DMA Write(RX Endpoint);
         *          1 = DMA Read(TX Endpoint)
         */
        uint16_t dma_dir            : 1;
        /* USB OTG DMA Mode Select
         * When operating in DMA Mode 0, the DMA controller can be only load/unload one packet,
         * so processor intervention is required for each packet transferred over the USB. This
         * mode can be used with any endpoint,whether it uses Control,Bulk,Isochronous,or Interrupt
         * transaction(i.e.including Endpoint 0).
         *
         * When operating in DMA Mode 1, the DMA controller can be programmed to load/unload a
         * complete bulk transfer(which can be many packets). Once set up,the DMA controller will
         * load/unload all packets of the transfer,interrupting the processor only when the transfer
         * has completed. DMA Mode 1 can only be used with endpoints that use Bulk transaction.
         */
        uint16_t dma_mode           : 1;
        /* DMA Interrupt Enable */
        uint16_t dma_ie             : 1;
        /* The endpoint number this channel is assigned to */
        uint16_t dma_ep             : 4;
        uint16_t dma_err            : 1;
        /* USB OTG DMA Burst Mode Select
         *          00 = Burst Mode 0: Burst of unspecified length
         *          01 = Burst Mode 1: INCR4 or unspecified length
         *          10 = Burst Mode 2: INCR8,INCR4 or unspecified length
         *          11 = Burst Mode 3: INCR16,INCR8,INCR4 or unspecified length
         */
        uint16_t dma_brstm          : 2;
        uint16_t reserved11_15      : 5;
    } b;
} USB_OTG_DMA_CNTL_TypeDef;
/**
  * This register identifies the current memory address of the corresponding DMA channel.
  */
typedef union _USB_OTG_DMA_ADDR_TypeDef {
    uint32_t d32;
    struct {
        uint32_t dma_addr0_7            : 8;
        uint32_t dma_addr8_15           : 8;
        uint32_t dma_addr16_23          : 8;
        uint32_t dma_addr24_31          : 8;
    } b;
} USB_OTG_DMA_ADDR_TypeDef;
/**
  * This register identifies the current DMA count of the transfer.
  */
typedef union _USB_OTG_DMA_COUNT_TypeDef {
    uint32_t d32;
    struct {
        uint32_t dma_count0_7            : 8;
        uint32_t dma_count8_15           : 8;
        uint32_t dma_count16_23          : 8;
        uint32_t dma_count24_31          : 8;
    } b;
} USB_OTG_DMA_COUNT_TypeDef;
/**
  * TxFIFOsz is a 5-bit register which controls the size of the selected TX endpoint FIFO.
  */
typedef union _USB_OTG_TXFIFOSZ_TypeDef {
    uint8_t d8;
    struct {
        uint8_t size                        : 4;
        uint8_t dpb                         : 1;
        uint8_t reserved5_7                 : 3;
    } b;
} USB_OTG_TXFIFOSZ_TypeDef;
/**
  * RxFIFOsz is a 5-bit register which controls the size of the selected Rx endpoint FIFO.
  */
typedef union _USB_OTG_RXFIFOSZ_TypeDef {
    uint8_t d8;
    struct {
        uint8_t size                        : 4;
        uint8_t dpb                         : 1;
        uint8_t reserved5_7                 : 3;
    } b;
} USB_OTG_RXFIFOSZ_TypeDef;
/**
  * TxFIFOadd is a 14-bit register which controls the start address of the
  * selected Tx endpoint FIFO.
  */
typedef union _USB_OTG_TXFIFOADD_TypeDef {
    uint16_t d16;
    struct {
        uint16_t ad                 : 13;
        uint16_t reserved13         : 1;
        uint16_t reserved14_15      : 2;
    } b;
} USB_OTG_TXFIFOADD_TypeDef;
/**
  * RxFIFOadd is a 14-bit register which controls the start address of the
  * selected Rx endpoint FIFO.
  */
typedef union _USB_OTG_RXFIFOADD_TypeDef {
    uint16_t d16;
    struct {
        uint16_t ad                 : 13;
        uint16_t reserved13         : 1;
        uint16_t reserved14_15      : 2;
    } b;
} USB_OTG_RXFIFOADD_TypeDef;
/**
  * This register is used to define the attributes of an LPM transaction and sleep cycle.In both the Host
  * mode and the Peripheral mode,the meaning of this register is the same however the source of data is
  * different for Host and Peripheral as follows:
  * In Peripheral mode:
  * In Peripheral mode,the values in this register will contain the equivalent attributes that were received in
  * the last LPM transaction that was accepted.This register is updated with the LPM packet contents of the response
  * to the LPM transaction was an ACK.This register can be update via software.In all other cases,this register will
  * hold its current value.
  * In Host mode:
  * In Host mode software will set-up the values in this register to define the next LPM transaction that will be transmitted
  * These values will be inserted in the payload of the next LPM Transaction.
  */
typedef union _USB_OTG_LPM_ATTR_TypeDef {
    uint16_t d16;
    struct {
        uint16_t link_state          : 4;   /* This value is provoided by the host to the peripheral to indicate what state the peripheral
                                             * must transition to after the receipt and acceptance of a LPM transaction.
                                             * LinkState = 4'h0 - Reserved
                                             * LinkState = 4'h1 - Slep State(L1)
                                             * LinkState = 4'h2 - Reserved
                                             * LinkState = 4'h3 - Reserved
                                             */
        uint16_t HIRD                : 4;   /* This is the Host Initiated Resume Duration.This value is the minimum time the host will
                                             * drive resume on the Bus. The value in this register corresponds to an actual resume time
                                             * of:
                                             * Resume Time = 50us + HIRD * 75us.This results a range 50us to 1200us.
                                             */
        uint16_t RmtWak              : 1;   /* This bit is the remote wakeup enable bit:
                                             * RmtWak = 1'b0:Remote wakeup is not enabled.
                                             * RmtWak = 1'b1:Remote wakeup is enabled.
                                             * This bit is applied on a temporary basis only and is only applied to the current suspend
                                             * state.After the current suspend cycle,the remote wakeup capability that was negotiated
                                             * upon enumeration will apply.
                                             */
        uint16_t reserved9_11        : 3;   /* Reserved;Always returns 0 on read */
        uint16_t EndPoint            : 4;   /* This is the EndPnt that in the Token Packet of the LPM transaction. -*/
    } b;
} USB_OTG_LPM_ATTR_TypeDef;
/**
  * LPM_CNTRL
  */
typedef union _USB_OTG_LPM_CNTRL_IN_PERIPHERAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t lpmxmt              : 1;
        uint8_t lpmres              : 1;
        uint8_t lpmen               : 2;
        uint8_t lpmnak              : 1;
        uint8_t reserved5_7         : 3;
    } b;
} USB_OTG_LPM_CNTRL_IN_PERIPHERAL_TypeDef;

typedef union _USB_OTG_LPM_CNTRL_IN_HOST_TypeDef {
    uint8_t d8;
    struct {
        uint8_t lpmxmt              : 1;
        uint8_t lpmres              : 1;
        uint8_t reserved2_7         : 6;
    } b;
} USB_OTG_LPM_CNTRL_IN_HOST_TypeDef;
/**
  * The LPM_INTREN is a 6 bit register that provides enable bits for the
  * interrupts in the LPM_INTR register.If a bit in this register is set to 1,MC_NINT will be asserted(low)
  * when the corresponding interrupt in the LPM_INTR register is set.If a bit in this register is set to 0.
  * the corresponding register in LPM_INTR is still set but MC_NINT will not be asserted(low).On reset, all
  * bits in this register are reset to 0.
  */
typedef union _USB_OTG_LPM_INTEREN_TypeDef {
    uint8_t d8;
    struct {
        uint8_t lpmtoen             : 1;    /* 1'b0:Disable the LPMTO interrupt
                                             * 1'b1:Enable the LPMTO interrupt
                                             */
        uint8_t lpmsten             : 1;    /* 1'b0:Disable the LPMST interrupt
                                             * 1'b1:Enable the LPMST interrupt
                                             */
        uint8_t lpmnyen             : 1;    /* 1'b0:Disable the LPMNY interrupt
                                             * 1'b1:Enable the LPMNY interrupt
                                             */
        uint8_t lpmacken            : 1;    /* 1'b0:Disable the LPMACK interrupt
                                             * 1'b1:Enable the LPMACK interrupt
                                             */
        uint8_t lpmresen            : 1;    /* 1'b0:Disable the LPMRES interrupt
                                             * 1'b1:Enable the LPMRES interrupt
                                             */
        uint8_t lpmerren            : 1;    /* 1'b0:Disable the LPMERR interrupt
                                             * 1'b1:Enable the LPMERR interrupt
                                             */
        uint8_t reserved6_7         : 2;    /* Reserved-Always returns 0x0 on read */
    } b;
} USB_OTG_LPM_INTEREN_TypeDef;
/**
  * The LPM_INTR is a 7 bit register that provides status of the LPM Power state. When a bit is set to 1,if the
  * corresponding enable bit is also set to 1 the output MC_NINT is asserted(low),If the corresponding enable bit
  * is set to 0,then the output MC_NINT is not asserted.On reset,all bits in this register are reaet to 0.This
  * register is clear on read.
  */
typedef union _USB_OTG_LPM_INTR_IN_PERIPHERAL_TypeDef {
    uint8_t d8;
    struct {
        uint8_t lpmst               : 1;
        uint8_t lpmny               : 1;
        uint8_t lpmack              : 1;
        uint8_t lpmnc               : 1;
        uint8_t lpmres              : 1;
        uint8_t lpmerr              : 1;
        uint8_t reserved6_7         : 2;
    } b;
} USB_OTG_LPM_INTR_IN_PERIPHERAL_TypeDef;

typedef union _USB_OTG_LPM_INTR_IN_HOST_TypeDef {
    uint8_t d8;
    struct {
        uint8_t lpmst               : 1;
        uint8_t lpmny               : 1;
        uint8_t lpmack              : 1;
        uint8_t lpmnc               : 1;
        uint8_t lpmres              : 1;
        uint8_t reserved5_7         : 3;
    } b;
} USB_OTG_LPM_INTR_IN_HOST_TypeDef;
/**
  * Relavant in Host mode only!
  * The LPM_FADDR is the function address that will be placed in the LPM paylosd.
  */
typedef union _USB_OTG_LPM_FADDR_TypeDef {
    uint8_t d8;
    struct {
        uint8_t function_address    : 7;
        uint8_t reserved7           : 1;
    } b;
} USB_OTG_LPM_FADDR_TypeDef;

/**
  * USBPHY_CR1_TypeDef
  * Offset: 0x0108
  * Default: 0x004921AE
  * Access: RW
  * Width: 32bit
  */
typedef union _USBPHY_CR1_TypeDef {
    uint32_t d32;
    struct {
        uint32_t txpreemphasistune          : 1;    /* */
        uint32_t txrisetune                 : 2;    /* */
        uint32_t txvreftune                 : 4;    /* */
        uint32_t txfslstune                 : 4;    /* */
        uint32_t sqrxtune                   : 3;    /* */
        uint32_t compdistune                : 3;    /* */
        uint32_t otgtune                    : 3;    /* */
        uint32_t loopback_enb               : 1;    /* */
        uint32_t otg_disable                : 1;    /* */
        uint32_t commononn                  : 1;    /* */
        uint32_t vatestenb                  : 1;    /* */
        uint32_t lsbist                     : 1;    /* */
        uint32_t fsbist                     : 1;    /* */
        uint32_t hsbist                     : 1;    /* */
        uint32_t bisten                     : 1;    /* */
        uint32_t usb_iddq                   : 1;    /* */
        uint32_t stop_ck_for_suspend        : 1;    /* */
        uint32_t bist_done                  : 1;    /* */
        uint32_t bist_error                 : 1;    /* */
    } b;
} USBPHY_CR1_TypeDef;

/**
  * USBPHY_CR3_TypeDef
  * Offset: 0x010C
  * Default: 0x00000000
  * Access: RW
  * Width: 32bit
  */
typedef union _USBPHY_CR3_TypeDef {
    uint32_t d32;
    struct {
        uint32_t idpullup                   : 1;    // USB PHY ID Pin pull up, set to enable ID detection
        uint32_t iddig                      : 1;    // USB PHY ID Status
        uint32_t reserved2_31               : 29;
    } b;
} USBPHY_CR3_TypeDef;

#ifdef __cplusplus
}
#endif

#endif  /* __USB_OTG_REGS_H__ */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
