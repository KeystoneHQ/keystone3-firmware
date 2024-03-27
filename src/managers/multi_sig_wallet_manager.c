#include "multi_sig_wallet_manager.h"
#include "account_public_info.h"
#include "stdint.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include "user_memory.h"

typedef struct MultiSigWalletNode {
    MultiSigWalletItem_t *value;
    struct MultiSigWalletNode *next;
} MultiSigWalletNode_t;

struct MultiSigWalletList {
    MultiSigWalletNode_t *head;
    int length;
};

static void createMultiSigWalletList(MultiSigWalletManager_t* manager);
static void insertNode(MultiSigWalletList_t *list, MultiSigWalletItem_t* item);
static void deleteNode(MultiSigWalletList_t *list, char *verifyCode);
static int getLength(MultiSigWalletList_t *list);
static void traverseList(MultiSigWalletList_t *list, void (*callback)(MultiSigWalletItem_t*, void*), void *any);
static void destroyMultiSigWalletList(MultiSigWalletManager_t* manager);
static void saveToFlash(const char *password, MultiSigWalletManager_t *manager);
static void modifyNode(MultiSigWalletList_t *list, char *verifyCode, MultiSigWalletItem_t* newItem);
static MultiSigWalletItem_t* findNode(MultiSigWalletList_t *list, char *verifyCode);

MultiSigWalletManager_t* initMultiSigWalletManager()
{
    MultiSigWalletManager_t *manager = (MultiSigWalletManager_t*)MULTI_SIG_MALLOC(sizeof(MultiSigWalletManager_t));
    manager->createMultiSigWalletList = createMultiSigWalletList;
    manager->insertNode = insertNode;
    manager->deleteNode = deleteNode;
    manager->getLength = getLength;
    manager->destroyMultiSigWalletList = destroyMultiSigWalletList;
    manager->traverseList = traverseList;
    manager->saveToFlash = saveToFlash;
    manager->modifyNode = modifyNode;
    manager->findNode = findNode;
    createMultiSigWalletList(manager);
    return manager;
}

static void createMultiSigWalletList(MultiSigWalletManager_t* manager)
{
    if (manager->list != NULL) {
        destroyMultiSigWalletList(manager);
        manager->list = NULL;
    }

    MultiSigWalletList_t *list = (MultiSigWalletList_t*)MULTI_SIG_MALLOC(sizeof(MultiSigWalletList_t));
    list->head = NULL;
    list->length = 0;
    manager->list = list;
}

static void insertNode(MultiSigWalletList_t *list, MultiSigWalletItem_t* item)
{
    MultiSigWalletNode_t *newNode = (MultiSigWalletNode_t*)MULTI_SIG_MALLOC(sizeof(MultiSigWalletNode_t));
    newNode->value = item;
    newNode->next = NULL;

    if (list->head == NULL) {
        list->head = newNode;
    } else {
        MultiSigWalletNode_t *temp = list->head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
    list->length++;
}

static void modifyNode(MultiSigWalletList_t *list, char *verifyCode, MultiSigWalletItem_t* newItem)
{
    MultiSigWalletNode_t *temp = list->head;

    while (temp != NULL) {
        if (strcmp(temp->value->verifyCode, verifyCode) == 0) {
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

static MultiSigWalletItem_t* findNode(MultiSigWalletList_t *list, char *verifyCode)
{
    MultiSigWalletNode_t *temp = list->head;

    while (temp != NULL) {
        if (strcmp(temp->value->verifyCode, verifyCode) == 0) {
            return temp->value;
        }
        temp = temp->next;
    }
    return NULL;
}

static void deleteNode(MultiSigWalletList_t *list, char *verifyCode)
{
    MultiSigWalletNode_t *temp = list->head;
    MultiSigWalletNode_t *prev = NULL;

    if (temp != NULL && strcmp(temp->value->verifyCode, verifyCode) == 0) {
        list->head = temp->next;
        MULTI_SIG_FREE(temp->value->verifyCode);
        MULTI_SIG_FREE(temp->value->name);
        MULTI_SIG_FREE(temp->value->walletConfig);
        MULTI_SIG_FREE(temp->value);
        MULTI_SIG_FREE(temp);
        list->length--;
        return;
    }

    while (temp != NULL && strcmp(temp->value->verifyCode, verifyCode) != 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
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

static int getLength(MultiSigWalletList_t *list)
{
    return list->length;
}

static void traverseList(MultiSigWalletList_t *list, void (*callback)(MultiSigWalletItem_t*, void*), void *any)
{
    MultiSigWalletNode_t *current = list->head;

    while (current != NULL) {
        callback(current->value, any);
        current = current->next;
    }
}

static void destroyMultiSigWalletList(MultiSigWalletManager_t *manager)
{
    MultiSigWalletNode_t *current = manager->list->head;
    MultiSigWalletNode_t *next;

    while (current != NULL) {
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

static void saveToFlash(const char *password, MultiSigWalletManager_t *manager)
{
    //todo  判断是否需要更新
    MultiSigWalletSave(password, manager);
}