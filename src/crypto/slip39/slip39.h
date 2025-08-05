#ifndef _SLIP39_H
#define _SLIP39_H

#include <stdbool.h>
#include <stdint.h>

#define SLIP39_OK                               (0)
#define SLIP39_NOT_ENOUGH_MNEMONIC_WORDS        (-1)
#define SLIP39_INVALID_MNEMONIC_CHECKSUM        (-2)
#define SLIP39_SECRET_TOO_SHORT                 (-3)
#define SLIP39_INVALID_GROUP_THRESHOLD          (-4)
#define SLIP39_INVALID_SINGLETON_MEMBER         (-5)
#define SLIP39_INSUFFICIENT_SPACE               (-6)
#define SLIP39_INVALID_SECRET_LENGTH            (-7)
#define SLIP39_INVALID_PASSPHRASE               (-8)
#define SLIP39_INVALID_SHARD_SET                (-9)
#define SLIP39_EMPTY_MNEMONIC_SET               (-10)
#define SLIP39_DUPLICATE_MEMBER_INDEX           (-11)
#define SLIP39_NOT_ENOUGH_MEMBER_SHARDS         (-12)
#define SLIP39_INVALID_MEMBER_THRESHOLD         (-13)
#define SLIP39_INVALID_PADDING                  (-14)
#define SLIP39_NOT_ENOUGH_GROUPS                (-15)
#define SLIP39_INVALID_SHARD_BUFFER             (-16)
#define SLIP39_INVALID_MNEMONIC_WORD            (-17)
#define SLIP39_NOT_BELONG_THIS_WALLET           (-18)

#define DIGEST_INDEX                            (254)
#define SECRET_INDEX                            (255)

#define SLIP39_DEFAULT_MEMBER_COUNT             (5)
#define SLIP39_DEFAULT_MEMBER_THRESHOLD         (3)
#define SLIP39_MNEMONIC_WORDS_MAX               (33)

#define SLIP39_INVALID_MNEMONIC_INDEX           (~0)
#define PBKDF2_BASE_ITERATION_COUNT             (2500)
#define PBKDF2_ROUND_COUNT                      (4)
#define SHAMIR_SALT_HEAD_LEN                    (8)
#define SHAMIR_MAX_LEN                          (32)
#define RADIX_BITS                              (10)
#define METADATA_LENGTH_WORDS                   (7)
#define RADIX_BITS                              (10)
#define SHAMIR_SALT_HEAD                        "shamir"

#define SLIP39_NEED_COMPARE_HEAD_LEN            (3 * sizeof(uint16_t))

int Slip39OneSliceCheck(char *wordsList, uint8_t wordCnt, uint16_t id, uint8_t eb, uint8_t ie, uint8_t *threshold);
int Slip39CheckFirstWordList(char *wordsList, uint8_t wordCnt, uint8_t *threshold);
void GetSlip39MnemonicsWords(uint8_t *masterSecret, uint8_t *ems, uint8_t wordCnt, uint8_t memberCnt, uint8_t memberThreshold,
                             char *wordsList[], uint16_t *id, bool *eb, uint8_t *ie);
int Slip39GetMasterSecret(uint8_t threshold, uint8_t wordsCount, uint8_t *ems, uint8_t *masterSecret,
                          char *wordsList[], uint16_t *id, uint8_t *eb, uint8_t *ie);
int Slip39GetSeed(uint8_t *ems, uint8_t *seed, uint8_t emsLen, const char *passphrase, uint8_t ie, bool eb, uint16_t id);

#endif /* _SLIP39_H */

