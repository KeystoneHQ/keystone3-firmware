#ifndef _DATA_PARSER_TASK_H
#define _DATA_PARSER_TASK_H

#include "stdint.h"
#include "stdbool.h"

void CreateDataParserTask(void);
void TestMpu(void);
uint8_t *GetDataParserPubKey(void);
void DataEncrypt(uint8_t *data, uint16_t len);
uint8_t *GetDeviceParserPubKey(uint8_t *webPub, uint16_t len);

#endif
