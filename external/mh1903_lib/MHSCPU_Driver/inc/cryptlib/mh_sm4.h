/* sm4.h - header file for sm4.c
 */

#ifndef __MH_SM4_H
#define __MH_SM4_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include "mh_crypt.h"

#define MH_RET_SM4_SUCCESS                      (('S'<<24)|('4'<<16)|('S'<<8)|('U'))
#define MH_RET_SM4_FAILURE                      (('S'<<24)|('4'<<16)|('S'<<8)|('F'))
#define MH_RET_SM4_KEY_IS_NULL                  (('S'<<24)|('4'<<16)|('K'<<8)|('N'))
#define MH_RET_SM4_PACK_MODE_ERROR              (('S'<<24)|('4'<<16)|('P'<<8)|('E'))
#define MH_RET_SM4_OUTBUF_TOO_SMALL             (('S'<<24)|('4'<<16)|('O'<<8)|('S'))
#define MH_RET_SM4_INPUT_SIZE_ERROR             (('S'<<24)|('4'<<16)|('P'<<8)|('S'))
#define MH_RET_SM4_KEY_READBACK_ERROR           (('S'<<24)|('4'<<16)|('K'<<8)|('R'))
#define MH_RET_SM4_RESULT_CHECK_ERROR           (('S'<<24)|('4'<<16)|('R'<<8)|('C'))
#define MH_RET_SM4_NO_RNG_ERROR                 (('S'<<24)|('4'<<16)|('N'<<8)|('R'))
#define MH_RET_SM4_IV_ERROR                     (('S'<<24)|('4'<<16)|('I'<<8)|('V'))

#define MH_SM4_KEY_SIZE         (128 / 8)
#define MH_SM4_IV_SIZE          (128 / 8)

typedef uint8_t mh_sm4_key_def[MH_SM4_KEY_SIZE];
typedef uint8_t mh_sm4_iv_def[MH_SM4_IV_SIZE];

/**
  * @method mh_sm4_enc
  * @brief  sm4 encrypt function
  * @param  pack_mode       :block encryption mode ECB or CBC
  * @param  output          :the pointer of output data buffer
  * @param  obytes          :the size of output data buffer
                            ("obytes" must be bigger or equal then "iBytes")
  * @param  input           :the pointer of input data buffer
  * @param  ibytes          :the size of input data buffer
                            ("ibytes" must be an integer multiple of key length)
  * @param  key             :SM4 key
  * @param  iv              :initialization vector
  * @retval                 :SM4 return value
  */
uint32_t mh_sm4_enc(mh_pack_mode_def pack_mode,
                    uint8_t *output, uint32_t obytes,
                    uint8_t *input, uint32_t ibytes,
                    mh_sm4_key_def key, mh_sm4_iv_def iv,
                    mh_rng_callback f_rng, void *p_rng);

/**
  * @method mh_sm4_dec
  * @brief  SM4 decrypt function
  * @param  pack_mode       :block encryption mode ECB or CBC
  * @param  output          :the pointer of output data buffer
  * @param  obytes          :the size of output data buffer
                            ("obytes" must be bigger or equal then "iBytes")
  * @param  input           :the pointer of input data buffer
  * @param  ibytes          :the size of input data buffer
                            ("ibytes" must be an integer multiple of key length)
  * @param  key             :SM4 key
  * @param  iv              :initialization vector
  * @retval                 :SM4 return value
  */
uint32_t mh_sm4_dec(mh_pack_mode_def pack_mode,
                    uint8_t *output, uint32_t obytes,
                    uint8_t *input, uint32_t ibytes,
                    mh_sm4_key_def key, mh_sm4_iv_def iv,
                    mh_rng_callback f_rng, void *p_rng);

#ifdef __cplusplus
}
#endif

#endif
