/* srand.c - random operation routines
 */

#include <math.h>
#include <stdio.h>
//#include "mh_crypt_cm3.h"
#include "mh_rand.h"
//#include "mh_cephes.h"
//#include "mh_crypt_config.h"
#include "mh_misc.h"
#include "mhscpu.h"

#ifdef __CC_ARM
#pragma diag_suppress 177
#endif

const int8_t mh_freq_tab[] = {-8, -6, -6, -4, -6, -4, -4, -2, -6, -4, -4, -2, -4, -2, -2,  0,
                              -6, -4, -4, -2, -4, -2, -2,  0, -4, -2, -2,  0, -2,  0,  0,  2,
                              -6, -4, -4, -2, -4, -2, -2,  0, -4, -2, -2,  0, -2,  0,  0,  2,
                              -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4,
                              -6, -4, -4, -2, -4, -2, -2,  0, -4, -2, -2,  0, -2,  0,  0,  2,
                              -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4,
                              -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4,
                              -2,  0,  0,  2,  0,  2,  2,  4,  0,  2,  2,  4,  2,  4,  4,  6,
                              -6, -4, -4, -2, -4, -2, -2,  0, -4, -2, -2,  0, -2,  0,  0,  2,
                              -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4,
                              -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4,
                              -2,  0,  0,  2,  0,  2,  2,  4,  0,  2,  2,  4,  2,  4,  4,  6,
                              -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4,
                              -2,  0,  0,  2,  0,  2,  2,  4,  0,  2,  2,  4,  2,  4,  4,  6,
                              -2,  0,  0,  2,  0,  2,  2,  4,  0,  2,  2,  4,  2,  4,  4,  6,
                              0,  2,  2,  4,  2,  4,  4,  6,  2,  4,  4,  6,  4,  6,  6,  8
                              };

double mh_frequency(uint8_t *s, uint32_t n)
{
    int     i;
    double  s_obs, p_value, sum, sqrt2 = 1.41421356237309504880;

    if (n * 8 < 100)
        return 0.0;

    sum = 0.0;
    for (i = 0; i < n; i++)
        sum += mh_freq_tab[s[i]];

    s_obs = (double)fabs(sum) / (double)sqrt(n * 8) / sqrt2;
    p_value = erfc(s_obs);

    return p_value;
}

const int8_t mh_blk_freq_tab[] = {0,  1,  1,  2,  1,  2,  2,  3,  1,  2,  2,  3,  2,  3,  3,  4,
                                  1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
                                  1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
                                  2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                  1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
                                  2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                  2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                  3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
                                  1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
                                  2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                  2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                  3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
                                  2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                  3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
                                  3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
                                  4,  5,  5,  6,  5,  6,  6,  7,  5,  6,  6,  7,  6,  7,  7,  8
                                 };
//����û��ʹ��
double mh_block_frequency(uint8_t *s, uint32_t n)
{
//    int     i, j, N, M;
//    double  p_value, sum, pi, v, x_obs;
////  if (n * 8 < 100)
////      return 0.0;
////  //M �� 0.1n
////  M = n * 8 / 10 / 8 * 8;
////  //N = n / M
////  N = n * 8 / M;
//
////  sum = 0.0;
//
////  for ( i=0; i<N; i++ )
////  {
////      pi = 0;
////      for (j=0; j<M/8; j++)
////          pi += (double)mh_blk_freq_tab[s[i*(M/8)+j]];
////      pi = pi/(double)M;
////      v = pi-0.5;
////      sum += v*v;
////  }
////  x_obs = 4.0 * M * sum;
////  p_value = mh_cephes_igamc(N/2.0, x_obs/2.0);
//
//    return p_value;
    return 0;
}

