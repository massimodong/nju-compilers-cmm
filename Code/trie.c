#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

Trie *newTrieNode(Trie *ot){
  Trie *t = malloc(sizeof(Trie));
  if(ot) memcpy(t, ot, sizeof(Trie));
  else{
    //TODO: init trie
  }
  return t;
}

int trieCharId(char c){
  if('A' <= c && c <= 'Z') return c - 'A';
  else if('a' <= c && c <= 'z') return c - 'z' + 26;
  else if('0' <= c && c <= '9') return c - '0' + 26 + 26;
  else if(c == '_') return 26 + 26 + 10;
  else assert(0);
}

SymTabEntry *trieQuery(Trie *t, const char *str){
  if(t == NULL) return NULL;
  for(int i=0;str[i];++i){
    int d = trieCharId(str[i]);
    if(t->go[d] == NULL) return NULL;
    t = t->go[d];
  }
  return t->entry;
}

int trieInsert(Trie **tp, const char *str, SymTabEntry *entry, int depth){
  Trie *t = *tp = newTrieNode(*tp);
  for(int i=0;str[i];++i){
    int d = trieCharId(str[i]);
    t = t->go[d] = newTrieNode(t->go[d]);
  }

  if(t->entry == NULL || t->depth != depth){
    t->entry = entry;
    t->depth = depth;
    return 1;
  }else{
    return 0;
  }
}
