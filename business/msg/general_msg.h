/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2025. All rights reserved.
 * Description: Generic messaging based on publish/subscribe mechanism.
 * Author: leon sun
 * Create: 2022-11-8
 ************************************************************************************************/

#ifndef _GENERAL_MSG_H
#define _GENERAL_MSG_H

#include "stdint.h"
#include "stdbool.h"


#define MSG_SUCCESS                     0
#define MSG_MESSAGE_ID_ERROR            1
#define MSG_SEND_ERROR                  2
#define MSG_MEM_ERROR                   3
#define MSG_NULL_QUEUE                  4
#define MSG_QUEUE_ALREADY_REG_IN_MID    5
#define MSG_OS_NOT_START                6


#define INVALID_QUEUE_ID                0xFFFFFFFF


typedef struct {
    uint32_t id;
    uint32_t value;
    uint8_t *buffer;
    uint32_t length;
} Message_t;

uint32_t PubBufferMsg(uint32_t messageID, void *buffer, uint32_t length);
uint32_t PubValueMsg(uint32_t messageID, uint32_t value);
uint32_t GetWaitingMsgCount(uint32_t messageID);
uint32_t SubMessageID(uint32_t messageID, void* queue);



#endif
