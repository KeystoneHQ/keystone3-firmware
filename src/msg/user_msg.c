/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2025. All rights reserved.
 * Description: User message defining and subscription.
 * Author: leon sun
 * Create: 2022-11-8
 ************************************************************************************************/

#include "user_msg.h"
#include "cmsis_os.h"
#include "general_msg.h"

osMessageQueueId_t g_cmdQueue = NULL;
osMessageQueueId_t g_testQueue = NULL;
osMessageQueueId_t g_qrDecodeQueue = NULL;
osMessageQueueId_t g_uiQueue = NULL;
osMessageQueueId_t g_backgroundQueue = NULL;
osMessageQueueId_t g_logQueue = NULL;
osMessageQueueId_t g_usbQueue = NULL;
osMessageQueueId_t g_lowPowerQueue = NULL;

void UserMsgInit(void)
{
    //消息机制的队列在此创建
    g_cmdQueue = osMessageQueueNew(10, sizeof(Message_t), NULL);
    g_testQueue = osMessageQueueNew(10, sizeof(Message_t), NULL);
    g_qrDecodeQueue = osMessageQueueNew(5, sizeof(Message_t), NULL);
    g_uiQueue = osMessageQueueNew(5, sizeof(Message_t), NULL);
    g_backgroundQueue = osMessageQueueNew(5, sizeof(Message_t), NULL);
    g_logQueue = osMessageQueueNew(16, sizeof(Message_t), NULL);
    g_usbQueue = osMessageQueueNew(32, sizeof(Message_t), NULL);
    g_lowPowerQueue = osMessageQueueNew(5, sizeof(Message_t), NULL);


    //所有消息在此统一注册
    SubMessageID(MSG_TEST_CMD_FRAME, g_cmdQueue);

    SubMessageID(TEST_MSG_MEASUREMENT, g_testQueue);
    SubMessageID(TEST_MSG_ASYNPRINT, g_testQueue);
    SubMessageID(TEST_MSG_LEDGER_DATA_RCV, g_testQueue);

    SubMessageID(QRDECODE_MSG_START, g_qrDecodeQueue);
    SubMessageID(QRDECODE_MSG_STOP, g_qrDecodeQueue);
    SubMessageID(QRDECODE_MSG_MINUTE, g_qrDecodeQueue);

    SubMessageID(UI_MSG_SWITCH_HANDLER, g_uiQueue);
    SubMessageID(UI_MSG_EMIT_SIGNAL_TO_WORKING_VIEW, g_uiQueue);
    SubMessageID(UI_MSG_CLOSE_CURRENT_VIEW, g_uiQueue);
    SubMessageID(UI_MSG_ACTIVATE_LOOP, g_uiQueue);
    SubMessageID(UI_MSG_SCREEN_SHOT, g_uiQueue);
    SubMessageID(UI_MSG_PREPARE_RECEIVE_UR_USB, g_uiQueue);

    SubMessageID(BACKGROUND_MSG_EXECUTE, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_EXECUTE_RUNNABLE, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_MINUTE, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_CHANGER_INSERT, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_RESET, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_BATTERY_INTERVAL, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_TAMPER, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_SD_CARD_CHANGE, g_backgroundQueue);

    SubMessageID(LOG_MSG_WRITE, g_logQueue);
    SubMessageID(LOG_MSG_ERASE, g_logQueue);
    SubMessageID(LOG_MSG_EXPORT, g_logQueue);

    SubMessageID(USB_MSG_ISR_HANDLER, g_usbQueue);
    SubMessageID(USB_MSG_SET_STATE, g_usbQueue);
    SubMessageID(USB_MSG_INIT, g_usbQueue);
    SubMessageID(USB_MSG_DEINIT, g_usbQueue);

    SubMessageID(LOW_POWER_ENTER, g_lowPowerQueue);
    SubMessageID(LOW_POWER_QUIT, g_lowPowerQueue);
}

