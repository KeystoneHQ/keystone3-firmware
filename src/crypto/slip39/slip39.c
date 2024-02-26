#include <string.h>
#include "slip39.h"
#include "slip39_group.h"
#include "slip39_wordlist.h"
#include "log_print.h"
#include "hmac.h"
#include "memzero.h"
#include "rand.h"
#include "pbkdf2.h"
#include "rs1024.h"
#include "sha2.h"
#include "pbkdf2.h"
#include "user_memory.h"
#include "user_utils.h"
#include <stdlib.h>

void Slip39Error(int errNum);
extern void pbkdf2_hmac_sha256_slip39(const uint8_t *pass, int passlen, const uint8_t *salt,
                                      int saltlen, uint32_t iterations, uint8_t *key,
                                      int keylen);

static uint8_t LOG_TABLE[] = {
    0x00, 0x00, 0x19, 0x01, 0x32, 0x02, 0x1a, 0xc6, 0x4b, 0xc7, 0x1b, 0x68, 0x33, 0xee, 0xdf, 0x03,
    0x64, 0x04, 0xe0, 0x0e, 0x34, 0x8d, 0x81, 0xef, 0x4c, 0x71, 0x08, 0xc8, 0xf8, 0x69, 0x1c, 0xc1,
    0x7d, 0xc2, 0x1d, 0xb5, 0xf9, 0xb9, 0x27, 0x6a, 0x4d, 0xe4, 0xa6, 0x72, 0x9a, 0xc9, 0x09, 0x78,
    0x65, 0x2f, 0x8a, 0x05, 0x21, 0x0f, 0xe1, 0x24, 0x12, 0xf0, 0x82, 0x45, 0x35, 0x93, 0xda, 0x8e,
    0x96, 0x8f, 0xdb, 0xbd, 0x36, 0xd0, 0xce, 0x94, 0x13, 0x5c, 0xd2, 0xf1, 0x40, 0x46, 0x83, 0x38,
    0x66, 0xdd, 0xfd, 0x30, 0xbf, 0x06, 0x8b, 0x62, 0xb3, 0x25, 0xe2, 0x98, 0x22, 0x88, 0x91, 0x10,
    0x7e, 0x6e, 0x48, 0xc3, 0xa3, 0xb6, 0x1e, 0x42, 0x3a, 0x6b, 0x28, 0x54, 0xfa, 0x85, 0x3d, 0xba,
    0x2b, 0x79, 0x0a, 0x15, 0x9b, 0x9f, 0x5e, 0xca, 0x4e, 0xd4, 0xac, 0xe5, 0xf3, 0x73, 0xa7, 0x57,
    0xaf, 0x58, 0xa8, 0x50, 0xf4, 0xea, 0xd6, 0x74, 0x4f, 0xae, 0xe9, 0xd5, 0xe7, 0xe6, 0xad, 0xe8,
    0x2c, 0xd7, 0x75, 0x7a, 0xeb, 0x16, 0x0b, 0xf5, 0x59, 0xcb, 0x5f, 0xb0, 0x9c, 0xa9, 0x51, 0xa0,
    0x7f, 0x0c, 0xf6, 0x6f, 0x17, 0xc4, 0x49, 0xec, 0xd8, 0x43, 0x1f, 0x2d, 0xa4, 0x76, 0x7b, 0xb7,
    0xcc, 0xbb, 0x3e, 0x5a, 0xfb, 0x60, 0xb1, 0x86, 0x3b, 0x52, 0xa1, 0x6c, 0xaa, 0x55, 0x29, 0x9d,
    0x97, 0xb2, 0x87, 0x90, 0x61, 0xbe, 0xdc, 0xfc, 0xbc, 0x95, 0xcf, 0xcd, 0x37, 0x3f, 0x5b, 0xd1,
    0x53, 0x39, 0x84, 0x3c, 0x41, 0xa2, 0x6d, 0x47, 0x14, 0x2a, 0x9e, 0x5d, 0x56, 0xf2, 0xd3, 0xab,
    0x44, 0x11, 0x92, 0xd9, 0x23, 0x20, 0x2e, 0x89, 0xb4, 0x7c, 0xb8, 0x26, 0x77, 0x99, 0xe3, 0xa5,
    0x67, 0x4a, 0xed, 0xde, 0xc5, 0x31, 0xfe, 0x18, 0x0d, 0x63, 0x8c, 0x80, 0xc0, 0xf7, 0x70, 0x07,
};

