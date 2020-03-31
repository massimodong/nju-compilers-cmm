#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

Trie *newTrieNode(Trie *ot){
  Trie *t = malloc(sizeof(Trie));
  if(ot) memcpy(t, ot, sizeof(Trie));
  else memset(t, 0, sizeof(Trie));
  return t;
}

int trieCharId(char c){
  if('A' <= c && c <= 'Z') return c - 'A';
  else if('a' <= c && c <= 'z') return c - 'a' + 26;
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

void trieDfs(Trie *t){
  for(int i=0;i<63;++i){
    if(t->go[i]){
      printf("go to %d\n", i);
      trieDfs(t->go[i]);
      printf("back\n");
    }
  }
}

void trieInsert(Trie **tp, const char *str, SymTabEntry *entry){
  //printf("trie Insert %s\n", str);
  Trie *t = *tp = newTrieNode(*tp);
  //trieDfs(t);
  for(int i=0;str[i];++i){
    int d = trieCharId(str[i]);
    t = t->go[d] = newTrieNode(t->go[d]);
  }

  t->entry = entry;
}
