#ifndef __MH_CALC_ECC_H
#define __MH_CALC_ECC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mh_bignum.h"

#define MH_ECC_CURVE_BIT    (521)
#define MH_ECC_BYTE_SIZE    ((MH_ECC_CURVE_BIT + 7)>>3)
#define MH_ECC_WORD_SIZE    ((MH_ECC_BYTE_SIZE + 3)>>2)
typedef struct {
    uint32_t *x;
    uint32_t *y;
    uint32_t *_y;
} ecc_point_naf;
typedef struct {
    uint32_t *x;  //x轴坐标
    uint32_t *y;  //y轴坐标
} ecc_point_a; //仿射坐标
typedef struct {
    uint32_t *x;  //x轴坐标
    uint32_t *y;  //y轴坐标
    uint32_t *z;  //z轴坐标
} ecc_point_j; //雅可比坐标
typedef struct {
    uint32_t *x;  //x轴坐标
    uint32_t *y;  //y轴坐标
    uint32_t *z;  //z轴坐标
    uint32_t *t;  //t轴坐标
} ecc_point_mj; //修正雅可比坐标
typedef struct {
    uint32_t *p;                //模数--由外部传入
    uint32_t *a;                //椭圆曲线常数a--由外部传入
    uint32_t *b;                //椭圆曲线常数b--由外部传入
    ecc_point_a g;              //基点--由外部传入
    uint32_t *n;                //基点的阶--由外部传入
    uint32_t *p_c;              //以n为取模的常量c
    uint32_t p_q;               //以n为取模的调整因子q
    uint32_t *n_c;              //以p为取模的常量c
    uint32_t n_q;               //以p为取模的调整因子q
    uint32_t len_bits;          //曲线位数,比特
    uint32_t len_words;         //曲线长度,字
    uint32_t field;             //域
    uint32_t a_type;            //判断a是否等于-3 MH_ECC_A_IS_NEGATIVE_3 或 MH_ECC_A_NOT_NEGATIVE_3--由外部传入
} ecc_para;                     //ecc输入参数的数据结构
typedef struct {
    uint32_t *d;        //ecc私钥
    ecc_point_a e;      //ecc公钥
} ecc_key; //ecc密钥的数据结构


//add by gaoke

typedef struct {
    uint8_t *pu8Buf;
    uint32_t u32Len;
} ecc_bignum_data;

typedef struct {
    uint32_t *pu32Buf;
    uint32_t u32Len;
} ecc_bignum_key;

typedef struct {
    ecc_bignum_key x;
    ecc_bignum_key y;
} mh_ecc_public_key;

typedef struct {
    ecc_bignum_key d;
} mh_ecc_private_key;

typedef struct {
    uint32_t x[MH_ECC_WORD_SIZE];
    uint32_t y[MH_ECC_WORD_SIZE];
} mh_ecc_point_value;

typedef struct {
    uint32_t p[MH_ECC_WORD_SIZE];
    uint32_t a[MH_ECC_WORD_SIZE];
    uint32_t b[MH_ECC_WORD_SIZE];
    mh_ecc_point_value g;
    uint32_t n[MH_ECC_WORD_SIZE];
    uint32_t p_c[MH_ECC_WORD_SIZE];
    uint32_t n_c[MH_ECC_WORD_SIZE];
} mh_ecc_curve_para_inner;

typedef struct {
    uint32_t u32CurveBits;
    uint32_t a_type;            //判断a是否等于-3 MH_ECC_A_IS_NEGATIVE_3 或 MH_ECC_A_NOT_NEGATIVE_3
    uint32_t p[MH_ECC_WORD_SIZE];
    uint32_t a[MH_ECC_WORD_SIZE];
    uint32_t b[MH_ECC_WORD_SIZE];
    uint32_t n[MH_ECC_WORD_SIZE];
    mh_ecc_point_value g;
} mh_ecc_curve_para;

//add by gaoke end

//macro define
#define MH_ECC_P192 0
#define MH_ECC_P224 1
#define MH_ECC_P256 2
#define MH_ECC_B163 3
#define MH_ECC_B193 4
#define MH_ECC_B233 5

