#ifndef _USER_MSG_H
#define _USER_MSG_H

#include "stdint.h"
#include "stdbool.h"
#ifndef COMPILE_SIMULATOR
#include "cmsis_os.h"
#endif
#include "general_msg.h"

//GENERAL_MSG_BASE is an unclassified message, usually a broadcast message or a special message.
#define GENERAL_MSG_BASE                0x00000001
#define CMD_MSG_BASE                    0x00010000
#define TEST_MSG_BASE                   0x00020000
#define QRDECODE_MSG_BASE               0x00030000
#define UI_MSG_BASE                     0x00040000
#define BACKGROUND_MSG_BASE             0x00050000
#define LOG_MSG_BASE                    0x00060000
#define USB_MSG_BASE                    0x00070000
#define LOW_POWER_MSG_BASE              0x00080000

enum {
    MSG_TEST_CMD_FRAME = CMD_MSG_BASE,
};

enum {
    TEST_MSG_MEASUREMENT = TEST_MSG_BASE,
    TEST_MSG_ASYNPRINT,
    TEST_MSG_LEDGER_DATA_RCV,
};

enum {
    QRDECODE_MSG_START = QRDECODE_MSG_BASE,
    QRDECODE_MSG_STOP,
    QRDECODE_MSG_MINUTE,
};

enum {
    UI_MSG_SWITCH_HANDLER = UI_MSG_BASE,
    UI_MSG_EMIT_SIGNAL_TO_WORKING_VIEW,
    UI_MSG_CLOSE_CURRENT_VIEW,
    UI_MSG_ACTIVATE_LOOP,
    UI_MSG_SCREEN_SHOT,
    UI_MSG_PREPARE_RECEIVE_UR_USB,
    UI_MSG_OPEN_VIEW,
    UI_MSG_CLOSE_VIEW,
    UI_MSG_USB_TRANSPORT_VIEW,
};

enum {
    BACKGROUND_MSG_EXECUTE = BACKGROUND_MSG_BASE,
    BACKGROUND_MSG_EXECUTE_RUNNABLE,
    BACKGROUND_MSG_MINUTE,
    BACKGROUND_MSG_CHANGER_INSERT,
    BACKGROUND_MSG_RESET,
    BACKGROUND_MSG_BATTERY_INTERVAL,
    BACKGROUND_MSG_TAMPER,
    BACKGROUND_MSG_SD_CARD_CHANGE,
};

enum {
    LOG_MSG_WRITE = LOG_MSG_BASE,
    LOG_MSG_ERASE,
    LOG_MSG_EXPORT,
    LOG_MSG_WRITE_TO_SDCARD,
};

enum {
    USB_MSG_ISR_HANDLER = USB_MSG_BASE,
    USB_MSG_SET_STATE,
    USB_MSG_INIT,
    USB_MSG_DEINIT,
};

enum {
    LOW_POWER_ENTER = LOW_POWER_MSG_BASE,
    LOW_POWER_QUIT,
};

#ifndef COMPILE_SIMULATOR
extern osMessageQueueId_t g_cmdQueue;
extern osMessageQueueId_t g_testQueue;
extern osMessageQueueId_t g_qrDecodeQueue;
extern osMessageQueueId_t g_uiQueue;
extern osMessageQueueId_t g_backgroundQueue;
extern osMessageQueueId_t g_logQueue;
extern osMessageQueueId_t g_usbQueue;
extern osMessageQueueId_t g_lowPowerQueue;
#endif

void UserMsgInit(void);

#endif
