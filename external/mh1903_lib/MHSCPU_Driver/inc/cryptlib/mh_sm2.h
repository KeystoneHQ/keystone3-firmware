#ifndef __MH_SM2_H
#define __MH_SM2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "mh_ecc.h"
#include "mh_sm3.h"
#include "mh_misc.h"
#include "mh_rand.h"

#define MH_SM2_KEY_BITS         256
#define MH_SM2_KEY_BYTES        (MH_SM2_KEY_BITS + 7) / 8
#define MH_SM2_KEY_WORDS        (MH_SM2_KEY_BYTES + 3) / 4

typedef struct {
    uint8_t r[MH_SM2_KEY_BYTES];
    uint8_t s[MH_SM2_KEY_BYTES];
} mh_sm2_sign;

typedef struct {
    uint8_t x[MH_SM2_KEY_BYTES];
    uint8_t y[MH_SM2_KEY_BYTES];
} mh_sm2_point;

typedef struct {
    uint8_t p[MH_SM2_KEY_BYTES];                //模数--由外部传入
    uint8_t a[MH_SM2_KEY_BYTES];                //椭圆曲线常数a--由外部传入
    uint8_t b[MH_SM2_KEY_BYTES];                //椭圆曲线常数b--由外部传入
    mh_sm2_point g;                             //基点--由外部传入
    uint8_t n[MH_SM2_KEY_BYTES];                //基点的阶--由外部传入
} mh_sm2_ellipse_para;

typedef mh_sm2_point mh_sm2_public_key;

typedef struct {
    uint8_t d[MH_SM2_KEY_BYTES];
    mh_sm2_public_key e;
} mh_sm2_private_key;


/******************************************
升腾提出函数接口要求：
********************************************/
typedef struct __SM2_SIGN {
    mh_sm2_sign sign;
    uint8_t *id;
    uint32_t id_lenth;
} sm2_sign;





#define MH_RET_SM2_ENC_SUCCESS              (('R'<<24)|('S'<<16)|('E'<<8)|('S'))
#define MH_RET_SM2_ENC_FAILURE              (('R'<<24)|('S'<<16)|('E'<<8)|('F'))
#define MH_RET_SM2_DEC_SUCCESS              (('R'<<24)|('S'<<16)|('D'<<8)|('S'))
#define MH_RET_SM2_DEC_FAILURE              (('R'<<24)|('S'<<16)|('D'<<8)|('F'))
#define MH_RET_SM2_KDF_SUCCESS              (('R'<<24)|('S'<<16)|('K'<<8)|('S'))
#define MH_RET_SM2_KDF_FAILURE              (('R'<<24)|('S'<<16)|('K'<<8)|('F'))
#define MH_RET_SM2_SIGN_SUCCESS             (('R'<<24)|('S'<<16)|('S'<<8)|('S'))
#define MH_RET_SM2_SIGN_FAILURE             (('R'<<24)|('S'<<16)|('S'<<8)|('F'))
#define MH_RET_SM2_SIGN_ERROR               (('R'<<24)|('S'<<16)|('S'<<8)|('E'))
#define MH_RET_SM2_VERIFY_SUCCESS           (('R'<<24)|('S'<<16)|('V'<<8)|('S'))
#define MH_RET_SM2_VERIFY_FAILURE           (('R'<<24)|('S'<<16)|('V'<<8)|('F'))
#define MH_RET_SM2_VERIFY_ERROR             (('R'<<24)|('S'<<16)|('V'<<8)|('E'))
#define MH_RET_SM2_GEN_ERROR                (MH_RET_ECC_KEY_GENERATION_FAILED)
#define MH_RET_SM2_GEN_SUCCESS              (MH_RET_ECC_KEY_GENERATION_SUCCESS)


uint32_t sm2_verifykey(mh_sm2_private_key *key, const mh_sm2_ellipse_para *para,
                       mh_rng_callback f_rng, void *p_rng);

uint32_t sm2_genkey(mh_sm2_private_key *key, const mh_sm2_ellipse_para *para,
                    mh_rng_callback f_rng, void *p_rng);
uint32_t sm2_completeKey(mh_sm2_private_key *key, const mh_sm2_ellipse_para *para,
                         mh_rng_callback f_rng, void *p_rng);

uint32_t mh_sm2_kdf(uint8_t *K, uint32_t klen, const uint8_t *Z, uint32_t zlen);
/**
  * @method mh_sm2_hash_z
  * @brief  get sm2 sign para Za
  * @param  Za          :the Za para of sm2 sign
  * @param  IDa         :user id
  * @param  IDalen      :the bits of user id (8 * (number of bytes))
  * @param  para        :sm2 paras
  * @param  key         :sm2 public key
  * @retval             :nil
  */
