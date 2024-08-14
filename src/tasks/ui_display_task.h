#ifndef _UI_DISPLAY_TASK_H
#define _UI_DISPLAY_TASK_H

#include "stdint.h"
#include "stdbool.h"

typedef int32_t (*UIDisplayFunc_t)(const void *inData, uint32_t inDataLen);
typedef struct {
    UIDisplayFunc_t func;
    void *inData;
    uint32_t inDataLen;
} UIDisplay_t;

void CreateUiDisplayTask(void);
void SetLvglHandlerAndSnapShot(bool enable);
void LvglCloseCurrentView(void);
uint8_t *GetLvglGramAddr(void);
uint32_t GetLvglGramSize(void);
void ActivateUiTaskLoop(void);
void LvglImportMicroCardSigView(void);
void NftLockDecodeTouchQuit(void);
void SetNftLockState(void);

extern bool g_reboot;
#endif