#define MH_ECC_PRIME                            (('E'<<8)|('P'))
#define MH_ECC_BINARY                           (('E'<<8)|('B'))
#define MH_ECC_A_IS_NEGATIVE_3                  (('A'<<24)|('I'<<16)|('N'<<8)|('3'))
#define MH_ECC_A_NOT_NEGATIVE_3                 (('A'<<24)|('N'<<16)|('N'<<8)|('3'))
#define MH_ECC_SCALAR_IS_EVEN                   (('S'<<16)|('I'<<8)|('E'))
#define MH_ECC_SCALAR_IS_ODD                    (('S'<<16)|('I'<<8)|('O'))
#define MH_ECC_EMBEDED_PUBLIC_KEY_VERIFY        (('E'<<24)|('P'<<16)|('K'<<8)|('V'))
#define MH_ECC_COMMON_PUBLIC_KEY_VERIFY         (('C'<<24)|('P'<<16)|('K'<<8)|('V'))

#define MH_RET_ECC_POINT_SUCCESS                (('E'<<24)|('P'<<16)|('S'<<8)|('U'))
#define MH_RET_ECC_POINT_FAILED                 (('E'<<24)|('P'<<16)|('F'<<8)|('A'))
#define MH_RET_ECC_POINT_ADD_ERROR              (('E'<<24)|('P'<<16)|('A'<<8)|('E'))
#define MH_RET_ECC_POINT_MULT_ERROR             (('E'<<24)|('P'<<16)|('M'<<8)|('E'))
#define MH_RET_ECC_POINT_INFINITE_FAR           (('E'<<24)|('P'<<16)|('I'<<8)|('F'))
#define MH_RET_ECC_PUBLIC_KEY_FAILED            (('E'<<24)|('P'<<16)|('K'<<8)|('F'))
#define MH_RET_ECC_PUBLIC_KEY_PASS              (('E'<<24)|('P'<<16)|('K'<<8)|('P'))
#define MH_RET_ECC_KEY_GENERATION_SUCCESS       (('E'<<24)|('K'<<16)|('G'<<8)|('S'))
#define MH_RET_ECC_KEY_GENERATION_FAILED        (('E'<<24)|('K'<<16)|('G'<<8)|('F'))

#define MH_RET_ECC_SIGN_INIT                (('R'<<24)|('E'<<16)|('S'<<8)|('I'))
#define MH_RET_ECC_SIGN_SUCCESS             (('R'<<24)|('E'<<16)|('S'<<8)|('S'))
#define MH_RET_ECC_VERIGY_FAILURE           (('R'<<24)|('E'<<16)|('V'<<8)|('F'))
#define MH_RET_ECC_VERIFY_SUCCESS           (('R'<<24)|('E'<<16)|('V'<<8)|('S'))

#define MH_RET_ECC_ECIES_INIT               (('R'<<24)|('E'<<16)|('E'<<8)|('I'))
#define MH_RET_ECC_ECIES_FAILURE            (('R'<<24)|('E'<<16)|('E'<<8)|('F'))
#define MH_RET_ECC_ENC_FAILURE              (('R'<<24)|('E'<<16)|('N'<<8)|('F'))
#define MH_RET_ECC_DEC_FAILURE              (('R'<<24)|('E'<<16)|('D'<<8)|('F'))
#define MH_RET_ECC_ECIES_SUCCESS            (('R'<<24)|('E'<<16)|('E'<<8)|('S'))

#define MH_RET_ECC_INIT                     (('M'<<24)|('R'<<16)|('E'<<8)|('I'))
#define MH_RET_ECC_PARAM_ERR                (('M'<<24)|('E'<<16)|('P'<<8)|('E'))
#define MH_RET_ECC_SUCCESS                  (('M'<<24)|('R'<<16)|('E'<<8)|('S'))

/**
  * @method ecc_config(not use/ delete?)
  * @brief  config field base on ecc curve param.
  * @param  para        :ecc curve param
  * @param  config      :curve type
  * @retval             :void
  */
void ecc_config(ecc_para *para, uint32_t config);

/**
  * @method ecc_genkey
  * @brief  generate ecc key pair.
  * @param  key         :ecc key
  * @param  para        :ecc curve param
  * @param  f_rng       :true random number generation function pointer
  * @param  p_rng       :true random number generation para
  * @retval             :MH_RET_ECC_POINT_FAILED,MH_RET_ECC_KEY_GENERATION_FAILED,MH_RET_ECC_KEY_GENERATION_SUCCESS
  */
uint32_t ecc_genkey(ecc_key *key, ecc_para *para,
                    mh_rng_callback f_rng, void *p_rng);


/**
  * @method ecc_completeKey
  * @brief  complete ecc public key base on private key.
  * @param  key         :ecc key
  * @param  para        :ecc curve param
  * @param  f_rng       :true random number generation function pointer
  * @param  p_rng       :true random number generation para
  * @retval             :MH_RET_ECC_POINT_FAILED,MH_RET_ECC_KEY_GENERATION_FAILED,MH_RET_ECC_KEY_GENERATION_SUCCESS
  */
