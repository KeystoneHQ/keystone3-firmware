#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "se_manager.h"
#include "se_interface.h"
#include "user_utils.h"
#include "user_memory.h"
#include "assert.h"
#include "sha256.h"
#include "err_code.h"
#include "drv_trng.h"
#include "drv_atecc608b.h"
#include "cryptoauthlib.h"
#include "log_print.h"
#include "hash_and_salt.h"
#include "secret_cache.h"
#include "se_account_backend.h"

static int32_t SetNewKeyPieceToDs28s60(uint8_t accountIndex, uint8_t *piece, const char *password);
static int32_t GetKeyPieceFromDs28s60(uint8_t accountIndex, uint8_t *piece, const char *password);

// gen-1-specific 608 derivation lives in se_backend_gen1.c; gen-2 in se_backend_gen2.c. This file keeps
// the generation-agnostic pieces: the shared DS28S60 key piece, GetAccountSlot, the SeBackend() dispatcher
// and SE_* helpers.

static int32_t SetNewKeyPieceToDs28s60(uint8_t accountIndex, uint8_t *piece, const char *password)
{
    uint8_t passwordHash[32], randomKey[32], xData[32];
    int32_t ret;

    ASSERT(accountIndex <= 2);
    HashWithSalt(passwordHash, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "ds28s60 digest");
    TrngGet(randomKey, 32);
    for (uint32_t i = 0; i < 32; i++) {
        xData[i] = passwordHash[i] ^ randomKey[i];
    }
    do {
        ret = SE_HmacEncryptWrite(xData, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_KEY_PIECE);
        CHECK_ERRCODE_BREAK("write xData", ret);
        memcpy(piece, randomKey, 32);
    } while (0);
    CLEAR_ARRAY(passwordHash);
    CLEAR_ARRAY(randomKey);
    CLEAR_ARRAY(xData);

    return ret;
}

static int32_t GetKeyPieceFromDs28s60(uint8_t accountIndex, uint8_t *piece, const char *password)
{
    uint8_t passwordHash[32], xData[32];
    int32_t ret;

    ASSERT(accountIndex <= 2);
    do {
        HashWithSalt(passwordHash, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "ds28s60 digest");
        ret = SE_HmacEncryptRead(xData, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_KEY_PIECE);
        CHECK_ERRCODE_BREAK("read xData", ret);
        for (uint32_t i = 0; i < 32; i++) {
            piece[i] = passwordHash[i] ^ xData[i];
        }
    } while (0);
    CLEAR_ARRAY(passwordHash);
    CLEAR_ARRAY(xData);

    return ret;
}

void GetAccountSlot(AccountSlot_t *accountSlot, uint8_t accountIndex)
{
    ASSERT(accountIndex <= 2);
    switch (accountIndex) {
    case 0: {
        accountSlot->auth = SLOT_AUTH_KEY_1;
        accountSlot->rollKdf = SLOT_ROLL_KDF_KEY_1;
        accountSlot->hostKdf = SLOT_HOST_KDF_KEY_1;
    }
    break;
    case 1: {
        accountSlot->auth = SLOT_AUTH_KEY_2;
        accountSlot->rollKdf = SLOT_ROLL_KDF_KEY_2;
        accountSlot->hostKdf = SLOT_HOST_KDF_KEY_2;
    }
    break;
    case 2: {
        accountSlot->auth = SLOT_AUTH_KEY_3;
        accountSlot->rollKdf = SLOT_ROLL_KDF_KEY_3;
        accountSlot->hostKdf = SLOT_HOST_KDF_KEY_3;
    }
    break;
    default:
        break;
    }
}

