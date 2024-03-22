#include "general_msg.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "cmsis_os.h"

static void* GetQueueFromMid(uint32_t messageID, uint32_t index);

struct __QueueNode {
    void* queue;
    struct __QueueNode *next;
};
typedef struct __QueueNode QueueNode_t;

struct __MessageNode {
    uint32_t messageID;
    QueueNode_t *queueList;
    struct __MessageNode *next;
};
typedef struct __MessageNode MessageNode_t;


MessageNode_t messageListHead;

uint32_t PubBufferMsg(uint32_t messageID, void *buffer, uint32_t length)
{
    Message_t msg;
    osMessageQueueId_t queue;
    osStatus_t err = osOK;

    msg.id = messageID;
    msg.value = 0;
    msg.length = length;

    for (uint32_t i = 0;; i++) {
        queue = GetQueueFromMid(messageID, i);
        if (queue == NULL) {
            if (i == 0) {
                printf("msg id err,msg id=0x%08X\r\n", messageID);
                return MSG_MESSAGE_ID_ERROR;
            } else {
                return MSG_SUCCESS;
            }
        } else if (osMessageQueueGetSpace(queue) == 0) {  // queue is full
            return MSG_SEND_ERROR;
        }
        msg.buffer = (uint8_t *)SramMalloc(length);
        if (msg.buffer == NULL) {
            printf("msg malloc err\n");
            return MSG_MEM_ERROR;
        }
        memcpy(msg.buffer, buffer, length);
        err = osMessageQueuePut(queue, &msg, 0, 0);
        if (err != osOK) {
            SramFree(msg.buffer);
            printf("msg bugger write queue err,queue=0x%08X,msgID=0x%08X,errid=0x%08X\r\n", queue, messageID, err);
            continue;
        }
    }
}


uint32_t PubValueMsg(uint32_t messageID, uint32_t value)
{
    Message_t msg;
    osMessageQueueId_t queue;
    osStatus_t err = osOK;

    msg.id = messageID;
    msg.value = value;
    msg.buffer = NULL;
    msg.length = 0;


    for (uint32_t i = 0;; i++) {
        queue = GetQueueFromMid(messageID, i);
        if (queue == NULL) { // queue is empty
            if (i == 0) {
                printf("msg id err,msg id=0x%08X\r\n", messageID);
                return MSG_MESSAGE_ID_ERROR;
            } else {
                return MSG_SUCCESS;
            }
        } else if (osMessageQueueGetSpace(queue) == 0) {  // queue is full
            return MSG_SEND_ERROR;
        }
        err = osMessageQueuePut(queue, &msg, 0, 0);
        if (err != osOK) {
            printf("msg write queue err,queue=0x%08X,msgID=0x%08X,errid=0x%08X\r\n", queue, messageID, err);
            continue;
        }
    }
}


/// @brief Get the number of queued messages for all corresponding queues.
/// @param messageID
/// @return Number of queued messages.
uint32_t GetWaitingMsgCount(uint32_t messageID)
{
    osMessageQueueId_t queue;
    uint32_t count = 0;

    for (uint32_t i = 0;; i++) {
        queue = GetQueueFromMid(messageID, i);
        if (queue == NULL) {
            //The queue is empty.
            return count;
        } else {
            count += osMessageQueueGetCount(queue);
        }
    }
}



//Subscription, that is, the message registry, is managed using a two-dimensional linked list: one messageID per line
//
//mID1->    queue1->    queue2->    NULL
//mID2->    queue2->    NULL
//mID3->    queue3->    NULL
//mID4->    queue1->    NULL
//
//Register messages in the message registry. After registration, you can use messageID to query which queues need to send data.
uint32_t SubMessageID(uint32_t messageID, void* queue)
{
    if (queue == NULL) {
        return MSG_NULL_QUEUE;
    }

    if (messageID == 0) {
        return MSG_MESSAGE_ID_ERROR;
    }

    MessageNode_t *p_messageNode = &messageListHead;
    QueueNode_t *p_queueNode;

    if (p_messageNode->messageID == 0) {
        //New message form.
        p_messageNode->messageID = messageID;
    }

    while (1) {
        if (p_messageNode->messageID == messageID) {
            //Found the same row as messageID in the registry.
            p_queueNode = p_messageNode->queueList;
            while (1) {
                //Need to find the end of the column, and then add queue items at the end of the column.
                if (p_queueNode == NULL) {
                    //New line.
                    p_queueNode = (QueueNode_t *)SramMalloc(sizeof(QueueNode_t));
                    memset_s(p_queueNode, sizeof(QueueNode_t), 0, sizeof(QueueNode_t));
                    p_queueNode->queue = queue;
                    p_queueNode->next = NULL;
                    p_messageNode->queueList = p_queueNode;
                    //am_util_debug_printf("new line\r\n");
                    return MSG_SUCCESS;
                }

                if (p_queueNode->queue == queue) {
                    //Prevent re-registration
                    return MSG_QUEUE_ALREADY_REG_IN_MID;
                } else if (p_queueNode->next == NULL) {
                    //The end of the column has been reached, add a column and write to the queue.
                    p_queueNode->next = (QueueNode_t *)SramMalloc(sizeof(QueueNode_t));
                    memset_s(p_queueNode->next, sizeof(QueueNode_t), 0, sizeof(QueueNode_t));
                    p_queueNode = p_queueNode->next;
                    p_queueNode->queue = queue;
                    p_queueNode->next = NULL;
                    //am_util_debug_printf("add new col\r\n");
                    return MSG_SUCCESS;
                } else {
                    //Not found, go to the next column.
                    p_queueNode = p_queueNode->next;
                    //am_util_debug_printf("goto next col\r\n");
                }
            }
        } else if (p_messageNode->next == NULL) {
            //The end of the line has been reached, and a new line needs to be created, and the messageID of the new line is assigned the locally registered messageID.
            p_messageNode->next = (MessageNode_t *)SramMalloc(sizeof(MessageNode_t));
            memset_s(p_messageNode->next, sizeof(MessageNode_t), 0, sizeof(MessageNode_t));
            p_messageNode = p_messageNode->next;
            p_messageNode->messageID = messageID;
            p_messageNode->next = NULL;
        } else {
            //Not found, go to the next line.
            p_messageNode = p_messageNode->next;
        }
    }
}

//Query the queue in the message registry by messageID and index number.
//If found, return the queue handle, if not found, return NULL.
static void* GetQueueFromMid(uint32_t messageID, uint32_t index)
{
    MessageNode_t *p_messageNode = &messageListHead;
    QueueNode_t *p_queueNode;

    if (messageID == 0) {
        return NULL;
    }
    while (1) {
        if (p_messageNode->messageID == messageID) {
            p_queueNode = p_messageNode->queueList;
            if (p_queueNode == NULL) {
                return NULL;
            }
            //The corresponding messageID row is found.
            for (uint32_t i = 0; i < index; i++) {
                p_queueNode = p_queueNode->next;
                if (p_queueNode == NULL) {
                    return NULL;
                }
            }
            return p_queueNode->queue;
        } else if (p_messageNode->next == NULL) {
            //The line has been checked, and there is no corresponding messageID.
            return NULL;
        } else {
            //Next line.
            p_messageNode = p_messageNode->next;
        }
    }
}