uint32_t ecc_completeKey(ecc_key *key, ecc_para *para,
                         mh_rng_callback f_rng, void *p_rng);

/**
  * @method ecc_verifykey
  * @brief  verify ecc public key is valid.
  * @param  key         :ecc key
  * @param  para        :ecc curve param
  * @param  config      :ECC curve type  0:prime  1:binary
  * @param  f_rng       :true random number generation function pointer
  * @param  p_rng       :true random number generation para
  * @retval             :MH_RET_PUBLIC_KEY_PASS,MH_RET_PUBLIC_KEY_FAILED
  */
uint32_t ecc_verifykey(ecc_key *key, ecc_para *para, uint32_t config,
                       mh_rng_callback f_rng, void *p_rng);

/*************************************************
* @method ecc_pmul
* @brief  point multiplication in prime field.
* @param  r           :rusult of pmul
* @param  a           :point param
* @param  b           :scalar param
* @param  para        :ecc curve param
* @param  f_rng       :true random number generation function pointer
* @param  p_rng       :true random number generation para
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_POINT_INFINITE_FAR,MH_RET_ECC_POINT_FAILED,MH_RET_ECC_POINT_MULT_ERROR
*************************************************/
uint32_t ecc_pmul(ecc_point_a *r, ecc_point_a *a, uint32_t *b, ecc_para *para,
                  mh_rng_callback f_rng, void *p_rng);

/*************************************************
* @method ecc_pmul_f2m
* @brief  point multiplication in binary field.
* @param  r           :rusult of pmul
* @param  a           :point param
* @param  k           :scalar param
* @param  para        :ecc curve param
* @param  f_rng       :true random number generation function pointer
* @param  p_rng       :true random number generation para
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_POINT_INFINITE_FAR,MH_RET_ECC_POINT_MULT_ERROR
*************************************************/
uint32_t ecc_pmul_f2m(ecc_point_a *r, ecc_point_a *a, uint32_t *k, ecc_para *para,
                      mh_rng_callback f_rng, void *p_rng);

/*************************************************
* @method ecc_verifypoint
* @brief  verify point is on ecc curve.
* @param  a           :point param
* @param  para        :ecc curve param
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_ECC_POINT_FAILED
*************************************************/
uint32_t ecc_verifypoint(ecc_point_a *a, ecc_para *para);

/*************************************************
* @method ecc_pmul_calc
* @brief  main operation of point multiplication in prime field.
* @param  r           :rusult of pmul
* @param  a           :point param of pmul
* @param  k           :scalar param of pmul
* @param  para        :ecc curve param
* @param  f_rng       :true random number generation function pointer
* @param  p_rng       :true random number generation para
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_POINT_INFINITE_FAR,MH_RET_ECC_POINT_FAILED,MH_RET_ECC_POINT_MULT_ERROR
*************************************************/
uint32_t ecc_pmul_calc(ecc_point_a *r, ecc_point_a *a, uint32_t *k, ecc_para *para, mh_rng_callback f_rng, void *p_rng);

//delete
uint32_t ecc_fix_pmul_calc(ecc_point_a *r, ecc_point_a *a, uint32_t *k, ecc_para *para);

/*************************************************
* @method ecc_pmul_calc_blinding
* @brief  safe point multiplication in prime field.
* @param  r           :rusult of pmul
* @param  a           :point param of pmul
* @param  k           :scalar param of pmul
* @param  para        :ecc curve param
* @param  f_rng       :true random number generation function pointer
* @param  p_rng       :true random number generation para
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_POINT_INFINITE_FAR,MH_RET_ECC_POINT_FAILED,MH_RET_ECC_POINT_MULT_ERROR
*************************************************/
uint32_t ecc_pmul_calc_blinding(ecc_point_a *r, ecc_point_a *a, uint32_t *k, ecc_para *para,
                                mh_rng_callback f_rng, void *p_rng);

//delete
uint32_t ecc_fix_pmul_calc_blinding(ecc_point_a *r, ecc_point_a *a, uint32_t *k, ecc_para *para,
                                    mh_rng_callback f_rng, void *p_rng);

