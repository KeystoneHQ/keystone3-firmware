#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "se_manager.h"
#include "se_interface.h"
#include "se_account_backend.h"
#include "account_manager.h"
#include "user_utils.h"
#include "user_memory.h"
#include "assert.h"
#include "sha256.h"
#include "hmac.h"
#include "err_code.h"
#include "drv_trng.h"
#include "drv_atecc608b.h"
#include "cryptoauthlib.h"
#include "log_print.h"
#include "hash_and_salt.h"
#include "secret_cache.h"           // PASSWORD_MAX_LEN

// gen-2 SE account backend — isolated from se_manager.c.
// Selected by SeBackend() (se_manager.c) only when GetSeGen() == SE_GEN_2.

// ============================================================================
// attempt-Limit (match_count) helpers.
// ============================================================================
#define SE_MATCH_COUNT_N    192
#define SE_MATCH_COUNT_R    1

// Round up to a multiple of 32.
static uint32_t AlignUp32(uint32_t v)
{
    return (v + 31u) & ~31u;
}

int32_t SE_GetCounter(uint32_t *counter)
{
    return atcab_counter_read(0, counter);     // Counter0
}

int32_t SE_GetMatchCount(uint32_t *matchCount)
{
    uint8_t s8[32];
    int32_t ret = atcab_read_zone(ATCA_ZONE_DATA, SLOT_MATCH_COUNT, 0, 0, s8, sizeof(s8));
    if (ret == ATCA_SUCCESS) {
        *matchCount = (uint32_t)s8[0] | ((uint32_t)s8[1] << 8) |
                      ((uint32_t)s8[2] << 16) | ((uint32_t)s8[3] << 24);
    }
    CLEAR_ARRAY(s8);
    return ret;
}

// Encrypted-write the threshold to slot 8 (WriteKey = slot 13). The 32-bit value is stored in bytes 0-3
// and duplicated in bytes 4-7 as the slot's write format requires.
int32_t SE_SetMatchCount(uint32_t matchCount, const uint8_t *R)
{
    uint8_t data[32];
    uint8_t numIn[NONCE_NUMIN_SIZE];
    memset(data, 0, sizeof(data));
    for (int i = 0; i < 4; i++) {
        data[i] = (uint8_t)(matchCount >> (8 * i));
        data[4 + i] = data[i];
    }
    TrngGet(numIn, NONCE_NUMIN_SIZE);                                              // host nonce for the encrypted write
    int32_t ret = atcab_write_enc(SLOT_MATCH_COUNT, 0, data, R, SLOT_RESET_KEY, numIn);
    CLEAR_ARRAY(data);
    CLEAR_ARRAY(numIn);
    return ret;
}

// Re-arm the attempt Limit after a successful derivation. R must be the recovered plaintext reset key.
int32_t SE_RearmMatchCount(const uint8_t *R)
{
    uint32_t c = 0;
    int32_t ret = SE_GetCounter(&c);
    if (ret != ATCA_SUCCESS) {
        return ret;
    }
    uint32_t B = AlignUp32(c + SE_MATCH_COUNT_R);
#ifndef BUILD_PRODUCTION
    // [SEARM] DEBUG: dump Counter0 vs match_count right before the match_count write. If Counter0 has caught
    // up to match_count(cur), the SE refuses the write with ATCA_EXECUTION_ERROR (0xF4).
    uint32_t curMatch = 0;
    (void)SE_GetMatchCount(&curMatch);                  // best-effort read for the log
    printf("[SEARM] Counter0=%u match_count(cur)=%u match_count(new)=%u\r\n",
           (unsigned)c, (unsigned)curMatch, (unsigned)(B + SE_MATCH_COUNT_N));
#endif
    return SE_SetMatchCount(B + SE_MATCH_COUNT_N, R);   // write the new threshold
}

// ============================================================================
// derivation helpers.
// ============================================================================
// Transient KDF message derived from the PIN. Never stored.
static void Gen2PinMsg(const char *password, uint8_t out[32])
{
    HashWithSalt(out, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "se608bg2-pin-msg");
}

// Derive K608, the per-account key used to mask R_wrapped on DS28S60.
static int32_t Gen2DeriveK608(const uint8_t piece608[32], uint8_t k608[32])
{
    uint8_t info[8];
    memcpy(info, "R-wrap", 6);
    info[6] = 0x01;
    return (hmac_sha256(piece608, 32, info, 7, k608) == 0) ? SUCCESS_CODE : ERR_GENERAL_FAIL;
}