const int8_t mh_runs_tab[] = {0,  1,  2,  1,  2,  3,  2,  1,  2,  3,  4,  3,  2,  3,  2,  1,
                              2,  3,  4,  3,  4,  5,  4,  3,  2,  3,  4,  3,  2,  3,  2,  1,
                              2,  3,  4,  3,  4,  5,  4,  3,  4,  5,  6,  5,  4,  5,  4,  3,
                              2,  3,  4,  3,  4,  5,  4,  3,  2,  3,  4,  3,  2,  3,  2,  1,
                              2,  3,  4,  3,  4,  5,  4,  3,  4,  5,  6,  5,  4,  5,  4,  3,
                              4,  5,  6,  5,  6,  7,  6,  5,  4,  5,  6,  5,  4,  5,  4,  3,
                              2,  3,  4,  3,  4,  5,  4,  3,  4,  5,  6,  5,  4,  5,  4,  3,
                              2,  3,  4,  3,  4,  5,  4,  3,  2,  3,  4,  3,  2,  3,  2,  1,
                              1,  2,  3,  2,  3,  4,  3,  2,  3,  4,  5,  4,  3,  4,  3,  2,
                              3,  4,  5,  4,  5,  6,  5,  4,  3,  4,  5,  4,  3,  4,  3,  2,
                              3,  4,  5,  4,  5,  6,  5,  4,  5,  6,  7,  6,  5,  6,  5,  4,
                              3,  4,  5,  4,  5,  6,  5,  4,  3,  4,  5,  4,  3,  4,  3,  2,
                              1,  2,  3,  2,  3,  4,  3,  2,  3,  4,  5,  4,  3,  4,  3,  2,
                              3,  4,  5,  4,  5,  6,  5,  4,  3,  4,  5,  4,  3,  4,  3,  2,
                              1,  2,  3,  2,  3,  4,  3,  2,  3,  4,  5,  4,  3,  4,  3,  2,
                              1,  2,  3,  2,  3,  4,  3,  2,  1,  2,  3,  2,  1,  2,  1,  0
                             };


double mh_runs(uint8_t *s, uint32_t n)
{
    int     S, i, N = (n * 8);
    double  pi, V, x_obs, p_value;

    if (n * 8 < 100)
        return 0.0;

    S = 0;
    for (i = 0; i < n; i++)
        S += mh_blk_freq_tab[s[i]];
    pi = (double)S / (double)N;

    if (fabs(pi - 0.5) > (2.0 / sqrt(N))) {
        p_value = 0.0;
    } else {
        V = 0;

        for (i = 0; i < n; i++) {
            V += mh_runs_tab[s[i]];
            if (i < n - 1)
                if (((s[i] & 0x80) && !(s[i + 1] & 0x01)) || (!(s[i] & 0x80) && (s[i + 1] & 0x01)))
                    V++;
        }
        x_obs = fabs(V - 2.0 * N * pi * (1 - pi)) / (2.0 * pi * (1 - pi) * sqrt(2 * N));
        p_value = erfc(x_obs);
    }

    return p_value;
}

const int8_t mh_longest_run_tab[] = {0,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                     1,  1,  1,  2,  1,  1,  2,  3,  2,  2,  2,  2,  3,  3,  4,  5,
                                     1,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                     2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  4,  4,  5,  6,
                                     1,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                     1,  1,  1,  2,  1,  1,  2,  3,  2,  2,  2,  2,  3,  3,  4,  5,
                                     2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2,  3,  4,
                                     3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  6,  7,
                                     1,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                     1,  1,  1,  2,  1,  1,  2,  3,  2,  2,  2,  2,  3,  3,  4,  5,
                                     1,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                     2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  4,  4,  5,  6,
                                     2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2,  3,  4,
                                     2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  3,  3,  4,  5,
                                     3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,
                                     4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  7,  8
                                    };
//����û��ʹ��
//double mh_longest_run_of_ones(uint8_t *s, uint32_t N)
//{
//  double          p_value, chi2, pi[7];
//  int             v_n_obs, n, i, j, K, M, V[7];
//  unsigned int    nu[7] = { 0, 0, 0, 0, 0, 0, 0 };

//  K = 3;
//  M = 8;
//  V[0] = 1; V[1] = 2; V[2] = 3; V[3] = 4;
//  pi[0] = 0.21484375;
//  pi[1] = 0.3671875;
//  pi[2] = 0.23046875;
//  pi[3] = 0.1875;

//  n = N * M;

//  if (6272 <= n || n < 128)
//      return 0.0;

//  for ( i=0; i<N; i++ ) {
//      v_n_obs = mh_longest_run_tab[s[i]];
//      if ( v_n_obs < V[0] )
//          nu[0]++;
//      for ( j=0; j<=K; j++ )
//      {
//          if ( v_n_obs == V[j] )
//              nu[j]++;
//      }
//      if ( v_n_obs > V[K] )
//          nu[K]++;
//  }

//  chi2 = 0.0;
//  for ( i=0; i<=K; i++ )
//      chi2 += ((nu[i] - N * pi[i]) * (nu[i] - N * pi[i])) / (N * pi[i]);

