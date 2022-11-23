#ifndef __MH_MISC_H
#define __MH_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "mh_crypt.h"

#define MH_RET_MEM_CPY_SUCCESS          (('R'<<24)|('M'<<16)|('C'<<8)|('S'))
#define MH_RET_MEM_CPY_FAILURE          (('R'<<24)|('M'<<16)|('C'<<8)|('F'))
#define MH_RET_MEM_SWAP_SUCCESS         (('R'<<24)|('M'<<16)|('S'<<8)|('S'))
#define MH_RET_MEM_SWAP_FAILURE         (('R'<<24)|('M'<<16)|('S'<<8)|('F'))
#define MH_RET_XOR_SUCCESS              (('R'<<24)|('M'<<16)|('X'<<8)|('S'))
#define MH_RET_XOR_FAILURE              (('R'<<24)|('M'<<16)|('X'<<8)|('F'))
#define MH_RET_SUM_SUCCESS              (('R'<<24)|('M'<<16)|('A'<<8)|('S'))
#define MH_RET_SUM_FAILURE              (('R'<<24)|('M'<<16)|('A'<<8)|('F'))

void mh_swap_u32(uint32_t *dst, const uint32_t *src);
uint32_t mh_is_equal_swap_u32(uint32_t *dst, const uint32_t *src);
/**
  * @method mh_memcmp
  * @brief  memory compare function
  * @param  dest    :destination
  * @param  src     :source
  * @param  size    :the byte count of memory
  * @retval         :dest > src = BIGGER; src < dest = SMALLER; src == dest = EQUAL
                     BIGGER/SMALLER/EQUAL are defined in mh_crypt.h
  */
uint32_t mh_memcmp(const void *dest, const void *src, uint32_t size);

/**
  * @method mh_is_equal
  * @brief  memory compare function
  * @param  dest    :destination
  * @param  src     :source
  * @param  size    :the byte count of memory
  * @retval         :dest > src = BIGGER; src < dest = SMALLER; src == dest = EQUAL
                     BIGGER/SMALLER/EQUAL are defined in mh_crypt.h
  */
uint32_t mh_is_equal(const void *dest, const void *src, uint32_t size,
                     mh_rng_callback f_rng, void *p_rng);

/**
  * @method mh_memcpy
  * @brief  memory copy function
  * @param  dest    :destination
  * @param  src     :source
  * @param  size    :the byte count of memory
  * @retval         :MH_RET_MEM_CPY_SUCCESS or MH_RET_MEM_CPY_FAILURE
  */
uint32_t mh_memcpy(void *dest, const void *src, uint16_t size,
                   mh_rng_callback f_rng, void *p_rng);

/**
  * @method MEM_Endian(safe function)
  * @brief  memory copy and swap the data endian:
            the endian from big to little or little to big
            the size of dest must be equal or bigger than size of src
  * @param  dest    :destination
  * @param  dsize   :the byte count of dest memory
  * @param  src     :source
  * @param  ssize   :the byte count of src memory
  * @retval         :MH_RET_MEM_ENDIAN_SUCCESS or MH_RET_MEM_ENDIAN_FAILURE
  */
uint32_t mh_memswap(void *dest, uint32_t dsize, const void *src, uint32_t ssize,
                    mh_rng_callback f_rng, void *p_rng);


uint32_t memswap(void *dest, uint32_t dsize, const void *src, uint32_t ssize);

uint32_t mh_xor_u32(uint32_t *xor, uint32_t *src, uint16_t nwords, mh_rng_callback f_rng, void *p_rng);

uint32_t mh_sum_u32(uint32_t *sum, uint32_t *src, uint16_t nwords, mh_rng_callback f_rng, void *p_rng);

uint32_t mh_memswap_byte(void *dst, uint32_t dbytes, const void *src, uint32_t sbytes, mh_rng_callback f_rng, void *p_rng);
#ifdef __cplusplus
}
#endif

#endif
