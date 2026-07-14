#ifndef _SE_ACCOUNT_BACKEND_H_
#define _SE_ACCOUNT_BACKEND_H_

#include "stdint.h"

// Generation-isolated SE account backend.
// One const vtable per SE generation; SeBackend() selects it once from GetSeGen(). Gen-specific code
// lives behind these pointers; shared crypto (combine/HMAC/AES, DS piece, try-all) stays in the shared
// layer. All SE-account calls MUST route through SeBackend() (never the gen1/gen2 symbols directly) so the
// single binary never runs the wrong-generation path.
typedef struct {
    // 608-side key piece — the only derivation that differs between generations.
    // ⚠️ ARG ORDER: (accountIndex, password, piece608) — password BEFORE the output buffer. This is the
    // REVERSE of the legacy GetKeyPieceFromAtecc608b(accountIndex, piece, password). Passing them swapped
    // compiles (uint8_t* vs const char* is warn-only) but hashes the wrong buffer as the password and
    // bricks unlock with 608 CheckMac 0xD1. Always call as derive_608(idx, password, pieces).
    int32_t (*derive_608)(uint8_t accountIndex, const char *password, uint8_t piece608[32]);    // verify/login
    // create account. R = existing device-wide key to reuse (add-wallet, from an existing wallet),
    // or NULL = new device -> generate a fresh one. Same ⚠️ ARG ORDER caveat as derive_608 for (password, piece608).
    int32_t (*provision_608)(uint8_t accountIndex, const char *password, uint8_t piece608[32], const uint8_t *R);

    // Recover the device-wide key R from an ALREADY-PROVISIONED wallet, so a new wallet can be
    // provisioned with it (add-wallet). existingIdx/existingPassword identify+authenticate that wallet.
    // gen-2: derives R from that wallet's SE state + password. gen-1: ERR (no R). Caller CLEAR_ARRAY(R_out).
    int32_t (*recover_reset_key)(uint8_t existingIdx, const char *existingPassword, uint8_t *R_out);

    // Called once after the shared HMAC has verified the unlock.
    //   gen-2: restore per-session SE state.   gen-1: no-op.
    int32_t (*on_unlock_success)(uint8_t accountIndex);

    // Lifecycle. Change-PIN reuses derive_608 + provision_608, but gen-2 must first prepare state from
    // the old PIN; gen-1 is a no-op. The account-status state machine remains in keystore.c ChangePassword.
    int32_t (*prepare_change_pin)(uint8_t accountIndex, const char *oldPassword);
    int32_t (*erase_account)(uint8_t accountIndex);    // forget one wallet
    int32_t (*wipe_all)(void);                          // full wipe

    // One-time boot work.   gen-1: legacy page-8 PIN-hash wipe.   gen-2: no-op (page 8 holds other data).
    int32_t (*boot_migrate)(void);

    // Wipe transient session secrets. Called from ClearSecretCache() (and lock/logout) so SE-side session
    // material shares the passcode cache's lifetime — never outliving the operation.
    //   gen-2: clear the stashed session key.   gen-1: no-op (no session secret).
    void (*clear_session)(void);
} SeAccountBackend;

// Each generation's backend is defined in its own isolated file (se_backend_gen1.c / se_backend_gen2.c);
// the SeBackend() dispatcher lives in se_manager.c and selects between them from GetSeGen().
extern const SeAccountBackend g_seBackendGen1;
extern const SeAccountBackend g_seBackendGen2;

// Returns the vtable for the detected generation, or NULL for SE_GEN_UNPROVISIONED / SE_GEN_INVALID
// (callers must fail closed — a shipped device is always SE_GEN_1 or SE_GEN_2).
const SeAccountBackend *SeBackend(void);

#endif
