/*
 * Copyright (c) Fenda Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: UI resource version
 * Create: 2020-08-09
 */
#ifndef UIRES_VER_H
#define UIRES_VER_H
#include "startup.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_RES_IDENT 0x4644  // 'FD'
#define UI_FONT_VER  1  // 2字节
#define UI_IMAGE_VER 1  // 2字节

//font
#define UI_RES_FONT_MARK        FONT_FILE_HEAD_MARK
#define UI_RES_FONT_MAJOR       0
#define UI_RES_FONT_MINOR       47
#define UI_RES_FONT_BUILD       0

#define FONT_VALID_MASK         0x3FFF


//image
#define UI_RES_IMAGE_MARK       IMG_FILE_HEAD_MARK
#define UI_RES_IMAGE_MAJOR       0
#define UI_RES_IMAGE_MINOR       50
#define UI_RES_IMAGE_BUILD       1


//Dictionary
#define UI_RES_DICT_MAJOR       0
#define UI_RES_DICT_MINOR       0
#define UI_RES_DICT_BUILD       39

#define WATCHFACE_VERSION_MAJOR  0
#define WATCHFACE_VERSION_MINOR  10
#define WATCHFACE_VERSION_BUILD  0

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* UIRES_VER_H */
