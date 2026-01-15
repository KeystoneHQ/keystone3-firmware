#ifdef WEB3_VERSION
#include "rsa.h"
#include "user_utils.h"

static uint32_t GetRsaAddress();
static void RsaHashWithSalt(const uint8_t *data, uint8_t *hash);
static bool HasMatchingPrimesHash(Rsa_primes_t *primes, const uint8_t targethash[SPI_FLASH_RSA_HASH_SIZE]);

static uint32_t GetRsaAddress()
{
    switch (GetCurrentAccountIndex()) {
    case 0:
        return SPI_FLASH_RSA_USER1_DATA;
    case 1:
        return SPI_FLASH_RSA_USER2_DATA;
    case 2:
        return SPI_FLASH_RSA_USER3_DATA;
    default:
        ASSERT(false);
    }
}

static void RsaHashWithSalt(const uint8_t *data, uint8_t *hash)
{
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    char hexString[2 * sizeof(mfp) + 1];
    char *hexPtr = hexString;
    for (size_t i = 0; i < sizeof(mfp); ++i) {
        sprintf(hexPtr, "%02X", mfp[i]);
        hexPtr += 2;
    }
    *hexPtr = '\0';
    const char *constHexString = hexString;
    HashWithSalt(hash, (uint8_t *)data, SPI_FLASH_RSA_ORIGIN_DATA_SIZE, constHexString);
}

static bool HasMatchingPrimesHash(Rsa_primes_t *primes, const uint8_t targethash[SPI_FLASH_RSA_HASH_SIZE])
{
    uint8_t bytes[SPI_FLASH_RSA_ORIGIN_DATA_SIZE];
    memcpy_s(bytes, SPI_FLASH_RSA_PRIME_SIZE, primes->p, SPI_FLASH_RSA_PRIME_SIZE);
    memcpy_s(bytes + SPI_FLASH_RSA_PRIME_SIZE, SPI_FLASH_RSA_PRIME_SIZE, primes->q, SPI_FLASH_RSA_PRIME_SIZE);
    uint8_t *sourceHash = SRAM_MALLOC(SPI_FLASH_RSA_HASH_SIZE);
    RsaHashWithSalt(bytes, sourceHash);
    memset_s(bytes, SPI_FLASH_RSA_ORIGIN_DATA_SIZE, 0, SPI_FLASH_RSA_ORIGIN_DATA_SIZE);
    bool ret = memcmp(sourceHash, targethash, SPI_FLASH_RSA_HASH_SIZE) == 0;
    SRAM_FREE(sourceHash);
    return ret;
}


Rsa_primes_t *FlashReadRsaPrimes(void)
{
    int ret = -1;
    Rsa_primes_t *primes = NULL;
    SimpleResponse_u8 *encData = NULL;
    uint8_t fullData[SPI_FLASH_RSA_DATA_FULL_SIZE] = {0};
    uint8_t cryptData[SPI_FLASH_RSA_DATA_SIZE] = {0};
    uint8_t hash[SPI_FLASH_RSA_HASH_SIZE] = {0};
    uint8_t seed[SEED_LEN] = {0};

    do {
        primes = SRAM_MALLOC(sizeof(Rsa_primes_t));
        int readLen = Gd25FlashReadBuffer(GetRsaAddress(), fullData, sizeof(fullData));
#ifndef COMPILE_SIMULATOR
        ASSERT(readLen == sizeof(fullData));
#endif
        int len = (GetMnemonicType() == MNEMONIC_TYPE_BIP39) ? (int)sizeof(seed) : GetCurrentAccountEntropyLen();
        if (SecretCacheGetPassword() == NULL) {
            printf("password is empty\n");
            break;
        }
        ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        CHECK_ERRCODE_BREAK("GetAccountSeed", ret);

        memcpy_s(cryptData, sizeof(cryptData), fullData, sizeof(cryptData));
        encData = aes256_decrypt_primes(seed, len, cryptData);
        CHECK_ERRCODE_BREAK("aes256_decrypt_primes", encData->error_code);

        memcpy_s(primes->p, SPI_FLASH_RSA_PRIME_SIZE, encData->data, SPI_FLASH_RSA_PRIME_SIZE);
        memcpy_s(primes->q, SPI_FLASH_RSA_PRIME_SIZE, encData->data + SPI_FLASH_RSA_PRIME_SIZE, SPI_FLASH_RSA_PRIME_SIZE);

        memcpy_s(hash, sizeof(hash), fullData + SPI_FLASH_RSA_DATA_SIZE, sizeof(hash));
        ASSERT(HasMatchingPrimesHash(primes, hash));
    } while (0);

    if (encData) {
        free_simple_response_u8(encData);
    }
    CLEAR_ARRAY(seed);
    CLEAR_ARRAY(fullData);
    CLEAR_ARRAY(cryptData);
    CLEAR_ARRAY(hash);
    if (ret != SUCCESS_CODE) {
        if (primes) {
            CLEAR_ARRAY(primes->p);
            CLEAR_ARRAY(primes->q);
            SRAM_FREE(primes);
            primes = NULL;
        }
    }
    return primes;
}

int FlashWriteRsaPrimes(const uint8_t *data)
{
    int ret = -1;
    SimpleResponse_u8 *cryptData = NULL;
    uint8_t fullData[SPI_FLASH_RSA_DATA_FULL_SIZE] = {0};
    uint8_t seed[64] = {0};
    uint8_t *hash = NULL;

    do {
        int len = (GetMnemonicType() == MNEMONIC_TYPE_BIP39) ? (int)sizeof(seed) : GetCurrentAccountEntropyLen();
        if (GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword()) != 0) {
            printf("Failed to get account seed\n");
            break;
        }

        cryptData = aes256_encrypt_primes(seed, len, (PtrBytes)data);
        if (cryptData == NULL || cryptData->error_code != 0) {
            printf("Failed to encrypt RSA primes\n");
            break;
        }

        memcpy_s(fullData, SPI_FLASH_RSA_DATA_SIZE, cryptData->data, SPI_FLASH_RSA_DATA_SIZE);

        hash = SRAM_MALLOC(SPI_FLASH_RSA_HASH_SIZE);
        if (!hash) {
            printf("Failed to alloc hash buffer\n");
            break;
        }
        RsaHashWithSalt(data, hash);
        memcpy_s(fullData + SPI_FLASH_RSA_DATA_SIZE, SPI_FLASH_RSA_HASH_SIZE, hash, SPI_FLASH_RSA_HASH_SIZE);

        Gd25FlashSectorErase(GetRsaAddress());
        int32_t wret = Gd25FlashWriteBuffer(GetRsaAddress(), fullData, sizeof(fullData));
        ASSERT(wret == sizeof(fullData));

        uint8_t verifyBuf[SPI_FLASH_RSA_DATA_FULL_SIZE] = {0};
        Gd25FlashReadBuffer(GetRsaAddress(), verifyBuf, sizeof(verifyBuf));
        if (memcmp(verifyBuf, fullData, sizeof(fullData)) != 0) {
            printf("Flash verify mismatch after write\n");
            ASSERT(false);
        }
        CLEAR_ARRAY(verifyBuf);

        ret = 0;
    } while (0);

    if (hash) {
        SRAM_FREE(hash);
    }
    if (cryptData) {
        free_simple_response_u8(cryptData);
    }
    CLEAR_ARRAY(fullData);
    CLEAR_ARRAY(seed);
    return ret;
}
#endif