#include "define.h"
#include "screenshot.h"
#include "stdio.h"
#include "lvgl.h"
#include "user_fatfs.h"
#include "cmsis_os.h"
#include "touchpad_task.h"
#include "user_msg.h"
#include "drv_motor.h"
#include "user_memory.h"

#pragma pack(1)

typedef struct _BitMapFileHead {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BitMapFileHead_t;

typedef struct _BitMapInfoHead {
    uint32_t biSize;
    int32_t width;
    int32_t height;
    uint16_t alwaysOne;
    uint16_t pixelBit;
    uint32_t compress;
    uint32_t imgSize;
    uint32_t xPixelPerMeter;
    uint32_t yPixelPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
    uint32_t redOccupySpace;
    uint32_t greenOccupySpace;
    uint32_t blueOccupySpace;
    uint32_t bfReserved;
} BitMapInfoHead_t;

#pragma pack()


void ScreenShot(uint8_t *imgData)
{
    char *imgFilePathName = NULL;
    uint8_t *screenBuffer = NULL;
    BitMapFileHead_t *pFileHead;
    BitMapInfoHead_t *pFfileInfo;
    uint32_t i;
    uint16_t *pixel;
    char fileName[BUFFER_SIZE_32];

    imgFilePathName = EXT_MALLOC(BUFFER_SIZE_128);
    uint32_t bmpHorByte = LV_HOR_RES * (LV_COLOR_DEPTH / 8);
    if (bmpHorByte % 4 != 0) {
        bmpHorByte = bmpHorByte + (4 - bmpHorByte % 4);
    }
    uint32_t headSize = sizeof(BitMapFileHead_t) + sizeof(BitMapInfoHead_t);
    uint32_t fileSize = headSize + bmpHorByte * LV_VER_RES;
    screenBuffer = EXT_MALLOC(fileSize);

    if (!imgFilePathName || !screenBuffer) {
        printf("screen shot malloc failed\r\n");
        goto EXIT;
    }
    printf("start snap screen\r\n");
    MotorCtrl(MOTOR_LEVEL_MIDDLE, 300);
    pFileHead = (BitMapFileHead_t *)screenBuffer;
    pFfileInfo = (BitMapInfoHead_t *)(screenBuffer + sizeof(BitMapFileHead_t));
    memset_s(pFileHead, sizeof(BitMapFileHead_t), 0, sizeof(BitMapFileHead_t));
    pFileHead->bfType = 0x4D42;
    pFileHead->bfSize = LV_HOR_RES * LV_VER_RES * (LV_COLOR_DEPTH / 8) + sizeof(BitMapFileHead_t) + sizeof(BitMapInfoHead_t);
    pFileHead->bfOffBits = sizeof(BitMapFileHead_t) + sizeof(BitMapInfoHead_t);
    memset_s(pFfileInfo, sizeof(BitMapInfoHead_t), 0, sizeof(BitMapInfoHead_t));
    pFfileInfo->biSize = sizeof(BitMapInfoHead_t);
    pFfileInfo->width = LV_HOR_RES;
    pFfileInfo->height = -1 * LV_VER_RES;
    pFfileInfo->alwaysOne = 1;
    pFfileInfo->pixelBit = LV_COLOR_DEPTH;
    pFfileInfo->compress = 3;
    pFfileInfo->imgSize = LV_HOR_RES * LV_VER_RES;
    pFfileInfo->xPixelPerMeter = 3780;
    pFfileInfo->yPixelPerMeter = 3780;
    pFfileInfo->redOccupySpace = 0xF800;
    pFfileInfo->greenOccupySpace = 0x07E0;
    pFfileInfo->blueOccupySpace = 0x001F;

    pixel = (uint16_t *)imgData;
    for (i = 0; i < LV_HOR_RES * LV_VER_RES; i++) {
        pixel[i] = (pixel[i] >> 8) | (pixel[i] << 8);
    }
    for (i = 0; i < LV_VER_RES; i++) {
        memcpy(screenBuffer + headSize + i * bmpHorByte, imgData + i * LV_HOR_RES * (LV_COLOR_DEPTH / 8), LV_HOR_RES * (LV_COLOR_DEPTH / 8));
    }
    snprintf_s(fileName, BUFFER_SIZE_32, "0:screenshot_%d.bmp", osKernelGetTickCount());
    printf("start save file\r\n");
    FatfsFileWrite(fileName, screenBuffer, fileSize);
    printf("save file over\r\n");

EXIT:
    EXT_FREE(imgFilePathName);
    EXT_FREE(screenBuffer);
}


#define SCREEN_SHOT_AREA_X_START 413
#define SCREEN_SHOT_AREA_Y_START 0
#define SCREEN_SHOT_AREA_X_END SCREEN_SHOT_AREA_X_START + 64
#define SCREEN_SHOT_AREA_Y_END SCREEN_SHOT_AREA_Y_START + 64

#define SCREEN_SHOT_LONG_TOUCH_TICK           3000

void ScreenShotTouch(void)
{
    static uint32_t lastTick = 0;
    TouchStatus_t *pStatus;

    pStatus = GetTouchStatus();
    if (pStatus->touch == false) {
        lastTick = 0;
        return;
    }
    if (pStatus->x >= SCREEN_SHOT_AREA_X_START && pStatus->x <= SCREEN_SHOT_AREA_X_END &&
            pStatus->y >= SCREEN_SHOT_AREA_Y_START && pStatus->y <= SCREEN_SHOT_AREA_Y_END) {
        if (lastTick == 0) {
            lastTick = osKernelGetTickCount();
            printf("start det long press\n");
            return;
        }
        if (osKernelGetTickCount() - lastTick > SCREEN_SHOT_LONG_TOUCH_TICK) {
            printf("long press detected\n");
            MotorCtrl(MOTOR_LEVEL_HIGH, 300);
            lastTick = 0;
            PubValueMsg(UI_MSG_SCREEN_SHOT, 0);
        }
    } else {
        lastTick = 0;
    }
}

