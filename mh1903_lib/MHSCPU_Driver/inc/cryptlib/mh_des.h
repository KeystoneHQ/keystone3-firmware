/* des.h - header file for des.c
 */

#ifndef __MH_DES_H
#define __MH_DES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mh_crypt.h"

#define MH_RET_DES_SUCCESS                      (('R'<<24)|('D'<<16)|('S'<<8)|('C'))
#define MH_RET_DES_FAILURE                      (('R'<<24)|('D'<<16)|('F'<<8)|('A'))
#define MH_RET_DES_KEY_IS_NULL                  (('R'<<24)|('D'<<16)|('K'<<8)|('N'))
#define MH_RET_DES_KEY_CRC_ERROR                (('R'<<24)|('D'<<16)|('K'<<8)|('C'))
#define MH_RET_DES_KEY_READBACK_ERROR           (('R'<<24)|('D'<<16)|('K'<<8)|('R'))
#define MH_RET_DES_IV_CRC_ERROR                 (('R'<<24)|('D'<<16)|('I'<<8)|('C'))
#define MH_RET_DES_PACK_MODE_ERROR              (('R'<<24)|('D'<<16)|('P'<<8)|('E'))
#define MH_RET_DES_OUTBUF_TOO_SMALL             (('R'<<24)|('D'<<16)|('O'<<8)|('S'))
#define MH_RET_DES_INPUT_SIZE_ERROR             (('R'<<24)|('D'<<16)|('I'<<8)|('S'))
#define MH_RET_DES_RESULT_CHECK_ERROR           (('R'<<24)|('D'<<16)|('R'<<8)|('C'))
#define MH_RET_DES_NO_RNG_ERROR                 (('R'<<24)|('D'<<16)|('N'<<8)|('R'))

#define MH_DES_KEY_SIZE                         (64 / 8)
#define MH_DES_IV_SIZE                          (64 / 8)

typedef uint8_t mh_des_key_def[MH_DES_KEY_SIZE];
typedef uint8_t mh_des_iv_def[MH_DES_IV_SIZE];

typedef struct {
    mh_des_key_def k1;
    mh_des_key_def k2;
    mh_des_key_def k3;
} mh_tdes_key_def;

/**
  * @method mh_des_enc
  * @brief  DES encrypt function
  * @param  pack_mode       :block encryption mode ECB or CBC
  * @param  output          :the pointer of output data buffer
  * @param  obytes          :the size of output data buffer
                            ("oBytes" must be bigger or equal then "iBytes")
  * @param  input           :the pointer of input data buffer
  * @param  ibytes          :the size of input data buffer
                            ("iBytes" must be an integer multiple of key length)
  * @param  key             :DES key
  * @param  iv              :initialization vector
  * @param  f_rng           :true random number generation function point
  * @param  p_rng           :true random number generation para
  * @retval                 :DES return value
  */

uint32_t mh_des_enc(mh_pack_mode_def pack_mode,
                    uint8_t *output, uint32_t obytes,
                    uint8_t *input, uint32_t ibytes,
                    mh_des_key_def key, mh_des_iv_def iv,
                    mh_rng_callback f_rng, void *p_rng);

/**
  * @method DES_Decrypt
  * @brief  DES decrypt function
  * @param  pack_mode       :block encryption mode ECB or CBC
  * @param  output          :the pointer of output data buffer
  * @param  oBytes          :the size of output data buffer
                            ("oBytes" must be bigger or equal then "iBytes")
  * @param  input           :the pointer of input data buffer
  * @param  iBytes          :the size of input data buffer
                            ("iBytes" must be an integer multiple of key length)
  * @param  key             :DES key
  * @param  iv              :initialization vector
  * @param  f_rng           :true random number generation function point
  * @param  p_rng           :true random number generation para
  * @retval                 :DES return value
  */
uint32_t mh_des_dec(mh_pack_mode_def pack_mode,
                    uint8_t *output, uint32_t obytes,
                    uint8_t *input, uint32_t ibytes,
                    mh_des_key_def key, mh_des_iv_def iv,
                    mh_rng_callback f_rng, void *p_rng);

/**
  * @method TDES_Encrypt
  * @brief  TDES encrypt function
  * @param  pack_mode       :block encryption mode ECB or CBC
  * @param  output          :the pointer of output data buffer
  * @param  oBytes          :the size of output data buffer
                            ("oBytes" must be bigger or equal then "iBytes")
  * @param  input           :the pointer of input data buffer
  * @param  iBytes          :the size of input data buffer
                            ("iBytes" must be an integer multiple of key length)
  * @param  key             :DES key
  * @param  iv              :initialization vector
  * @param  f_rng           :true random number generation function point
  * @param  p_rng           :true random number generation para
  * @retval                 :TDES return value
  */
uint32_t mh_tdes_enc(mh_pack_mode_def pack_mode,
                     uint8_t *output, uint32_t obytes,
                     uint8_t *input, uint32_t ibytes,
                     mh_tdes_key_def *key, mh_des_iv_def iv,
                     mh_rng_callback f_rng, void *p_rng);
/**
  * @method TDES_Decrypt
  * @brief  TDES decrypt function
  * @param  pack_mode       :block encryption mode ECB or CBC
  * @param  output          :the pointer of output data buffer
  * @param  oBytes          :the size of output data buffer
                            ("oBytes" must be bigger or equal then "iBytes")
  * @param  input           :the pointer of input data buffer
  * @param  iBytes          :the size of input data buffer
                            ("iBytes" must be an integer multiple of key length)
  * @param  key             :DES key
  * @param  iv              :initialization vector
  * @param  f_rng           :true random number generation function point
  * @param  p_rng           :true random number generation para
  * @retval                 :TDES return value
  */
uint32_t mh_tdes_dec(mh_pack_mode_def pack_mode,
                     uint8_t *output, uint32_t obytes,
                     uint8_t *input, uint32_t ibytes,
                     mh_tdes_key_def *key, mh_des_iv_def iv,
                     mh_rng_callback f_rng, void *p_rng);

#ifdef __cplusplus
}
#endif

#endif
