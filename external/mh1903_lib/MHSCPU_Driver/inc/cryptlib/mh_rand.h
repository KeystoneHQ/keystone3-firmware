/* rand.h - header file for rand.c
 */

#ifndef __MH_RAND_H
#define __MH_RAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>

#define MH_RET_RAND_CHECK_FAILURE               (('M'<<24)|('R'<<16)|('C'<<8)|('F'))
#define MH_RET_RAND_CHECK_SUCCESS               (('M'<<24)|('R'<<16)|('C'<<8)|('S'))
#define MH_RET_RAND_CHECK_DATA_LENGTH_ERROR     (('M'<<24)|('R'<<16)|('C'<<8)|('L'))

/**
  * @method mh_rand
  * @brief  get random bytes
  * @param  rand    :random data buffer
  * @param  bytes   :size of rand data
  * @retval         :MH_RET_RAND_GET_FAILURE
  *                 :MH_RET_RAND_GET_SUCCESS
  */
uint32_t mh_rand(void *rand, uint32_t bytes);
uint32_t mh_rand_p(void *rand, uint32_t bytes, void *para);

/**
  * @method mh_rand_check
  * @brief  check whether the data meet the requirement
  * @param  rand    :random data buf
  * @param  bytes   :range: 16 < bytes < 784
  * @retval         :MH_RET_RAND_CHECK_FAILURE
  *                 :MH_RET_RAND_CHECK_SUCCESS
  *                 :MH_RET_RAND_CHECK_DATA_LENGTH_ERROR
  */
uint32_t mh_rand_check(void *rand, uint32_t bytes);


uint32_t mh_rand_init(void);


#ifdef __cplusplus
}
#endif

#endif


