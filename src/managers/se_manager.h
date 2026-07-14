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
// Don't use this page in the future usage, it is only for legacy password hash
#define PAGE_INDEX_LEGACY_PASSWORD_HASH             8
// gen-2: page 8 is repurposed for per-account gen-2 data (same offset; gen-1 never touches it).
#define PAGE_INDEX_R_WRAPPED                        8
#define PAGE_INDEX_PARAM                            9
#define PAGE_INDEX_MULTISIG_CONFIG_HASH             10
// (per-account index 11 is unused)

// Per-account lifecycle status — ABSOLUTE pages (NOT the per-account *PAGE_NUM_PER_ACCOUNT scheme), placed
// in the free 36~71 gap. Kept OUTSIDE each account's 0~11 data block so the status survives the
// DestroyAccount / AccountsDataCheck zero-loops (those loops stay unchanged). One page per account:
//   account i status page = PAGE_ACCOUNT_STATUS_BASE + i   (i = 0..2).
#define PAGE_ACCOUNT_STATUS_BASE                    36   // account 0 status; account i = BASE + i
#define PAGE_ACCOUNT_STATUS_0                        36
#define PAGE_ACCOUNT_STATUS_1                        37
#define PAGE_ACCOUNT_STATUS_2                        38
// RSA hash page, 39 -> PAGE_WALLET1_RSA_PRIMES_HASH, 40 -> PAGE_WALLET2_RSA_PRIMES_HASH, 41 -> PAGE_WALLET3_RSA_PRIMES_HASH
// clearly defined the 3 pages for RSA hash, so the page 39~41 is not used for other purpose.
// PAGE_WALLET2_RSA_PRIMES_HASH, PAGE_WALLET3_RSA_PRIMES_HASH are not used in the code.
#define PAGE_WALLET1_RSA_PRIMES_HASH                 39
#define PAGE_WALLET2_RSA_PRIMES_HASH                 40
#define PAGE_WALLET3_RSA_PRIMES_HASH                 41

// Status page layout: a 4-byte magic + a 1-byte state (rest zero). The magic lets a blank/legacy/wiped page
// (all-zero, or random) be told apart from a real state — without it we fall back to the coarse CheckEntropy
// boot check, so existing accounts keep working. A bootloader/firmware wipe zeros the page -> magic absent
// -> UNKNOWN. Every mutating op brackets its writes: in-progress state FIRST, terminal state LAST.
#define ACCOUNT_STATUS_MAGIC                        0x4B335354u   // 'K''3''S''T'
typedef enum {
    ACCOUNT_STATUS_UNKNOWN      = 0,   // no valid magic (blank / legacy / wiped) -> caller does coarse check
    ACCOUNT_STATUS_CREATING     = 1,   // provision in progress           -> boot: erase the partial account
    ACCOUNT_STATUS_CREATED      = 2,   // valid, complete account
    ACCOUNT_STATUS_CHANGING_PIN = 3,   // change-PIN re-wrap in progress  -> boot: erase (user restores seed)
    ACCOUNT_STATUS_DELETING     = 4,   // delete in progress              -> boot: erase (finish the deletion)
} AccountStatus_t;

//page 76~85 encrypted password
#define PAGE_PF_ENCRYPTED_PASSWORD                  72
#define PAGE_PF_AES_KEY                             82
#define PAGE_PF_RESET_KEY                           83
#define PAGE_PF_INFO                                84
#define PAGE_WALLET1_PUB_KEY_HASH                   85
#define PAGE_WALLET2_PUB_KEY_HASH                   86
#define PAGE_WALLET3_PUB_KEY_HASH                   87


#define PAGE_PUBLIC_INFO                        88

// SE generation, resolved once at runtime from the locked ATECC608B config.
// A shipped device is pre-provisioned, so a normal boot is SE_GEN_1 or SE_GEN_2;
// UNPROVISIONED / INVALID are treated as errors.
typedef enum {
    SE_GEN_UNKNOWN = 0,      // not yet resolved (initial g_seGen value before SeManagerInit / lazy resolve)
    SE_GEN_1 = 1,            // gen-1: fielded / current-locked config
    SE_GEN_2 = 2,            // gen-2: new production
    SE_GEN_UNPROVISIONED,    // blank / config or data zone not locked
    SE_GEN_INVALID,          // both zones locked but the manifest matches neither generation
} SeGen_t;

