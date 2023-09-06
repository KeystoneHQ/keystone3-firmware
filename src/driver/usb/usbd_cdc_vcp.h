/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_cdc_vcp.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Header for usbd_cdc_vcp.c file.
 *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_VCP_H
#define __USBD_CDC_VCP_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_core.h"

/* Exported typef ------------------------------------------------------------*/
/* The following structures groups all needed parameters to be configured for the
    ComPort. These parameters can modified on the fly by the host through CDC class
    command class requests. */
typedef struct {
    uint32_t bitrate;
    uint8_t  format;
    uint8_t  paritytype;
    uint8_t  datatype;
} LINE_CODING;

/* Exported constants --------------------------------------------------------*/
/* The following define is used to route the USART IRQ handler to be used.
   The IRQ handler function is implemented in the usbd_cdc_vcp.c file. */

#define DEFAULT_CONFIG 0
#define OTHER_CONFIG   1

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern void     VCP_SetTxChar(uint8_t txChar);
extern uint16_t VCP_DataTx(uint8_t* Buf, uint32_t Len);
extern uint32_t VCP_GetTxBuflen(void);
extern uint8_t* VCP_GetTxBufrsaddr(void);

extern int32_t  VCP_GetRxChar(void);
extern uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len);
extern uint32_t VCP_GetRxBuflen(void);
extern uint8_t* VCP_GetRxBufrsaddr(void);

// Undef this macro to disable demo code
#define VCP_LOOPBACK_DEMO 3

#if VCP_LOOPBACK_DEMO == 1
extern void VCP_FastLoopback(void);
#define VCP_Loopback VCP_FastLoopback
#elif VCP_LOOPBACK_DEMO == 2
extern void VCP_SlowLoopback(void);
#define VCP_Loopback VCP_SlowLoopback
#elif VCP_LOOPBACK_DEMO == 3
extern void VCP_SlowestLoopback(void);
#define VCP_Loopback VCP_SlowestLoopback
#else
#define VCP_Loopback(void)
#endif

extern CDC_IF_Prop_TypeDef VCPHandle;

#endif /* __USBD_CDC_VCP_H */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
