#ifndef MULTI_SIG_WALLET_H
#define MULTI_SIG_WALLET_H

#include "librust_c.h"

#define MULTI_SIG_MALLOC(size)      EXT_MALLOC(size)
#define MULTI_SIG_FREE(ptr)         EXT_FREE(ptr)

#define MULTI_SIG_STR_CACHE_LENGTH 3*1024

#define FORMAT_P2WSH "P2WSH"
#define FORMAT_P2WSH_P2SH "P2WSH_P2SH"
#define FORMAT_P2SH "P2SH"

typedef struct MultiSigWalletItem {
    int order;
    char *verifyCode;
    char *name;
    char *format;
    int network;
    char *walletConfig;
} MultiSigWalletItem_t;

typedef struct MultiSigWalletList MultiSigWalletList_t;

typedef struct MultiSigWalletManager {
    MultiSigWalletList_t *list;
    void (*createMultiSigWalletList)();
    void (*insertNode)(MultiSigWalletItem_t *item);
    void (*deleteNode)(char *verifyCode);
    int (*getLength)();
    void (*traverseList)(void (*callback)(MultiSigWalletItem_t*, void*), void *any);
    void (*modifyNode)(char *verifyCode, MultiSigWalletItem_t* newItem);
    MultiSigWalletItem_t* (*findNode)(char *verifyCode);

    void (*destroyMultiSigWalletList)();
    void (*saveToFlash)(const char *password);

} MultiSigWalletManager_t;

MultiSigWalletManager_t* initMultiSigWalletManager();
void LoadCurrentAccountMultisigWallet(const char* password);
MultiSigWalletItem_t *AddMultisigWalletToCurrentAccount(MultiSigWallet *wallet, const char *password);
MultiSigWalletItem_t *GetMultisigWalletByVerifyCode(const char* verifyCode);
MultiSigWalletManager_t* GetMultisigWalletManager();
int GetCurrentAccountMultisigWalletNum(void);

#endif