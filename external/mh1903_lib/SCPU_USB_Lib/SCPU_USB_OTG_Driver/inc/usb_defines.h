/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_defines.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Header of the Core Layer.
 *****************************************************************************/

#ifndef __USB_DEF_H__
#define __USB_DEF_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usb_conf.h"


/** @defgroup _CORE_DEFINES_
  * @{
  */
#define USB_OTG_SPEED_PARAM_HIGH 0
#define USB_OTG_SPEED_PARAM_HIGH_IN_FULL 1
#define USB_OTG_SPEED_PARAM_FULL 3

#define USB_OTG_SPEED_HIGH      0
#define USB_OTG_SPEED_FULL      1
/**
  * @}
  */


/** @defgroup _OnTheGo_DEFINES_
  * @{
  */
#define MODE_HNP_SRP_CAPABLE                   0
#define MODE_SRP_ONLY_CAPABLE                  1
#define MODE_NO_HNP_SRP_CAPABLE                2
#define MODE_SRP_CAPABLE_DEVICE                3
#define MODE_NO_SRP_CAPABLE_DEVICE             4
#define MODE_SRP_CAPABLE_HOST                  5
#define MODE_NO_SRP_CAPABLE_HOST               6
#define A_HOST                                 1
#define A_SUSPEND                              2
#define A_PERIPHERAL                           3
#define B_PERIPHERAL                           4
#define B_HOST                                 5

#define DEVICE_MODE                            0
#define HOST_MODE                              1
#define OTG_MODE                               2
/**
  * @}
  */


/** @defgroup __DEVICE_DEFINES_
  * @{
  */
#define EP_TYPE_CTRL                           0
#define EP_TYPE_ISOC                           1
#define EP_TYPE_BULK                           2
#define EP_TYPE_INTR                           3
#define EP_TYPE_MSK                            3
/**
  * @}
  */


/** @defgroup __HOST_DEFINES_
  * @{
  */
#define HC_PID_DATA0                           0
#define HC_PID_DATA2                           1
#define HC_PID_DATA1                           2
#define HC_PID_SETUP                           3

#define HPRT0_PRTSPD_HIGH_SPEED                1
#define HPRT0_PRTSPD_FULL_SPEED                2
#define HPRT0_PRTSPD_LOW_SPEED                 3

#define MIN(a, b)      (((a) < (b)) ? (a) : (b))
#define MAX(a, b)      (((a) > (b)) ? (a) : (b))
/**
  * @}
  */


/** @defgroup USB_DEFINES_Exported_Types
  * @{
  */
typedef enum {
    USB_OTG_HS_CORE_ID = 0,
    USB_OTG_FS_CORE_ID = 1
} USB_OTG_CORE_ID_TypeDef;
/**
  * @}
  */


/** @defgroup Internal_Macro's
  * @{
  */
#define USB_OTG_READ_REG8(reg)          (*(__IO uint8_t *)  reg)
#define USB_OTG_READ_REG16(reg)         (*(__IO uint16_t *) reg)
#define USB_OTG_READ_REG32(reg)         (*(__IO uint32_t *) reg)
#define USB_OTG_WRITE_REG8(reg,value)   (*(__IO uint8_t *)  reg = value)
#define USB_OTG_WRITE_REG16(reg,value)  (*(__IO uint16_t *) reg = value)
#define USB_OTG_WRITE_REG32(reg,value)  (*(__IO uint32_t *) reg = value)

#define USB_OTG_MODIFY_REG8(reg, clear_mask, set_mask)  \
        USB_OTG_WRITE_REG8(reg, (((USB_OTG_READ_REG8(reg)) & ~clear_mask) | set_mask))

#define USB_OTG_MODIFY_REG16(reg, clear_mask, set_mask)  \
        USB_OTG_WRITE_REG16(reg, (((USB_OTG_READ_REG16(reg)) & ~clear_mask) | set_mask))

#define USB_OTG_MODIFY_REG32(reg, clear_mask, set_mask)  \
        USB_OTG_WRITE_REG32(reg, (((USB_OTG_READ_REG32(reg)) & ~clear_mask) | set_mask))

/********************************************************************************
                              ENUMERATION TYPE
********************************************************************************/
enum USB_OTG_SPEED {
    USB_SPEED_UNKNOWN = 0,
    USB_SPEED_LOW,
    USB_SPEED_FULL,
    USB_SPEED_HIGH
};
#ifdef __cplusplus
}
#endif

#endif  /* __USB_DEF_H__ */


/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