static uint8_t EXP_TABLE[] = {
    0x01, 0x03, 0x05, 0x0f, 0x11, 0x33, 0x55, 0xff, 0x1a, 0x2e, 0x72, 0x96, 0xa1, 0xf8, 0x13, 0x35,
    0x5f, 0xe1, 0x38, 0x48, 0xd8, 0x73, 0x95, 0xa4, 0xf7, 0x02, 0x06, 0x0a, 0x1e, 0x22, 0x66, 0xaa,
    0xe5, 0x34, 0x5c, 0xe4, 0x37, 0x59, 0xeb, 0x26, 0x6a, 0xbe, 0xd9, 0x70, 0x90, 0xab, 0xe6, 0x31,
    0x53, 0xf5, 0x04, 0x0c, 0x14, 0x3c, 0x44, 0xcc, 0x4f, 0xd1, 0x68, 0xb8, 0xd3, 0x6e, 0xb2, 0xcd,
    0x4c, 0xd4, 0x67, 0xa9, 0xe0, 0x3b, 0x4d, 0xd7, 0x62, 0xa6, 0xf1, 0x08, 0x18, 0x28, 0x78, 0x88,
    0x83, 0x9e, 0xb9, 0xd0, 0x6b, 0xbd, 0xdc, 0x7f, 0x81, 0x98, 0xb3, 0xce, 0x49, 0xdb, 0x76, 0x9a,
    0xb5, 0xc4, 0x57, 0xf9, 0x10, 0x30, 0x50, 0xf0, 0x0b, 0x1d, 0x27, 0x69, 0xbb, 0xd6, 0x61, 0xa3,
    0xfe, 0x19, 0x2b, 0x7d, 0x87, 0x92, 0xad, 0xec, 0x2f, 0x71, 0x93, 0xae, 0xe9, 0x20, 0x60, 0xa0,
    0xfb, 0x16, 0x3a, 0x4e, 0xd2, 0x6d, 0xb7, 0xc2, 0x5d, 0xe7, 0x32, 0x56, 0xfa, 0x15, 0x3f, 0x41,
    0xc3, 0x5e, 0xe2, 0x3d, 0x47, 0xc9, 0x40, 0xc0, 0x5b, 0xed, 0x2c, 0x74, 0x9c, 0xbf, 0xda, 0x75,
    0x9f, 0xba, 0xd5, 0x64, 0xac, 0xef, 0x2a, 0x7e, 0x82, 0x9d, 0xbc, 0xdf, 0x7a, 0x8e, 0x89, 0x80,
    0x9b, 0xb6, 0xc1, 0x58, 0xe8, 0x23, 0x65, 0xaf, 0xea, 0x25, 0x6f, 0xb1, 0xc8, 0x43, 0xc5, 0x54,
    0xfc, 0x1f, 0x21, 0x63, 0xa5, 0xf4, 0x07, 0x09, 0x1b, 0x2d, 0x77, 0x99, 0xb0, 0xcb, 0x46, 0xca,
    0x45, 0xcf, 0x4a, 0xde, 0x79, 0x8b, 0x86, 0x91, 0xa8, 0xe3, 0x3e, 0x42, 0xc6, 0x51, 0xf3, 0x0e,
    0x12, 0x36, 0x5a, 0xee, 0x29, 0x7b, 0x8d, 0x8c, 0x8f, 0x8a, 0x85, 0x94, 0xa7, 0xf2, 0x0d, 0x17,
    0x39, 0x4b, 0xdd, 0x7c, 0x84, 0x97, 0xa2, 0xfd, 0x1c, 0x24, 0x6c, 0xb4, 0xc7, 0x52, 0xf6, 0x00,
};

size_t slip39_word_count_for_bytes(size_t bytes)
{
    return (bytes * 8 + RADIX_BITS - 1) / RADIX_BITS;
}

size_t slip39_byte_count_for_words(size_t words)
{
    return (words * RADIX_BITS) / 8;
}

uint16_t slip39_word_for_string(const char *word)
{
    int16_t hi = WORDLIST_SIZE;
    int16_t lo = SLIP39_INVALID_MNEMONIC_INDEX;

    while (hi > lo + 1) {
        int16_t mid = (hi + lo) / 2;
        int16_t cmp = strcmp(word, slip39_wordlists[mid]);
        if (cmp > 0) {
            lo = mid;
        } else if (cmp < 0) {
            hi = mid;
        } else {
            return mid;
        }
    }
    return SLIP39_INVALID_MNEMONIC_INDEX;
}

int32_t slip39_words_for_data(const uint8_t *buffer, uint32_t size, uint16_t *words, uint32_t max)
{
    // The bottom bit of the last byte should always line up with
    // the bottom bit of the last word.

    // calculate the padding bits to add to the first byte to get
    // the last byte and the last word bottom bits to align
    //
    // bytes  5       4       3       2       1       0
    //        |...,...|...,...|...,...|...,...|...,...+
    //        X         X         X         X         *
    // words  4         3         2         1         0
    //
    // Looks like the number of zero bit padding to add
    // is 2x the remainder when your divide the number of
    // bytes by 5.

    uint32_t byte = 0;
    uint32_t word = 0;

    uint8_t bits = (size % 5) * 2; // padded so that bottom bits align

    uint16_t i = 0;

    if (max < slip39_word_count_for_bytes(size)) {
        printf("Not enough space to encode into 10-bit words \n");
        return -1;
    }

    while (byte < size && word < max) {
        while (bits < 10) {
            i =  i << 8;
            bits += 8;
            if (byte < size) {
                i = i | buffer[byte++];
            }
        }

        words[word++] = (i >> (bits - 10));
        i = i & ((1 << (bits - 10)) - 1);
        bits -= 10;
    }

    return word;
}

