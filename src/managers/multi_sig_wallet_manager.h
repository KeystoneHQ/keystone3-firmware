#ifndef MULTI_SIG_WALLET_H
#define MULTI_SIG_WALLET_H

#define MULTI_SIG_MALLOC(size)   EXT_MALLOC(size)
#define MULTI_SIG_FREE(size)     EXT_FREE(size)

#define MULTI_SIG_STR_CACHE_LENGTH 3*1024

typedef struct MultiSigWalletItem {
    int order;
    char *verifyCode;
    char *name;
    int network;
    char *walletConfig;
} MultiSigWalletItem_t;

typedef struct MultiSigWalletList MultiSigWalletList_t;

typedef struct MultiSigWalletManager {
    MultiSigWalletList_t *list;
    void (*createMultiSigWalletList)(struct MultiSigWalletManager *manager);
    void (*insertNode)(MultiSigWalletList_t *list, MultiSigWalletItem_t *item);
    void (*deleteNode)(MultiSigWalletList_t *list, char *verifyCode);
    int (*getLength)(MultiSigWalletList_t *list);
    void (*traverseList)(MultiSigWalletList_t *list, void (*callback)(MultiSigWalletItem_t*, void*), void *any);
    void (*modifyNode)(MultiSigWalletList_t *list, char *verifyCode, MultiSigWalletItem_t* newItem);
    MultiSigWalletItem_t* (*findNode)(MultiSigWalletList_t *list, char *verifyCode);

    void (*destroyMultiSigWalletList)(struct MultiSigWalletManager* manager);
    void (*saveToFlash)(const char *password, struct MultiSigWalletManager *manager);

} MultiSigWalletManager_t;

MultiSigWalletManager_t* initMultiSigWalletManager();

#endif