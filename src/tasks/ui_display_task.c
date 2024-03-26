#include "ui_display_task.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "hal_lcd.h"
#include "lvgl.h"
#include "log_print.h"
#include "user_msg.h"
#include "drv_virtual_touch.h"
#include "touchpad_task.h"
#include "gui_views.h"
#include "gui_framework.h"
#include "user_memory.h"
#include "drv_lcd_bright.h"
#include "device_setting.h"
#include "anti_tamper.h"
#include "screenshot.h"

#define LVGL_FAST_TICK_MS                   5
#define LVGL_IDLE_TICK_MS                   100
#define LVGL_GRAM_PIXEL         LCD_DISPLAY_WIDTH * 450


bool GuiLetterKbStatusError(void);
static void UiDisplayTask(void *argument);
static void RefreshLvglTickMode(void);
static void SetLvglTick(uint32_t dynamicTick);
static void LvglTickTimerFunc(void *argument);
static void LcdFlush(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
static void InputDevReadCb(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static uint8_t *GetActSnapShot(void);
static void __SetLvglHandlerAndSnapShot(uint32_t value);

osThreadId_t g_uiDisplayTaskHandle;
osTimerId_t g_lvglTickTimer;

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf_1[LVGL_GRAM_PIXEL];
static bool lvglHandlerEnable = true;
static volatile bool snapShotDone = false;
static volatile uint32_t g_dynamicTick, g_fastModeCount;

bool g_reboot = false;

void CreateUiDisplayTask(void)
{
    const osThreadAttr_t testtTask_attributes = {
        .name = "UiDisplayTask",
        .stack_size = 1024 * 16,
        .priority = (osPriority_t) osPriorityHigh,
    };
    g_uiDisplayTaskHandle = osThreadNew(UiDisplayTask, NULL, &testtTask_attributes);
    g_lvglTickTimer = osTimerNew(LvglTickTimerFunc, osTimerPeriodic, NULL, NULL);
}


static void UiDisplayTask(void *argument)
{
    static lv_disp_drv_t dispDrv;
    static lv_indev_drv_t indevDrv;
    Message_t rcvMsg;
    osStatus_t ret;
#ifdef ENABLE_SCREEN_SHOT
    uint8_t *screenData;
#endif

    lv_init();
    lv_disp_drv_init(&dispDrv);
    dispDrv.flush_cb = LcdFlush;
    dispDrv.draw_buf = &disp_buf;
    lv_disp_drv_register(&dispDrv);
    lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LVGL_GRAM_PIXEL);

    lv_indev_drv_init(&indevDrv);
    indevDrv.type = LV_INDEV_TYPE_POINTER;
    indevDrv.read_cb = InputDevReadCb;
    lv_indev_drv_register(&indevDrv);

    g_dynamicTick = LVGL_FAST_TICK_MS;
    g_fastModeCount = 0;
    osTimerStart(g_lvglTickTimer, g_dynamicTick);
    osDelay(100);

    printf("start ui display loop\r\n");
    printf("LV_HOR_RES=%d,LV_VER_RES=%d\r\n", LV_HOR_RES, LV_VER_RES);
    printf("Tampered()=%d\n", Tampered());
    g_reboot = true;
    GuiFrameOpenView(&g_initView);
    // SetLcdBright(GetBright());

    while (1) {
        RefreshLvglTickMode();
        ret = osMessageQueueGet(g_uiQueue, &rcvMsg, NULL, g_dynamicTick);
        if (ret == osOK) {
            switch (rcvMsg.id) {
            case UI_MSG_ACTIVATE_LOOP: {
                SetLvglTick(LVGL_FAST_TICK_MS);
            }
            break;
            case UI_MSG_SWITCH_HANDLER: {
                __SetLvglHandlerAndSnapShot(rcvMsg.value);
            }
            break;
            case UI_MSG_EMIT_SIGNAL_TO_WORKING_VIEW: {
                GuiFrameWorkViewHandleMsg(rcvMsg.buffer, rcvMsg.length);
            }
            break;
            case UI_MSG_CLOSE_CURRENT_VIEW: {
                GuiCLoseCurrentWorkingView();
            }
            break;
            case UI_MSG_OPEN_VIEW: {
                GUI_VIEW *view = (GUI_VIEW *)rcvMsg.value;
                GuiFrameOpenView(view);
            }
            break;
            case UI_MSG_CLOSE_VIEW: {
                GUI_VIEW *view = (GUI_VIEW *)rcvMsg.value;
                GuiFrameCLoseView(view);
            }
            break;
            default:
                break;
            }
            SRAM_FREE(rcvMsg.buffer);
        }
        if (lvglHandlerEnable) {
            lv_timer_handler();
            GuiLetterKbStatusError();
        }
    }
}

static void RefreshLvglTickMode(void)
{
    if (g_dynamicTick == LVGL_FAST_TICK_MS) {
        //fast mode.
        g_fastModeCount++;
        if (g_fastModeCount >= 1000) {
            g_dynamicTick = LVGL_IDLE_TICK_MS;
            osTimerStart(g_lvglTickTimer, g_dynamicTick);
        } else if (lv_anim_count_running() > 0 || GetTouchPress()) {
            g_fastModeCount = 0;
        }
    } else {
        //idle mode.
        if (lv_anim_count_running() > 0 || GetTouchPress()) {
            //switch to fast mode
            g_dynamicTick = LVGL_FAST_TICK_MS;
            osTimerStart(g_lvglTickTimer, g_dynamicTick);
            g_fastModeCount = 0;
            printf("lvgl switch to fast mode\r\n");
        }
    }
}


static void SetLvglTick(uint32_t dynamicTick)
{
    if (dynamicTick != g_dynamicTick) {
        g_dynamicTick = dynamicTick;
        osTimerStart(g_lvglTickTimer, g_dynamicTick);
        g_fastModeCount = 0;
    }
}


static void LvglTickTimerFunc(void *argument)
{
    //printf("lvgl tick\r\n");
    lv_tick_inc(g_dynamicTick);
}


static void LcdFlush(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    LcdDraw(area->x1, area->y1, area->x2, area->y2, (uint16_t *)color_p);
    while (LcdBusy()) {
        osDelay(1);
    }
    lv_disp_flush_ready(disp_drv);
}


static void InputDevReadCb(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    //data->state = GetVirtualTouch((uint16_t *)&data->point.x, (uint16_t *)&data->point.y) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    TouchStatus_t *pStatus;
    pStatus = GetTouchStatus();
    memset_s(data, sizeof(lv_indev_data_t), 0, sizeof(lv_indev_data_t));
    data->state = pStatus->touch ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = pStatus->x;
    data->point.y = pStatus->y;
    data->continue_reading = pStatus->continueReading;
}


static void __SetLvglHandlerAndSnapShot(uint32_t value)
{
    uint32_t tick1, tick2;
    bool enable = value != 0 ? true : false;
    static uint8_t *snapShotAddr = NULL;
    if (lvglHandlerEnable == true && enable == false) {
        //snap shot
        while (LcdBusy()) {
            osDelay(1);
        }
        tick1 = osKernelGetTickCount();
        snapShotAddr = GetActSnapShot();
        tick2 = osKernelGetTickCount();
        printf("t=%d\r\n", tick2 - tick1);
        //PrintU16Array("snapShotAddr", (uint16_t *)snapShotAddr, LCD_DISPLAY_WIDTH * 100);
    } else if (lvglHandlerEnable == false && enable == true) {
        //recovery
        while (LcdBusy()) {
            osDelay(1);
        }
        LcdDraw(0, 0, LCD_DISPLAY_WIDTH - 1, LCD_DISPLAY_HEIGHT - 1, (uint16_t *)snapShotAddr);
        while (LcdBusy()) {
            osDelay(1);
        }
        if (snapShotAddr != NULL) {
            EXT_FREE(snapShotAddr);
        }
    }
    lvglHandlerEnable = enable;
    snapShotDone = true;
}

bool GetLvglHandlerStatus(void)
{
    return lvglHandlerEnable;
}

/// @brief
/// @param enable
/// true-enable LVGL handler and recovery the image saved in PSRAM to LCD.
/// false-disable LVGL handler and save the LCD image snapshot to PSRAM.
void SetLvglHandlerAndSnapShot(bool enable)
{
    snapShotDone = false;
    PubValueMsg(UI_MSG_SWITCH_HANDLER, enable ? 1 : 0);
    while (snapShotDone == false) {
        osDelay(1);
    }
}


void LvglCloseCurrentView(void)
{
    PubValueMsg(UI_MSG_CLOSE_CURRENT_VIEW, 0);
}


uint8_t *GetLvglGramAddr(void)
{
    return (uint8_t *)buf_1;
}


uint32_t GetLvglGramSize(void)
{
    return sizeof(buf_1);
}


static uint8_t *GetActSnapShot(void)
{
    lv_img_dsc_t imgDsc;
    uint32_t snapShotSize = LCD_DISPLAY_WIDTH * LCD_DISPLAY_HEIGHT * (LV_COLOR_DEPTH / 8);
    //printf("snap size=%d\r\n", snapShotSize);
    uint8_t *buffer = EXT_MALLOC(snapShotSize);
    //printf("buffer addr=0x%X\r\n", buffer);
    lv_snapshot_take_to_buf(lv_scr_act(), LV_IMG_CF_TRUE_COLOR, &imgDsc, buffer, snapShotSize);
    return buffer;
}


void ActivateUiTaskLoop(void)
{
    if (g_dynamicTick == LVGL_IDLE_TICK_MS) {
        PubValueMsg(UI_MSG_ACTIVATE_LOOP, 0);
    }
}
