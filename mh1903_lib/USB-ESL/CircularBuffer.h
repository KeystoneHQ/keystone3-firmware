#ifndef __CIRCULARBUFFER_H__
#define __CIRCULARBUFFER_H__

#include "USBESL.h"

typedef struct CircularBufferStruct_s CircularBufferStruct;

extern bool CircularBufferConstractor(CircularBufferStruct* self, uint8_t popAlign);
extern uint32_t CircularBufferFreeSize(CircularBufferStruct* self);
extern uint32_t CircularBufferUsedSize(CircularBufferStruct* self);

struct CircularBufferStruct_s {
    void (*Init)(CircularBufferStruct* self, uint8_t* buffer, uint32_t length);

    uint32_t (*Push)(CircularBufferStruct* self, uint8_t* buffer, uint32_t maxPushSize, bool isPartialPush);
    bool (*StartPush)(CircularBufferStruct* self, uint32_t maxPushSize, bool isContinuous, bool isPartialPush);
    uint32_t (*EndPush)(CircularBufferStruct* self, uint32_t pushedSize);

    uint32_t (*Pop)(CircularBufferStruct* self, uint8_t* buffer, uint32_t maxPopSize, bool isPartialPop);
    bool (*StartPop)(CircularBufferStruct* self, uint32_t maxPopSize, bool isContinuous, bool isPartialPop);
    uint32_t (*EndPop)(CircularBufferStruct* self, uint32_t poppedSize);

    uint8_t* Buffer;
    uint32_t Length;
    uint32_t Content;

    uint8_t PopAlign;
    uint8_t PopPadding;

    uint32_t PushOffset;
    uint32_t PushSize;
    uint32_t PushSplit;

    uint32_t PopOffset;
    uint32_t PopSize;
    uint32_t PopSplit;
};

#endif
