#ifndef __MH_CRYPT_H
#define __MH_CRYPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CMPERR          (('C'<<24)|('M'<<16)|('P'<<8)|('E'))
#define EQUAL           (('E'<<24)|('Q'<<16)|('U'<<8)|('A'))
#define UNEQUAL         (('U'<<24)|('E'<<16)|('Q'<<8)|('U'))
#define BIGGER          (('B'<<24)|('I'<<16)|('G'<<8)|('G'))
#define SMALLER         (('S'<<24)|('M'<<16)|('A'<<8)|('L'))

typedef enum {
    ECB = (('P' << 24) | ('E' << 16) | ('C' << 8) | ('B')),
    CBC = (('P' << 24) | ('C' << 16) | ('B' << 8) | ('C'))
} mh_pack_mode_def;

typedef enum {
    ENC = (('C' << 24) | ('E' << 16) | ('N' << 8) | ('C')),
    DEC = (('C' << 24) | ('D' << 16) | ('E' << 8) | ('C'))
} mh_crypt_mode_def;

typedef uint32_t (*mh_rng_callback)(void *rand, uint32_t size, void *p_rng);

void mh_crypt_reset(void);
void mh_crypt_it_clear(void);
//void mh_crypt_data_rand(mh_rng_callback f_rng, void *p_rng);


#ifdef __cplusplus
}
#endif

#endif // __MH_CRYPT_H
