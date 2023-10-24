#ifndef _GUI_APT_H
#define _GUI_APT_H

#include "librust_c.h"

void GuiSetAptosUrData(void *data, bool multi);
void *GuiGetAptosData(void);
void FreeAptosMemory(void);
int GetAptosDetailLen(void *param);
void GetAptosDetail(void *indata, void *param);
UREncodeResult *GuiGetAptosSignQrCodeData(void);
bool IsAptosMsg(ViewType viewType);

#endif
