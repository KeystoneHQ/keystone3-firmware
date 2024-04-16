#include "rsa.h"

static uint32_t GetRsaAddress()
{
  switch (GetCurrentAccountIndex())
  {
  case 0:
    return SPI_FLASH_RSA_USER1_DATA;
  case 1:
    return SPI_FLASH_RSA_USER2_DATA;
  case 2:
    return SPI_FLASH_RSA_USER3_DATA;
  default:
    return SPI_FLASH_RSA_USER1_DATA;
  }
}

static void RsaHashWithSalt(const uint8_t *data, uint8_t *hash)
{
  uint8_t mfp[4] = {0};
  GetMasterFingerPrint(mfp);
  char hexString[2 * sizeof(mfp) + 1];
  char *hexPtr = hexString;
  for (size_t i = 0; i < sizeof(mfp); ++i)
  {
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
  int ret = memcmp(sourceHash, targethash, SPI_FLASH_RSA_HASH_SIZE) == 0;
  SRAM_FREE(sourceHash);
  return ret;
}

bool HasGeneratedRsaPrimes()
{
  Gd25FlashSectorErase(GetRsaAddress());
  Rsa_primes_t *primes = FlashReadRsaPrimes();
  bool ret = false;
  if (primes != NULL)
  {
    SRAM_FREE(primes);
    ret = true;
  }
  return ret;
}

Rsa_primes_t *FlashReadRsaPrimes()
{
  Rsa_primes_t *primes = SRAM_MALLOC(sizeof(Rsa_primes_t));
  if (!primes)
  {
    printf("Failed to allocate memory for RSA primes\n");
    SRAM_FREE(primes);
    return NULL;
  }

  uint8_t fullData[SPI_FLASH_RSA_DATA_FULL_SIZE];
  Gd25FlashReadBuffer(GetRsaAddress(), fullData, sizeof(fullData));
  uint8_t seed[64];
  int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
  int32_t ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
  if (ret != 0)
  {
    SRAM_FREE(primes);
    return NULL;
  }
  uint8_t cryptData[SPI_FLASH_RSA_DATA_SIZE];
  memcpy_s(cryptData, SPI_FLASH_RSA_DATA_SIZE, fullData, SPI_FLASH_RSA_DATA_SIZE);
  SimpleResponse_u8 *encData = aes256_decrypt_primes(seed, len, cryptData);
  if (encData->error_code != 0)
  {
    SRAM_FREE(primes);
    return NULL;
  }
  memcpy_s(primes->p, SPI_FLASH_RSA_PRIME_SIZE, encData->data, SPI_FLASH_RSA_PRIME_SIZE);
  memcpy_s(primes->q, SPI_FLASH_RSA_PRIME_SIZE, encData->data + SPI_FLASH_RSA_PRIME_SIZE, SPI_FLASH_RSA_PRIME_SIZE);

  uint8_t hash[SPI_FLASH_RSA_HASH_SIZE];
  memcpy_s(hash, SPI_FLASH_RSA_HASH_SIZE, fullData + SPI_FLASH_RSA_DATA_SIZE, SPI_FLASH_RSA_HASH_SIZE);
  if (!HasMatchingPrimesHash(primes, hash))
  {
    SRAM_FREE(primes);
    return NULL;
  }

  return primes;
}

int FlashWriteRsaPrimes(const uint8_t *data)
{
  uint8_t fullData[SPI_FLASH_RSA_DATA_FULL_SIZE];
  uint8_t seed[64];
  int len = GetMnemonicType() == MNEMONIC_TYPE_BIP39 ? sizeof(seed) : GetCurrentAccountEntropyLen();
  int32_t seed_ret = GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
  ASSERT(seed_ret == 0);
  SimpleResponse_u8 *cryptData = aes256_encrypt_primes(seed, len, data);

  memcpy_s(fullData, SPI_FLASH_RSA_DATA_SIZE, cryptData->data, SPI_FLASH_RSA_DATA_SIZE);
  uint8_t *hash = SRAM_MALLOC(SPI_FLASH_RSA_HASH_SIZE);
  RsaHashWithSalt(data, hash);
  memcpy_s(fullData + SPI_FLASH_RSA_DATA_SIZE, SPI_FLASH_RSA_HASH_SIZE, hash, SPI_FLASH_RSA_HASH_SIZE);
  SRAM_FREE(hash);
  Gd25FlashSectorErase(GetRsaAddress());
  int32_t ret = Gd25FlashWriteBuffer(GetRsaAddress(), fullData, sizeof(fullData));

  if (ret != SPI_FLASH_RSA_DATA_FULL_SIZE)
  {
    printf("Flash write failed with error code: %d\n", ret);
    return -1;
  }

  return 0;
}