//  p_value = mh_cephes_igamc((double)(K/2.0), chi2 / 2.0);

//  return p_value;
//}


uint32_t mh_rand_check(void *rand, uint32_t bytes)
{
    uint32_t ret = MH_RET_RAND_CHECK_SUCCESS;
    if (6272 <= bytes * 8 || bytes * 8 < 128)
        ret = MH_RET_RAND_CHECK_DATA_LENGTH_ERROR;

#ifdef MH_RAND_USE_CHECK_FREQUENCY
    if (ret == MH_RET_RAND_CHECK_SUCCESS)
        if (mh_frequency((uint8_t*)rand, bytes) < 0.01)
            ret = MH_RET_RAND_CHECK_FAILURE;
#endif

#ifdef MH_RAND_USE_CHECK_BLOCK_FREQUENCY
    if (ret == MH_RET_RAND_CHECK_SUCCESS)
        if (mh_block_frequency((uint8_t*)rand, bytes) < 0.01)
            ret = MH_RET_RAND_CHECK_FAILURE;
#endif

#ifdef MH_RAND_USE_CHECK_RUNS
    if (ret == MH_RET_RAND_CHECK_SUCCESS)
        if (mh_runs((uint8_t*)rand, bytes) < 0.01)
            ret = MH_RET_RAND_CHECK_FAILURE;
#endif

//#ifdef MH_RAND_USE_CHECK_LONGEST_RUN
//  if (ret == MH_RET_RAND_CHECK_SUCCESS)
//      if (mh_longest_run_of_ones((uint8_t*)rand, bytes) < 0.01)
//          ret = MH_RET_RAND_CHECK_FAILURE;
//#endif

    return ret;
}

#define MH_RAND_BUFFER_SIZE             256
#define MH_RAND_BUFFER_INITED           (0x5A5A5A5A)
#define MH_RAND_BUFFER_ATTACKED         (0xA5A5A5A5)

typedef struct RingBuffer {
    uint32_t buf[MH_RAND_BUFFER_SIZE];
    uint32_t put_index, get_index;
    volatile uint32_t count;
    uint32_t inited;
    uint32_t attacked;
} RingBufferTypeDef;

void mh_trand_buf_init(RingBufferTypeDef * buf);
uint32_t mh_trand_buf_put(RingBufferTypeDef * buf, void *rand, uint32_t size);

RingBufferTypeDef g_trng_buf = {0};

void mh_trand_buf_init(RingBufferTypeDef *buf)
{
    memset(buf->buf, 0, sizeof(buf->buf));
    buf->get_index = 0;
    buf->put_index = 0;
    buf->count = 0;
    buf->inited = MH_RAND_BUFFER_INITED;
    buf->attacked = 0;
}



#define MH_RAND_USE_TRNG                                1

/************ bit definition for TRNG RNG_AMA REGISTER ************/
#define MH_TRNG_RNG_AMA_PD_TRNG0_Pos                    (12)
#define MH_TRNG_RNG_AMA_PD_TRNG0_Mask                   (0x0FU<<MH_TRNG_RNG_AMA_PD_TRNG0_Pos)


/************ bit definition for TRNG RNG_CSR REGISTER ************/
#define MH_TRNG_RNG0_CSR_S128_Pos                       (0)
#define MH_TRNG_RNG0_CSR_S128_Mask                      (0x01U<<MH_TRNG_RNG0_CSR_S128_Pos)

#define MH_TRNG_RNG0_CSR_ATTACK_Pos                     (2)
#define MH_TRNG_RNG0_CSR_ATTACK_Mask                    (0x01U<<MH_TRNG_RNG0_CSR_ATTACK_Pos)

#define MH_TRNG_RNG_CSR_INTP_EN_Pos                     (4)
#define MH_TRNG_RNG_CSR_INTP_EN_Mask                    (0x01U<<MH_TRNG_RNG_CSR_INTP_EN_Pos)

typedef struct {
    volatile uint32_t RNG_CSR;
    volatile uint32_t RNG0_DATA;
    volatile uint32_t rsvd;
    volatile uint32_t RNG_AMA;
    volatile uint32_t RNG_PN;
} mh_trng_type_def;


#define MH_TRNG                         ((mh_trng_type_def *)(0x4001E000UL))
#define MH_TRNG_WORDS                   (4)
#define MH_TRNG_BYTES                   (MH_TRNG_WORDS*4)