int32_t GetKeyPieceFromSE(uint8_t accountIndex, uint8_t *pieces, const char *password)
{
    int32_t ret;
    const SeAccountBackend *be = SeBackend();

    if (be == NULL) {
        return ERR_GENERAL_FAIL;       // fail closed: UNPROVISIONED / INVALID must never derive
    }
    do {
        // 608 piece is generation-specific; DS28S60 piece is shared.
        ret = be->derive_608(accountIndex, password, pieces);
        CHECK_ERRCODE_BREAK("atecc piece", ret);
        // KEYSTORE_PRINT_ARRAY("608 piece", pieces, 32);
        ret = GetKeyPieceFromDs28s60(accountIndex, pieces + KEY_PIECE_LEN, password);
        CHECK_ERRCODE_BREAK("ds28s60 piece", ret);
        // KEYSTORE_PRINT_ARRAY("ds28s60 piece", pieces + KEY_PIECE_LEN, 32);
    } while (0);

    return ret;
}

// Add-wallet provisioning context. Holds the existing wallet's index + passcode so the gen-2 backend can
// reuse that wallet's existing SE key material during SetNewKeyPieceToSE. This is a DEDICATED holder with its
// own lifecycle (NOT the shared secret cache, which the add-wallet navigation clears before provision runs):
//   arm   = Add-Wallet passcode verify (captures idx + passcode while both are valid)
//   clear = consumed by the next provision (one-shot)  OR  logout  OR  lock-screen turn-on.
// So the passcode never outlives the one add-wallet operation / the unlocked session. 0xFF = not armed.
#define PROVISION_RECOVER_IDX_NONE   0xFF
static uint8_t g_provisionRecoverIdx = PROVISION_RECOVER_IDX_NONE;
static char    g_provisionRecoverPassword[PASSWORD_MAX_LEN + 1] = {0};

void SE_ArmProvisionRecovery(uint8_t existingIndex, const char *existingPassword)
{
    g_provisionRecoverIdx = existingIndex;
    memset_s(g_provisionRecoverPassword, sizeof(g_provisionRecoverPassword), 0, sizeof(g_provisionRecoverPassword));
    if (existingPassword != NULL) {
        strcpy_s(g_provisionRecoverPassword, sizeof(g_provisionRecoverPassword), existingPassword);
    }
}

void SE_DisarmProvisionRecovery(void)
{
    g_provisionRecoverIdx = PROVISION_RECOVER_IDX_NONE;
    memset_s(g_provisionRecoverPassword, sizeof(g_provisionRecoverPassword), 0, sizeof(g_provisionRecoverPassword));
}

// True while an add-wallet / forget-pass provision recovery is armed (pending consume). Lets a re-openable
// sub-view avoid clobbering the page-lock hold that the operation ROOT owns for the whole armed interval
// (page-lock is a plain bool, not refcounted). Always false in gen-1 / simulator (never armed).
bool SE_IsProvisionRecoveryArmed(void)
{
    return g_provisionRecoverIdx != PROVISION_RECOVER_IDX_NONE;
}

// Semantic alias for the UI: call this at an add-wallet / forget-pass OPERATION BOUNDARY when the user leaves
// the flow without provisioning (notice-tile teardown, forget-pass DeInit, backing past the proven step). It
// makes the intent explicit at the call site instead of reading like incidental view-teardown cleanup. Must
// only be used at the operation root boundary — never in a re-openable sub-step (e.g. the create-wallet set-pin
// view), or the arm is dropped mid-flow and the eventual provision fails recovery. The consume path
// (SetNewKeyPieceToSE), the change-PIN in-function disarm, and the lock-screen session catch-all keep calling
// SE_DisarmProvisionRecovery() directly — those are not "user abandoned the operation" boundaries.
void AbandonProvisionRecovery(void)
{
    SE_DisarmProvisionRecovery();
}

