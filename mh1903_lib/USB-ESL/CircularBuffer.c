#include "CircularBuffer.h"

static void Init(CircularBufferStruct* self, uint8_t* buffer, uint32_t length)
{
    self->Buffer = buffer;
    self->Length = length;

    self->Content    = 0;
    self->PushOffset = 0;
    self->PushSize   = 0;
    self->PushSplit  = 0;
    self->PopOffset  = 0;
    self->PopSize    = 0;
    self->PopSplit   = 0;
}

static uint32_t Push(CircularBufferStruct* self, uint8_t* buffer, uint32_t maxPushSize, bool isPartialPush)
{
    if (!self->StartPush(self, maxPushSize, false, isPartialPush))
        return 0;

    memcpy(self->Buffer + self->PushOffset, buffer, self->PushSize - self->PushSplit);

    if (self->PushSplit)
        memcpy(self->Buffer, buffer + self->PushSize - self->PushSplit, self->PushSplit);

    return self->EndPush(self, self->PushSize);
}

inline uint32_t CircularBufferFreeSize(CircularBufferStruct* self)
{
    return self->Length - self->Content;
}

inline uint32_t CircularBufferContinuousFreeSize(CircularBufferStruct* self)
{
    uint32_t freeSize  = CircularBufferFreeSize(self);
    int      overSplit = freeSize + self->PushOffset - self->Length;
    return overSplit > 0 ? freeSize -= overSplit : freeSize;
}

static inline uint32_t GetFreeSize(CircularBufferStruct* self, uint32_t maxFreeSize, uint32_t* freeSize, uint32_t* splitSize, bool isContinuous)
{
    *freeSize  = self->Length - self->Content;
    *freeSize  = MIN(maxFreeSize, *freeSize);
    *splitSize = 0;

    int overSplit = *freeSize + self->PushOffset - self->Length;

    if (overSplit > 0) {
        if (isContinuous) {
            *freeSize -= overSplit;
            return *freeSize;
        }
        *splitSize = overSplit;
    }

    return *freeSize;
}

static bool StartPush(CircularBufferStruct* self, uint32_t maxPushSize, bool isContinuous, bool isPartialPush)
{
    if (self->PushSize)
        return false;

    if (!GetFreeSize(self, maxPushSize, &self->PushSize, &self->PushSplit, isContinuous))
        return false;

    if (!isPartialPush && self->PushSize < maxPushSize) {
        self->PushSize  = 0;
        self->PushSplit = 0;
        return false;
    }

    return true;
}

static uint32_t EndPush(CircularBufferStruct* self, uint32_t pushedSize)
{
    if (!pushedSize)
        return 0;

    __disable_irq();
    self->PushOffset += pushedSize;
    self->PushOffset %= self->Length;
    self->PushSize -= MIN(pushedSize, self->PushSize);
    self->Content += pushedSize;
    __enable_irq();

    return pushedSize;
}

inline uint32_t CircularBufferUsedSize(CircularBufferStruct* self)
{
    return self->Content;
}

inline uint32_t CircularBufferContinuousUsedSize(CircularBufferStruct* self)
{
    uint32_t usedSize  = self->Content;
    int      overSplit = usedSize + self->PopOffset - self->Length;
    return overSplit > 0 ? usedSize -= overSplit : usedSize;
}

static inline uint32_t GetUsedSize(CircularBufferStruct* self, uint32_t maxUsedSize, uint32_t* usedSize, uint32_t* splitSize, bool isContinuous)
{
    *usedSize  = self->Content;
    *usedSize  = MIN(maxUsedSize, *usedSize);
    *splitSize = 0;

    int overSplit = *usedSize + self->PopOffset - self->Length;

    if (overSplit > 0) {
        if (isContinuous) {
            *usedSize -= overSplit;
            return *usedSize;
        }
        *splitSize = overSplit;
    }

    return *usedSize;
}

static uint32_t Pop(CircularBufferStruct* self, uint8_t* buffer, uint32_t maxPopSize, bool isPartialPop)
{
    if (!self->StartPop(self, maxPopSize, false, isPartialPop))
        return 0;

    memcpy(buffer, self->Buffer + self->PopOffset, self->PopSize - self->PopSplit);

    if (self->PopSplit)
        memcpy(buffer + self->PopSize - self->PopSplit, self->Buffer, self->PopSplit);

    return self->EndPop(self, self->PopSize);
}

static bool StartPop(CircularBufferStruct* self, uint32_t maxPopSize, bool isContinuous, bool isPartialPop)
{
    if (self->PopSize)
        return false;

    if (!GetUsedSize(self, maxPopSize, &self->PopSize, &self->PopSplit, isContinuous))
        return false;

    if (!isPartialPop && !self->PopSize) {
        self->PopSize   = 0;
        self->PushSplit = 0;
        return false;
    }

    return true;
}

static uint32_t EndPop(CircularBufferStruct* self, uint32_t poppedSize)
{
    if (!poppedSize)
        return 0;

    __disable_irq();
    self->PopOffset += poppedSize;
    self->PopOffset %= self->Length;
    self->PopSize -= poppedSize;
    self->Content -= poppedSize;
    __enable_irq();

    return poppedSize;
}

static bool StartPopAlign(CircularBufferStruct* self, uint32_t maxPopSize, bool isContinuous, bool isPartialPop)
{
    if (!StartPop(self, maxPopSize, isContinuous, isPartialPop))
        return false;

    __disable_irq();
    // Align out end offset
    if (self->PopAlign && self->PushSize) {
        int alignBytes = self->PopSize % self->PopAlign;
        self->PopSize -= alignBytes;
        self->PopSplit = self->PopSplit >= alignBytes ? self->PopSplit - alignBytes : 0;
        if (!self->PopSize)
            return false;
    }

    self->PopPadding = (self->PopAlign && self->PopSize == self->Content) ? (self->PopSize % self->PopAlign) : 0;
    if (self->PopPadding) {
        self->PopPadding = self->PopAlign - self->PopPadding;
        self->PushOffset += self->PopPadding;
        self->PushOffset %= self->Length;
    }
    __enable_irq();

    return true;
}

static uint32_t EndPopAlign(CircularBufferStruct* self, uint32_t poppedSize)
{
    if (!poppedSize)
        return 0;

    __disable_irq();
    self->PopOffset += poppedSize;
    if (self->PopPadding && poppedSize == self->PopSize) {
        self->PopOffset += self->PopPadding;
    }
    self->PopOffset %= self->Length;
    self->PopSize -= poppedSize;
    self->Content -= poppedSize;
    __enable_irq();

    return poppedSize;
}

bool CircularBufferConstractor(CircularBufferStruct* self, uint8_t popAlign)
{
    self->Init = Init;

    self->Push      = Push;
    self->StartPush = StartPush;
    self->EndPush   = EndPush;
    self->Pop       = Pop;
    self->StartPop  = StartPop;
    self->EndPop    = EndPop;

    if (popAlign) {
        self->PopAlign = popAlign;
        self->StartPop = StartPopAlign;
        self->EndPop   = EndPopAlign;
    }

    return true;
}
