/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_hcd_int.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Host Interrupt subroutines.
 *****************************************************************************/


#ifndef __HCD_INT_H__
#define __HCD_INT_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usb_hcd.h"
#include "usbh_core.h"
/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/* Exported variables -------------------------------------------------------*/
/** @addtogroup USB_OTG_DRIVER
  * @{
  */

/** @defgroup USB_HCD_INT
  * @brief This file is the
  * @{
  */


/** @defgroup USB_HCD_INT_Exported_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USB_HCD_INT_Exported_Types
  * @{
  */

typedef struct _USBH_HCD_INT {
    uint8_t (* USBH_Resume)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* USBH_Babble)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* USBH_SessReq)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* USBH_VbusError)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* SOF)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* DevConnected)(USB_OTG_CORE_HANDLE *pdev);
    uint8_t (* DevDisconnected)(USB_OTG_CORE_HANDLE *pdev);

} USBH_HCD_INT_cb_TypeDef;

extern USBH_HCD_INT_cb_TypeDef *USBH_HCD_INT_fops;
/**
  * @}
  */


/** @defgroup USB_HCD_INT_Exported_Macros
  * @{
  */

#define CLEAR_HC_INT(HC_REGS, intr) \
    {\
    USB_OTG_HCINTn_TypeDef  hcint_clear; \
    hcint_clear.d32 = 0; \
    hcint_clear.b.intr = 1; \
    USB_OTG_WRITE_REG32(&((HC_REGS)->HCINT), hcint_clear.d32);\
    }\

#define MASK_HOST_INT_CHH(hc_num) { USB_OTG_HCINTMSK_TypeDef  INTMSK; \
    INTMSK.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[hc_num]->HCINTMSK); \
    INTMSK.b.chhltd = 0; \
    USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[hc_num]->HCINTMSK, INTMSK.d32);}

#define UNMASK_HOST_INT_CHH(hc_num) { USB_OTG_HCINTMSK_TypeDef  INTMSK; \
    INTMSK.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[hc_num]->HCINTMSK); \
    INTMSK.b.chhltd = 1; \
    USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[hc_num]->HCINTMSK, INTMSK.d32);}

#define MASK_HOST_INT_ACK(hc_num) { USB_OTG_HCINTMSK_TypeDef  INTMSK; \
    INTMSK.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[hc_num]->HCINTMSK); \
    INTMSK.b.ack = 0; \
    USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[hc_num]->HCINTMSK, GINTMSK.d32);}

#define UNMASK_HOST_INT_ACK(hc_num) { USB_OTG_HCGINTMSK_TypeDef  INTMSK; \
    INTMSK.d32 = USB_OTG_READ_REG32(&pdev->regs.HC_REGS[hc_num]->HCINTMSK); \
    INTMSK.b.ack = 1; \
    USB_OTG_WRITE_REG32(&pdev->regs.HC_REGS[hc_num]->HCINTMSK, INTMSK.d32);}

/**
  * @}
  */

/** @defgroup USB_HCD_INT_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_HCD_INT_Exported_FunctionsPrototype
  * @{
  */
/* Callbacks handler */
void ConnectCallback_Handler(USB_OTG_CORE_HANDLE *pdev);
void Disconnect_Callback_Handler(USB_OTG_CORE_HANDLE *pdev);
void Overcurrent_Callback_Handler(USB_OTG_CORE_HANDLE *pdev);
uint32_t USBH_OTG_ISR_Handler(USB_OTG_CORE_HANDLE *pdev, USBH_HOST *phost);
/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif  /* __HCD_INT_H__ */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
