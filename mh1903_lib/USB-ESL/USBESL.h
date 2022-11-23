#ifndef __USBESL_H__
#define __USBESL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* USB Target Config Start                                                    */
#define USB_TARGET_GENERIC_MHSCPU 1


#include "USBVendorConfig.h"

#if CONFIG_USB_TARGET < 1
#error "Selected Unknown USB_TARGET, please Check CONFIG_USB_TARGET !!!"
#endif

#if (CONFIG_USB_TARGET == USB_TARGET_GENERIC_MHSCPU)
#include "mhscpu.h"
#endif

#include "USBTarget.h"

/* USB Target Config End                                                      */

/* Common Macro Start                                                         */
#if CONFIG_DEBUG
#define printf_dbg(...) printf(__VA_ARGS__)
#define util_assert(expression)                              \
    if (!(expression)) {                                     \
        printf_dbg("Asserted %s, %d\n", __FILE__, __LINE__); \
        volatile uint32_t assertTimeout = 0x80000000;        \
        while (assertTimeout--) {}                           \
    }

#else
#define printf_dbg(...)
#define util_assert(experssion)
#endif

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define MB(size) (size * 1024 * 1024)
#define KB(size) (size * 1024)

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define ROUND_UP(value, boundary)   ((value) + ((boundary) - (value)) % (boundary))
#define ROUND_DOWN(value, boundary) ((value) - ((value) % (boundary)))

#ifndef BIT
#define BIT(n) (1UL << (n))
#endif

#ifndef BITS
#define BITS(H_start, L_end) ((0xFFFFFFFF << (H_start + 1)) ^ (0xFFFFFFFF << (L_end)))
#endif

#define ToUintSize(size) ((size + 3) >> 2)

/* Common Macro End                                                           */

#if defined(__CC_ARM) && !defined(__GNUC__)
#pragma anon_unions
#endif

#ifdef __cplusplus
}
#endif

#endif
