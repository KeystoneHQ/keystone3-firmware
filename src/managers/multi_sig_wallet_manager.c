#include "multi_sig_wallet_manager.h"
#include "account_public_info.h"
#include "stdint.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include "user_memory.h"
#include "account_manager.h"
#include "assert.h"

#define MAX_NAME_LENGTH 64
#define MAX_FORMAT_LENGTH 12
#define MAX_VERIFY_CODE_LENGTH 12
#define MAX_WALLET_CONFIG_TEXT_LENGTH 2048

#define ASSERT_WALLET_MANAGER_EXIST assert(g_multisigWalletManager != NULL);

typedef struct MultiSigWalletNode
{
    MultiSigWalletItem_t *value;
    struct MultiSigWalletNode *next;
} MultiSigWalletNode_t;

struct MultiSigWalletList
{
    MultiSigWalletNode_t *head;
    int length;
};

static MultiSigWalletManager_t *g_multisigWalletManager = NULL;

static void createMultiSigWalletList();
static void insertNode(MultiSigWalletItem_t *item);
static void deleteNode(char *verifyCode);
static int getLength();
static void traverseList(void (*callback)(MultiSigWalletItem_t *, void *), void *any);
static void destroyMultiSigWalletList();
static void saveToFlash(const char *password);
static void modifyNode(char *verifyCode, MultiSigWalletItem_t *newItem);
static MultiSigWalletItem_t *findNode(char *verifyCode);
static void DestoryMultisigWalletManager(MultiSigWalletManager_t *manager);

MultiSigWalletManager_t *initMultiSigWalletManager()
{
    if (g_multisigWalletManager != NULL)
    {
        DestoryMultisigWalletManager(g_multisigWalletManager);
    }
    g_multisigWalletManager = (MultiSigWalletManager_t *)MULTI_SIG_MALLOC(sizeof(MultiSigWalletManager_t));
    g_multisigWalletManager->createMultiSigWalletList = createMultiSigWalletList;
    g_multisigWalletManager->insertNode = insertNode;
    g_multisigWalletManager->deleteNode = deleteNode;
    g_multisigWalletManager->getLength = getLength;
    g_multisigWalletManager->destroyMultiSigWalletList = destroyMultiSigWalletList;
    g_multisigWalletManager->traverseList = traverseList;
    g_multisigWalletManager->saveToFlash = saveToFlash;
    g_multisigWalletManager->modifyNode = modifyNode;
    g_multisigWalletManager->findNode = findNode;
    createMultiSigWalletList();
    return g_multisigWalletManager;
}

MultiSigWalletManager_t *GetMultisigWalletManager()
{
    ASSERT_WALLET_MANAGER_EXIST
    return g_multisigWalletManager;
}

void LoadCurrentAccountMultisigWallet(const char *password)
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletGet(GetCurrentAccountIndex(), password, g_multisigWalletManager);
}

MultiSigWalletItem_t *GetMultisigWalletByVerifyCode(const char *verifyCode)
{
    ASSERT_WALLET_MANAGER_EXIST
    return g_multisigWalletManager->findNode(verifyCode);
}

static void DestoryMultisigWalletManager(MultiSigWalletManager_t *manager)
{
    if (manager == NULL)
    {
        return;
    }
    manager->destroyMultiSigWalletList(manager);
    MULTI_SIG_FREE(manager);
    manager = NULL;
}

MultiSigWalletItem_t *AddMultisigWalletToCurrentAccount(MultiSigWallet *wallet, const char *password)
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletManager_t *manager = g_multisigWalletManager;
    MultiSigWalletItem_t *walletItem = SRAM_MALLOC(sizeof(MultiSigWalletItem_t));

    walletItem->name = SRAM_MALLOC(MAX_NAME_LENGTH);
    strcpy_s(walletItem->name, MAX_NAME_LENGTH, wallet->name);

    walletItem->network = wallet->network;

    walletItem->order = manager->getLength(manager->list);

    walletItem->verifyCode = SRAM_MALLOC(MAX_VERIFY_CODE_LENGTH);
    strcpy_s(walletItem->verifyCode, MAX_VERIFY_CODE_LENGTH, wallet->verify_code);

    walletItem->format = SRAM_MALLOC(MAX_FORMAT_LENGTH);
    strcpy_s(walletItem->format, MAX_FORMAT_LENGTH, wallet->format);

    walletItem->walletConfig = SRAM_MALLOC(MAX_WALLET_CONFIG_TEXT_LENGTH);
    strcpy_s(walletItem->walletConfig, MAX_WALLET_CONFIG_TEXT_LENGTH, wallet->config_text);

    manager->insertNode(walletItem);
    manager->saveToFlash(password);
    return walletItem;
}