int32_t slip39_data_for_words(
    const uint16_t *words, // words to decode
    uint8_t wordsize,       // number of words to decode
    uint8_t *buffer,          // space for result
    size_t size            // total space available
)
{
    // The bottom bit of the last byte will always show up in
    // the bottom bit of the last word.

    // calculate the padding bits to add to the first byte to get
    // the last byte and the last word bottom bits to align
    //
    // bytes  5       4       3       2       1       0
    //        |...,...|...,...|...,...|...,...|...,...+
    //        X         X         X         X         *
    // words  4         3         2         1         0
    //

    uint32_t word = 0;
    int16_t bits = -2 * (wordsize % 4);

    // A negative number indicates a number of padding bits. Those bits
    // must be zero.
    if (bits < 0 && (words[0] & (1023 << (10 + bits)))) {
        return SLIP39_INVALID_PADDING;
    }

    // If the number of words is an odd multiple of 5, and the top
    // byte is all zeros, we should probably discard it to get a
    // resulting buffer that is an even number of bytes

    uint8_t discard_top_zeros = (wordsize % 4 == 0) && (wordsize & 4);
    uint32_t byte = 0;
    uint16_t i = 0;

    if (size < slip39_byte_count_for_words(wordsize)) {
        return SLIP39_INSUFFICIENT_SPACE;
    }

    while (word < wordsize && byte < size) {
        i = (i << 10) | words[word++];
        bits += 10;

        if (discard_top_zeros && (i & 1020) == 0) {
            discard_top_zeros = 0;
            bits -= 8;
        }

        while (bits >= 8 && byte < size) {
            buffer[byte++] = (i >> (bits - 8));
            i = i & ((1 << (bits - 8)) - 1);
            bits -= 8;
        }
    }

    return byte;
}

const char *slip39_string_for_word(int16_t word)
{
    if (word < 1024) {
        return slip39_wordlists[word];
    }

    return "";
}

char* slip39_strings_for_words(
    const uint16_t* words,
    size_t words_len
)
{
    if (words_len == 0) {
        char* result = SRAM_MALLOC(1);
        result[0] = '\0';
        return result;
    }

    size_t result_len = words_len; // space characters + nul
    const char* strings[words_len];
    for (int i = 0; i < words_len; i++) {
        strings[i] = slip39_string_for_word(words[i]);
        result_len += strlen(strings[i]);
    }
    char* result_string = SRAM_MALLOC(result_len);
    result_string[0] = '\0';

    for (int i = 0; i < words_len; i++) {
        strcat(result_string, strings[i]);
        if (i != words_len - 1) {
            strcat(result_string, " ");
        }
    }

    return result_string;
}

uint32_t slip39_words_for_strings(
    const char *words_string,
    uint16_t *words,
    uint32_t words_length
)
{
    char buf[16];
    uint8_t i = 0;
    uint32_t j = 0;

    const char *p = words_string;

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

        if (j < words_length) {
            int16_t w = slip39_word_for_string(buf);
            if (w < 0) {
                printf("%s is not valid.\n", buf);
                return SLIP39_INVALID_MNEMONIC_INDEX;
            } else {
                words[j] = w;
            }
        }
        j++;

        while (*p && (*p < 'a' || *p > 'z')) {
            p++;
        }
    }

    return j;
}

int FormatShareMnemonic(Slip39Shared_t *shared, uint16_t *destination, uint32_t destination_length)
{
    uint16_t gt = (shared->groupThreshold - 1) & 0xF;
    uint16_t gc = (shared->groupCount - 1) & 0xF;
    uint16_t mi = (shared->memberIndex) & 0xF;
    uint16_t mt = (shared->memberThreshold - 1) & 0xF;

    destination[0] = (shared->identifier >> 5) & 0x3FF; // ID high 10bit
    destination[1] = ((shared->identifier & 0x1F) << 5) | (shared->iteration & 0x1F); // ID low 5bit | ie 5bit
    destination[2] = ((shared->groupIndex & 0xF) << 6) | ((gt & 0xF) << 2) | ((gc & 0xF) >> 2);
    destination[3] = ((gc & 0xF) << 8) | ((mi & 0xF) << 4) | (mt & 0xF);

    uint32_t words = slip39_words_for_data(shared->value, shared->valueLength, destination + 4, destination_length - METADATA_LENGTH_WORDS);
    rs1024_create_checksum(destination, words + METADATA_LENGTH_WORDS);

    return words + METADATA_LENGTH_WORDS;
}