uint32_t mh_sm2_hash_z(uint8_t Za[32], uint8_t *IDa, uint16_t IDalen, mh_sm2_ellipse_para *para, mh_sm2_public_key *key);
/**
  * @method mh_sm2_hash_e
  * @brief  get sm2 sign para e
  * @param  e           :the e para of sm2 sign (e is already cover bytes to integer)
  * @param  Za          :the Za para of sm2 sign
  * @param  m           :message
  * @param  mlen        :bytes of message
  * @retval             :nil
  */
uint32_t mh_sm2_hash_e(uint32_t e[8], uint8_t Za[32], uint8_t *m, uint32_t mlen);

uint32_t mh_sm2_enc(uint8_t *output, uint32_t *olen,
                    const uint8_t *input, uint32_t ilen,
                    const mh_sm2_public_key *key, const mh_sm2_ellipse_para *para,
                    mh_rng_callback f_rng, void *p_rng);
uint32_t mh_sm2_dec(uint8_t *output, uint32_t *olen,
                    const uint8_t *input, uint32_t ilen,
                    const mh_sm2_private_key *key, const mh_sm2_ellipse_para *para,
                    mh_rng_callback f_rng, void *p_rng);

uint32_t mh_sm2_digital_sign(mh_sm2_sign *sign, uint8_t Za[32],
                             uint8_t *msg, uint32_t mlen,
                             mh_sm2_private_key *key, mh_sm2_ellipse_para *para,
                             mh_rng_callback f_rng, void *p_rng);
uint32_t mh_sm2_verify_sign(mh_sm2_sign *sign, uint8_t Za[32],
                            uint8_t *msg, uint32_t mlen,
                            mh_sm2_public_key *key, mh_sm2_ellipse_para *para,
                            mh_rng_callback f_rng, void *p_rng);
uint32_t mh_sm2_digital_sign_with_e(mh_sm2_sign *sign, uint32_t e[8],
                                    uint8_t *msg, uint32_t mlen,
                                    mh_sm2_private_key *key, mh_sm2_ellipse_para *para,
                                    mh_rng_callback f_rng, void *p_rng);
uint32_t mh_sm2_verify_sign_with_e(mh_sm2_sign *sign, uint32_t e[8],
                                   uint8_t *msg, uint32_t mlen,
                                   mh_sm2_public_key *key, mh_sm2_ellipse_para *para,
                                   mh_rng_callback f_rng, void *p_rng);

uint32_t mh_sm2_key_ex_equation_0(uint8_t _x[32], uint32_t *_xlen,
                                  const uint8_t x[32], uint32_t w);
void mh_sm2_key_ex_equation_1(uint8_t *t, const uint8_t *d,
                              const uint8_t *x, const uint8_t *r,
                              mh_sm2_ellipse_para *para);
void mh_sm2_key_ex_equation_2(mh_sm2_point *P, uint8_t *t,
                              uint8_t *x, mh_sm2_point *R,
                              mh_sm2_ellipse_para *para, mh_sm2_public_key *key,
                              mh_rng_callback f_rng, void *p_rng);

uint32_t mh_sm2_key_ex_section_0(mh_sm2_point *R, uint8_t *t, uint8_t *rand,
                                 mh_sm2_ellipse_para *para, mh_sm2_private_key *key,
                                 mh_rng_callback f_rng, void *p_rng);
uint32_t mh_sm2_key_ex_section_1(uint8_t *K, uint32_t Klen, mh_sm2_point *P,
                                 mh_sm2_point *R, uint8_t *t,
                                 uint8_t *Za, uint8_t *Zb,
                                 mh_sm2_ellipse_para *para, mh_sm2_public_key *key,
                                 mh_rng_callback f_rng, void *p_rng);
uint32_t mh_sm2_key_ex_hash(uint8_t *Hash, uint8_t HashHead,
                            mh_sm2_point *P, uint8_t *Za, uint8_t *Zb,
                            mh_sm2_point *Ra, mh_sm2_point *Rb);



int MH_SM2Sign(sm2_sign *info, uint8_t *msg, uint32_t mlen,
               mh_sm2_private_key *key, mh_sm2_ellipse_para *para,
               mh_rng_callback f_rng, void *p_rng);

int MH_SM2Verify(sm2_sign *info, uint8_t *msg, uint32_t mlen,
                 mh_sm2_public_key *key, mh_sm2_ellipse_para *para,
                 mh_rng_callback f_rng, void *p_rng);


#ifdef __cplusplus
}
#endif

#endif