__STATIC_INLINE void mh_trand_init(void)
{
    MH_TRNG->RNG_CSR |= 0x10;   //ʹ���ж�
    MH_TRNG->RNG_AMA &= ~TRNG_RNG_AMA_PD_ALL_Mask;

}

__STATIC_INLINE void mh_trand_start(void)
{
    MH_TRNG->RNG_CSR &= ~MH_TRNG_RNG0_CSR_S128_Mask;

}

__STATIC_INLINE uint32_t mh_trand_get(uint32_t rand[4])
{
    uint32_t ret;

    /*
     *  check until the random number has been generated
     */
    while (!(MH_TRNG->RNG_CSR & MH_TRNG_RNG0_CSR_S128_Mask));
    /*
     *  check if the TRNG is attacked
     */
    if (MH_TRNG->RNG_CSR & MH_TRNG_RNG0_CSR_ATTACK_Mask) {
        ret = 0;
        goto cleanup;
    }
    /*
     *  copy random number to RNG_Buf
     */
    rand[0] = MH_TRNG->RNG0_DATA;
    rand[1] = MH_TRNG->RNG0_DATA;
    rand[2] = MH_TRNG->RNG0_DATA;
    rand[3] = MH_TRNG->RNG0_DATA;

    ret = sizeof(uint32_t) * 4;

cleanup:
    return ret;

}


uint32_t mh_trand_polling(void *rand, uint32_t bytes)
{
    uint32_t ret = 0;
    uint32_t m, r, i, check;
    uint8_t *_rand = (uint8_t *)rand;
    uint32_t rand_buf[4];

    memset(rand, 0, bytes);

    m = bytes / MH_TRNG_BYTES;
    r = bytes % MH_TRNG_BYTES;
    if (r)
        m++;

    mh_trand_init();
    mh_trand_start();

    for (i = 0, check = 0; i < m; i++) {
        if (i != check) {
            ret = 0;
            goto cleanup;
        }

        if (0 == mh_trand_get(rand_buf)) {
            ret = 0;
            goto cleanup;
        }
        mh_trand_start();

        if (i < (m - 1)) {
            memcpy(_rand, &rand_buf[0], sizeof(rand_buf));
            _rand = _rand + sizeof(rand_buf);
            ret += sizeof(rand_buf);
        } else if (r == 0) {
            memcpy(_rand, &rand_buf[0], sizeof(rand_buf));
            _rand = _rand + sizeof(rand_buf);
            ret += sizeof(rand_buf);
        } else {
            memcpy(_rand, &rand_buf[0], r);
            ret += r;
        }

        check++;
    }

cleanup:
    if (ret != bytes) {
        memset(rand, 0, bytes);
        ret = 0;
    }

    return ret;
}


uint32_t mh_trand_buf_attack_get(RingBufferTypeDef * buf)
{
    if (buf->attacked == MH_RAND_BUFFER_ATTACKED)
        return 1;
    else
        return 0;
}

void mh_trand_buf_attack_clean(RingBufferTypeDef * buf)
{
    buf->attacked = 0;
}

uint32_t mh_trand_buf_count(RingBufferTypeDef * buf)
{
    return buf->count;
}

//static uint32_t mh_trand_buf_get()
//{
//    uint32_t r;
//
//    if (g_trng_buf.count == 0) {
//        NVIC_EnableIRQ(TRNG_IRQn);
//    }
//    while (g_trng_buf.count == 0) {
//    }
//    r = g_trng_buf.buf[g_trng_buf.get_index++];
//    if (g_trng_buf.get_index >= MH_RAND_BUFFER_SIZE) {
//        g_trng_buf.get_index = 0;
//    }
//    NVIC_DisableIRQ(TRNG_IRQn);
//    g_trng_buf.count--;
//    NVIC_EnableIRQ(TRNG_IRQn);
//
//    return r;
//}

typedef struct trng_ext_buf_s {
    int len;
    unsigned char buf[32];
} trng_ext_buf;

trng_ext_buf g_trng_ext_buf = {0};

