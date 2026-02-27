#ifndef __MH_ECDH_H
#define __MH_ECDH_H

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
#include "mh_sm2.h"

uint32_t ecdh_get_key_A(mh_sm2_private_key *key, mh_sm2_ellipse_para *para, mh_rng_callback f_rng, void *p_rng);
uint32_t mh_ecdh_ex_A(uint8_t *K, mh_sm2_public_key *keyb, uint8_t *smsg, uint32_t slen,
                      mh_sm2_private_key *key, mh_sm2_ellipse_para *para,
                      mh_rng_callback f_rng, void *p_rng);
uint32_t mh_ecdh_ex_B(uint8_t*K, mh_sm2_public_key *key,
                      uint8_t *smsg, uint32_t slen,
                      mh_sm2_public_key *keya, mh_sm2_ellipse_para *para,
                      mh_rng_callback f_rng, void *p_rng);


#ifdef __cplusplus
}
#endif

#endif