// K608 from the most recent successful gen-2 derive, held for on_unlock_success. The all-zero state is the
// "not set" sentinel (a real key is effectively always non-zero), so there is no separate valid flag. Never
// touch the buffer directly — go through the Gen2K608* accessors so set / clear / validity stay in one place.
static uint8_t g_gen2K608[32] = {0};

static void Gen2K608Set(const uint8_t k608[32])
{
    memcpy(g_gen2K608, k608, 32);
}

static void Gen2K608Clear(void)
{
    CLEAR_ARRAY(g_gen2K608);
}

// Copy the stashed K608 into out. Returns ERR_GENERAL_FAIL if it is unset (all-zero). Caller must
// CLEAR_ARRAY(out) once done.
static int32_t Gen2K608Get(uint8_t out[32])
{
    uint8_t acc = 0;
    for (int i = 0; i < 32; i++) {
        acc |= g_gen2K608[i];
    }
    if (acc == 0) {
        return ERR_GENERAL_FAIL;
    }
    memcpy(out, g_gen2K608, 32);
    return SUCCESS_CODE;
}

// R is never held in a persistent global: recover it into a caller-owned local, use it, CLEAR_ARRAY it on
// the same call. K608 is the only gen-2 secret that must span two vtable calls (derive_608 ->
// on_unlock_success), so it alone stays a static (cleared defensively).

// Recover R from this account's page 8 (PAGE_INDEX_R_WRAPPED), unmasking it with the stashed K608. Requires
// a K608 stashed by a prior Gen2Derive608. Caller must CLEAR_ARRAY(R) once done.
static int32_t Gen2RecoverR(uint8_t accountIndex, uint8_t R[32])
{
    uint8_t wrapped[32], k608[32];
    int32_t ret;

    ret = Gen2K608Get(k608);                                 // fails if no K608 stashed (wrong order / not derived)
    if (ret != SUCCESS_CODE) {
        return ret;
    }
    ret = SE_HmacEncryptRead(wrapped, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_R_WRAPPED);
    if (ret == SUCCESS_CODE) {
        // Fail closed if page 8 is blank/zeroed: an all-zero read means this account holds no live R
        // (destroyed or never provisioned), so recovery fails here instead of returning garbage. Notably
        // guards the forget-pass case where CreateNewAccount runs DestroyAccount(A) — zeroing A's page 8 —
        // BEFORE recovery, so recovering from A fails cleanly here instead of pushing garbage on-chip.
        if (!CheckEntropy(wrapped, 32)) {
            ret = ERR_GENERAL_FAIL;
        } else {
            for (int i = 0; i < 32; i++) {
                R[i] = wrapped[i] ^ k608[i];
            }
        }
    }
    CLEAR_ARRAY(wrapped);
    CLEAR_ARRAY(k608);
    return ret;
}

// Wrap R under K608 and store it on the account's page 8. Inverse of Gen2RecoverR. Used by provisioning
// (and later change-PIN) to (re-)write R_wrapped under a given K608.
static int32_t Gen2StoreRWrapped(uint8_t accountIndex, const uint8_t R[32], const uint8_t k608[32])
{
    uint8_t wrapped[32];
    int32_t ret;
    for (int i = 0; i < 32; i++) {
        wrapped[i] = R[i] ^ k608[i];
    }
    ret = SE_HmacEncryptWrite(wrapped, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_R_WRAPPED);
    CLEAR_ARRAY(wrapped);
    return ret;
}

