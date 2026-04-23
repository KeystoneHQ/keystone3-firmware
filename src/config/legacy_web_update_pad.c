#include <stdint.h>
#include "legacy_web_update_pad.h"

#ifndef LEGACY_USB_PAD_LEN
#define LEGACY_USB_PAD_LEN 2U
#endif

#if (LEGACY_USB_PAD_LEN < 1) || (LEGACY_USB_PAD_LEN > 15)
#error "LEGACY_USB_PAD_LEN must be in range [1, 15]"
#endif

static const uint8_t g_legacyUsbPad[LEGACY_USB_PAD_LEN] __attribute__((used)) = {
#if LEGACY_USB_PAD_LEN >= 1
    0x13,
#endif
#if LEGACY_USB_PAD_LEN >= 2
    0x57,
#endif
#if LEGACY_USB_PAD_LEN >= 3
    0x9B,
#endif
#if LEGACY_USB_PAD_LEN >= 4
    0xDF,
#endif
#if LEGACY_USB_PAD_LEN >= 5
    0x24,
#endif
#if LEGACY_USB_PAD_LEN >= 6
    0x68,
#endif
#if LEGACY_USB_PAD_LEN >= 7
    0xAC,
#endif
#if LEGACY_USB_PAD_LEN >= 8
    0xF0,
#endif
#if LEGACY_USB_PAD_LEN >= 9
    0x35,
#endif
#if LEGACY_USB_PAD_LEN >= 10
    0x79,
#endif
#if LEGACY_USB_PAD_LEN >= 11
    0xBD,
#endif
#if LEGACY_USB_PAD_LEN >= 12
    0xE1,
#endif
#if LEGACY_USB_PAD_LEN >= 13
    0x46,
#endif
#if LEGACY_USB_PAD_LEN >= 14
    0x8A,
#endif
#if LEGACY_USB_PAD_LEN >= 15
    0xCE,
#endif
};

static volatile uint8_t g_legacyUsbPadSink = 0;

void LegacyWebUpdatePadTouch(void)
{
    const volatile uint8_t *pad = (const volatile uint8_t *)g_legacyUsbPad;
    uint8_t mix = 0;
    uint32_t i;

    for (i = 0; i < LEGACY_USB_PAD_LEN; i++) {
        mix ^= (uint8_t)(pad[i] + (uint8_t)i);
    }

    g_legacyUsbPadSink ^= mix;
}
