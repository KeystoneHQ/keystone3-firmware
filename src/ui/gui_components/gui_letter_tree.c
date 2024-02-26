#include <string.h>
#include "bip39_english.h"
#include "slip39_wordlist.h"
#include "gui_letter_tree.h"
#include "user_memory.h"

#pragma GCC optimize ("O0")

#define GUI_KEYBOARD_CANDIDATE_WORDS_CNT                        (3)
#define GUI_KEYBOARD_CANDIDATE_WORDS_LEN                        (32)
TrieSTPtr rootTree = NULL;
extern char g_wordBuf[GUI_KEYBOARD_CANDIDATE_WORDS_CNT][GUI_KEYBOARD_CANDIDATE_WORDS_LEN];

TrieNodePtr createTrieNode(char key)
{
    TrieNodePtr t = (TrieNodePtr)EXT_MALLOC(sizeof(TrieNode));
    if (t == NULL) {
        return NULL;
    }
    memset_s(t, sizeof(TrieNode), 0, sizeof(TrieNode));
    t->value = key;
    return t;
}

TrieSTPtr createTrie(void)
{
    TrieSTPtr t = (TrieSTPtr)EXT_MALLOC(sizeof(TrieNode));
    memset_s(t, sizeof(TrieNode), 0, sizeof(TrieNode));
    return t;
}

void insert(TrieSTPtr root, const char *key)
{
    int i = 0;
    TrieSTPtr tmp = root;
    while (*(key + i) != '\0') {
        if (tmp->next[*(key + i) - 'a'] == NULL) {
            TrieNodePtr t = createTrieNode(*(key + i));
            if (t == NULL) {
                printf("mallor error\n");
                return;
            }
            tmp->next[*(key + i) - 'a'] = t;
            tmp->count++;
        }
        tmp = tmp->next[*(key + i) - 'a'];
        i++;
    }
    tmp->isEndOfWord = true;
}

void deleteTrie(TrieSTPtr t)
{
    for (int i = 0; i < CHAR_LENGTH; i++) {
        if (t->next[i] != NULL) {
            deleteTrie(t->next[i]);
            EXT_FREE(t->next[i]);
            t->next[i] = NULL;
        }
    }
}

void wordsTraversal(TrieNodePtr t, int num, int n)
{
    g_wordBuf[num][n] = t->value;
    for (int i = 0; i < 'z' - 'a'; i++) {
        if (t->next[i] != NULL) {
            wordsTraversal(t->next[i], num, ++n);
        }
    }
}

int searchTrie(TrieSTPtr root, const char *str)
{
    if (root == NULL)
        return 0;
    int num = 0;
    TrieSTPtr tmp = root;
    int i = 0;
    while (str[i] != '\0') {
        if (tmp->next[str[i] - 'a'] != NULL) {
            tmp = tmp->next[str[i] - 'a'];
        } else
            return false;
        i++;
    }
    if (tmp->isEndOfWord == true) {
        strcpy_s(g_wordBuf[num], GUI_KEYBOARD_CANDIDATE_WORDS_LEN, str);
        ++num;
    }
    TrieSTPtr record = tmp;
    for (int j = 0; j <= 'z' - 'a'; j++) {
        if (tmp->next[j] != NULL) {
            tmp = tmp->next[j];
            if (strlen(str) >= 3) {
                strcpy_s(g_wordBuf[num], GUI_KEYBOARD_CANDIDATE_WORDS_LEN, str);
                wordsTraversal(tmp, num, strlen(g_wordBuf[num]));
            }
            ++num;
            tmp = record;
        } else {
            //printf("%c need disable\n", j);
        }
    }
    return num;
}

TrieSTPtr deleteKey(TrieSTPtr root, char *key, int d)
{
    if (root == NULL)
        return NULL;
    if (d != strlen(key)) {
        char c = *(key + d);
        TrieSTPtr tmp = root->next[(int)c];
        tmp = deleteKey(root->next[(int)c], key, d + 1);
        if (tmp == NULL) {
            if (root->count != 0)
                root->count--;
            if (root->isEndOfWord == true) {
                return root;
            } else if (root->count == 0 && root->isEndOfWord == false) {
                EXT_FREE(root);
                root = NULL;
                return root;
            }
        } else {
            return tmp;
        }
    } else {
        if (root->count == 0) {
            EXT_FREE(root);
            root = NULL;
        } else {
            root->isEndOfWord = false;
        }
        return root;
    }
    return NULL;
}

void UpdateRootTree(bool bip39)
{
    if (rootTree != NULL) {
        deleteTrie(rootTree);
    }
    rootTree = createTrie();
    if (bip39) {
        for (int i = 0; i < 2048; i++) {
            insert(rootTree, wordlist[i]);
        }
    } else {
        for (int i = 0; i < 1024; i++) {
            insert(rootTree, slip39_wordlists[i]);
        }
    }
}