typedef struct {
    uint8_t auth;
    uint8_t rollKdf;
    uint8_t hostKdf;
} AccountSlot_t;

int32_t GetKeyPieceFromSE(uint8_t accountIndex, uint8_t *piece, const char *password);
int32_t SetNewKeyPieceToSE(uint8_t accountIndex, uint8_t *piece, const char *password);
// Add-wallet (gen-2): arm at the Add-Wallet passcode verify with the existing wallet's index + passcode,
// captured into a dedicated holder that the next SetNewKeyPieceToSE consumes. Cleared
// when consumed by provision (one-shot), on logout, and on lock-screen turn-on.
void SE_ArmProvisionRecovery(uint8_t existingIndex, const char *existingPassword);
void SE_DisarmProvisionRecovery(void);
// UI operation-boundary alias for SE_DisarmProvisionRecovery() (add-wallet notice / forget-pass abandon).
void AbandonProvisionRecovery(void);
// True while a provision-recovery is armed (pending consume); used to avoid re-enabling page-lock mid-operation.
bool SE_IsProvisionRecoveryArmed(void);
int32_t SetFpEncryptedPassword(uint32_t index, const uint8_t *encryptedPassword);
int32_t SetFpStateInfo(uint8_t *info);
int32_t GetFpStateInfo(uint8_t *info);
int32_t GetFpEncryptedPassword(uint32_t index, uint8_t *encryptedPassword);
int32_t SetFpCommAesKey(const uint8_t *aesKey);
int32_t GetFpCommAesKey(uint8_t *aesKey);
int32_t SetFpResetKey(const uint8_t *resetKey);
int32_t GetFpResetKey(uint8_t *resetKey);
bool FpAesKeyExist();
void GetAccountSlot(AccountSlot_t *accountSlot, uint8_t accountIndex);
void SeManagerInit(void);          // resolve SE generation once at boot (after Atecc608bInit)
SeGen_t GetSeGen(void);            // cached value resolved by SeManagerInit

// gen-2 attempt-limit (match_count) helpers.
int32_t SE_GetCounter(uint32_t *counter);                       // SE monotonic counter
int32_t SE_GetMatchCount(uint32_t *matchCount);                 // reads match_count (slot 8)
int32_t SE_SetMatchCount(uint32_t matchCount, const uint8_t *R);// encrypted-write slot 8
int32_t SE_RearmMatchCount(const uint8_t *R);                   // re-arm match_count; no counter bump
void SE_ClearSessionSecrets(void);                              // wipe SE-side transient session secrets (gen-2)
int32_t SE_OnUnlockSuccess(uint8_t accountIndex);              // post-verify re-arm (gen-2); no-op (gen-1)
int32_t SE_PrepareChangePin(uint8_t accountIndex, const char *oldPassword); // pre-reprovision hook: no-op (gen-1); arm provision recovery (gen-2)
int32_t SE_EraseAccount(uint8_t accountIndex);                 // per-wallet SE erase (gen-2); no-op (gen-1)
int32_t SE_WipeAll(void);                                       // full SE wipe (gen-2); no-op (gen-1)
int32_t SE_BootMigrate(void);                                   // one-time boot: legacy page-8 wipe (gen-1); no-op (gen-2); NULL->skip
// Per-account lifecycle status page (PAGE_ACCOUNT_STATUS_BASE + idx). SetAccountStatus(UNKNOWN) zeros it.
// GetAccountStatus returns UNKNOWN when the magic is absent (blank/legacy) so the caller falls back to the
// coarse boot check. Single-page write/read -> each transition is atomic.
int32_t SE_SetAccountStatus(uint8_t accountIndex, AccountStatus_t state);
int32_t SE_GetAccountStatus(uint8_t accountIndex, AccountStatus_t *state);
int32_t SignMessageWithDeviceKey(uint8_t *messageHash, uint8_t *signaure);
int32_t GetDevicePublicKey(uint8_t *pubkey);
int32_t SetWalletDataHash(uint8_t index, uint8_t *info);
bool VerifyWalletDataHash(uint8_t index, uint8_t *info);
int32_t SetRsaPrimesHash(uint8_t index, uint8_t *info);
bool VerifyRsaPrimesHash(uint8_t index, uint8_t *info);
int32_t SetMultisigDataHash(uint8_t index, uint8_t *info);
bool VerifyMultisigWalletDataHash(uint8_t index, uint8_t *info);

#endif
