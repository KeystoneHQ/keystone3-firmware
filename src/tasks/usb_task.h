#ifndef _USB_TASK_H
#define _USB_TASK_H

#include "stdint.h"
#include "stdbool.h"


void CreateUsbTask(void);
void SetUsbState(bool enable);
bool GetUsbState(void);
void OpenUsb(void);
void CloseUsb(void);

void UsbTest(int argc, char *argv[]);

#endif

