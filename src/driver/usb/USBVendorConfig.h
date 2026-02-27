#ifndef __USB_VENDOR_CONFIG_H__
#define __USB_VENDOR_CONFIG_H__

// define USB Target Chip
#define CONFIG_USB_TARGET USB_TARGET_GENERIC_MHSCPU

// set 1 to enalbe debug
#define CONFIG_DEBUG 0

/* USB Device Config Start                                                    */

// Max interface count
#define CONFIG_USB_DEVICE_MAX_INTERFACE_COUNT (4)

// USB Device Descriptor buffer size = max size of DEVICE_DESCRIPTOR_LENGTH, USB_DEVICE_MAX_STRING_DESCRIPTOR_SIZE and
// USB_DEVICE_MAX_CONFIGURATION_DESCRIPTOR_SIZE
#define CONFIG_USB_DEVICE_MAX_STRING_DESCRIPTOR_SIZE        (128)
#define CONFIG_USB_DEVICE_MAX_CONFIGURATION_DESCRIPTOR_SIZE (256)

/* USB Device Config End                                                      */

/* USB Interfaces Default Config End                                          */

#endif
