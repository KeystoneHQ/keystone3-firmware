/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: cmd任务
 * Author: leon sun
 * Create: 2022-11-8
 ************************************************************************************************/

#ifndef _CMD_TASK_H
#define _CMD_TASK_H

#include "stdint.h"


void CreateCmdTask(void);
void CmdIsrRcvByte(uint8_t byte);
void TestCmdRcvByte(uint8_t byte);

#endif
