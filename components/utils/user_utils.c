/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : user_utils.c
 * Description:
 * author     : stone wang
 * data       : 2022-12-29 16:42
**********************************************************************/

#include "user_utils.h"
#include "lvgl.h"

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
    char *str_c = str;
    int i, j = 0;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ' && str[i] != '\r' && str[i] != '\n' && str[i] != '\t')
            str_c[j++] = str[i];
    }
    str_c[j] = '\0';
    str = str_c;
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
//        printf("buf = %s\n", buf);
        strcpy(wordsList[j], buf);
        j++;

        while (*p && (*p < 'a' || *p > 'z')) {
            p++;
        }
    }

    return j;
}

void ArrayRandom(char *words, char *out, int count)
{
    // strcpy(out, words);
    // return;
    int index = count - 1;
    char wordList[33][10];
    memset(out, 0, 512);
    WordsListSlice(words, wordList, count);
    char tempBuf[16] = {0};
    if (count  % 2 == 1) {
        index = count - 2;
    }
    for (int i = 0; i < index; i++) {
        int num = i + lv_rand(0, 2048) % (index - i);
        strcpy(tempBuf, wordList[i]);
        strcpy(wordList[i], wordList[num]);
        strcpy(wordList[num], tempBuf);
    }

    int num = lv_rand(0, 2048) % (index - 1);
    strcpy(tempBuf, wordList[count - 1]);
    strcpy(wordList[count - 1], wordList[num]);
    strcpy(wordList[num], tempBuf);

    for (int i = 0; i < count - 1; i++) {
        strcat(out, wordList[i]);
        strcat(out, " ");
    }
    strcat(out, wordList[count - 1]);
}