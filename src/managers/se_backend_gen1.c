#include <stdio.h>
#include "string.h"
#include "se_manager.h"
#include "se_interface.h"
#include "se_account_backend.h"
#include "user_utils.h"
#include "user_memory.h"           // safec strnlen_s macro (-> _strnlen_s_chk)
#include "assert.h"
#include "sha256.h"
#include "err_code.h"
#include "drv_trng.h"
#include "drv_atecc608b.h"
#include "cryptoauthlib.h"
#include "hash_and_salt.h"
#include "secret_cache.h"
#include "account_manager.h"       // IsPinHashWiped / SetPinHashWiped (legacy page-8 wipe, boot_migrate)

// gen-1 SE account backend (legacy: per-account Authorize + roll/host KDF).
// Isolated from se_manager.c, mirroring se_backend_gen2.c. Selected by SeBackend() only when
// GetSeGen() == SE_GEN_1. The shared DS28S60 key piece + dispatchers stay in se_manager.c.

#define SHA256_COUNT                            3

static int32_t NormalizeAteccAuthError(int32_t ret)
{
    return (ret == ATCA_CHECKMAC_VERIFY_FAILED) ? ERR_KEYSTORE_AUTH : ret;
}

// gen-1 608 provisioning: write per-account auth key, derive the roll-kdf slot, write a host-random kdf
// slot, then KDF(roll)->KDF(host)->3x sha256 to produce the 608 piece.
static int32_t SetNewKeyPieceToAtecc608b(uint8_t accountIndex, uint8_t *piece, const char *password)
{
    uint8_t authKey[32], hostRandom[32], inData[32], outData[32];
    int32_t ret;
    AccountSlot_t accountSlot;

    ASSERT(accountIndex <= 2);
    do {
        HashWithSalt(authKey, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "auth_key");
        GetAccountSlot(&accountSlot, accountIndex);
        ret = SE_EncryptWrite(accountSlot.auth, 0, authKey);
        CHECK_ERRCODE_BREAK("write auth", ret);
        ret = SE_DeriveKey(accountSlot.rollKdf, authKey);
        CHECK_ERRCODE_BREAK("derive key", ret);
        TrngGet(hostRandom, 32);
        ret = SE_EncryptWrite(accountSlot.hostKdf, 0, hostRandom);
        CHECK_ERRCODE_BREAK("write kdf", ret);

        HashWithSalt(outData, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "password_atecc608b");
        memcpy(inData, outData, 32);
        ret = SE_Kdf(accountSlot.rollKdf, authKey, inData, 32, outData);
        CHECK_ERRCODE_BREAK("kdf", ret);
        memcpy(inData, outData, 32);
        ret = SE_Kdf(accountSlot.hostKdf, authKey, inData, 32, outData);
        CHECK_ERRCODE_BREAK("kdf", ret);
        for (uint32_t i = 0; i < SHA256_COUNT; i++) {
            memcpy(inData, outData, 32);
            sha256((struct sha256 *)outData, inData, 32);
        }
        memcpy(piece, outData, 32);
    } while (0);
    CLEAR_ARRAY(authKey);
    CLEAR_ARRAY(hostRandom);
    CLEAR_ARRAY(inData);
    CLEAR_ARRAY(outData);

    return ret;
}

// gen-1 608 derive: KDF(roll)->KDF(host)->3x sha256, matching the provisioning above.
static int32_t GetKeyPieceFromAtecc608b(uint8_t accountIndex, uint8_t *piece, const char *password)
{
    uint8_t authKey[32], inData[32], outData[32];
    int32_t ret;
    AccountSlot_t accountSlot;

    ASSERT(accountIndex <= 2);
    do {
        HashWithSalt(authKey, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "auth_key");
        HashWithSalt(outData, (uint8_t *)password, strnlen_s(password, PASSWORD_MAX_LEN), "password_atecc608b");
        memcpy(inData, outData, 32);

        GetAccountSlot(&accountSlot, accountIndex);
        ret = SE_Kdf(accountSlot.rollKdf, authKey, inData, 32, outData);
        ret = NormalizeAteccAuthError(ret);
        CHECK_ERRCODE_BREAK("kdf", ret);
        memcpy(inData, outData, 32);
        ret = SE_Kdf(accountSlot.hostKdf, authKey, inData, 32, outData);
        ret = NormalizeAteccAuthError(ret);
        CHECK_ERRCODE_BREAK("kdf", ret);
        for (uint32_t i = 0; i < SHA256_COUNT; i++) {
            memcpy(inData, outData, 32);
            sha256((struct sha256 *)outData, inData, 32);
        }
        memcpy(piece, outData, 32);
    } while (0);
    CLEAR_ARRAY(authKey);
    CLEAR_ARRAY(inData);
    CLEAR_ARRAY(outData);

    return ret;
}