int32_t SetNewKeyPieceToSE(uint8_t accountIndex, uint8_t *pieces, const char *password)
{
    //TODO: deal with error code
    int32_t ret;
    const SeAccountBackend *be = SeBackend();
    uint8_t rBuf[32];
    const uint8_t *R = NULL;

    if (be == NULL) {
        return ERR_GENERAL_FAIL;       // fail closed
    }
    // Add-wallet: an existing wallet was authenticated at the Add-Wallet verify (its index armed here, its
    // passcode preserved in the secret cache). Recover its existing SE key material so provision REUSES it
    // instead of creating new. First wallet / gen-1 stays armed-NONE -> R stays NULL. If recovery fails
    // (e.g. passcode no longer cached) R stays NULL and the gen-2 backend refuses.
    if (g_provisionRecoverIdx != PROVISION_RECOVER_IDX_NONE) {
        int32_t rr = be->recover_reset_key(g_provisionRecoverIdx, g_provisionRecoverPassword, rBuf);
        if (rr == SUCCESS_CODE) {
            R = rBuf;
        }
        SE_DisarmProvisionRecovery();   // consume (one-shot): clear index + passcode holder
    }
    ret = be->provision_608(accountIndex, password, pieces, R);
    CLEAR_ARRAY(rBuf);
    CHECK_ERRCODE_RETURN_INT(ret);

    // KEYSTORE_PRINT_ARRAY("608 piece", pieces, 32);
    ret = SetNewKeyPieceToDs28s60(accountIndex, pieces + KEY_PIECE_LEN, password);
    CHECK_ERRCODE_RETURN_INT(ret);
    // KEYSTORE_PRINT_ARRAY("ds28s60 piece", pieces + KEY_PIECE_LEN, 32);
    return ret;
}

// Refresh the per-account SE state after a verified unlock. Called by the keystore once the account HMAC has
// confirmed the PIN (gen-2: refreshes SE session state; gen-1: no-op). Fail-closed on NULL backend.
int32_t SE_OnUnlockSuccess(uint8_t accountIndex)
{
    const SeAccountBackend *be = SeBackend();

    if (be == NULL) {
        return ERR_GENERAL_FAIL;
    }
    return be->on_unlock_success(accountIndex);
}

// Prepare the generation-specific SE state for change-PIN after PIN_old has been verified by LoadAccountSecret.
//   gen-1: no-op.   gen-2: arm recovery so SaveAccountSecret's provision_608 reuses the existing SE key material.
int32_t SE_PrepareChangePin(uint8_t accountIndex, const char *oldPassword)
{
    const SeAccountBackend *be = SeBackend();

    if (be == NULL) {
#ifdef COMPILE_SIMULATOR
        return SUCCESS_CODE;
#else
        return ERR_GENERAL_FAIL;
#endif
    }
    return be->prepare_change_pin(accountIndex, oldPassword);
}

// Per-wallet SE-side cryptographic erase, called after DestroyAccount has zeroed the account's pages
// (gen-2: rotates the account's SE key material; gen-1: no-op). Simulator has no SE backend.
int32_t SE_EraseAccount(uint8_t accountIndex)
{
    const SeAccountBackend *be = SeBackend();

    if (be == NULL) {
#ifdef COMPILE_SIMULATOR
        return SUCCESS_CODE;
#else
        return ERR_GENERAL_FAIL;
#endif
    }
    return be->erase_account(accountIndex);
}

// Full SE-side cryptographic wipe, called from WipeDevice (gen-2: overwrites the shared SE key material;
// gen-1: no-op). Fail-closed on NULL backend.
int32_t SE_WipeAll(void)
{
    const SeAccountBackend *be = SeBackend();

    if (be == NULL) {
        return ERR_GENERAL_FAIL;
    }
    return be->wipe_all();
}

// One-time boot migration, dispatched to the generation backend (called from AccountManagerInit).
//   gen-1: wipe the legacy page-8 PIN hash.   gen-2: no-op (page 8 is used differently).
// Unlike the other dispatchers, a NULL backend (UNPROVISIONED/INVALID — should never reach here; SeManagerInit
// asserts gen-1/gen-2 at boot) returns SUCCESS = SKIP, not ERR: a boot cleanup must never fail boot or touch
// page 8 on an unknown generation.
int32_t SE_BootMigrate(void)
{
    const SeAccountBackend *be = SeBackend();

    if (be == NULL) {
        return SUCCESS_CODE;   // fail-closed = skip the wipe (never fail boot / never touch page 8)
    }
    return be->boot_migrate();
}

