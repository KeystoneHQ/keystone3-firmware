//#include "internal.h"
#include "mnemonic.h"
#include "wordlist.h"
#include "hmac.h"
#include "sha256.h"
#include "sha512.h"
#include "bip39.h"

#include "bip39_english.inc"
#include "user_memory.h"
#include "err_code.h"
#include "pbkdf2.h"

#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#else
#define memset_s(p, s, c, l) memset(p, c, l)
#endif

static const struct {
    const char name[4];
    const struct words *words;
} lookup[] = {
    { "en", &en_words},
};

int bip39_get_languages(char **output)
{
    if (!output)
        return -2;

    *output = SRAM_MALLOC(strlen("en") + 1);
    strcpy(*output, "en");
//#endif
    return *output ? SUCCESS_CODE : -1;
}

int bip39_get_wordlist(const char *lang, struct words **output)
{
    size_t i;

    if (!output)
        return -2;

    *output = (struct words *)&en_words; /* Fallback to English if not found */

    if (lang)
        for (i = 0; i < sizeof(lookup) / sizeof(lookup[0]); ++i)
            if (!strcmp(lang, lookup[i].name)) {
                *output = (struct words *)lookup[i].words;
                break;
            }
    return SUCCESS_CODE;
}

int bip39_get_word(const struct words *w, size_t idx,
                   char **output)
{
    const char *word;

    if (output)
        *output = NULL;

    w = w ? w : &en_words;

    word = wordlist_lookup_index(w, idx);
    if (!output || !word)
        return -2;

    *output = SRAM_MALLOC(strlen(word) + 1);
    strcpy(*output, word);
    return *output ? SUCCESS_CODE : -1;
}

/* Convert an input entropy length to a mask for checksum bits. As it
 * returns 0 for bad lengths, it serves as a validation function too.
 */
static size_t len_to_mask(size_t len)
{
    switch (len) {
    case BIP39_ENTROPY_LEN_128:
        return 0xf0;
    case BIP39_ENTROPY_LEN_160:
        return 0xf8;
    case BIP39_ENTROPY_LEN_192:
        return 0xfc;
    case BIP39_ENTROPY_LEN_224:
        return 0xfe;
    case BIP39_ENTROPY_LEN_256:
        return 0xff;
    case BIP39_ENTROPY_LEN_288:
        return 0x80ff;
    case BIP39_ENTROPY_LEN_320:
        return 0xC0ff;
    }
    return 0;
}

static size_t bip39_checksum(const unsigned char *bytes, size_t bytes_len, size_t mask)
{
    struct sha256 sha;
    size_t ret;
    sha256(&sha, bytes, bytes_len);
    ret = sha.u.u8[0] | (sha.u.u8[1] << 8);
    memset_s(&sha, sizeof(sha), 0, sizeof(sha));
    return ret & mask;
}

int bip39_mnemonic_from_bytes(const struct words *w,
                              const unsigned char *bytes, size_t bytes_len,
                              char **output)
{
    unsigned char tmp_bytes[BIP39_ENTROPY_MAX_LEN];
    size_t checksum, mask;

    if (output)
        *output = NULL;

    if (!bytes || !bytes_len || !output)
        return -2;

    w = w ? w : &en_words;

    mask = len_to_mask(bytes_len);
    if (w->bits != 11u || !mask)
        return -2;

    memcpy(tmp_bytes, bytes, bytes_len);
    checksum = bip39_checksum(bytes, bytes_len, mask);
    tmp_bytes[bytes_len] = checksum & 0xff;
    if (mask > 0xff)
        tmp_bytes[++bytes_len] = (checksum >> 8) & 0xff;
    *output = mnemonic_from_bytes(w, tmp_bytes, bytes_len + 1);
    memset_s(tmp_bytes, sizeof(tmp_bytes), 0, sizeof(tmp_bytes));
    return *output ? SUCCESS_CODE : -1;
}

