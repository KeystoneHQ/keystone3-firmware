#ifndef _SE_MANAGER_H_
#define _SE_MANAGER_H_

#include "stdint.h"
#include "stdbool.h"

#define KEY_PIECE_LEN                               32

//DS28S60 page map
#define PAGE_NUM_PER_ACCOUNT                        12
#define PAGE_INDEX_IV                               0
#define PAGE_INDEX_ENTROPY_OR_TON_ENTROPY_H32       1
#define PAGE_INDEX_SEED_H32                         2
#define PAGE_INDEX_SEED_L32                         3
//page 4/5 reserved
#define PAGE_INDEX_SLIP39_EMS_OR_TON_ENTROPY_L32    4
#define PAGE_INDEX_RESERVED                         5
#define PAGE_INDEX_HMAC                             6
#define PAGE_INDEX_KEY_PIECE                        7
#define PAGE_INDEX_PASSWORD_HASH                    8
#define PAGE_INDEX_PARAM                            9
#define PAGE_INDEX_MULTISIG_CONFIG_HASH             10
//page 76~85 encrypted password
#define PAGE_PF_ENCRYPTED_PASSWORD                  72
#define PAGE_PF_AES_KEY                             82
#define PAGE_PF_RESET_KEY                           83
#define PAGE_PF_INFO                                84
#define PAGE_WALLET1_PUB_KEY_HASH                   85
#define PAGE_WALLET2_PUB_KEY_HASH                   86
#define PAGE_WALLET3_PUB_KEY_HASH                   87

#define PAGE_PUBLIC_INFO                        88

typedef struct {
    uint8_t auth;
    uint8_t rollKdf;
    uint8_t hostKdf;
} AccountSlot_t;

int32_t GetKeyPieceFromSE(uint8_t accountIndex, uint8_t *piece, const char *password);
int32_t SetNewKeyPieceToSE(uint8_t accountIndex, uint8_t *piece, const char *password);
int32_t SetFpEncryptedPassword(uint32_t index, const uint8_t *encryptedPassword);
int32_t SetFpStateInfo(uint8_t *info);
int32_t GetFpStateInfo(uint8_t *info);
int32_t GetFpEncryptedPassword(uint32_t index, uint8_t *encryptedPassword);
int32_t SetFpCommAesKey(const uint8_t *aesKey);
int32_t GetFpCommAesKey(uint8_t *aesKey);
int32_t SetFpResetKey(const uint8_t *resetKey);
int32_t GetFpResetKey(uint8_t *resetKey);
bool FpAesKeyExist();
bool FpResetKeyExist();
void GetAccountSlot(AccountSlot_t *accountSlot, uint8_t accountIndex);
int32_t SignMessageWithDeviceKey(uint8_t *messageHash, uint8_t *signaure);
int32_t GetDevicePublicKey(uint8_t *pubkey);
int32_t SetWalletDataHash(uint8_t index, uint8_t *info);
bool VerifyWalletDataHash(uint8_t index, uint8_t *info);
int32_t SetMultisigDataHash(uint8_t index, uint8_t *info);
bool VerifyMultisigWalletDataHash(uint8_t index, uint8_t *info);

#endif