// Write the per-account lifecycle status page (magic + state byte). state == ACCOUNT_STATUS_UNKNOWN writes an
// all-zero page (clears the status -> reads back as UNKNOWN -> coarse fallback). Single page write = atomic.
int32_t SE_SetAccountStatus(uint8_t accountIndex, AccountStatus_t state)
{
    uint8_t page[32] = {0};
    int32_t ret;

    ASSERT(accountIndex <= 2);
    if (state != ACCOUNT_STATUS_UNKNOWN) {
        uint32_t magic = ACCOUNT_STATUS_MAGIC;
        memcpy(page, &magic, sizeof(magic));   // bytes 0-3 = magic
        page[4] = (uint8_t)state;              // byte 4    = state
    }
    ret = SE_HmacEncryptWrite(page, PAGE_ACCOUNT_STATUS_BASE + accountIndex);
    CLEAR_ARRAY(page);
    return ret;
}

// Read the status page. *state = ACCOUNT_STATUS_UNKNOWN when the magic is absent (blank / legacy / wiped /
// read error) so the caller falls back to the coarse boot check.
int32_t SE_GetAccountStatus(uint8_t accountIndex, AccountStatus_t *state)
{
    uint8_t page[32];
    uint32_t magic;
    int32_t ret;

    ASSERT(accountIndex <= 2);
    *state = ACCOUNT_STATUS_UNKNOWN;
    ret = SE_HmacEncryptRead(page, PAGE_ACCOUNT_STATUS_BASE + accountIndex);
    if (ret != SUCCESS_CODE) {
        return ret;
    }
    memcpy(&magic, page, sizeof(magic));
    if (magic == ACCOUNT_STATUS_MAGIC && page[4] >= ACCOUNT_STATUS_CREATING && page[4] <= ACCOUNT_STATUS_DELETING) {
        *state = (AccountStatus_t)page[4];
    }
    CLEAR_ARRAY(page);
    return SUCCESS_CODE;
}

/// @brief Set the fingerprint encrypted password, store in SE.
/// @param[in] index
/// @param[in] encryptedPassword 32 bytes.
/// @return err code.
int32_t SetFpEncryptedPassword(uint32_t index, const uint8_t *encryptedPassword)
{
    ASSERT(index < 10);
    return SE_HmacEncryptWrite(encryptedPassword, PAGE_PF_ENCRYPTED_PASSWORD + index);
}

/// @brief Set the fingerprint state info.
/// @param[in] info 32 byte info.
/// @return err code.
int32_t SetFpStateInfo(uint8_t *info)
{
    uint8_t data[32] = {0};
    int32_t ret;

    memcpy(data, info, 32);
    ret = SE_HmacEncryptWrite(data, PAGE_PF_INFO);
    return ret;
}

/// @brief Get the fingerprint state info.
/// @param[out] info 32 byte info.
/// @return err code.
int32_t GetFpStateInfo(uint8_t *info)
{
    uint8_t data[32];
    int32_t ret;

    ret = SE_HmacEncryptRead(data, PAGE_PF_INFO);
    CHECK_ERRCODE_RETURN_INT(ret);
    memcpy(info, data, 32);
    return ret;
}

/// @brief Set the wallet data hash.
/// @param[in] index
/// @param[in] info 32 byte info.
/// @return err code.
int32_t SetWalletDataHash(uint8_t index, uint8_t *info)
{
    uint8_t data[32] = {0};
    int32_t ret;

    ASSERT(index <= 2);

    memcpy(data, info, 32);
    ret = SE_HmacEncryptWrite(data, PAGE_WALLET1_PUB_KEY_HASH + index);
    return ret;
}