static bool checksum_ok(const unsigned char *bytes, size_t idx, size_t mask)
{
    /* The checksum is stored after the data to sum */
    size_t calculated = bip39_checksum(bytes, idx, mask);
    size_t stored = bytes[idx];
    if (mask > 0xff)
        stored |= (bytes[idx + 1] << 8);
    return (stored & mask) == calculated;
}

int bip39_mnemonic_to_bytes(const struct words *w, const char *mnemonic,
                            unsigned char *bytes_out, size_t len,
                            size_t *written)
{
    unsigned char tmp_bytes[BIP39_ENTROPY_MAX_LEN];
    size_t mask, tmp_len;
    int ret;

    /* Ideally we would infer the wordlist here. Unfortunately this cannot
     * work reliably because the default word lists overlap. In combination
     * with being sorted lexographically, this means the default lists
     * were poorly chosen. But we are stuck with them now.
     *
     * If the caller doesn't know which word list to use, they should iterate
     * over the available ones and try any resulting list that the mnemonic
     * validates against.
     */
    w = w ? w : &en_words;

    if (written)
        *written = 0;

    if (w->bits != 11u || !mnemonic || !bytes_out)
        return -2;

    ret = mnemonic_to_bytes(w, mnemonic, tmp_bytes, sizeof(tmp_bytes), &tmp_len);

    if (!ret) {
        /* Remove checksum bytes from the output length */
        --tmp_len;
        if (tmp_len > BIP39_ENTROPY_LEN_256)
            --tmp_len; /* Second byte required */

        if (tmp_len > sizeof(tmp_bytes))
            ret = -2; /* Too big for biggest supported entropy */
        else {
            if (tmp_len <= len) {
                mask = len_to_mask(tmp_len);
                if (!mask ||
                        !checksum_ok(tmp_bytes, tmp_len, mask)) {
                    tmp_len = 0;
                    ret = -2; /* Bad checksum */
                } else
                    memcpy(bytes_out, tmp_bytes, tmp_len);
            }
        }
    }
    memset_s(tmp_bytes, sizeof(tmp_bytes), 0, sizeof(tmp_bytes));
    if (!ret && written)
        *written = tmp_len;
    return ret;
}

int bip39_mnemonic_validate(const struct words *w, const char *mnemonic)
{
    unsigned char buf[BIP39_ENTROPY_MAX_LEN];
    size_t len;
    int ret = bip39_mnemonic_to_bytes(w, mnemonic, buf, sizeof(buf), &len);
    memset_s(buf, sizeof(buf), 0, sizeof(buf));
    return ret;
}

int bip39_mnemonic_to_seed(const char *mnemonic, const char *passphrase,
                           unsigned char *bytes_out, size_t len,
                           size_t *written)
{
    const uint32_t bip9_cost = 2048u;
    const char *prefix = "mnemonic";
    const size_t prefix_len = strlen(prefix);
    const size_t passphrase_len = passphrase ? strlen(passphrase) : 0;
    const size_t salt_len = prefix_len + passphrase_len;
    unsigned char *salt;
    int ret;

    if (written)
        *written = 0;

    if (!mnemonic || !bytes_out || len != BIP39_SEED_LEN_512)
        return -2;

    salt = SRAM_MALLOC(salt_len);
    if (!salt)
        return -1;

    memcpy(salt, prefix, prefix_len);
    if (passphrase_len)
        memcpy(salt + prefix_len, passphrase, passphrase_len);

    ret = pbkdf2_hmac_sha512((unsigned char *)mnemonic, strlen(mnemonic),
                             salt, salt_len, 0,
                             bip9_cost, bytes_out, len);

    if (!ret && written)
        *written = BIP39_SEED_LEN_512; /* Succeeded */

    memset_s(salt, salt_len, 0, salt_len);
    SRAM_FREE(salt);

    return ret;
}

int bip39_mnemonic_to_seed512(const char *mnemonic, const char *passphrase,
                              unsigned char *bytes_out, size_t len)
{
    size_t written;
    return bip39_mnemonic_to_seed(mnemonic, passphrase, bytes_out, len, &written);
}
