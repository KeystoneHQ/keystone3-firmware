#ifndef _GUI_LETTER_TREE_H
#define _GUI_LETTER_TREE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define CHAR_LENGTH                                          26

typedef struct TrieNode {
    struct TrieNode *next[CHAR_LENGTH];
    uint8_t isEndOfWord;
    uint8_t count;
    char value;
} TrieNode, *TrieNodePtr, *TrieSTPtr;

TrieSTPtr deleteKey(TrieSTPtr root, char *key, int d);
int searchTrie(TrieSTPtr root, const char *str);
void wordsTraversal(TrieNodePtr t, int num, int n);
void deleteTrie(TrieSTPtr t);
void insert(TrieSTPtr root, const char *key);
TrieSTPtr createTrie(void);
TrieNodePtr createTrieNode(char key);
void UpdateRootTree(bool bip39);

#endif