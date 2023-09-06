/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_cdc_core.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : header file for the usbd_cdc_core.c file.
 *****************************************************************************/

/* Includes ------------------------------------------------------------------*/

#ifndef __USB_CDC_CORE_H_
#define __USB_CDC_CORE_H_

#include "usbd_ioreq.h"
#include "CircularBuffer.h"

#ifndef CONFIG_USB_DEVICE_VCP
/* VCP default Config Start */
#define CDC_IN_EP                0x83
#define CDC_OUT_EP               0x03
#define CDC_DATA_MAX_PACKET_SIZE 64

#define CDC_CMD_PACKET_SIZE 8
// #define CDC_CMD_EP 0x81

#define CDC_IN_FRAME_INTERVAL 5
#define CDC_APP_TX_DATA_SIZE  1024
#define CDC_APP_RX_DATA_SIZE  1024

#define APP_FOPS VCPHandle
/* VCP default Config End */
#endif

#ifdef CDC_CMD_EP
#define USB_CDC_CONFIG_DESC_SIZ (68 + 7)
#define USB_CDC_ITF_EP_NUM      (0x01)
#else
#define USB_CDC_CONFIG_DESC_SIZ (32)
#define USB_CDC_ITF_EP_NUM      (0x00)
#endif

#define USB_CDC_DESC_SIZ (USB_CDC_CONFIG_DESC_SIZ - 9)

#define CDC_DESCRIPTOR_TYPE 0x21

#define DEVICE_CLASS_CDC    0x02
#define DEVICE_SUBCLASS_CDC 0x00

#define USB_DEVICE_DESCRIPTOR_TYPE        0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE 0x02
#define USB_STRING_DESCRIPTOR_TYPE        0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE     0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE      0x05

#define STANDARD_ENDPOINT_DESC_SIZE 0x09

#define CDC_DATA_IN_PACKET_SIZE CDC_DATA_MAX_PACKET_SIZE

#define CDC_DATA_OUT_PACKET_SIZE CDC_DATA_MAX_PACKET_SIZE

#ifndef CDC_APP_RX_DATA_SIZE
#define CDC_APP_RX_DATA_SIZE 1024
#endif

#ifndef CDC_APP_TX_DATA_SIZE
#define CDC_APP_TX_DATA_SIZE 1024
#endif

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/

/**************************************************/
/* CDC Requests                                   */
/**************************************************/
#define SEND_ENCAPSULATED_COMMAND 0x00
#define GET_ENCAPSULATED_RESPONSE 0x01
#define SET_COMM_FEATURE          0x02
#define GET_COMM_FEATURE          0x03
#define CLEAR_COMM_FEATURE        0x04
#define SET_LINE_CODING           0x20
#define GET_LINE_CODING           0x21
#define SET_CONTROL_LINE_STATE    0x22
#define SEND_BREAK                0x23
#define NO_CMD                    0xFF

typedef struct _CDC_IF_PROP {
    uint16_t (*pIf_Init)(void);
    uint16_t (*pIf_DeInit)(void);
    uint16_t (*pIf_Ctrl)(uint32_t Cmd, uint8_t* Buf, uint32_t Len);
    uint16_t (*pIf_DataTx)(uint8_t* Buf, uint32_t Len);
    uint16_t (*pIf_DataRx)(uint8_t* Buf, uint32_t Len);
} CDC_IF_Prop_TypeDef;

typedef struct {
    volatile uint8_t COM_config_cmp;
    CircularBufferStruct* SendBuffer;
    CircularBufferStruct* ReadBuffer;
} CDC_Data_TypeDef;

extern CDC_Data_TypeDef CDCData;

extern USBD_Class_cb_TypeDef USBD_CDC_cb;

#endif // __USB_CDC_CORE_H_

/************************ (C) COPYRIGHT Megahuntmicro *****END OF FILE****/
