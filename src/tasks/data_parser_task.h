#ifndef _DATA_PARSER_TASK_H
#define _DATA_PARSER_TASK_H

#include "stdint.h"
#include "stdbool.h"

void CreateDataParserTask(void);
void TestMpu(void);
uint8_t *GetDataParserPubKey(void);
uint8_t *GetDeviceParserPubKey(uint8_t *webPub, uint16_t len);
void DataEncrypt(uint8_t *data, uint16_t len);
void DataDecrypt(uint8_t *data, uint8_t *plain, uint16_t len);
void SetDeviceParserIv(uint8_t *iv);

#endif
