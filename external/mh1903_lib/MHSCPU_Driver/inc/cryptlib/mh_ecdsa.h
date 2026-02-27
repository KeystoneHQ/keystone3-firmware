#ifndef __MH_ECDSA_H
#define __MH_ECDSA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "mh_ecc.h"
#include "mh_sm2.h"
#include "mh_sm3.h"
#include "mh_misc.h"
#include "mh_rand.h"



typedef struct {
    ecc_bignum_key eb_r;
    ecc_bignum_key eb_s;
} mh_ecc_sign;



/*************************************************
* @method mh_ecdsa_digital_sign
* @brief  perform ecdsa signature.
* @param  pSign           :out signature result
* @param  pBigE           :input message hash value
* @param  pKey            :private key
* @param  pEccPara        :ecc curve param
* @param  f_rng           :true random number generation function pointer
* @param  p_rng           :true random number generation para
* @retval                 :MH_RET_ECC_SIGN_SUCCESS,MH_RET_ECC_PARAM_ERR,MH_RET_ECC_POINT_FAILED
*************************************************/
uint32_t mh_ecdsa_digital_sign(mh_ecc_sign *pSign, ecc_bignum_key *pBigE, mh_ecc_private_key *pKey, mh_ecc_curve_para *pEccPara,
                               mh_rng_callback f_rng, void *p_rng);

/*************************************************
* @method mh_ecdsa_digital_sign
* @brief  perform ecdsa verification.
* @param  pSign           :input signature result
* @param  pBigE           :input message hash value
* @param  pKey            :public key
* @param  pEccPara        :ecc curve param
* @param  f_rng           :true random number generation function pointer
* @param  p_rng           :true random number generation para
* @retval                 :MH_RET_ECC_VERIFY_SUCCESS,MH_RET_ECC_PARAM_ERR,MH_RET_ECC_VERIGY_FAILURE
*************************************************/
uint32_t mh_ecdsa_verify_sign(mh_ecc_sign *pSign, ecc_bignum_key *pBigE, mh_ecc_public_key *pubKey,
                              mh_ecc_curve_para *pEccPara, mh_rng_callback f_rng, void *p_rng);

/*************************************************
* @method mh_ecc_hash_e
* @brief  get hash value of message.
* @param  pBigE             :ouput hash value
* @param  pBigMsg           :input message
* @param  para              :ecc curve param
* @retval                 :MH_RET_ECC_SUCCESS,MH_RET_ECC_PARAM_ERR
*************************************************/
uint32_t mh_ecc_hash_e(ecc_bignum_key *pBigE, ecc_bignum_data *pBigMsg, mh_ecc_curve_para *para);
#ifdef __cplusplus
}
#endif

#endif

