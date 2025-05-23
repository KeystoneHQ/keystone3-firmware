#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "../lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_IMGBATTERYPOWER20
#define LV_ATTRIBUTE_IMG_IMGBATTERYPOWER20
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_IMGBATTERYPOWER20 uint8_t imgBatteryPower20_map[] = {
#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP != 0
  /*Pixel format:  Blue: 5 bit Green: 6 bit, Red: 5 bit, Alpha 8 bit  BUT the 2  color bytes are swapped*/
  0xE2, 0x46, 0xFE, 0xE2, 0x65, 0xFE, 0xE2, 0xA6, 0xFF, 0xE2, 0xC5, 0xFE, 
  0xE2, 0x46, 0xFF, 0xE2, 0x65, 0xFE, 0xE2, 0xA6, 0xFF, 0xE2, 0xC6, 0xFE, 
  0xE2, 0x46, 0xFE, 0xE2, 0x66, 0xFE, 0xE2, 0xA6, 0xFF, 0xE2, 0xC6, 0xFE, 
  0xE2, 0x46, 0xFF, 0xE2, 0x86, 0xFF, 0xE2, 0xA6, 0xFF, 0xE2, 0xC6, 0xFF, 
  0xE2, 0x46, 0xFE, 0xE2, 0x66, 0xFF, 0xE2, 0xA6, 0xFF, 0xE2, 0xC6, 0xFE, 
  0xE2, 0x46, 0xFF, 0xE2, 0x66, 0xFF, 0xE2, 0xA6, 0xFF, 0xE2, 0xC6, 0xFF, 
  0xE2, 0x46, 0xFF, 0xE2, 0x66, 0xFF, 0xE2, 0xA6, 0xFF, 0xE2, 0xC6, 0xFF, 
  0xE2, 0x46, 0xFF, 0xE2, 0x66, 0xFE, 0xE2, 0xA5, 0xFF, 0xE2, 0xC6, 0xFF, 
  0xE2, 0x46, 0xFF, 0xE2, 0x66, 0xFF, 0xE2, 0xA6, 0xFF, 0xE2, 0xC5, 0xFE, 
  0xE2, 0x46, 0xFE, 0xE2, 0x66, 0xFE, 0xE2, 0xA6, 0xFF, 0xE2, 0xC6, 0xFF
#endif
};

const lv_img_dsc_t imgBatteryPower20 = {
  .header.always_zero = 0,
  .header.w = 4,
  .header.h = 10,
  .data_size = 40 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .data = imgBatteryPower20_map,
};
