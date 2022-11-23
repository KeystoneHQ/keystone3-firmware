#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "slip39.h"
#include "slip39_group.h"
#define SHARE_BUFFER_SIZE               4096

int fromhex(uint8_t *buf, uint32_t *buf_len, const char *str)
{
    size_t i, len = strlen(str) / 2;

    if (len > *buf_len)
        return -1;

    for (i = 0; i < len; i++) {
        uint8_t c = 0;
        if (str[i * 2] >= '0' && str[i * 2] <= '9') c += (str[i * 2] - '0') << 4;
        if ((str[i * 2] & ~0x20) >= 'A' && (str[i * 2] & ~0x20) <= 'F') c += (10 + (str[i * 2] & ~0x20) - 'A') << 4;
        if (str[i * 2 + 1] >= '0' && str[i * 2 + 1] <= '9') c += (str[i * 2 + 1] - '0');
        if ((str[i * 2 + 1] & ~0x20) >= 'A' && (str[i * 2 + 1] & ~0x20) <= 'F') c += (10 + (str[i * 2 + 1] & ~0x20) - 'A');
        buf[i] = c;
    }
    *buf_len = len;
    return 0;
}

int main(int argc, char **argv)
{
#if 0
    uint8_t master_secret[32];
    uint8_t masterSecret[32] = {0xdb, 0xc4, 0xac, 0x53, 0xfc, 0xc6, 0xd3, 0x3e, 0x38, 0xc6, 0x3f, 0xde, 0x60, 0xda, 0xe8, 0x9f};
    uint8_t *passPhrase = "";
    uint8_t iterationExponent  = 0;

    uint8_t memberCnt = 5;
    uint8_t memberThreshold = 3;

    uint8_t groupCnt = 1;
    uint8_t groupThereshold = 1;

    GroupDesc_t groups[] = {
        {memberCnt, memberThreshold, NULL},
    };

    uint16_t shareBufferSize = SHARE_BUFFER_SIZE;
    uint16_t *sharesBuff[SHARE_BUFFER_SIZE];

//    GenerateMnemonics(masterSecret, strlen(masterSecret), iterationExponent, passPhrase, groupCnt, groupThereshold, groups, sharesBuff,shareBufferSize);
    GetSlip39MnemonicsWords(20, 5, 3, wordsList);
#endif
#if 0
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < 20; i++) {
            printf("%s ", wordsList[j][i]);
        }
        printf("\n");
    }
#endif
    char *words[3];
    FILE *fp = fopen("../menmonics_list.txt", "r");
    if (fp == NULL) {
        printf("open mnemonics_list error\n");
        return;
    }
    for (int i = 0; i < 3 ; i++) {
        words[i] = malloc(33 * 10);
        memset(words[i], 0, 33 * 10);
        fgets(words[i], 33 * 10, fp);
    }

    Slip39MnemonicWordsCombline(words);

    return 0;
}
