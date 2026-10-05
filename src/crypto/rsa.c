#include "assert.h"
#ifdef WEB3_VERSION
#include "rsa.h"
#include "user_utils.h"
#include "se_manager.h"
#include "err_code.h"
#include "account_public_info.h"

bool ArKeyNeedsSetup(int32_t status)
{
    return status == ERR_AR_NOT_SETUP || status == ERR_AR_DATA_INVALID;
}

static uint32_t GetRsaAddress(void)
{
    const uint32_t addresses[] = {SPI_FLASH_RSA_USER1_DATA, SPI_FLASH_RSA_USER2_DATA, SPI_FLASH_RSA_USER3_DATA};
    uint32_t accountIndex = GetCurrentAccountIndex();
    ASSERT(accountIndex < sizeof(addresses) / sizeof(addresses[0]));
    return addresses[accountIndex];
}

static void RsaHashWithSalt(const uint8_t *data, uint8_t *hash)
{
    uint8_t mfp[4] = {0};
    char salt[9] = {0};
    GetMasterFingerPrint(mfp);
    for (size_t i = 0; i < sizeof(mfp); i++) {
        snprintf_s(salt + 2 * i, sizeof(salt) - 2 * i, "%02X", mfp[i]);
    }
    HashWithSalt(hash, (uint8_t *)data, SPI_FLASH_RSA_ORIGIN_DATA_SIZE, salt);
}


static void FreePrimesResponse(SimpleResponse_u8 *response)
{
    if (response != NULL) {
        if (response->error_code == SUCCESS_CODE && response->data != NULL) {
            memset_s(response->data, SPI_FLASH_RSA_ORIGIN_DATA_SIZE, 0, SPI_FLASH_RSA_ORIGIN_DATA_SIZE);
        }
        free_simple_response_u8(response);
    }
}

int32_t LoadAndValidateArKey(const char *password, Rsa_primes_t **out, SimpleResponse_c_char **publicKeyOut)
{
    if (out != NULL) {
        *out = NULL;
    }
    if (publicKeyOut != NULL) {
        *publicKeyOut = NULL;
    }
    if (GetCurrentAccountIndex() > 2 || GetIsTempAccount()) {
        return ERR_KEYSTORE_NOT_LOGIN;
    }
    if (password == NULL) {
        return ERR_AR_AUTH_REQUIRED;
    }
    int32_t ret = ERR_AR_DATA_INVALID;
    uint8_t seed[64] = {0}, fullData[SPI_FLASH_RSA_DATA_FULL_SIZE] = {0};
    uint8_t hash[32] = {0}, seHash[32] = {0};
    SimpleResponse_u8 *decoded = NULL;
    SimpleResponse_c_char *publicKey = NULL;
    do {
        ret = GetAccountSeed(GetCurrentAccountIndex(), seed, password);
        if (ret != SUCCESS_CODE) {
            break;
        }
        ret = ERR_AR_STORAGE;
        if (Gd25FlashReadBuffer(GetRsaAddress(), fullData, sizeof(fullData)) != sizeof(fullData) ||
                GetRsaPrimesHash(GetCurrentAccountIndex(), seHash) != SUCCESS_CODE) {
            break;
        }
        if (CheckAllFF(fullData, sizeof(fullData))) {
            ret = ERR_AR_NOT_SETUP;
            break;
        }
        ret = ERR_AR_DATA_INVALID;
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        decoded = aes256_decrypt_primes(seed, len, fullData);
        if (decoded == NULL) {
            ret = ERR_GENERAL_FAIL;
            break;
        }
        if (decoded->error_code != SUCCESS_CODE || decoded->data == NULL) {
            break;
        }
        RsaHashWithSalt(decoded->data, hash);
        if (timingsafe_bcmp(hash, fullData + SPI_FLASH_RSA_DATA_SIZE, sizeof(hash)) != 0) {
            printf("AR load: Flash hash mismatch\n");
            break;
        }
        publicKey = generate_rsa_public_key(decoded->data, 256, decoded->data + 256, 256);
        if (publicKey == NULL) {
            ret = ERR_GENERAL_FAIL;
            break;
        }
        if (publicKey->error_code != SUCCESS_CODE ||
                !IsHexStringWithLen(publicKey->data, 1024)) {
            break;
        }
        if (CheckAllFF(seHash, sizeof(seHash)) || CheckAllZero(seHash, sizeof(seHash))) {
            if (SetRsaPrimesHash(GetCurrentAccountIndex(), hash) != SUCCESS_CODE ||
                    GetRsaPrimesHash(GetCurrentAccountIndex(), seHash) != SUCCESS_CODE ||
                    timingsafe_bcmp(hash, seHash, sizeof(hash)) != 0) {
                ret = ERR_AR_STORAGE;
                break;
            }
        } else if (timingsafe_bcmp(hash, seHash, sizeof(hash)) != 0) {
            printf("AR load: SE hash mismatch\n");
            break;
        }
        if (out != NULL) {
            *out = SRAM_MALLOC(sizeof(**out));
            if (*out == NULL) {
                ret = ERR_GENERAL_FAIL;
                break;
            }
            if (memcpy_s(*out, sizeof(**out), decoded->data, sizeof(**out)) != 0) {
                SRAM_FREE(*out);
                *out = NULL;
                ret = ERR_GENERAL_FAIL;
                break;
            }
        }
        if (publicKeyOut != NULL) {
            *publicKeyOut = publicKey;
            publicKey = NULL;
        }
        ret = SUCCESS_CODE;
    } while (0);
    FreePrimesResponse(decoded);
    if (publicKey != NULL) {
        free_simple_response_c_char(publicKey);
    }
    CLEAR_ARRAY(seed);
    CLEAR_ARRAY(fullData);
    CLEAR_ARRAY(hash);
    CLEAR_ARRAY(seHash);
    return ret;
}