/***********************************************************************
 * name       : interpolate
 * author     : stone wang
 * data       : 2023-02-22 15:35
 * Description: For i such that T − 2 < i ≤ N compute yi = Interpolation(i − 1, {(0, y1), ... , (T − 3, yT−2), (254, D), (255, S)}).
 * param      : num     : total point num
                xi      : point x coordinate
                yl      : length of y coordinate array
                yi      : y value
                n       : point index
                result  : result
 * return     :
***********************************************************************/
int interpolate(const uint8_t* xi, const uint8_t **yi, uint8_t n, uint8_t* result, uint8_t len, uint8_t threshold)
{
    size_t i, j;

#if 0
    for (i = 0; i < threshold; i++) {
        for (j = 0; j < threshold; j++) {
            if ((i != j) && (xi[i] == xi[j]))
                return false;
        }
    }
#endif

    for (i = 0; i < threshold; i++) {
        if (n == xi[i]) {
            memcpy(result, yi[i], len);
            return SLIP39_OK;
        }
    }

    memzero(result, len);
    for (i = 0; i < threshold; i++) {
        uint8_t t;
        uint32_t d;

        d = 0;
        for (j = 0; j < threshold; j++)
            if (i != j) {
                d += LOG_TABLE[n ^ xi[j]];
                d += 255 - LOG_TABLE[xi[i] ^ xi[j]];
                d %= 255;
            }

        for (j = 0; j < len; j++) {
            t = yi[i][j];
            result[j] ^= t ? EXP_TABLE[(LOG_TABLE[t] + d) % 255] : 0;
        }
    }

    return SLIP39_OK;
}

/***********************************************************************
 * name       : SplitSecret
 * author     : stone wang
 * data       : 2023-02-22 14:27
 * Description: If T is 1, then let yi = S for all i, 1 ≤ i ≤ N, and return.
                Let n be the length of S in bytes. Generate R ∈ GF(256)n−4 randomly with uniform distribution and
                let D be the concatenation of the first 4 bytes of HMAC-SHA256(key=R, msg=S) with the n − 4 bytes of R.
                Let y1, ... , yT−2 ∈ GF(256)n be generated randomly, independently with uniform distribution.
 * param      :
 * return     :
***********************************************************************/
int SplitSecret(uint8_t count, uint8_t threshold, uint8_t *enMasterSecret, uint8_t enMasterSecretLen, uint8_t *groupsBuff)
{
    int ret = 0;
    uint8_t xi[threshold];
    uint8_t pool[enMasterSecretLen * threshold];
    uint8_t *tempShare[threshold];
    uint8_t sha256Hash[SHA256_DIGEST_LENGTH];
    // 0 < T <= N <= 16
    if (threshold <= 0 || threshold > count || count > 16) {
        return SLIP39_INVALID_GROUP_THRESHOLD;
    }

    // If T is 1, then let yi = S for all i, 1 ≤ i ≤ N, and return.
    if (count == 1) {
        memcpy(groupsBuff, enMasterSecret, enMasterSecretLen);
        return SLIP39_OK;
    }

    /*
     * Let n be the length of S in bytes. Generate R ∈GF(256)n−4 randomly with uniform distribution and
     * let D be the concatenation of the first 4 bytes of HMAC-SHA256(key=R, msg=S) with the n − 4 bytes of R.
    */
    for (int i = 0; i < threshold - 2; i++) {
        tempShare[i] = (uint8_t *)(pool + (i * enMasterSecretLen));
        xi[i] = i;
        random_buffer(tempShare[i], enMasterSecretLen);
    }
    // DIGEST_INDEX
    xi[threshold - 2] = DIGEST_INDEX;
    tempShare[threshold - 2] = (uint8_t *)(pool + ((threshold - 2) * enMasterSecretLen));
    random_buffer(&tempShare[threshold - 2][4], enMasterSecretLen - 4);
    hmac_sha256(&tempShare[threshold - 2][4], enMasterSecretLen - 4, enMasterSecret, enMasterSecretLen, sha256Hash);
    memcpy(tempShare[threshold - 2], sha256Hash, 4);

    // SECRET_INDEX
    xi[threshold - 1] = SECRET_INDEX;
    tempShare[threshold - 1] = (uint8_t *)(pool + ((threshold - 1) * enMasterSecretLen));
    memcpy(tempShare[threshold - 1], enMasterSecret, enMasterSecretLen);
    for (int i = 0; i < count; i++) {
        ret = interpolate(xi, (const uint8_t **)tempShare, i, groupsBuff + i * enMasterSecretLen, enMasterSecretLen, threshold);
        if (ret != SLIP39_OK) {
            return SLIP39_NOT_ENOUGH_MNEMONIC_WORDS;
        }
    }

    return SLIP39_OK;
}

