/**
 * \file version.h
 *
 */

#ifndef __MH_CRYPT_VERSION_H
#define __MH_CRYPT_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/**
  * @method mh_get_version
  * @brief  Get the version number.
  * @param  void
  * @retval The constructed version number in the format:MMNNPP00 (Major, Minor, Patch).
  */
uint32_t mh_crypt_version(void);

uint32_t mh_crypt_aes_secure_config(void);
uint32_t mh_crypt_des_secure_config(void);
uint32_t mh_crypt_tdes_secure_config(void);

#ifdef __cplusplus
}
#endif

#endif /* version.h */
