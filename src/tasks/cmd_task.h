#ifndef _CMD_TASK_H
#define _CMD_TASK_H

#include "stdint.h"

void CreateCmdTask(void);
void CmdIsrRcvByte(uint8_t byte);
void TestCmdRcvByte(uint8_t byte);

#endif