/***********************************************************************
 * name       : MasterSecretEncrypt
 * author     : stone wang
 * data       : 2023-02-22 11:20
 * Description: L = EMS[:len(EMS)/2]
                R = EMS[len(EMS)/2:]
                for i in [3,2,1,0]:
                    (L, R) = (R, L xor F(i, R))
                MS = R || L
 * param      : enMasterSecret:     encrypted master secret, a string
                identifier:         random identifier
                iterationExponent:  the total number of iterations to be used in PBKDF2
 * return     :
***********************************************************************/
int MasterSecretEncrypt(uint8_t *masterSecret, uint8_t masterSecretLen, uint8_t iterationExponent, uint16_t identifier, uint8_t *passPhrase,
                        uint8_t *enMasterSecret)
{
    if (masterSecret == NULL) {
        return -1;
    }
    uint8_t halfLen = masterSecretLen / 2;
    uint8_t left[halfLen], right[halfLen], rightTemp[halfLen], key[halfLen];
    uint8_t salt[SHAMIR_SALT_HEAD_LEN + halfLen];
    uint8_t passPhraseLen = strlen((const char *)passPhrase) + 1;
    uint8_t pass[passPhraseLen];
    uint32_t iterations;

    // L = EMS[:len(EMS)/2]
    memcpy(left, masterSecret, halfLen);

    // R = EMS[len(EMS)/2:]
    memcpy(right, masterSecret + halfLen, halfLen);

    // get salt
    memcpy(salt, SHAMIR_SALT_HEAD, strlen((const char *)masterSecret));
    salt[6] = identifier >> 8;
    salt[7] = identifier & 0xFF;

    // todo pass
    memset(pass, 0, sizeof(pass));
    // memcpy(pass, passPhraseLen + 1, strlen(passPhrase));

    iterations = PBKDF2_BASE_ITERATION_COUNT << iterationExponent;
    for (int i = 0; i < PBKDF2_ROUND_COUNT; i++) {
        pass[0] = i;
        memcpy(salt + 8, right, halfLen);
        pbkdf2_hmac_sha256_slip39(pass, sizeof(pass), salt, sizeof(salt), iterations, key, sizeof(key));

        // (L, R) = (R, L xor F(i, R))
        for (int j = 0; j < halfLen; j++) {
            rightTemp[j] = left[j] ^ key[j];
        }
        memcpy(left, right, sizeof(left));
        memcpy(right, rightTemp, sizeof(right));
    }
    memcpy(enMasterSecret + halfLen, left, halfLen);
    memcpy(enMasterSecret, right, halfLen);

    return 0;
}


/***********************************************************************
 * name       : GenerateMnemonics
 * author     : stone wang
 * data       : 2023-02-22 14:32
 * Description:
                 0 < T ≤ N ≤ 16
                 The length of S in bits is at least 128 and a multiple of 16.
                 If any of these conditions is not satisfied, then abort.
 * param      :
 * return     :
***********************************************************************/
int GenerateMnemonics(uint8_t *masterSecret, uint8_t masterSecretLen, uint8_t *ems, uint8_t iterationExponent, uint16_t identifier, uint8_t *passPhrase,
                      uint8_t groupCount, uint8_t groupThreshold, GroupDesc_t *groups, uint16_t *sharesBuffer,
                      uint16_t sharesBufferLen)
{
    if (masterSecretLen % 2 == 1) {
        return SLIP39_INVALID_MEMBER_THRESHOLD;
    }

    // 0 < T <= N <= 16
    if (groupThreshold <= 0 || groupThreshold > groupCount || groupCount > 16) {
        return SLIP39_INVALID_GROUP_THRESHOLD;
    }

    // The length of S in bits is at least 128 and a multiple of 16.
    if (masterSecretLen * 8 < 128 || masterSecretLen * 8 > 256) {
        return SLIP39_INVALID_SECRET_LENGTH;
    }

    uint8_t enMasterSecret[masterSecretLen];
    int ret;
    Slip39Shared_t shards[groups[0].count];

    // identifier = identifier & 0x7FFF; // a 15-bit positive integer
    MasterSecretEncrypt(masterSecret, masterSecretLen, iterationExponent, identifier, passPhrase, enMasterSecret);
    memcpy(ems, enMasterSecret, masterSecretLen);

    for (int i = 0; i < groupCount; i++) {
        uint8_t groupsBuff[groups[i].count * masterSecretLen];
        ret = SplitSecret(groups[i].count, groups[i].threshold, enMasterSecret, masterSecretLen, groupsBuff);
        if (ret != SLIP39_OK) {
            return ret;
        }
        for (int j = 0; j < groups[i].count; j++) {
            shards[j].identifier = identifier;
            shards[j].iteration = iterationExponent;
            shards[j].groupIndex = i;
            shards[j].groupThreshold = groupThreshold;
            shards[j].groupCount = groupCount;
            shards[j].memberIndex = j;
            shards[j].memberThreshold = groups[i].threshold;
            shards[j].valueLength = masterSecretLen;
            memset(shards[j].value, 0, 32);
            memcpy(shards[j].value, groupsBuff + j * masterSecretLen, masterSecretLen);
        }
        memset(groupsBuff, 0, sizeof(groupsBuff));
    }
    memset(enMasterSecret, 0, sizeof(enMasterSecret));

    uint16_t *mnemonic = sharesBuffer;
    unsigned int word_count = 0;
    for (int i = 0; i < groups[0].count; i++) {
        int words = FormatShareMnemonic(&shards[i], mnemonic, sharesBufferLen);
        word_count = words;
        sharesBufferLen -= word_count;
        mnemonic += word_count;
    }

    return 0;
}

