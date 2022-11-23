#ifndef __MH_BIGNUM_H
#define __MH_BIGNUM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "mh_crypt.h"
#define MH_RET_CRYPT_SUCCESS                    (0)
#define MH_RET_CRYPT_FUNCTION_ID_ERR            (1)
#define MH_RET_CRYPT_LENGTH_ERR                 (2)
#define MH_RET_GCD_ISNOT_ONE                    (3)
#define MH_RET_OVERFLOW                         (4)
#define MH_RET_PARAM_ERR                        (5)

#define MH_RET_CRYPT_SUCCESS_WITH_OVERFLOW      (6)

typedef struct {
    uint32_t *p;
    uint32_t *p_c;
    uint32_t p_q;
} bn_type_p;

/*
 *  get sizeof bignum
 */

void bn_set_int(uint32_t *r, uint32_t a, uint32_t len);
void bn_cpy(uint32_t *r, uint32_t *a, uint32_t len);
uint32_t bn_cmp(uint32_t *a, uint32_t *b, uint32_t len);
uint32_t bn_swap(uint32_t *a, uint32_t *b, uint32_t len);
uint32_t bn_cmp_int(uint32_t *a, uint32_t b, uint32_t len);

int32_t bn_add(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len);
int32_t bn_sub(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len);
int32_t bn_add_int(uint32_t *r, uint32_t *a, uint32_t b, uint32_t len);
int32_t bn_sub_int(uint32_t *r, uint32_t *a, uint32_t b, uint32_t len);
uint32_t bn_modadd(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t *p, uint32_t len);
uint32_t bn_modsub(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t *p, uint32_t len);
uint32_t bn_mul(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len_a, uint32_t len_b);
uint32_t bn_div(uint32_t *r_q, uint32_t *r_r, uint32_t *a, uint32_t *b, uint32_t len_a, uint32_t len_b);


void bn_set_p(bn_type_p *type_p, uint32_t *p, uint32_t *p_c, uint32_t p_q);

uint32_t bn_modexp_p(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len_b, bn_type_p *p, uint32_t len);
uint32_t bn_get_C(uint32_t *r, uint32_t *p, uint32_t len);
uint32_t bn_get_Q(uint32_t *r, uint32_t *p);
uint32_t bn_modmul_p(uint32_t *r, uint32_t *a, uint32_t *b, bn_type_p *p, uint32_t len);
uint32_t bn_modmul(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len);
uint32_t bn_modinv_1024(uint32_t *r, uint32_t *a, uint32_t *p, uint32_t len_a, uint32_t len_b);
uint32_t bn_modinv(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len_a, uint32_t len_b);

uint32_t bn_s_cpy(uint32_t *r, uint32_t *a, uint32_t len, mh_rng_callback f_rng, void *p_rng);

uint32_t bn_s_mul(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len_a, uint32_t len_b, mh_rng_callback f_rng, void *p_rng);
uint32_t bn_s_div(uint32_t *r_q, uint32_t *r_r, uint32_t *a, uint32_t *b, uint32_t len_a, uint32_t len_b, mh_rng_callback f_rng, void *p_rng);

uint32_t bn_s_modexp_p(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len_b, bn_type_p *p, uint32_t len, mh_rng_callback f_rng, void *p_rng);
uint32_t bn_s_get_C(uint32_t *r, uint32_t *p, uint32_t len, mh_rng_callback f_rng, void *p_rng);
uint32_t bn_s_get_Q(uint32_t *r, uint32_t *p, mh_rng_callback f_rng, void *p_rng);
uint32_t bn_s_modmul_p(uint32_t *r, uint32_t *a, uint32_t *b, bn_type_p *p, uint32_t len, mh_rng_callback f_rng, void *p_rng);
uint32_t bn_s_modmul(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len, mh_rng_callback f_rng, void *p_rng);
uint32_t bn_s_modinv_1024(uint32_t *r, uint32_t *a, uint32_t *p, uint32_t len_a, uint32_t len_b, mh_rng_callback f_rng, void *p_rng);


uint32_t bn_get_C_f2m(uint32_t *r, uint32_t *p, uint32_t len);
uint32_t bn_get_Q_f2m(uint32_t *r, uint32_t *a);
uint32_t bn_modmul_p_f2m(uint32_t *r, uint32_t *a, uint32_t *b, bn_type_p *p, uint32_t len);
uint32_t bn_modmul_f2m(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len);
uint32_t bn_modinv_f2m(uint32_t *r, uint32_t *a, uint32_t *p, uint32_t len);


uint32_t bn_gcd(uint32_t *a, uint32_t *b, uint32_t len);
uint32_t bn_gcd_int(uint32_t a, uint32_t *b, uint32_t len);

#define bn_size(bn) (sizeof(bn)/sizeof(uint32_t))
void bn_shift_r(uint32_t *r, uint32_t *a, uint32_t len, uint32_t bits);
void bn_shift_l(uint32_t *r, uint32_t *a, uint32_t len, uint32_t bits);
uint32_t bn_lsb(const uint32_t *a, uint32_t len);
uint32_t bn_msb(const uint32_t *a, uint32_t len);
uint32_t bn_byte(const uint32_t *a, uint32_t len);
uint32_t bn_len(const uint32_t *a, uint32_t len);
uint32_t bn_xor(uint32_t *r, uint32_t *a, uint32_t *b, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif

