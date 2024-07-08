
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" \
                            issue*/
#include "lv_drivers/sdl/sdl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/lvgl.h"
#include <SDL2/SDL.h>

#include "device_setting.h"
#include "gui.h"
#include "gui_api.h"
#include "gui_framework.h"
#include "gui_views.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// static const uint8_t g_webUsbUpdatePubKey[] = {
//     0x04, 0x3f, 0xe7, 0x7f, 0xd7, 0xa1, 0x92, 0x27, 0xb2, 0x63, 0x48, 0x05, 0xeb, 0xed, 0x40, 0xca,
//     0x22, 0x6a, 0xa4, 0x05, 0xb9, 0x7f, 0x8d, 0x61, 0xd4, 0xea, 0x26, 0x65, 0x9d, 0x9e, 0xb1, 0x1f,
//     0xaf, 0xa7, 0xfb, 0x35, 0xe9, 0x33, 0x41, 0x46, 0x10, 0x27, 0xff, 0x44, 0x17, 0x86, 0xa1, 0x64,
//     0x17, 0x5d, 0x3c, 0xe3, 0x1c, 0x82, 0x2f, 0xa3, 0x91, 0xff, 0x27, 0x5d, 0x57, 0x37, 0x13, 0xef,
//     0x99};

static uint8_t g_webUsbUpdatePubKey[]  = {
    4, 63, 231, 127, 215, 161, 146, 39, 178, 99, 72, 5, 235, 237, 64, 202, 34, 106, 164, 5, 185, 127, 141, 97, 212, 234, 38, 101, 157, 218, 241, 31, 235, 233, 251,
    53, 233, 51, 65, 68, 24, 39, 255, 68, 23, 134, 161, 100, 23, 215, 79, 56, 199, 34, 47, 163, 145, 255, 39, 93, 87, 55, 19, 239, 153
};

int hexstr_to_uint8_array(const char *hexstr, uint8_t *buf, size_t buf_len) {
    int count = 0;
    while (*hexstr && count < buf_len) {
        unsigned int byte;
        if (sscanf(hexstr, "%2x", &byte) != 1) {
            break;
        }
        buf[count++] = byte;
        hexstr += 2;
    }
    return count;
}

int main(int argc, char **argv)
{
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    printf("start");

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init();

    DeviceSettingsInit();

    GuiStyleInit();
#include "librust_c.h"
    uint8_t privKey[] = {36, 152, 38, 220, 181, 219, 183, 145, 246, 234, 111, 76, 161, 118, 67, 239, 70, 95, 241, 130, 17, 82, 24, 232, 53, 216, 250, 63, 93, 81, 164, 129};
    char *pubkeystr = "02f4599fff0b104401aefb131f29254f54c33e56186d4cabdc194706f031c9af2a";
    uint8_t pubkey[] = {3, 161, 210, 17, 253, 221, 40, 129, 137, 80, 105, 46, 126, 230, 80, 221, 82, 51, 206, 214, 24, 144, 201, 11, 99, 132, 251, 21, 68, 169, 44, 42, 251};
    SimpleResponse_u8 *simpleResponse = k1_generate_ecdh_sharekey(privKey, sizeof(privKey), pubkey, sizeof(pubkey));
    // SimpleResponse_u8 *simpleResponse = k1_generate_pubkey_by_privkey(privKey, 32);
    for (int i = 0; i < 32; i++)
    {
        printf("%d ", simpleResponse->data[i]);
    }
    LanguageInit();

    uint8_t pubKey1[33] = {
        0x03, 0x92, 0xa4, 0x88, 0xbe, 0xbd, 0x4a, 0x6a, 0x31, 0x13, 0x99, 0x60, 0xe3, 0x70, 0xe2, 0x34,
        0x09, 0xec, 0xd4, 0x6c, 0xb4, 0x9b, 0x12, 0xbf, 0x03, 0xb7, 0x19, 0xd3, 0xe4, 0xf2, 0xf4, 0xf6, 0x21
    };

    // uint8_t public_key[] = {
    //     0x03, 0x92, 0xa4, 0x88, 0xbe, 0xbd, 0x4a, 0x6a,
    //     0x31, 0x13, 0x99, 0x60, 0xe3, 0x70, 0xe2, 0x34,
    //     0x09, 0xec, 0xd4, 0x6c, 0xb4, 0x9b, 0x12, 0xbf,
    //     0x03, 0xb7, 0x19, 0xd3, 0xe4, 0xf2, 0xf4, 0xf6,
    //     0x21
    // };

    // uint8_t pubKey[] = {165, 254, 177, 253, 17, 144, 6, 123, 17, 160, 94, 213, 218, 173, 200, 16, 113, 45, 52, 196, 163, 197, 192, 107, 87, 51, 51, 161, 115, 226, 13, 2};

    // uint8_t pubKeySign[] = {
    //     0x30, 0x46, 0x02, 0x21, 0x00, 0xe9, 0x6c, 0xf5, 0x43, 0xdd, 0x47, 0x09, 0x4d, 0x27, 0x20, 0xfc,
    //     0xe0, 0x70, 0x3f, 0x6e, 0xdc, 0x26, 0x03, 0x5d, 0x00, 0xa2, 0x2d, 0x3b, 0x23, 0x7f, 0x83, 0x7b,
    //     0xd0, 0x13, 0xb9, 0xaf, 0xc5, 0x02, 0x21, 0x00, 0xa8, 0x01, 0x05, 0x21, 0x24, 0x74, 0x5b, 0xf7,
    //     0xe1, 0xba, 0x1a, 0xf6, 0x1c, 0xd0, 0xa2, 0xae, 0x26, 0xaa, 0xbf, 0x4c, 0x46, 0x7f, 0x76, 0xca,
    //     0xc7, 0x55, 0xa7, 0x3c, 0x8d, 0x6a, 0x90, 0x55
    // };

// uint8_t pubKeySign[] = {
//     0x30, 0x44, 0x02, 0x20, 0x44, 0x2a, 0x21, 0x05, 
//     0xa0, 0x32, 0x2d, 0x3f, 0x6f, 0xbf, 0x8a, 0xc8,
//     0xb0, 0xb9, 0x61, 0x7e, 0x5e, 0xa6, 0x8c, 0x18,
//     0x3c, 0xa7, 0xe0, 0x7d, 0x70, 0x03, 0x80, 0x74,
//     0x96, 0xd2, 0xfc, 0xcc, 0x02, 0x20, 0x68, 0xfa,
//     0x89, 0xa5, 0x36, 0x9c, 0x14, 0xe2, 0x47, 0x92,
//     0x5d, 0x13, 0x9a, 0xfa, 0x2d, 0x98, 0x29, 0x3d,
//     0x59, 0x18, 0x0d, 0x95, 0xaa, 0xc5, 0x07, 0x45,
//     0xdc, 0x1c, 0xe2, 0x14, 0xd6, 0x77
// };

// uint8_t pubKeySign[64] = {
//     0x44, 0x2a, 0x21, 0x05, 0xa0, 0x32, 0x2d, 0x3f,
//     0x6f, 0xbf, 0x8a, 0xc8, 0xb0, 0xb9, 0x61, 0x7e,
//     0x5e, 0xa6, 0x8c, 0x18, 0x3c, 0xa7, 0xe0, 0x7d,
//     0x70, 0x03, 0x80, 0x74, 0x96, 0xd2, 0xfc, 0xcc,
//     0x68, 0xfa, 0x89, 0xa5, 0x36, 0x9c, 0x14, 0xe2,
//     0x47, 0x92, 0x5d, 0x13, 0x9a, 0xfa, 0x2d, 0x98,
//     0x29, 0x3d, 0x59, 0x18, 0x0d, 0x95, 0xaa, 0xc5,
//     0x07, 0x45, 0xdc, 0x1c, 0xe2, 0x14, 0xd6, 0x77
// };

    // uint8_t hash[32] = {0};
    // sha256((struct sha256 *)hash, pubKey1, 33);
    // printf("\nhash:");
    // for (int i = 0; i < sizeof(hash); i++)
    // {
    //     printf("%02x", hash[i]);
    // }
    // printf("\n");

    // char *pubKeySignstr = "442a2105a0322d3f6fbf8ac8b0b9617e5ea68c183ca7e07d7003807496d2fccc68fa89a5369c14e247925d139afa2d98293d59180d95aac50745dc1ce214d677";
    // uint8_t pubKeySign[64] = {0};
    // hexstr_to_uint8_array(pubKeySignstr, pubKeySign, 64);
    // printf("pubkeysign:\n");
    // for (int i = 0; i < sizeof(pubKeySign); i++) {
    //     printf("%02x", pubKeySign[i]);
    // }
    // printf("\n");

    // char *hashStr = "cf17c1f50e7ea35b56313e1af084d2a49e4f6c93da6f77ae0571d5736a2a951f";
    // uint8_t hash[32] = {0};
    // hexstr_to_uint8_array(hashStr, hash, 32);
    // printf("hash:\n");
    // for (int i = 0; i < sizeof(hash); i++) {
    //     printf("%02x", hash[i]);
    // }
    // printf("\n");

    // char *g_webUsbUpdatePubKeyStr = "76ecd96d8b847ade106103c05fe1a7b1f3abc8c9d7aa89789cff63688abea9d26082a0d3a2ec5ddd8288156da05273f9a60ed9f9afe92caadba3e17f575cda63";
    // uint8_t g_webUsbUpdatePubKey[65] = {0};
    // hexstr_to_uint8_array(g_webUsbUpdatePubKeyStr, g_webUsbUpdatePubKey, 65);
    // printf("g_webUsbUpdatePubKey:\n");
    // for (int i = 0; i < sizeof(g_webUsbUpdatePubKey); i++) {
    //     printf("%02x", g_webUsbUpdatePubKey[i]);
    // }
    // printf("\n");

    // if (k1_verify_signature(pubKeySign, hash, (uint8_t *)g_webUsbUpdatePubKey) == false)
    // {
    //     printf("verify signature fail\n");
    // }
    // else
    // {
    //     printf("verify signature success\n");
    // }

    // uint8_t msg[] = {
    //     0x0D, 0x94, 0xD0, 0x45, 0xA7, 0xE0, 0xD4, 0x54,
    //     0x7E, 0x16, 0x1A, 0xC3, 0x60, 0xC7, 0x35, 0x81,
    //     0xA9, 0x53, 0x83, 0x43, 0x5A, 0x48, 0xD8, 0x86,
    //     0x9A, 0xB0, 0x8F, 0xF3, 0x4A, 0x8D, 0xB5, 0xE7};

    // printf("msg:");
    // for (int i = 0; i < sizeof(msg); i++)
    // {
    //     printf("%02x", msg[i]);
    // }
    // printf("\n");

    // uint8_t testpubkey[] = {
    //     0x04, 0xb5, 0xc8, 0xd5, 0xf8, 0xde, 0xfd, 0x3b,
    //     0xe0, 0x89, 0xc0, 0xed, 0xda, 0xfc, 0xb2, 0x75,
    //     0xa5, 0xab, 0x15, 0xa4, 0xf7, 0x1b, 0xc4, 0x0f,
    //     0x86, 0x41, 0x56, 0x1e, 0x89, 0x7a, 0x85, 0xd7,
    //     0x50, 0x42, 0xc1, 0xa1, 0x68, 0xef, 0x84, 0xa1,
    //     0x47, 0x34, 0x36, 0x2f, 0xf7, 0x4b, 0x26, 0xf8,
    //     0xc4, 0xc1, 0x9f, 0x62, 0xb3, 0xa3, 0x6e, 0x3d,
    //     0xdd, 0x55, 0xfb, 0xae, 0xc2, 0x11, 0xd9, 0x68,
    //     0x02
    // };
    // printf("testpubkey:");
    // for (int i = 0; i < sizeof(testpubkey); i++) {
    //   printf("%02x", testpubkey[i]);
    // }
    // printf("\n");
    // uint8_t sig[] = {
    //     0x26, 0x93, 0x70, 0xa9, 0x76, 0xb8, 0x27, 0xaa,
    //     0xa6, 0x31, 0x6a, 0x01, 0xbc, 0xbb, 0x7e, 0x41,
    //     0xbe, 0xdf, 0xbc, 0x45, 0xba, 0xea, 0xe0, 0x7a,
    //     0x86, 0x8d, 0xe3, 0xc5, 0xd9, 0x4d, 0x6b, 0x04,
    //     0x09, 0xad, 0x26, 0x33, 0xc7, 0x86, 0x68, 0x13,
    //     0x16, 0xe5, 0x56, 0xa3, 0xba, 0x15, 0x59, 0x65,
    //     0xab, 0x50, 0x78, 0xe3, 0x53, 0x14, 0x34, 0xca,
    //     0x96, 0xa1, 0xf7, 0xa4, 0xb8, 0x60, 0x99, 0x70
    // };
    // printf("sig:");
    // for (int i = 0; i < sizeof(sig); i++) {
    //   printf("%02x", sig[i]);
    // }
    // printf("\n");
    // bool result = k1_verify_signature(sig, msg, testpubkey);
    // printf("signature verification passed is: %s\n", result ? "true" : "false");
    GuiFrameOpenView(&g_initView);

    //  lv_example_switch_1();
    //  lv_example_calendar_1();
    //  lv_example_btnmatrix_2();
    //  lv_example_checkbox_1();
    //  lv_example_colorwheel_1();
    //  lv_example_chart_6();
    //  lv_example_table_2();
    //  lv_example_scroll_2();
    //  lv_example_textarea_1();
    //  lv_example_msgbox_1();
    //  lv_example_dropdown_2();
    //  lv_example_btn_1();
    //  lv_example_scroll_1();
    //  lv_example_tabview_1();
    //  lv_example_tabview_1();
    //  lv_example_flex_3();
    //  lv_example_label_1();

    while (1)
    {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
        usleep(5 * 1000);
    }

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static void hal_init(void)
{
    /* Use the 'monitor' driver which creates window on PC's monitor to simulate a
     * display*/
    sdl_init();

    /*Create a display buffer*/
    static lv_disp_draw_buf_t disp_buf1;
    static lv_color_t buf1_1[SDL_HOR_RES * 100];
    lv_disp_draw_buf_init(&disp_buf1, buf1_1, NULL, SDL_HOR_RES * 100);

    /*Create a display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf1;
    disp_drv.flush_cb = sdl_display_flush;
    disp_drv.hor_res = SDL_HOR_RES;
    disp_drv.ver_res = SDL_VER_RES;

    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    lv_theme_t *th = lv_theme_default_init(
        disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
        LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, th);

    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);

    /* Add the mouse as input device
     * Use the 'mouse' driver which reads the PC's mouse*/
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse
     * position and state*/
    indev_drv_1.read_cb = sdl_mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb = sdl_keyboard_read;
    lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
    lv_indev_set_group(kb_indev, g);

    static lv_indev_drv_t indev_drv_3;
    lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
    indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_3.read_cb = sdl_mousewheel_read;
    lv_indev_t *enc_indev = lv_indev_drv_register(&indev_drv_3);
    lv_indev_set_group(enc_indev, g);
}