/***********************************************************************
 * name       : DecodeMnemonics
 * author     : stone wang
 * data       : 2023-02-24 11:20
 * Description:
 * param      :
 * return     :
***********************************************************************/
int DecodeMnemonics(uint16_t *wordsIndex, uint8_t wordsCount, Slip39Shared_t *shard)
{
    /*
     * The checksum of each share MUST be valid. Implementations SHOULD NOT implement correction beyond potentially
     * suggesting to the user where in the mnemonic an error might be found, without suggesting the correction to make5.
     */
    if (!rs1024_verify_checksum(wordsIndex, wordsCount)) {
        return SLIP39_INVALID_MNEMONIC_CHECKSUM;
    }

    shard->identifier = wordsIndex[0] << 5 | (wordsIndex[1] >> 5);
    shard->iteration = wordsIndex[1] & 0x1F;
    shard->groupThreshold = ((wordsIndex[2] >> 2) & 0XF) + 1;
    shard->groupIndex = wordsIndex[2] >> 6;
    shard->groupCount = (((wordsIndex[2] & 0x3) << 8) | ((wordsIndex[3] >> 8) & 0x3)) + 1;
    shard->memberIndex = (wordsIndex[3] >> 4) & 0xF;
    shard->memberThreshold = (wordsIndex[3] & 0xF) + 1;

    if (shard->groupThreshold > shard->groupCount) {
        return SLIP39_INVALID_GROUP_THRESHOLD;
    }

    int32_t len = slip39_data_for_words(wordsIndex + 4, wordsCount - 7, shard->value, 32);
    if (len < 0) {
        return len;
    }
    shard->valueLength = len;

    return shard->valueLength;
}

#define MAX_SAHRE_VALUE_LEN 32
#define DIGEST_LENGTH_BYTES 4           /* The length of the digest of the shared secret in bytes. */
static int _recover_secret(uint8_t t, int sl, uint8_t *gsi, const uint8_t **gs, uint8_t *result)
{
    uint8_t shared_secret[sl];
    uint8_t digest_share[sl];
    uint8_t hash[SHA256_DIGEST_LENGTH];

    if (t == 1) {
        memcpy(result, gs[0], sl);
        return 0;
    }

    if (interpolate(gsi, gs, SECRET_INDEX, shared_secret, sl, t) != SLIP39_OK) {
        return -1;
    }
    if (interpolate(gsi, gs, DIGEST_INDEX, digest_share, sl, t) != SLIP39_OK) {
        return -2;
    }

    hmac_sha256(&digest_share[DIGEST_LENGTH_BYTES], sl - DIGEST_LENGTH_BYTES, shared_secret, sl, hash);
    if (memcmp(digest_share, hash, DIGEST_LENGTH_BYTES) != 0) {
        return -3;
    }

    memcpy(result, shared_secret, sl);

    memzero(shared_secret, sl);
    memzero(digest_share, sl);

    return 0;
}

static int _decrypt(uint8_t *ems, int emsl, uint8_t *ms, int msl,
                    uint8_t *pp, int ppl, uint8_t ie, uint16_t id)
{
    int j, hl = emsl / 2;
    uint8_t l[hl], r[hl];
    uint8_t _r[hl], f[hl];
    int csl = strlen(SHAMIR_SALT_HEAD);
    uint8_t salt[hl + csl + 2];
    uint8_t pass[ppl + 1];
    int i;
    uint32_t it;

    if (msl != emsl)
        return -1;
    if (emsl & 1)
        return -2;

    memcpy(l, ems, hl);
    memcpy(r, ems + hl, hl);

    // salt
    memcpy(salt, SHAMIR_SALT_HEAD, csl);
    salt[csl] = id >> 8;
    salt[csl + 1] = id & 0xff;

    // pass
    memcpy(pass + 1, pp, ppl);

    // iterations
    it = PBKDF2_BASE_ITERATION_COUNT << ie;

    for (i = PBKDF2_ROUND_COUNT - 1; i >= 0; i--) {
        // salt
        memcpy(salt + 8, r, hl);
        // pass
        pass[0] = i;
        // PBKDF2
        pbkdf2_hmac_sha256_slip39(pass, sizeof(pass), salt, sizeof(salt), it, f, sizeof(f));

        for (j = 0; j < hl; j++)
            _r[j] = l[j] ^ f[j];

        memcpy(l, r, hl);
        memcpy(r, _r, hl);
    }

    memcpy(ms, r, hl);
    memcpy(ms + hl, l, hl);

    memzero(pass, sizeof(pass));
    memzero(salt, sizeof(salt));
    memzero(l, sizeof(l));
    memzero(r, sizeof(r));
    memzero(_r, sizeof(_r));

    return 0;
}

