#ifndef _RSA_H
#define _RSA_H

#include "flash_address.h"
#include "stdio.h"
#include "define.h"
#include "stdlib.h"
#include "safe_mem_lib.h"
#include "user_memory.h"
#include "drv_gd25qxx.h"
#include "sha256.h"
#include "log_print.h"
#include "hash_and_salt.h"
#include "keystore.h"
#include "rust.h"
#include "assert.h"
#include "account_manager.h"

#define SPI_FLASH_RSA_ORIGIN_DATA_SIZE 512
#define SPI_FLASH_RSA_DATA_SIZE SPI_FLASH_RSA_ORIGIN_DATA_SIZE + 16
#define SPI_FLASH_RSA_HASH_SIZE 32
#define SPI_FLASH_RSA_DATA_FULL_SIZE SPI_FLASH_RSA_DATA_SIZE + SPI_FLASH_RSA_HASH_SIZE
#define SPI_FLASH_RSA_PRIME_SIZE SPI_FLASH_RSA_ORIGIN_DATA_SIZE / 2

typedef struct {
  uint8_t p[SPI_FLASH_RSA_PRIME_SIZE];
  uint8_t q[SPI_FLASH_RSA_PRIME_SIZE];
} Rsa_primes_t;

Rsa_primes_t *FlashReadRsaPrimes();
int FlashWriteRsaPrimes(const uint8_t *data);
bool HasGeneratedRsaPrimes();

#endif