int FlashWriteRsaPrimes(const uint8_t *data)
{
    if (data == NULL || GetCurrentAccountIndex() > 2 || GetIsTempAccount()) {
        return ERR_GENERAL_FAIL;
    }
    int32_t ret = ERR_GENERAL_FAIL;
    uint8_t seed[64] = {0}, fullData[SPI_FLASH_RSA_DATA_FULL_SIZE] = {0};
    uint8_t verifyBuf[SPI_FLASH_RSA_DATA_FULL_SIZE] = {0}, seHash[32] = {0};
    SimpleResponse_u8 *encrypted = NULL;
    do {
        ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
        if (ret != SUCCESS_CODE) {
            break;
        }
        int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
        encrypted = aes256_encrypt_primes(seed, len, (uint8_t *)data);
        ret = ERR_GENERAL_FAIL;
        if (encrypted == NULL || encrypted->error_code != SUCCESS_CODE || encrypted->data == NULL) {
            break;
        }
        if (memcpy_s(fullData, sizeof(fullData), encrypted->data, SPI_FLASH_RSA_DATA_SIZE) != 0) {
            break;
        }
        RsaHashWithSalt(data, fullData + SPI_FLASH_RSA_DATA_SIZE);
        uint32_t address = GetRsaAddress();
        ret = ERR_AR_STORAGE;
        if (Gd25FlashSectorErase(address) != SUCCESS_CODE ||
                Gd25FlashWriteBuffer(address, fullData, sizeof(fullData)) != sizeof(fullData) ||
                Gd25FlashReadBuffer(address, verifyBuf, sizeof(verifyBuf)) != sizeof(verifyBuf) ||
                memcmp(fullData, verifyBuf, sizeof(fullData)) != 0) {
            break;
        }
        if (SetRsaPrimesHash(GetCurrentAccountIndex(), fullData + SPI_FLASH_RSA_DATA_SIZE) != SUCCESS_CODE ||
                GetRsaPrimesHash(GetCurrentAccountIndex(), seHash) != SUCCESS_CODE ||
                timingsafe_bcmp(seHash, fullData + SPI_FLASH_RSA_DATA_SIZE, sizeof(seHash)) != 0) {
            break;
        }
        ret = SUCCESS_CODE;
    } while (0);
    if (encrypted != NULL) {
        if (encrypted->error_code == SUCCESS_CODE && encrypted->data != NULL) {
            memset_s(encrypted->data, SPI_FLASH_RSA_DATA_SIZE, 0, SPI_FLASH_RSA_DATA_SIZE);
        }
        free_simple_response_u8(encrypted);
    }
    CLEAR_ARRAY(seed);
    CLEAR_ARRAY(fullData);
    CLEAR_ARRAY(verifyBuf);
    CLEAR_ARRAY(seHash);
    return ret;
}
#endif