/// @brief verify the wallet data hash.
/// @param[in] index
/// @param[in] info 32 byte info.
/// @return result of verify.
bool VerifyWalletDataHash(uint8_t index, uint8_t *info)
{
    uint8_t data[32];
    int32_t ret;

    ASSERT(index <= 2);

    ret = SE_HmacEncryptRead(data, PAGE_WALLET1_PUB_KEY_HASH + index);
    if (ret == SUCCESS_CODE && !memcmp(data, info, 32)) {
        return true;
    } else {
        if (CheckAllFF(data, 32) || CheckAllZero(data, 32)) {
            SetWalletDataHash(index, data);
            return true;
        } else {
            return false;
        }
    }
}

int32_t SetMultisigDataHash(uint8_t index, uint8_t *info)
{
    uint8_t data[32] = {0};
    int32_t ret;

    ASSERT(index <= 2);

    memcpy(data, info, 32);
    ret = SE_HmacEncryptWrite(data, index * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_MULTISIG_CONFIG_HASH);
    return ret;
}

bool VerifyMultisigWalletDataHash(uint8_t index, uint8_t *info)
{
    uint8_t data[32];
    int32_t ret;

    ASSERT(index <= 2);

    ret = SE_HmacEncryptRead(data, index * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_MULTISIG_CONFIG_HASH);
    if (ret == SUCCESS_CODE && !memcmp(data, info, 32)) {
        return true;
    } else {
        if (CheckAllFF(data, 32) || CheckAllZero(data, 32)) {
            SetMultisigDataHash(index, data);
            return true;
        } else {
            return false;
        }
    }
}

/// @brief Get the fingerprint encrypted password which stored in SE.
/// @param[in] index
/// @param[out] encryptedPassword 32 bytes.
/// @return err code.
int32_t GetFpEncryptedPassword(uint32_t index, uint8_t *encryptedPassword)
{
    ASSERT(index < 10);
    return SE_HmacEncryptRead(encryptedPassword, PAGE_PF_ENCRYPTED_PASSWORD + index);
}

/// @brief Set the fingerprint communication AES key.
/// @param[in] aesKey length 16bytes.
/// @return err code.
int32_t SetFpCommAesKey(const uint8_t *aesKey)
{
    return SE_HmacEncryptWrite(aesKey, PAGE_PF_AES_KEY);
}

/// @brief Get the fingerprint communication AES key.
/// @param[out] aesKey length 16bytes.
/// @return err code.
int32_t GetFpCommAesKey(uint8_t *aesKey)
{
    int32_t ret = SE_HmacEncryptRead(aesKey, PAGE_PF_AES_KEY);
    return ret;
}

/// @brief Set the fingerprint reset AES key.
/// @param[in] resetKey length 16bytes.
/// @return err code.
int32_t SetFpResetKey(const uint8_t *resetKey)
{
    return SE_HmacEncryptWrite(resetKey, PAGE_PF_RESET_KEY);
}

/// @brief Get the fingerprint reset AES key.
/// @param[out] resetKey length 16bytes.
/// @return err code.
int32_t GetFpResetKey(uint8_t *resetKey)
{
    return SE_HmacEncryptRead(resetKey, PAGE_PF_RESET_KEY);
}

/// @brief Get whether the fingerprint AES key exists.
/// @return true - exists.
bool FpAesKeyExist()
{
    uint8_t key[32];
    bool ret;

    if (SE_HmacEncryptRead(key, PAGE_PF_AES_KEY) != SUCCESS_CODE) {
        return false;
    }
    ret = CheckEntropy(key, 32);
    CLEAR_ARRAY(key);
    return ret;
}

