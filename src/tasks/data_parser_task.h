#ifndef _DATA_PARSER_TASK_H
#define _DATA_PARSER_TASK_H

#include <stdint.h>
#include <stdbool.h>

void CreateDataParserTask(void);
void PushDataToField(uint8_t *data, uint16_t len);
uint8_t *GetDataParserCache(void);
void SetDeviceParserIv(uint8_t *iv);
uint8_t *GetDeviceParserPubKey(uint8_t *webPub, uint16_t len);
void DataEncrypt(uint8_t *data, uint16_t len);
void DataDecrypt(uint8_t *data, uint8_t *plain, uint16_t len);

#endif // DATA_PARSER_TASK_H