extern void TrngGet(void *buf, uint32_t len);
#define SHARE_BUFFER_SIZE               4096
void GetSlip39MnemonicsWords(uint8_t *masterSecret, uint8_t *ems, uint8_t wordCnt, uint8_t memberCnt, uint8_t memberThreshold,
                             char *wordsList[], uint16_t *id, uint8_t *ie)
{
    uint8_t *passPhrase = (uint8_t *)"";
    uint8_t iterationExponent  = 0;
    uint16_t identifier = 0;
    TrngGet(&identifier, 2);
    identifier = identifier & 0x7FFF;
    *ie = iterationExponent;
    *id = identifier;
    uint8_t masterSecretLen = (wordCnt == 20) ? 16 : 32;

    uint8_t groupCnt = 1;
    uint8_t groupThereshold = 1;

    GroupDesc_t groups[] = {
        {memberCnt, memberThreshold, NULL},
    };

    uint16_t shareBufferSize = SHARE_BUFFER_SIZE;
    uint16_t sharesBuff[SHARE_BUFFER_SIZE];

    GenerateMnemonics(masterSecret, masterSecretLen, ems, iterationExponent, identifier, passPhrase,
                      groupCnt, groupThereshold, groups, sharesBuff, shareBufferSize);

    printf("%s %d.\n", __func__, __LINE__);

    for (int i = 0; i < memberCnt; i++) {
        uint16_t* words = sharesBuff + (i * wordCnt);
        printf("i = %d %s %d.\n", i, __func__, __LINE__);
        wordsList[i] = slip39_strings_for_words(words, wordCnt);
    }
}

int Slip39GetSeed(uint8_t *ems, uint8_t *seed, uint8_t emsLen, const char *passphrase, uint8_t ie, uint16_t id)
{
    return _decrypt(ems, emsLen, seed, emsLen, (uint8_t *)passphrase, strlen(passphrase), ie, id);
}

int Sli39GetMasterSecret(uint8_t threshold, uint8_t wordsCount, uint8_t *ems, uint8_t *masterSecret,
                         char *wordsList[], uint16_t *id, uint8_t *ie)
{
    uint16_t wordsIndexBuf[threshold][wordsCount];
    Slip39Shared_t shards[threshold];
    int ret;
    uint8_t groupIndexMember[16];
    uint8_t groupMemberCount = 0;

    // mnemonics to index
    for (int i = 0; i < threshold; i++) {
        slip39_words_for_strings(wordsList[i], wordsIndexBuf[i], wordsCount);
#if 0
        // wrong num of mnemonic
        if (i > 0 && memcmp(wordsIndexBuf[0], wordsIndexBuf[i], SLIP39_NEED_COMPARE_HEAD_LEN)) {
            printf("wordList[%d] Invalid mnemonic checksum.\n", i);
            return SLIP39_INVALID_MNEMONIC_CHECKSUM;
        }
#endif
    }

    // decode mnemonics
    for (int i = 0; i < threshold; i++) {
        ret = DecodeMnemonics(wordsIndexBuf[i], wordsCount, &shards[i]);
        if (ret < 0) {
            printf("word list %d decode error\n", i);
#if 0
            printf("gs[i].id = %#x\n", shards[i].identifier);
            printf("gs[i].ie = %d\n", shards[i].iteration);
            printf("gs[i].gi = %d\n", shards[i].groupIndex);
            printf("gs[i].gt = %d\n", shards[i].groupThreshold);
            printf("gs[i].gc = %d\n", shards[i].groupCount);
            printf("gs[i].mi = %d\n", shards[i].memberIndex);
            printf("gs[i].mt = %d\n", shards[i].memberThreshold);
            printf("gs[i].c = %d\n", 0);
            printf("gs[i].sl = %d\n", shards[i].valueLength);
            for (int j = 0; j < shards[i].valueLength; j++) {
                printf("gs[j] = %#x\n", shards[i].value[j]);
            }
#endif
            return ret;
        }
    }

    uint16_t groupMemberThreshold, mic, gmic = 0;
    uint16_t identifier;
    uint16_t iteration;
    uint16_t groupThreshold, groupIndex, valueLength;
    int j, i, k;
    uint8_t *tempShare[16];
    uint8_t xi[16];
    uint8_t *m_share[16];
    uint8_t m_share_index[16];
    uint8_t gsv[MAX_SAHRE_VALUE_LEN];
    uint8_t *pp = NULL;
    uint8_t ppl = 0;

    identifier = shards[0].identifier;
    *id = identifier;
    iteration = shards[0].iteration;
    *ie = iteration;
    groupThreshold = shards[0].groupThreshold;
    valueLength = shards[0].valueLength;
    memset(groupIndexMember, 0, sizeof(groupIndexMember));

    for (int i = 0; i < threshold; i++) {
        groupIndexMember[shards[i].groupIndex]++;
    }

    for (int i = 0; i < 16; i++) {
        if (groupIndexMember[i] != 0) {
            groupMemberCount++;
        }
    }
    if (groupMemberCount < groupThreshold) {
        return SLIP39_INSUFFICIENT_SPACE;
    }

    for (i = 0; i < threshold; i++) {
        groupIndex = shards[i].groupIndex;
        for (j = 0; j < threshold; j++) {
            if ((i != j) && (groupIndex == shards[j].groupIndex)) {
                if (shards[i].memberThreshold != shards[j].memberThreshold) {
                    ret = -7;
                    goto exit;
                }
            }
        }
    }

    memset(tempShare, 0, sizeof(tempShare));
    memset(xi, 0, sizeof(xi));

    for (i = 0; i < 16; i++) {
        if (groupIndexMember[i] != 0) {
            mic = 0;
            groupMemberThreshold = 0;
            for (j = 0; j < threshold; j++) {
                if (i == shards[j].groupIndex) {
                    for (k = 0; k < threshold; k++) {
                        if ((j != k) && (i == shards[k].groupIndex) && (shards[j].memberIndex == shards[k].memberIndex)) {
                            ret = -8;
                            goto exit;
                        }
                    }
                    groupMemberThreshold = shards[j].memberThreshold;
                    m_share[mic] = shards[j].value;
                    m_share_index[mic] = shards[j].memberIndex;
                    mic++;
                }
            }
            ret = _recover_secret(groupMemberThreshold, valueLength, m_share_index, (const uint8_t **)m_share, gsv);
            if (ret == 0) {
                tempShare[gmic] = (uint8_t *)SRAM_MALLOC(MAX_SAHRE_VALUE_LEN);
                memcpy(tempShare[gmic], gsv, valueLength);

                xi[gmic] = i;
                gmic++;
            } else {
                printf("r = %d\n", ret);
                ret = -9;
                goto exit;
            }
            if (mic < groupMemberThreshold) {
                ret = -10;
                goto exit;
            }
        }
    }

#if 0
    if ((ms == NULL) || (*msl < sl)) {
        ret = -11;
        goto exit;
    }
#endif

    ret = _recover_secret(gmic, valueLength, xi, (const uint8_t **)tempShare, gsv);
    if (ret == 0) {
        memcpy(ems, gsv, valueLength);
        _decrypt(gsv, valueLength, gsv, valueLength, pp, ppl, iteration, identifier);
        memcpy(masterSecret, gsv, valueLength);
//        *msl = sl;
    } else {
        ret = -12;
    }

exit:
    printf("ret = %d\n", ret);

#if 0
    if (gs != NULL)
        free(gs);

    for (i = 0; i < gmic; i++)
        if (g_share[i] != NULL)
            free(g_share[i]);
#endif

    return ret;
}