uint32_t mh_trand(void *rand, uint32_t bytes)
{
    uint32_t rbytes = 0;
    uint32_t _bytes = bytes;
//  int n = 0;
    typedef union dword_u {
        uint32_t n;
        uint8_t c[4];
    } dword;
    dword *p = (dword *)rand;


    if (mh_trand_buf_attack_get(&g_trng_buf)) {
        rbytes = 0;
        goto cleanup;
    }
    if (g_trng_buf.count == 0) {
        mh_trand_init();
        NVIC_EnableIRQ(TRNG_IRQn);
    }
    do {
        //printf("wait...\n");
        while (g_trng_buf.count == 0);
        //printf("bytes = %d\n", bytes);
        while (_bytes >= 4 && g_trng_buf.count) {
            p->n = g_trng_buf.buf[g_trng_buf.get_index++];
            g_trng_buf.get_index %= MH_RAND_BUFFER_SIZE;
            NVIC_DisableIRQ(TRNG_IRQn);
            g_trng_buf.count--;
            NVIC_EnableIRQ(TRNG_IRQn);
            _bytes -= 4;
            rbytes += 4;
            p++;
        }
        if (_bytes < 4 && g_trng_buf.count) {
            unsigned char *pbyte = (unsigned char *)(g_trng_buf.buf + g_trng_buf.get_index);
            int i = 0;

            //printf("tail:%08X\n", g_trng_buf.buf[g_trng_buf.get_index]);
            g_trng_buf.get_index++;
            for (i = 0; i < _bytes; i++) {
                p->c[i] = *pbyte++;
                rbytes++;
            }
            _bytes = 0;
            g_trng_buf.get_index %= MH_RAND_BUFFER_SIZE;

            NVIC_DisableIRQ(TRNG_IRQn);
            g_trng_buf.count--;
            NVIC_EnableIRQ(TRNG_IRQn);
        }
    } while (_bytes > 0);

cleanup:
    if (rbytes != bytes) {
        memset(rand, 0, bytes);
        rbytes = 0;
    }

    return rbytes;
}

//extern uint32_t g_count;

uint32_t mh_frand(void *rand, uint32_t bytes)
{
    uint32_t i;

    for (i = 0; i < bytes; i++) {
        if (i % 2)
            ((uint8_t*)rand)[i] = (MH_TRNG->RNG_PN) & 0xFF;
        else
            ((uint8_t*)rand)[i] = ((MH_TRNG->RNG_PN) >> 3) & 0xFF;
    }
    return i;
}



uint32_t mh_rand(void *rand, uint32_t bytes)
{
    //memset(rand, 0x18, bytes);

    //return bytes;
#if MH_RAND_USE_TRNG
    return mh_trand(rand, bytes);
//      return mh_trand_polling(rand, bytes);
#else
    return mh_frand(rand, bytes);
#endif
}

/*
void TRNG_ClearITPendingBit(uint32_t TRNG_IT)
{
    MH_TRNG->RNG_CSR &= ~TRNG_IT;
}
*/
#define TRNG_IT_RNG0_S128           ((uint32_t)0x00000001)

void TRNG_IRQHandler(void)
{
    int i;
    /***************************************
     *  check if the TRNG is attacked
     **************************************/
    if ((MH_TRNG->RNG_CSR & MH_TRNG_RNG0_CSR_ATTACK_Mask)) {
        g_trng_buf.attacked = MH_RAND_BUFFER_ATTACKED;
    } else {
        volatile uint32_t *p = &MH_TRNG->RNG0_DATA;
        for (i = 0; i < 4; i ++) {
            if (g_trng_buf.count < MH_RAND_BUFFER_SIZE) {
                g_trng_buf.buf[g_trng_buf.put_index++] = *p;
                if (g_trng_buf.put_index >= MH_RAND_BUFFER_SIZE) {
                    g_trng_buf.put_index = 0;
                }
                g_trng_buf.count++;
            }
        }
        TRNG_ClearITPendingBit(TRNG_IT_RNG0_S128);
        if (g_trng_buf.count == MH_RAND_BUFFER_SIZE) {
            NVIC_DisableIRQ(TRNG_IRQn);
        }
    }
//  printf("come into trand interrupt\n");
//  TRNG_ClearITPendingBit(TRNG_IT_RNG0_S128);
}

uint32_t mh_rand_init(void)
{
    mh_trand_buf_init(&g_trng_buf);
    mh_trand_init();
    mh_trand_start();
    return 0;
}

uint32_t mh_rand_p(void *rand, uint32_t bytes, void *p_rng)
{
    //memset(rand, 0x01, bytes);
    //return bytes;
    return mh_rand(rand, bytes);
}