static void createMultiSigWalletList()
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletManager_t *manager = g_multisigWalletManager;
    if (manager->list != NULL)
    {
        destroyMultiSigWalletList(manager);
        manager->list = NULL;
    }

    MultiSigWalletList_t *list = (MultiSigWalletList_t *)MULTI_SIG_MALLOC(sizeof(MultiSigWalletList_t));
    list->head = NULL;
    list->length = 0;
    manager->list = list;
}

static void insertNode(MultiSigWalletItem_t *item)
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletList_t *list = g_multisigWalletManager->list;
    MultiSigWalletNode_t *newNode = (MultiSigWalletNode_t *)MULTI_SIG_MALLOC(sizeof(MultiSigWalletNode_t));
    newNode->value = item;
    newNode->next = NULL;

    if (list->head == NULL)
    {
        list->head = newNode;
    }
    else
    {
        MultiSigWalletNode_t *temp = list->head;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = newNode;
    }
    list->length++;
}

static void modifyNode(char *verifyCode, MultiSigWalletItem_t *newItem)
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletList_t *list = g_multisigWalletManager->list;
    MultiSigWalletNode_t *temp = list->head;

    while (temp != NULL)
    {
        if (strcmp(temp->value->verifyCode, verifyCode) == 0)
        {
            MULTI_SIG_FREE(temp->value->verifyCode);
            MULTI_SIG_FREE(temp->value->name);
            MULTI_SIG_FREE(temp->value->walletConfig);
            MULTI_SIG_FREE(temp->value);
            temp->value = newItem;
            return;
        }
        temp = temp->next;
    }
}

static MultiSigWalletItem_t *findNode(char *verifyCode)
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletList_t *list = g_multisigWalletManager->list;
    MultiSigWalletNode_t *temp = list->head;

    while (temp != NULL)
    {
        if (strcmp(temp->value->verifyCode, verifyCode) == 0)
        {
            return temp->value;
        }
        temp = temp->next;
    }
    return NULL;
}

static void deleteNode(char *verifyCode)
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletList_t *list = g_multisigWalletManager->list;
    MultiSigWalletNode_t *temp = list->head;
    MultiSigWalletNode_t *prev = NULL;

    if (temp != NULL && strcmp(temp->value->verifyCode, verifyCode) == 0)
    {
        list->head = temp->next;
        MULTI_SIG_FREE(temp->value->verifyCode);
        MULTI_SIG_FREE(temp->value->name);
        MULTI_SIG_FREE(temp->value->walletConfig);
        MULTI_SIG_FREE(temp->value);
        MULTI_SIG_FREE(temp);
        list->length--;
        return;
    }

    while (temp != NULL && strcmp(temp->value->verifyCode, verifyCode) != 0)
    {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL)
    {
        return;
    }

    prev->next = temp->next;
    MULTI_SIG_FREE(temp->value->verifyCode);
    MULTI_SIG_FREE(temp->value->name);
    MULTI_SIG_FREE(temp->value->walletConfig);
    MULTI_SIG_FREE(temp->value);
    MULTI_SIG_FREE(temp);
    list->length--;
}

static int getLength()
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletList_t *list = g_multisigWalletManager->list;
    return list->length;
}

static void traverseList(void (*callback)(MultiSigWalletItem_t *, void *), void *any)
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletList_t *list = g_multisigWalletManager->list;
    MultiSigWalletNode_t *current = list->head;

    while (current != NULL)
    {
        callback(current->value, any);
        current = current->next;
    }
}

static void destroyMultiSigWalletList()
{
    ASSERT_WALLET_MANAGER_EXIST
    MultiSigWalletManager_t *manager = g_multisigWalletManager;
    MultiSigWalletNode_t *current = manager->list->head;
    MultiSigWalletNode_t *next;

    while (current != NULL)
    {
        next = current->next;
        MULTI_SIG_FREE(current->value->verifyCode);
        MULTI_SIG_FREE(current->value->name);
        MULTI_SIG_FREE(current->value->walletConfig);
        MULTI_SIG_FREE(current->value);
        MULTI_SIG_FREE(current);
        current = next;
    }

    MULTI_SIG_FREE(manager->list);
    manager->list = NULL;
}

static void saveToFlash(const char *password)
{
    // todo  判断是否需要更新
    MultiSigWalletSave(password, g_multisigWalletManager);
}