int32_t SignMessageWithDeviceKey(uint8_t *messageHash, uint8_t *signaure)
{
    int32_t ret;
    do {
        ret = Atecc608bSignMessageWithDeviceKey(messageHash, signaure);
        CHECK_ERRCODE_BREAK("get device pubkey error", ret);
    } while (0);
    return ret;
}

/// @brief Get the device public key
/// @param pubkey
/// @return
int32_t GetDevicePublicKey(uint8_t *pubkey)
{
    int32_t ret;
    uint8_t pubkeyXY[64] = {0};
    do {
        ret = Atecc608bGenDevicePubkey(pubkeyXY);
        PrintArray("608B pubkey1", pubkeyXY, 64);
        CHECK_ERRCODE_BREAK("get device pubkey error", ret);
        pubkey[0] = 0x04;
        memcpy(pubkey + 1, pubkeyXY, 64);
        PrintArray("608B pubkey2", pubkey, 65);
    } while (0);
    return ret;
}

// SE generation, resolved ONCE at SeManagerInit() (the locked config is immutable per device, so it is
// deterministic). All callers read this cached value via GetSeGen() — no per-call chip access.
// SE_GEN_UNKNOWN (0) = not yet resolved.
static SeGen_t g_seGen = SE_GEN_UNKNOWN;

#ifndef COMPILE_SIMULATOR
/// @brief Resolve the SE generation from the locked ATECC608B config manifest. Verifies the FULL pinned
///        manifest so a partial/mis-provisioned chip is never mistaken for a valid generation. Internal —
///        run once from SeManagerInit().
static SeGen_t ResolveSeGen(void)
{
    uint8_t cfg[128];
    bool cfgLocked = false, dataLocked = false;

    if (atcab_read_config_zone(cfg) != ATCA_SUCCESS) {
        return SE_GEN_INVALID;              // cannot read config -> fail closed
    }
    atcab_is_locked(LOCK_ZONE_CONFIG, &cfgLocked);
    atcab_is_locked(LOCK_ZONE_DATA, &dataLocked);

    if (!cfgLocked || !dataLocked) {
        return SE_GEN_UNPROVISIONED;        // blank / partially provisioned
    }

    // ATECC608 config-zone field offsets: countMatch=byte18, slotConfig[s]=20+2s,
    // keyConfig[s]=96+2s, chipOptions=bytes 90-91 (little-endian).
    uint8_t countMatch = cfg[18];
    uint16_t chipOptions = (uint16_t)cfg[90] | ((uint16_t)cfg[91] << 8);
#define SE_SC(s) ((uint16_t)cfg[20 + (s) * 2] | ((uint16_t)cfg[20 + (s) * 2 + 1] << 8))
#define SE_KC(s) ((uint16_t)cfg[96 + (s) * 2] | ((uint16_t)cfg[96 + (s) * 2 + 1] << 8))

    bool gen2Manifest =
        (countMatch == 0x81) && (chipOptions == 0x0402) &&
        (SE_SC(8) == 0x4D00) && (SE_SC(13) == 0x42A0) &&
        (SE_KC(4) == 0x005C) && (SE_KC(7) == 0x005C) && (SE_KC(11) == 0x005C) &&
        (SE_KC(8) == 0x001C) && (SE_KC(13) == 0x005C);

    bool gen1Manifest =
        (countMatch == 0x00) && (chipOptions == 0x0402) &&
        (SE_SC(8) == 0x42C2) && (SE_SC(13) == 0x42C2);
#undef SE_SC
#undef SE_KC

    // Generation is decided by the IMMUTABLE locked config manifest ONLY — never by a mutable data value.
    if (gen2Manifest) {
        return SE_GEN_2;
    } else if (gen1Manifest) {
        return SE_GEN_1;
    }
    return SE_GEN_INVALID;
}
#endif

