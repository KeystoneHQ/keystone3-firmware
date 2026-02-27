#ifndef __GUI_PREPARE_KEY_PAIR_WIDGETS_H__
#define __GUI_PREPARE_KEY_PAIR_WIDGETS_H__

#include <stdio.h>
#include <stdint.h>

void GuiPrepareKeyPairInit(void);
void GuiPrepareKeyPairDeInit(void);
void GuiPrepareKeyPairRefresh(void);
void GuiPrepareKeyPairPrevTile(void);
void GuiPrepareKeyPairNextTile(void);
void GuiPrepareKeyPairOnPubkeyVerifySuccess(void *param, uint16_t usLen);

#endif
