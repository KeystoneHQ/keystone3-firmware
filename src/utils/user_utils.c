#include "user_utils.h"
#include "define.h"
#include "lvgl.h"
#include "user_memory.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#else
#include "safe_str_lib.h"
#endif

uint32_t StrToHex(uint8_t *pbDest, const char *pbSrc)
{
    char h1, h2;
    unsigned char s1, s2;
    int i;
    for (i = 0;; i++) {
        h1 = pbSrc[2 * i];
        h2 = pbSrc[2 * i + 1];
        if (h1 == 0 || h2 == 0) {
            break;
        }
        s1 = toupper(h1) - 0x30;
        if (s1 > 9) {
            s1 -= 7;
        }
        s2 = toupper(h2) - 0x30;
        if (s2 > 9) {
            s2 -= 7;
        }
        pbDest[i] = s1 * 16 + s2;
    }
    return i;
}

void ByteArrayToHexStr(uint8_t *array, uint32_t len, char *hex)
{
    char hexAlphaLookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    for (int i = 0; i < len; i++) {
        hex[2 * i] = hexAlphaLookup[array[i] >> 4];
        hex[2 * i + 1] = hexAlphaLookup[array[i] & 0x0F];
    }
}

/// @brief Simply Check the entropy of an array.
/// @param[in] array Array to be checked.
/// @param[in] len Array length.
/// @return True if the array entropy is fine.
bool CheckEntropy(const uint8_t *array, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        if (array[i] != 0 && array[i] != 0xFF) {
            return true;
        }
    }
    return false;
}

/// @brief Check the array if all data are 0xFF.
/// @param[in] array Array to be checked.
/// @param[in] len Array length.
/// @return True if the array data are all 0xFF.
bool CheckAllFF(const uint8_t *array, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        if (array[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

/// @brief Check the array if all data are zero.
/// @param[in] array Array to be checked.
/// @param[in] len Array length.
/// @return True if the array data are all zero.
bool CheckAllZero(const uint8_t *array, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        if (array[i] != 0) {
            return false;
        }
    }
    return true;
}

void RemoveFormatChar(char *str)
{
#ifndef COMPILE_SIMULATOR
    char *str_c = str;
    int i, j = 0;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ' && str[i] != '\r' && str[i] != '\n' && str[i] != '\t')
            str_c[j++] = str[i];
    }
    str_c[j] = '\0';
    str = str_c;
#endif
}

int WordsListSlice(char *words, char wordsList[][10], uint8_t wordsCount)
{
    char buf[16];
    uint8_t i = 0;
    uint8_t j = 0;

    const char *p = words;

    while (*p) {
        for (i = 0; *p >= 'a' && *p <= 'z'; i++, p++) {
            if (i < 15) {
                buf[i] = *p;
            } else {
                buf[15] = 0;
            }
        }
        if (i < 15) {
            buf[i] = 0;
        }
        strcpy_s(wordsList[j], 16, buf);
        j++;

        while (*p && (*p < 'a' || *p > 'z')) {
            p++;
        }
    }

    return j;
}

void ArrayRandom(char *words, char *out, int count)
{
#ifndef BUILD_PRODUCTION
    strcpy_s(out, BUFFER_SIZE_512, words);
    return;
#endif
    char wordList[33][10];
    memset_s(out, BUFFER_SIZE_512, 0, BUFFER_SIZE_512);
    WordsListSlice(words, wordList, count);

    char *pointerList[33];
    for (int i = 0; i < count; ++i) {
        pointerList[i] = wordList[i];
    }

    for (int i = 0; i < count - 1; ++i) {
        int num = i + lv_rand(0, 2048) % (count - i);
        char *temp = pointerList[i];
        pointerList[i] = pointerList[num];
        pointerList[num] = temp;
    }

    char *outPtr = out;
    for (int i = 0; i < count; ++i) {
        strcpy_s(outPtr, 10, pointerList[i]);
        outPtr += strlen(pointerList[i]);

        if (i < count - 1) {
            *outPtr++ = ' ';
        } else {
            *outPtr = '\0';
        }
    }
}

void ConvertToLowerCase(char *str)
{
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}

inline bool CheckContainsNull(const char *str, size_t maxLen)
{
    for (size_t len = 0; len < maxLen; ++len) {
        if (str[len] == '\0') {
            return true;
        }
    }

    return false;
}

int FindStringCharPosition(const char *str, const char destChar, int index)
{
    int slashCount = 0;

    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == destChar) {
            ++slashCount;
            if (slashCount == index) {
                return i;
            }
        }
    }

    return -1;
}

/**
 * @brief       Get integer value from cJSON object.
 * @param[in]   obj : cJSON object.
 * @param[in]   key : key name.
 * @param[in]   defaultValue : if key does not exist, return this value.
 * @retval      integer value to get.
 */
int32_t GetIntValue(const cJSON *obj, const char *key, int32_t defaultValue)
{
    cJSON *intJson = cJSON_GetObjectItem((cJSON *)obj, key);
    if (intJson != NULL) {
        return (uint32_t)intJson->valuedouble;
    }
    printf("key:%s does not exist\r\n", key);
    return defaultValue;
}

/**
 * @brief       Get string value from cJSON object.
 * @param[in]   obj : cJSON object.
 * @param[in]   key : key name.
 * @param[out]  value : return string value, if the acquisition fails, the string will be cleared.
 * @retval
 */
void GetStringValue(const cJSON *obj, const char *key, char *value, uint32_t maxLen)
{
    cJSON *json;
    uint32_t len;
    char *strTemp;

    json = cJSON_GetObjectItem((cJSON *)obj, key);
    if (json != NULL) {
        strTemp = json->valuestring;
        len = strnlen_s(strTemp, maxLen);
        if (len < maxLen) {
            strcpy_s(value, maxLen, strTemp);
        } else {
            value[0] = '\0';
        }
    } else {
        printf("key:%s does not exist\r\n", key);
        value[0] = '\0';
    }
}

bool GetBoolValue(const cJSON *obj, const char *key, bool defaultValue)
{
    cJSON *boolJson = cJSON_GetObjectItem((cJSON *)obj, key);
    if (boolJson != NULL) {
        return boolJson->valueint != 0;
    }
    printf("key:%s does not exist\r\n", key);
    return defaultValue;
}

void CutAndFormatFileName(char *out, uint32_t maxLen, const char *fileName, const char *contain)
{
    if (strlen(fileName) >= 20 + strlen(contain)) {
        char fname[BUFFER_SIZE_128] = {0};
        strncpy_s(fname, BUFFER_SIZE_128, fileName, strrchr(fileName, '.') - fileName);
        CutAndFormatString(out, maxLen, fname, 16);
        printf("out = %s\n", out);
        strcat_s(out, maxLen, contain);
    } else {
        strcpy_s(out, maxLen, fileName);
    }
}

void CutAndFormatString(char *out, uint32_t maxLen, const char *string, uint32_t targetLen)
{
    memset_s(out, maxLen, '\0', maxLen);
    uint32_t len = strnlen_s(string, maxLen + 1);

    if (len < targetLen) {
        strcpy_s(out, len + 1, string);
    } else {
        size_t halfLen = targetLen / 2;
        strncpy_s(out, maxLen, string, halfLen);
        strcat_s(out, maxLen, "...");
        strncat_s(out, maxLen, string + len - halfLen, halfLen);
    }
}