/// @brief Resolve and cache the SE generation. Call ONCE at boot, after Atecc608bInit() and before any
///        SE-account use (GetKeyPieceFromSE / AccountManagerInit / SeBackend()). Idempotent.
void SeManagerInit(void)
{
#ifdef COMPILE_SIMULATOR
    g_seGen = SE_GEN_1;
#else
    g_seGen = ResolveSeGen();
#endif
    printf("SeManagerInit: GetSeGen=%d (1=gen1 2=gen2 3=unprovisioned 4=invalid)\r\n", (int)g_seGen);
    // A shipped device is ALWAYS provisioned -> must be gen-1 or gen-2. SE_GEN_UNKNOWN/UNPROVISIONED/INVALID
    // here means a read failure, blank, mis-provisioned, or tampered SE — must never happen on a real device.
    ASSERT(g_seGen == SE_GEN_1 || g_seGen == SE_GEN_2);

    // gen-2 boot brick check: if the SE's counter shows the account can no longer derive, fail the device
    // cleanly here at boot rather than asserting mid-unlock. Guarded by the read return codes so a transient
    // SE read glitch can never false-brick a healthy device. (On a fresh/healthy unit this is a no-op.)
    // Not in the simulator: gen-2 is forced off above, and SE_GetCounter/SE_GetMatchCount live in the
    // chip-only se_backend_gen2.c which the simulator doesn't compile.
#ifndef COMPILE_SIMULATOR
    if (g_seGen == SE_GEN_2) {
        uint32_t counter = 0, matchCount = 0;
        int32_t rc = SE_GetCounter(&counter);
        int32_t rm = SE_GetMatchCount(&matchCount);
        if (rc == SUCCESS_CODE && rm == SUCCESS_CODE) {
            ASSERT(counter < matchCount);   // exhausted -> halt the device
        }
    }
#endif
}

/// @brief SE generation (resolved at SeManagerInit). Deterministic; UNPROVISIONED/INVALID mean a
///        manufacturing defect or tamper — callers must fail closed. Defensive lazy resolve: if a caller
///        reaches here before SeManagerInit() ran (g_seGen still SE_GEN_UNKNOWN), resolve on the spot so
///        the value is never the 0 sentinel.
/// @return SeGen_t (SE_GEN_1 / SE_GEN_2 / SE_GEN_UNPROVISIONED / SE_GEN_INVALID)
SeGen_t GetSeGen(void)
{
#ifdef COMPILE_SIMULATOR
    return SE_GEN_1;
#else
    if (g_seGen == SE_GEN_UNKNOWN) {
        g_seGen = ResolveSeGen();
    }
    ASSERT(g_seGen == SE_GEN_1 || g_seGen == SE_GEN_2);
    return g_seGen;
#endif
}

// ============================================================================
// Backend dispatcher. The gen-1 and gen-2 backends + all their logic are isolated in
// se_backend_gen1.c / se_backend_gen2.c; SeBackend() selects between them from GetSeGen().
// ============================================================================

const SeAccountBackend *SeBackend(void)
{
#ifdef COMPILE_SIMULATOR
    // The simulator routes keystore ops through SimulatorXxx stubs and never compiles the (chip-dependent)
    // gen-1/gen-2 backend files, so there is no real backend to dispatch to. Return NULL — every SeBackend()
    // caller already fails closed on NULL, and those paths aren't exercised under COMPILE_SIMULATOR anyway.
    return NULL;
#else
    switch (GetSeGen()) {
    case SE_GEN_1:
        return &g_seBackendGen1;
    case SE_GEN_2:
        return &g_seBackendGen2;
    default:
        return NULL;               // UNPROVISIONED / INVALID -> fail closed (callers must handle)
    }
#endif
}

// Wipe SE-side transient session secrets for the active generation. Safe to call anytime (NULL backend ->
// no-op). Wired into ClearSecretCache() so SE session material dies with the passcode cache.
void SE_ClearSessionSecrets(void)
{
    const SeAccountBackend *be = SeBackend();
    if (be != NULL && be->clear_session != NULL) {
        be->clear_session();
    }
}
