#ifndef _DRV_BPK_H
#define _DRV_BPK_H

#include "mhscpu_bpk.h"

#define BPK_KEY_LENGTH                          16

ErrorStatus SetBpkValue(uint32_t *data, uint32_t len, uint32_t offset);
ErrorStatus GetBpkValue(uint32_t *data, uint32_t len, uint32_t offset);
ErrorStatus ClearBpkValue(uint32_t offset);
void PrintBpkValue(uint32_t offset);

#endif /* _DRV_BPK_H */