int Slip39OneSliceCheck(char *wordsList, uint8_t wordCnt, uint16_t id, uint8_t ie, uint8_t *threshold)
{
    uint16_t wordIndexBuf[wordCnt];
    Slip39Shared_t shards;
    char words[wordCnt][10];
    WordsListSlice(wordsList, words, wordCnt);

    for (int i = 0; i < wordCnt; i++) {
        wordIndexBuf[i] = slip39_word_for_string(words[i]);
    }

    shards.identifier = wordIndexBuf[0] << 5 | (wordIndexBuf[1] >> 5);
    shards.iteration = wordIndexBuf[1] & 0x1F;
    if (id != shards.identifier || ie != shards.iteration) {
        return SLIP39_NOT_BELONG_THIS_WALLET;
    }

    int ret = DecodeMnemonics(wordIndexBuf, wordCnt, &shards);
    if (ret < 0) {
        Slip39Error(ret);
        return ret;
    }
    *threshold = shards.memberThreshold;
    return ret;
}

int Slip39CheckFirstWordList(char *wordsList, uint8_t wordCnt, uint8_t *threshold)
{
    uint16_t wordIndexBuf[wordCnt];
    Slip39Shared_t shards;
    char words[wordCnt][10];
    WordsListSlice(wordsList, words, wordCnt);

    for (int i = 0; i < wordCnt; i++) {
        wordIndexBuf[i] = slip39_word_for_string(words[i]);
    }

    int ret = DecodeMnemonics(wordIndexBuf, wordCnt, &shards);
    if (ret < 0) {
        Slip39Error(ret);
        return ret;
    }
    *threshold = shards.memberThreshold;
    return ret;
}

void Slip39Error(int errNum)
{
    const char *str =
        "OK\0" "SLIP39_NOT_ENOUGH_MNEMONIC_WORDS\0" "SLIP39_INVALID_MNEMONIC_CHECKSUM\0" "SLIP39_SECRET_TOO_SHORT\0" "SLIP39_INVALID_GROUP_THRESHOLD\0" "SLIP39_INVALID_SINGLETON_MEMBER\0"
        "SLIP39_INSUFFICIENT_SPACE\0" "SLIP39_INVALID_SECRET_LENGTH\0" "SLIP39_INVALID_PASSPHRASE\0" "SLIP39_INVALID_SHARD_SET\0" "SLIP39_EMPTY_MNEMONIC_SET\0"
        "SLIP39_DUPLICATE_MEMBER_INDEX\0" "SLIP39_NOT_ENOUGH_MEMBER_SHARDS\0" "SLIP39_INVALID_MEMBER_THRESHOLD\0" "SLIP39_INVALID_PADDING\0" "SLIP39_NOT_ENOUGH_GROUPS\0"
        "SLIP39_INVALID_SHARD_BUFFER\0" "SLIP39_INVALID_MNEMONIC_WORD\0";

    for (int i = SLIP39_OK; i != errNum && *str; i--) {
        while (*str++) ;
    }
    printf("errNum = %d %s\n", errNum, str);
}

