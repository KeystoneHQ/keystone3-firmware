/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_HID_BSP_H__
#define __USBD_HID_BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ioreq.h"

#define CONFIG_USB_DEVICE_HID_DEFAULT

#if defined(CONFIG_USB_DEVICE_HID_DEFAULT)
/* HID Default Config Start */
#define HID_IN_EP  0x83
#define HID_IN_PACKET_SIZE  0x40
#define HID_IN_BUFFER_COUNT 16

// #define HID_OUT_EP 0x03
#define HID_OUT_PACKET_SIZE 0x40
#define HID_OUT_BUFFER_COUNT 16

#define HID_INTERVAL 0x01

#define HID_FRAME_INTERVAL 5
/* HID Default Config End */
#endif

/* */
#define HID_VERSION (0x0111)

/* HID Class */
#define HID_CLASS             (3)
#define HID_SUBCLASS_NONE     (0)
#define HID_SUBCLASS_BOOT     (1)
#define HID_PROTOCOL_NONE     (0)
#define HID_PROTOCOL_KEYBOARD (1)
#define HID_PROTOCOL_MOUSE    (2)

/* Descriptors */
#define HID_DESCRIPTOR      (0x21)
#define HID_DESCRIPTOR_SIZE (0x09)
#define REPORT_DESCRIPTOR   (0x22)

/* Class requests */
#define GET_REPORT   (0x01)
#define GET_IDLE     (0x02)
#define GET_PROTOCOL (0x03)
#define SET_REPORT   (0x09)
#define SET_IDLE     (0x0A)
#define SET_PROTOCOL (0x0B)

/* HID Class Report Descriptor */
/* Short items: size is 0, 1, 2 or 3 specifying 0, 1, 2 or 4 (four) bytes */
/* of data as per HID Class standard */

/* Main items */
#define INPUT(size)          (0x80 | size)
#define OUTPUT(size)         (0x90 | size)
#define FEATURE(size)        (0xb0 | size)
#define COLLECTION(size)     (0xa0 | size)
#define END_COLLECTION(size) (0xc0 | size)

/* Global items */
#define USAGE_PAGE(size)       (0x04 | size)
#define LOGICAL_MINIMUM(size)  (0x14 | size)
#define LOGICAL_MAXIMUM(size)  (0x24 | size)
#define PHYSICAL_MINIMUM(size) (0x34 | size)
#define PHYSICAL_MAXIMUM(size) (0x44 | size)
#define UNIT_EXPONENT(size)    (0x54 | size)
#define UNIT(size)             (0x64 | size)
#define REPORT_SIZE(size)      (0x74 | size)
#define REPORT_ID(size)        (0x84 | size)
#define REPORT_COUNT(size)     (0x94 | size)
#define PUSH(size)             (0xa4 | size)
#define POP(size)              (0xb4 | size)

/* Local items */
#define USAGE(size)              (0x08 | size)
#define USAGE_MINIMUM(size)      (0x18 | size)
#define USAGE_MAXIMUM(size)      (0x28 | size)
#define DESIGNATOR_INDEX(size)   (0x38 | size)
#define DESIGNATOR_MINIMUM(size) (0x48 | size)
#define DESIGNATOR_MAXIMUM(size) (0x58 | size)
#define STRING_INDEX(size)       (0x78 | size)
#define STRING_MINIMUM(size)     (0x88 | size)
#define STRING_MAXIMUM(size)     (0x98 | size)
#define DELIMITER(size)          (0xa8 | size)


extern USB_OTG_CORE_HANDLE*  USBDevHandle;
extern USBD_Class_cb_TypeDef USBD_HID_cb;

// Put HID Input Report(s) to HOST
// If needed, 'report' buffer will be padded to align to HID_IN_PACKET_SIZE
// Returned the remaining length of Input Report(s)
extern uint32_t USBD_HID_PutInputReport(uint8_t* report, uint16_t len);

#if HID_OUT_BUFFER_COUNT
// Get HID Output Report(s) from HOST
// Returned the length of got Output Report(s) (align to HID_OUT_PACKET_SIZE)
extern uint32_t USBD_HID_GetOutputReport(uint8_t* report, uint32_t len);
#endif

// HID Output Report Event
// Will be fired when HID SetReport from HOST, called by USB interrupt.
// Both EP0 / HID_OUT_EP (if set) output report could fire this event.
// The 'reportBuffer' contains the output report from HOST.
extern void (*USBD_HID_OutputReport_Event)(const uint8_t* reportBuffer);

// Compatible with old API, Deprecated!!!
#define USBD_HID_SendReport USBD_HID_PutInputReport

#ifdef __cplusplus
}
#endif

#endif /* __USBD_HID_KEYBOARD_H__ */
