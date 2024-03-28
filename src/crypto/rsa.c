#include "rsa.h"
#include "keystore.h"

static uint32_t GetRsaAddress()
{
  switch (GetCurrentAccountIndex())
  {
  case 0:
    return SPI_FLASH_RSA_USER1_DATA;
    break;
  case 1:
    return SPI_FLASH_RSA_USER2_DATA;
    break;
  case 2:
    return SPI_FLASH_RSA_USER3_DATA;
    break;
  }
  return SPI_FLASH_RSA_USER1_DATA;
}

static bool HasMatchingPrimesHash(Rsa_primes_t *primes, const uint8_t hash[SPI_FLASH_RSA_HASH_SIZE])
{
  struct sha256 sha;
  uint8_t bytes[SPI_FLASH_RSA_DATA_SIZE];
  memcpy_s(bytes, SPI_FLASH_RSA_DATA_SIZE, primes->p, SPI_FLASH_RSA_PRIME_SIZE);
  memcpy_s(bytes + SPI_FLASH_RSA_PRIME_SIZE, SPI_FLASH_RSA_DATA_SIZE, primes->q, SPI_FLASH_RSA_PRIME_SIZE);

  sha256(&sha, bytes, SPI_FLASH_RSA_DATA_SIZE);

  return memcmp(sha.u.u8, hash, SPI_FLASH_RSA_HASH_SIZE) == 0;
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
  if (!data)
  {
    return -1;
  }

  uint8_t fullData[SPI_FLASH_RSA_DATA_FULL_SIZE];
  memcpy_s(fullData, SPI_FLASH_RSA_DATA_FULL_SIZE, data, SPI_FLASH_RSA_DATA_SIZE);

  struct sha256 sha;
  sha256(&sha, data, SPI_FLASH_RSA_DATA_SIZE);
  memcpy_s(fullData + SPI_FLASH_RSA_DATA_SIZE, SPI_FLASH_RSA_DATA_FULL_SIZE, sha.u.u8, SPI_FLASH_RSA_HASH_SIZE);

  Gd25FlashSectorErase(GetRsaAddress());
  int32_t ret = Gd25FlashWriteBuffer(GetRsaAddress(), fullData, sizeof(fullData));

  if (ret != 0)
  {
    printf("Flash write failed with error code: %d\n", ret);
    return -1;
  }

  return 0;
}