// ============================================================================
// gen-1 vtable. derive/provision/on_unlock are real; recover_reset_key /
// erase_account / wipe_all are no-ops on gen-1; boot_migrate does the legacy page-8 wipe.
// ============================================================================
static int32_t Gen1Derive608(uint8_t accountIndex, const char *password, uint8_t piece608[32])
{
    return GetKeyPieceFromAtecc608b(accountIndex, piece608, password);
}
static int32_t Gen1Provision608(uint8_t accountIndex, const char *password, uint8_t piece608[32], const uint8_t *R)
{
    (void)R;   // unused on gen-1
    return SetNewKeyPieceToAtecc608b(accountIndex, piece608, password);
}
static int32_t Gen1OnUnlockSuccess(uint8_t accountIndex)
{
    (void)accountIndex;
    return SUCCESS_CODE;
}
static int32_t Gen1RecoverResetKey(uint8_t existingIdx, const char *existingPassword, uint8_t *R_out)
{
    (void)existingIdx; (void)existingPassword; (void)R_out;
    return SUCCESS_CODE;        // no-op on gen-1
}
static int32_t Gen1PrepareChangePin(uint8_t accountIndex, const char *oldPassword)
{
    (void)accountIndex;
    (void)oldPassword;
    return SUCCESS_CODE;        // no-op on gen-1
}
static int32_t Gen1EraseAccount(uint8_t accountIndex)
{
    (void)accountIndex;
    return SUCCESS_CODE;        // gen-1 erase = the DS28S60 page-zeroing in DestroyAccount
}
static int32_t Gen1WipeAll(void)
{
    return SUCCESS_CODE;        // gen-1 wipe = page-zeroing + flash erase in WipeDevice
}
// Legacy page-8 PIN-hash wipe — gen-1 only. Idempotent via the pinHashWiped flag.
static int32_t WipeLegacyPasswordHashPages(void)
{
    uint8_t data[32] = {0};
    int32_t ret = SUCCESS_CODE;

    if (IsPinHashWiped()) {
        return SUCCESS_CODE;
    }
    // Erase old PIN verifiers. Do not add normal read/write users for this page.
    for (uint8_t accountIndex = 0; accountIndex < 3; accountIndex++) {
        ret = SE_HmacEncryptWrite(data, accountIndex * PAGE_NUM_PER_ACCOUNT + PAGE_INDEX_LEGACY_PASSWORD_HASH);
        CHECK_ERRCODE_BREAK("wipe legacy password hash", ret);
    }
    if (ret == SUCCESS_CODE) {
        ret = SetPinHashWiped(true);
    }
    CLEAR_ARRAY(data);
    return ret;
}
static int32_t Gen1BootMigrate(void)
{
    return WipeLegacyPasswordHashPages();   // one-time legacy page-8 PIN-hash wipe (gen-1 only)
}
static void Gen1ClearSession(void)
{
    // gen-1 holds no SE-side session secret — nothing to wipe.
}

const SeAccountBackend g_seBackendGen1 = {
    .derive_608        = Gen1Derive608,
    .provision_608     = Gen1Provision608,
    .recover_reset_key = Gen1RecoverResetKey,
    .on_unlock_success = Gen1OnUnlockSuccess,
    .prepare_change_pin = Gen1PrepareChangePin,
    .erase_account     = Gen1EraseAccount,
    .wipe_all          = Gen1WipeAll,
    .boot_migrate      = Gen1BootMigrate,
    .clear_session     = Gen1ClearSession,
};
