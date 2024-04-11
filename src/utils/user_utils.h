#ifndef _USER_UTILS_H
#define _USER_UTILS_H

#include <ctype.h>
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "cjson/cJSON.h"
#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#endif

#define CLEAR_ARRAY(array)                      memset_s(array, sizeof(array), 0, sizeof(array))
#define CLEAR_OBJECT(obj)                       memset_s(&obj, sizeof(obj), 0, sizeof(obj))
#define VALUE_CHECK(value, expect)              {if (value != expect) {printf("input err!\r\n"); return; }}

uint32_t StrToHex(uint8_t *pbDest, const char *pbSrc);
void ByteArrayToHexStr(uint8_t *array, uint32_t len, char *hex);
bool CheckEntropy(const uint8_t *array, uint32_t len);
bool CheckAllFF(const uint8_t *array, uint32_t len);
bool CheckAllZero(const uint8_t *array, uint32_t len);
void RemoveFormatChar(char *str);
void ArrayRandom(char *words, char *out, int count);
int WordsListSlice(char *words, char wordsList[][10], uint8_t wordsCount);
void ConvertToLowerCase(char *str);
int FindStringCharPosition(const char *str, const char destChar, int index);
int32_t GetIntValue(const cJSON *obj, const char *key, int32_t defaultValue);
void GetStringValue(const cJSON *obj, const char *key, char *value, uint32_t maxLen);
bool GetBoolValue(const cJSON *obj, const char *key, bool defaultValue);
void CutAndFormatString(char *out, uint32_t maxLen, const char *string, uint32_t targetLen);
void CutAndFormatFileName(char *out, uint32_t maxLen, const char *fileName, const char *contain);

#endif /* _USER_UTILS_H */
