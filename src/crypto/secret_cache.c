#include "secret_cache.h"
#include "string.h"
#include "user_memory.h"
#include "user_utils.h"
#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#else
#define memset_s(p, s, c, l) memset(p, c, l)
#endif

static char *g_passwordCache = NULL;
static char *g_newPasswordCache = NULL;
static char *g_passphraseCache = NULL;
static uint8_t *g_entropyCache = NULL;
static uint32_t g_entropyLen;
static uint8_t *g_emsCache = NULL;
static uint8_t g_checksumCache[32] = {0};
static uint32_t g_emsLen;
static char *g_mnemonicCache = NULL;
static char *g_slip39MnemonicCache[15];
static uint16_t g_identifier;
static uint16_t g_iteration;

void SecretCacheSetChecksum(uint8_t *checksum)
{
    memcpy(g_checksumCache, checksum, sizeof(g_checksumCache));
}

void SecretCacheGetChecksum(char *checksum)
{
    ByteArrayToHexStr(g_checksumCache, sizeof(g_checksumCache), checksum);
}

void SecretCacheSetPassword(char *password)
{
    if (g_passwordCache) {
        SRAM_FREE(g_passwordCache);
    }
    g_passwordCache = SRAM_MALLOC(strlen(password) + 1);
    strcpy(g_passwordCache, password);
}

char *SecretCacheGetPassword(void)
{
    return g_passwordCache;
}

void SecretCacheSetPassphrase(char *passPhrase)
{
    if (g_passphraseCache) {
        SRAM_FREE(g_passphraseCache);
    }
    g_passphraseCache = SRAM_MALLOC(strlen(passPhrase) + 1);
    strcpy(g_passphraseCache, passPhrase);
}

char *SecretCacheGetPassphrase(void)
{
    return g_passphraseCache;
}

void SecretCacheSetNewPassword(char *password)
{
    if (g_newPasswordCache) {
        SRAM_FREE(g_newPasswordCache);
    }
    g_newPasswordCache = SRAM_MALLOC(strlen(password) + 1);
    strcpy(g_newPasswordCache, password);
}

char *SecretCacheGetNewPassword(void)
{
    return g_newPasswordCache;
}

void SecretCacheSetIteration(uint8_t ie)
{
    g_iteration = ie;
}

uint8_t SecretCacheGetIteration(void)
{
    return g_iteration;
}

void SecretCacheSetIdentifier(uint16_t id)
{
    g_identifier = id;
}

uint16_t SecretCacheGetIdentifier(void)
{
    return g_identifier;
}

void SecretCacheSetEntropy(uint8_t *entropy, uint32_t len)
{
    if (g_entropyCache) {
        SRAM_FREE(g_entropyCache);
    }
    g_entropyCache = SRAM_MALLOC(len);
    g_entropyLen = len;
    memcpy(g_entropyCache, entropy, len);
}


uint8_t *SecretCacheGetEntropy(uint32_t *len)
{
    *len = g_entropyLen;
    return g_entropyCache;
}

void SecretCacheSetEms(uint8_t *ems, uint32_t len)
{
    if (g_emsCache) {
        SRAM_FREE(g_emsCache);
    }
    g_emsCache = SRAM_MALLOC(len);
    g_emsLen = len;
    memcpy(g_emsCache, ems, len);
}


uint8_t *SecretCacheGetEms(uint32_t *len)
{
    *len = g_emsLen;
    return g_emsCache;
}


void SecretCacheSetMnemonic(char *mnemonic)
{
    if (g_mnemonicCache) {
        SRAM_FREE(g_mnemonicCache);
    }
    g_mnemonicCache = SRAM_MALLOC(strlen(mnemonic) + 1);
    strcpy(g_mnemonicCache, mnemonic);
}

char *SecretCacheGetMnemonic(void)
{
    return g_mnemonicCache;
}

void SecretCacheSetSlip39Mnemonic(char *mnemonic, int index)
{
    if (g_slip39MnemonicCache[index] != NULL) {
        EXT_FREE(g_slip39MnemonicCache[index]);
    }
    g_slip39MnemonicCache[index] = EXT_MALLOC(strlen(mnemonic) + 1);
    strcpy(g_slip39MnemonicCache[index], mnemonic);
}

char *SecretCacheGetSlip39Mnemonic(int index)
{
    return g_slip39MnemonicCache[index];
}

void ClearSecretCache(void)
{
    uint32_t len;
    g_identifier = 0;
    g_iteration = 0;

    if (g_passwordCache != NULL) {
        len = strlen(g_passwordCache);
        memset_s(g_passwordCache, len, 0, len);
        SRAM_FREE(g_passwordCache);
        g_passwordCache = NULL;
    }

    if (g_passphraseCache != NULL) {
        len = strlen(g_passphraseCache);
        memset_s(g_passphraseCache, len, 0, len);
        SRAM_FREE(g_passphraseCache);
        g_passphraseCache = NULL;
    }

    if (g_newPasswordCache != NULL) {
        len = strlen(g_newPasswordCache);
        memset_s(g_newPasswordCache, len, 0, len);
        SRAM_FREE(g_newPasswordCache);
        g_newPasswordCache = NULL;
    }

    if (g_entropyCache != NULL) {
        memset_s(g_entropyCache, g_entropyLen, 0, g_entropyLen);
        g_entropyLen = 0;
        SRAM_FREE(g_entropyCache);
        g_entropyCache = NULL;
    }

    if (g_emsCache != NULL) {
        memset_s(g_emsCache, g_emsLen, 0, g_emsLen);
        g_emsLen = 0;
        SRAM_FREE(g_emsCache);
        g_emsCache = NULL;
    }

    if (g_mnemonicCache != NULL) {
        len = strlen(g_mnemonicCache);
        memset_s(g_mnemonicCache, len, 0, len);
        SRAM_FREE(g_mnemonicCache);
        g_mnemonicCache = NULL;
    }

    for (int i = 0; i < 15; i++) {
        if (g_slip39MnemonicCache[i] != NULL) {
            len = strlen(g_slip39MnemonicCache[i]);
            memset_s(g_slip39MnemonicCache[i], len, 0, len);
            EXT_FREE(g_slip39MnemonicCache[i]);
            g_slip39MnemonicCache[i] = NULL;
        }
    }

    memset_s(g_checksumCache, 32, 0, 32);
}
