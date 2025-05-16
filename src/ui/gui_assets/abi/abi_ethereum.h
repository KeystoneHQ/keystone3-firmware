#ifndef ABI_ETHEREUM
#define ABI_ETHEREUM

#include "abi_item.h"

extern const ABIItem_t ethereum_abi_map[];
extern const char *ethereum_erc20_json;
extern const char *safe_json;

uint32_t GetEthereumABIMapSize();

#endif