/* aes.h - header file for aes.c
 */

#ifndef __MH_AES_H
#define __MH_AES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mh_crypt.h"

#define MH_RET_AES_SUCCESS                      (('R'<<24)|('A'<<16)|('S'<<8)|('C'))
#define MH_RET_AES_FAILURE                      (('R'<<24)|('A'<<16)|('F'<<8)|('A'))
#define MH_RET_AES_KEY_IS_NULL                  (('R'<<24)|('A'<<16)|('K'<<8)|('N'))
#define MH_RET_AES_KEY_SIZE_ERROR               (('R'<<24)|('A'<<16)|('K'<<8)|('S'))
#define MH_RET_AES_PACK_MODE_ERROR              (('R'<<24)|('A'<<16)|('P'<<8)|('E'))
#define MH_RET_AES_OUTBUF_TOO_SMALL             (('R'<<24)|('A'<<16)|('O'<<8)|('S'))
#define MH_RET_AES_INPUT_SIZE_ERROR             (('R'<<24)|('A'<<16)|('I'<<8)|('S'))
#define MH_RET_AES_NO_RNG_ERROR                 (('R'<<24)|('A'<<16)|('N'<<8)|('R'))
#define MH_RET_AES_KEY_READBACK_ERROR           (('R'<<24)|('A'<<16)|('R'<<8)|('B'))
#define MH_RET_AES_RESULT_CHECK_ERROR           (('R'<<24)|('A'<<16)|('C'<<8)|('E'))
#define MH_RET_AES_IV_ERROR                     (('R'<<24)|('A'<<16)|('I'<<8)|('V'))

#define MH_AES_IV_SIZE                          (128 / 8)

typedef enum {
    MH_AES_128  = 128,
    MH_AES_192 = 192,
    MH_AES_256 = 256
} mh_aes_key_size_def;

typedef uint8_t mh_aes_iv_def[MH_AES_IV_SIZE];

/**
  * @method mh_aes_enc
  * @brief  AES encrypt function
  * @param  pack_mode       :block encryption mode ECB or CBC
  * @param  output          :the pointer of output data buffer
  * @param  oBytes          :the size of output data buffer
                            ("oBytes" must be bigger or equal then "iBytes")
  * @param  input           :the pointer of input data buffer
  * @param  iBytes          :the size of input data buffer
                            ("iBytes" must be an integer multiple of key length)
  * @param  key             :AES key
  * @param  kSize           :the size of AES key: AES_128 AES_192 or AES_256
  * @param  iv              :initialization vector
  * @retval                 :AES return value
  */

uint32_t mh_aes_enc(mh_pack_mode_def pack_mode,
                    uint8_t *output, uint32_t obytes,
                    uint8_t *input, uint32_t ibytes,
                    uint8_t *key, mh_aes_key_size_def kSize,
                    mh_aes_iv_def iv,
                    mh_rng_callback f_rng, void *p_rng);



/**
  * @method AES_Decrypt
  * @brief  AES decrypt function
  * @param  pack_mode       :block encryption mode ECB or CBC
  * @param  output          :the pointer of output data buffer
  * @param  oBytes          :the size of output data buffer
                            ("oBytes" must be bigger or equal then "iBytes")
  * @param  input           :the pointer of input data buffer
  * @param  iBytes          :the size of input data buffer
                            ("iBytes" must be an integer multiple of key length)
  * @param  key             :AES key
  * @param  kSize           :the size of AES key: AES_128 AES_192 or AES_256
  * @param  iv              :initialization vector
  * @retval                 :AES return value
  */
uint32_t mh_aes_dec(mh_pack_mode_def pack_mode,
                    uint8_t *output, uint32_t oBytes,
                    uint8_t *input, uint32_t iBytes,
                    uint8_t *key, mh_aes_key_size_def kSize,
                    mh_aes_iv_def iv,
                    mh_rng_callback f_rng, void *p_rng);

#ifdef __cplusplus
}
#endif

#endif
