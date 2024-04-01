#include "rsa.h"
#include "keystore.h"

static uint32_t GetRsaAddress() {
  switch (GetCurrentAccountIndex()) {
    case 0: return SPI_FLASH_RSA_USER1_DATA;
    case 1: return SPI_FLASH_RSA_USER2_DATA;
    case 2: return SPI_FLASH_RSA_USER3_DATA;
    default: return SPI_FLASH_RSA_USER1_DATA;
  }
}

static void GetDobuleHashOfPrimes(const uint8_t *data, uint8_t *hash)
{
  struct sha256 sha;
  struct sha256 dobuleHash;
  uint8_t mfp[4] = {0};
  GetMasterFingerPrint(mfp);
  sha256(&sha, data, SPI_FLASH_RSA_DATA_SIZE);
  uint8_t sha_u_u8_expanded[SPI_FLASH_RSA_HASH_SIZE + 4] = {0};
  memcpy_s(sha_u_u8_expanded, SPI_FLASH_RSA_HASH_SIZE, sha.u.u8, SPI_FLASH_RSA_HASH_SIZE);
  memcpy_s(sha_u_u8_expanded + SPI_FLASH_RSA_HASH_SIZE, 4, mfp, 4);
  sha256(&dobuleHash, sha_u_u8_expanded, SPI_FLASH_RSA_HASH_SIZE + 4);
  memcpy_s(hash, SPI_FLASH_RSA_HASH_SIZE, dobuleHash.u.u8, SPI_FLASH_RSA_HASH_SIZE);
}

static bool HasMatchingPrimesHash(Rsa_primes_t *primes, const uint8_t targethash[SPI_FLASH_RSA_HASH_SIZE])
{
  uint8_t bytes[SPI_FLASH_RSA_DATA_SIZE];
  memcpy_s(bytes, SPI_FLASH_RSA_PRIME_SIZE, primes->p, SPI_FLASH_RSA_PRIME_SIZE);
  memcpy_s(bytes + SPI_FLASH_RSA_PRIME_SIZE, SPI_FLASH_RSA_PRIME_SIZE, primes->q, SPI_FLASH_RSA_PRIME_SIZE);
  uint8_t *sourceHash = SRAM_MALLOC(SPI_FLASH_RSA_HASH_SIZE);
  GetDobuleHashOfPrimes(bytes, sourceHash);
  int ret = memcmp(sourceHash, targethash, SPI_FLASH_RSA_HASH_SIZE) == 0;
  SRAM_FREE(sourceHash);
  return ret;
}

Rsa_primes_t *FlashReadRsaPrimes()
{
  Rsa_primes_t *primes = SRAM_MALLOC(sizeof(Rsa_primes_t));
  if (!primes)
  {
    printf("Failed to allocate memory for RSA primes\n");
    return NULL;
  }

  uint8_t fullData[SPI_FLASH_RSA_DATA_FULL_SIZE];
  Gd25FlashReadBuffer(GetRsaAddress(), fullData, sizeof(fullData));

  memcpy_s(primes->p, SPI_FLASH_RSA_PRIME_SIZE, fullData, SPI_FLASH_RSA_PRIME_SIZE);
  memcpy_s(primes->q, SPI_FLASH_RSA_PRIME_SIZE, fullData + SPI_FLASH_RSA_PRIME_SIZE, SPI_FLASH_RSA_PRIME_SIZE);

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
  memcpy_s(fullData, SPI_FLASH_RSA_DATA_SIZE, data, SPI_FLASH_RSA_DATA_SIZE);
  uint8_t *hash = SRAM_MALLOC(SPI_FLASH_RSA_HASH_SIZE);
  GetDobuleHashOfPrimes(data, hash);
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