/*************************************************
* @method ecc_pmul_f2m_calc
* @brief  main operation of point multiplication in binary field.
* @param  r           :rusult of pmul
* @param  a           :point param of pmul
* @param  k           :scalar param of pmul
* @param  len         :bit length of k
* @param  para        :ecc curve param
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_POINT_INFINITE_FAR,MH_RET_ECC_POINT_MULT_ERROR
*************************************************/
uint32_t ecc_pmul_f2m_calc(ecc_point_a *r, ecc_point_a *a, uint32_t *k, uint32_t len, ecc_para *para);

/*************************************************
* @method ecc_pmul_f2m_calc_blinding
* @brief  safe point multiplication in bimary field.
* @param  r           :rusult of pmul
* @param  a           :point param of pmul
* @param  k           :scalar param of pmul
* @param  para        :ecc curve param
* @param  f_rng       :true random number generation function pointer
* @param  p_rng       :true random number generation para
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_POINT_INFINITE_FAR,MH_RET_ECC_POINT_MULT_ERROR
*************************************************/
uint32_t ecc_pmul_f2m_calc_blinding(ecc_point_a *r, ecc_point_a *a, uint32_t *k, ecc_para *para,
                                    mh_rng_callback f_rng, void *p_rng);

/*************************************************
* @method ecc_j2a
* @brief  convert jacobi to affine coordinate.
* @param  r           :output rusult of affine
* @param  a           :jacobi input
* @param  para        :ecc curve param
* @retval             :void
*************************************************/
void ecc_j2a(ecc_point_a *r, ecc_point_j *a, const ecc_para *para);

/*************************************************
* @method ecc_j2mj  (delete)
* @brief  convert jacobi to modified jacobi coordinate.
* @param  r           :output rusult of modified jacobi
* @param  a           :jacobi input
* @param  para        :ecc curve param
* @retval             :void
*************************************************/
void ecc_j2mj(ecc_point_mj *r, ecc_point_j *a, const ecc_para *para);

//delete
void ecc_j2mj_t(uint32_t *r, uint32_t *z, const ecc_para *para);

/*************************************************
* @method ecc_padd_a
* @brief  point addition of affine coordinate.
* @param  r           :output of point addition
* @param  a           :input point a
* @param  a           :input point b
* @param  para        :ecc curve param
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_POINT_INFINITE_FAR
*************************************************/
uint32_t ecc_padd_a(ecc_point_a *r, ecc_point_a *a, ecc_point_a *b, const ecc_para *para);

/*************************************************
* @method ecc_padd_a
* @brief  point addition of jacobi coordinate.
* @param  r           :output of point addition
* @param  a           :input point a
* @param  a           :input point b
* @param  para        :ecc curve param
* @retval             :MH_RET_ECC_POINT_SUCCESS,MH_RET_POINT_INFINITE_FAR
*************************************************/
uint32_t ecc_padd_ja(ecc_point_j *r, ecc_point_j *a, ecc_point_a *b, const ecc_para *para);

//not use
uint32_t ecc_pdbl_a(ecc_point_a *r, ecc_point_a *a, const ecc_para *para);
uint32_t ecc_pdbl_j(ecc_point_j *r, ecc_point_j *a, const ecc_para *para);
uint32_t ecc_pdbl_mj(ecc_point_mj *r, ecc_point_mj *a, const ecc_para *para);
void ecc_naf(int8_t *w, uint32_t *w_len, uint32_t *w_oe, const uint32_t *k, uint32_t k_len);
void ecc_naf_point(ecc_point_naf(*naf_p)[8], ecc_point_a *a, const ecc_para *para);

//add by gaoke
/**
  * @method mh_ecc_set_para
  * @brief  set ecc curve param.
  * @param  _eccPara     :ecc curve param of little endian
  * @param  eccPara      :source ecc curve param of big endian
  * @retval              :void
  */
void mh_ecc_set_para(ecc_para *_eccPara, const mh_ecc_curve_para *eccPara);

/**
  * @method mh_ecc_para_init
  * @brief  init ecc curve param, alloc space.
  * @param  para            :ecc curve param of pointer formal
  * @param  para_value      :ecc curve param of array formal
  * @retval                 :0
  */
uint32_t mh_ecc_para_init(ecc_para *para, mh_ecc_curve_para_inner *para_value);

/**
  * @method mh_ecc_point_init
  * @brief  init ecc point, alloc space.
  * @param  point           :ecc point of pointer formal
  * @param  point_value     :ecc point of array formal
  * @retval                 :0
  */
uint32_t mh_ecc_point_init(ecc_point_a *point, mh_ecc_point_value *point_value);
//add by gaoke END

#ifdef __cplusplus
}
#endif

#endif

