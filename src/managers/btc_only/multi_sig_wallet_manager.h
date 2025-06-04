#ifndef MULTI_SIG_WALLET_H
#define MULTI_SIG_WALLET_H

#include "librust_c.h"

#define MULTI_SIG_MALLOC(size)                          EXT_MALLOC(size)
#define MULTI_SIG_FREE(ptr)                             EXT_FREE(ptr)

#define MULTI_SIG_STR_CACHE_LENGTH                      (3*1024)
#define MAX_MULTI_SIG_WALLET_NUMBER                     (5)
#define MAX_MULTI_SIG_PASSPHRASE_WALLET_NUMBER          (1)
#define MAX_MULTI_SIG_WALLET_NUMBER_EXCEPT_PASSPHRASE   (4)

#define FORMAT_P2WSH "P2WSH"
#define FORMAT_P2WSH_P2SH "P2WSH_P2SH"
#define FORMAT_P2SH_P2WSH "P2SH_P2WSH"
#define FORMAT_P2WSH_P2SH_MID "P2WSH-P2SH"
#define FORMAT_P2SH_P2WSH_MID "P2SH-P2WSH"
#define FORMAT_P2SH "P2SH"

typedef struct MultiSigWalletItem {
    int order;
    char *verifyCode;
    char *name;
    char *format;
    int network;
    char *walletConfig;
    bool passphrase;
} MultiSigWalletItem_t;

typedef struct MultiSigWalletList MultiSigWalletList_t;

typedef struct MultiSigWalletManager {
    MultiSigWalletList_t *list;
    void (*createMultiSigWalletList)();
    void (*insertNode)(MultiSigWalletItem_t *item);
    int (*deleteNode)(char *verifyCode);
    int (*getLength)();
    void (*traverseList)(void (*callback)(MultiSigWalletItem_t*, void*), void *any);
    void (*modifyNode)(char *verifyCode, MultiSigWalletItem_t* newItem);
    MultiSigWalletItem_t* (*findNode)(char *verifyCode);

    void (*destroyMultiSigWalletList)();
    void (*saveToFlash)(void);

} MultiSigWalletManager_t;

MultiSigWalletManager_t* initMultiSigWalletManager();
int32_t LoadCurrentAccountMultisigWallet(const char* password);
MultiSigWalletItem_t *AddMultisigWalletToCurrentAccount(MultiSigWallet *wallet, const char *password);
MultiSigWalletItem_t *GetMultisigWalletByVerifyCode(const char* verifyCode);
MultiSigWalletManager_t* GetMultisigWalletManager();
int GetCurrentAccountMultisigWalletNum(bool isPassphrase);
MultiSigWalletItem_t *GetCurrenMultisigWalletByIndex(int index);
int DeleteMultisigWalletByVerifyCode(const char *verifyCode);
MultiSigWalletItem_t *GetDefaultMultisigWallet(void);

#endif