// ============================================================================
// gen-2 vtable. Change-PIN has no vtable method — it reuses provision_608 (keystore ChangePassword);
// boot_migrate is a no-op.
// ============================================================================
static int32_t Gen2Derive608(uint8_t accountIndex, const char *password, uint8_t piece608[32])
{
    uint8_t inData[32], x[32], k608[32];
    AccountSlot_t accountSlot;
    int32_t ret;

    ASSERT(accountIndex <= 2);
    GetAccountSlot(&accountSlot, accountIndex);                  // per-account ROLL_KDF slot
    Gen2K608Clear();
    do {
        Gen2PinMsg(password, inData);                            // transient PIN message
        ret = Atecc608bKdfNoAuth(accountSlot.rollKdf, inData, 32, x);   // first KDF over the PIN message
        CHECK_ERRCODE_BREAK("kdf kdf_i", ret);
        ret = Atecc608bKdfNoAuth(SLOT_RESET_KEY, x, 32, inData);       // second KDF using slot 13 (in/out must not alias)
        CHECK_ERRCODE_BREAK("kdf R", ret);
        sha256((struct sha256 *)piece608, inData, 32);              // piece608 = sha256 of the KDF output
        ret = Gen2DeriveK608(piece608, k608);                  // stash K608 for on_unlock_success
        CHECK_ERRCODE_BREAK("k608", ret);
        Gen2K608Set(k608);
    } while (0);
    CLEAR_ARRAY(inData);
    CLEAR_ARRAY(x);
    CLEAR_ARRAY(k608);
    return ret;
}
// gen-2 account-existence count. Unlike the shared GetExistAccountNum (which keys only off the IV page),
// an account counts as valid only when ALL THREE of its signals are present:
//   1. the lifecycle status page == CREATED, AND
//   2. a valid IV page        (seed blob present), AND
//   3. a valid R_wrapped page (page 8).
// Any account missing one of the three (interrupted create, half-erased, status not yet committed) is not
// counted. Used only to gate fresh-device provisioning in Gen2Provision608.
static int32_t Gen2GetExistAccountNum(uint8_t *accountNum)
{
    int32_t ret = SUCCESS_CODE;
    uint8_t iv[32], rWrapped[32], count = 0;
    AccountStatus_t status;

    for (uint8_t i = 0; i < 3; i++) {
        ret = SE_GetAccountStatus(i, &status);
        CHECK_ERRCODE_BREAK("read status", ret);
        if (status != ACCOUNT_STATUS_CREATED) {              // 1. status must be CREATED
            continue;
        }
        ret = SE_HmacEncryptRead(iv, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV);
        CHECK_ERRCODE_BREAK("read iv", ret);
        ret = SE_HmacEncryptRead(rWrapped, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_R_WRAPPED);
        CHECK_ERRCODE_BREAK("read R_wrapped", ret);
        if (CheckEntropy(iv, 32) && CheckEntropy(rWrapped, 32)) {   // 2. iv AND 3. R_wrapped both present
            count++;
        }
    }
    CLEAR_ARRAY(iv);
    CLEAR_ARRAY(rWrapped);
    if (ret == SUCCESS_CODE) {
        *accountNum = count;
    }
    return ret;
}
// gen-2 seed-state consistency check. On a gen-2 device every committed wallet has status CREATED and every
// in-progress one has CREATING/CHANGING_PIN/DELETING, so the ONE inconsistent combination is: a seed is
// actually present (IV AND R_wrapped both have entropy) while the lifecycle status page reads UNKNOWN — a
// desynced / corrupted status page, which Gen2GetExistAccountNum does not count. Sets *inconsistent = true
// if any of the 3 accounts is in that state. An in-progress account (e.g. the forgotten wallet during a
// single-wallet forget-pass re-save) is CREATING, not UNKNOWN, so it is never flagged here.
static int32_t Gen2HasInconsistentSeed(bool *inconsistent)
{
    int32_t ret = SUCCESS_CODE;
    uint8_t iv[32], rWrapped[32];
    AccountStatus_t status;

    *inconsistent = false;
    for (uint8_t i = 0; i < 3; i++) {
        ret = SE_GetAccountStatus(i, &status);
        CHECK_ERRCODE_BREAK("read status", ret);
        if (status != ACCOUNT_STATUS_UNKNOWN) {              // only UNKNOWN-with-seed is the invalid case
            continue;
        }
        ret = SE_HmacEncryptRead(iv, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_IV);
        CHECK_ERRCODE_BREAK("read iv", ret);
        ret = SE_HmacEncryptRead(rWrapped, i * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_R_WRAPPED);
        CHECK_ERRCODE_BREAK("read R_wrapped", ret);
        if (CheckEntropy(iv, 32) && CheckEntropy(rWrapped, 32)) {   // seed present but no status -> inconsistent
            *inconsistent = true;
            printf("account %d: seed present (iv+R_wrapped) but status UNKNOWN -> inconsistent, refusing R-birth\n", i);
            break;
        }
    }
    CLEAR_ARRAY(iv);
    CLEAR_ARRAY(rWrapped);
    return ret;
}
// Create (provision) a gen-2 account. Returns piece608 (mixed into the seed by the caller). Steps:
//   1. Establish R. `R` is supplied by the caller (add-wallet: recovered from an existing wallet).
//      `R == NULL` means a new device -> generate a fresh R and write slot 13, but only when no other wallet
//      exists; if other wallets exist we refuse rather than write a new R. When R is supplied we never
//      rewrite slot 13.
//   2. Birth this account's kdf_i (per-account ROLL_KDF slot; no-auth DeriveKey).
//   3. Derive piece608 the same way login will, which also stashes K608.
//   4. Store R_wrapped for this account under that K608.
//   5. Arm the attempt Limit (idempotent — converts the factory bootstrap value to the runtime window).
// R is local to this frame (no persistent session copy); the post-create login-derive re-recovers it.
static int32_t Gen2Provision608(uint8_t accountIndex, const char *password, uint8_t piece608[32], const uint8_t *R)
{
    uint8_t rKey[32], k608[32];
    AccountSlot_t accountSlot;
    uint8_t otherAccounts = 0;
    int32_t ret;

    ASSERT(accountIndex <= 2);
    GetAccountSlot(&accountSlot, accountIndex);
    do {
        // 1. Establish R. The caller supplies R when it already lives on-chip (add-wallet: recovered from an
        //    existing wallet). R == NULL means "new device" -> generate a fresh R and write slot 13, but only
        //    when no other wallet exists.
        if (R != NULL) {
            memcpy(rKey, R, 32);                       // add-wallet: reuse the live slot-13 R; do NOT rewrite slot 13
        } else {
            ret = Gen2GetExistAccountNum(&otherAccounts);
            CHECK_ERRCODE_BREAK("exist account num", ret);
            // A seed present with status UNKNOWN (a desynced status page) is NOT counted above
            // (status != CREATED). Refuse provisioning if any such inconsistent seed exists. The forgotten
            // wallet in a single-wallet forget-pass re-save is CREATING (not UNKNOWN), so it is never flagged
            // -> that flow still proceeds.
            bool inconsistentSeed = false;
            ret = Gen2HasInconsistentSeed(&inconsistentSeed);
            CHECK_ERRCODE_BREAK("seed consistency", ret);
            if (otherAccounts != 0 || inconsistentSeed) {
                ret = ERR_GENERAL_FAIL;                // R already on-chip / inconsistent seed; refuse
                break;
            }
            SE_GetTRng(rKey, 32);                      // fresh R from TRNG
            ret = SE_EncryptWrite(SLOT_RESET_KEY, 0, rKey);   // slot 13, encrypted write via slot 2
            CHECK_ERRCODE_BREAK("write R", ret);
        }
        // 2. Birth this account's kdf_i (no-auth DeriveKey on the ROLL_KDF slot).
        ret = Atecc608bDeriveKeyNoAuth(accountSlot.rollKdf);
        CHECK_ERRCODE_BREAK("derive kdf_i", ret);
        // 3. Derive piece608 and stash K608.
        ret = Gen2Derive608(accountIndex, password, piece608);
        CHECK_ERRCODE_BREAK("derive piece608", ret);
        ret = Gen2K608Get(k608);
        CHECK_ERRCODE_BREAK("k608", ret);
        // 4. Wrap + store R for this account.
        ret = Gen2StoreRWrapped(accountIndex, rKey, k608);
        CHECK_ERRCODE_BREAK("store R_wrapped", ret);
        // 5. Arm the attempt Limit.
        ret = SE_RearmMatchCount(rKey);
        CHECK_ERRCODE_BREAK("arm match_count", ret);
    } while (0);
    Gen2K608Clear();                       // K608 is single-use
    CLEAR_ARRAY(rKey);
    CLEAR_ARRAY(k608);
    return ret;
}
// Called once after a verified unlock (correct PIN). Recovers R from this account's R_wrapped using the
// K608 just stashed by Gen2Derive608, then re-arms the attempt Limit. R lives only on this stack frame;
// K608 is consumed (one-shot) here.
static int32_t Gen2OnUnlockSuccess(uint8_t accountIndex)
{
    uint8_t R[32];
    int32_t ret;

    ASSERT(accountIndex <= 2);
    do {
        ret = Gen2RecoverR(accountIndex, R);                // recover R from page 8
        CHECK_ERRCODE_BREAK("recover R", ret);
        ret = SE_RearmMatchCount(R);                        // re-arm the attempt Limit
        CHECK_ERRCODE_BREAK("re-arm match_count", ret);
    } while (0);
    // K608 is single-use; drop it whether or not re-arm succeeded.
    Gen2K608Clear();
    CLEAR_ARRAY(R);
    return ret;
}
// Recover R from an existing wallet so the add-wallet path can provision a new wallet with it (instead of
// generating a new R). Re-derives the existing wallet's K608 from its passcode, then recovers R from
// R_wrapped(existingIdx). Used just-in-time by SetNewKeyPieceToSE; the existing wallet was already
// authenticated at the Add-Wallet verify (its passcode preserved in the cache).
static int32_t Gen2RecoverResetKey(uint8_t existingIdx, const char *existingPassword, uint8_t *R_out)
{
    uint8_t piece[32];
    int32_t ret;

    ASSERT(existingIdx <= 2);
    if (existingPassword == NULL || strnlen_s(existingPassword, PASSWORD_MAX_LEN) == 0) {
        return ERR_GENERAL_FAIL;   // no existing-wallet passcode -> cannot recover; caller fails closed
    }
    do {
        ret = Gen2Derive608(existingIdx, existingPassword, piece);   // stash existing K608 (piece unused here)
        CHECK_ERRCODE_BREAK("recover-key derive", ret);
        ret = Gen2RecoverR(existingIdx, R_out);                      // recover R from R_wrapped(existingIdx)
        CHECK_ERRCODE_BREAK("recover-key R", ret);
    } while (0);
    Gen2K608Clear();               // existing K608 consumed; provision will re-stash the new wallet's K608
    CLEAR_ARRAY(piece);
    return ret;
}
static int32_t Gen2PrepareChangePin(uint8_t accountIndex, const char *oldPassword)
{
    SE_ArmProvisionRecovery(accountIndex, oldPassword);   // PIN_old: recover R just-in-time during re-provision
    return SUCCESS_CODE;
}
// Per-wallet cryptographic erase (forget one wallet, keep the others). The caller (DestroyAccount) has
// already zeroed this account's DS28S60 pages (seed blob + R_wrapped). Here we additionally roll the
// account's kdf_i via DeriveKey(Roll) (no PIN) so its piece608 can never be reproduced. R and the other
// wallets are untouched.
static int32_t Gen2EraseAccount(uint8_t accountIndex)
{
    AccountSlot_t accountSlot;
    ASSERT(accountIndex <= 2);
    GetAccountSlot(&accountSlot, accountIndex);
    return Atecc608bDeriveKeyNoAuth(accountSlot.rollKdf);   // roll kdf_i -> this wallet's piece608 dies
}
// Full wipe: overwrite R (slot 13) with fresh TRNG, making all seed blobs undecryptable at once. The R write
// is WriteConfig=Encrypt via slot 2. The caller (WipeDevice) also zeros the DS28S60 blobs + erases flash;
// this is the SE-side cryptographic kill.
static int32_t Gen2WipeAll(void)
{
    uint8_t newR[32];
    int32_t ret;
    SE_GetTRng(newR, 32);
    ret = SE_EncryptWrite(SLOT_RESET_KEY, 0, newR);   // slot 13, encrypted write via slot 2
    CLEAR_ARRAY(newR);
    return ret;
}
static int32_t Gen2BootMigrate(void)
{
    return SUCCESS_CODE;           // no gen-2 boot work yet
}
// Wipe transient gen-2 session secrets. R is never persisted (it lives only on the stack of the call that
// uses it), so the only thing to clear is K608 — which spans derive_608 -> on_unlock_success and would
// otherwise linger if an error aborts the unlock between those two calls.
static void Gen2ClearSession(void)
{
    Gen2K608Clear();
}

const SeAccountBackend g_seBackendGen2 = {
    .derive_608        = Gen2Derive608,
    .provision_608     = Gen2Provision608,
    .recover_reset_key = Gen2RecoverResetKey,
    .on_unlock_success = Gen2OnUnlockSuccess,
    .prepare_change_pin = Gen2PrepareChangePin,
    .erase_account     = Gen2EraseAccount,
    .wipe_all          = Gen2WipeAll,
    .boot_migrate      = Gen2BootMigrate,
    .clear_session     = Gen2ClearSession,
};
