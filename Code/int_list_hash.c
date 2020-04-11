#include "common.h"
#include <stdio.h>
#include <assert.h>

IntListHash ListHashAppendNum(IntListHash, int);
IntListHash ListHashAppend(IntListHash, IntListHash);
void ListHashInit(IntListHash *, int);

IntListHash ListHashAppendNum(IntListHash h, int v){
  assert(v < LIST_HASH_MAX);
  for(int i=0;i<HASH_NUM;++i){
    h.v[i] = (h.v[i] * (long long)LIST_HASH_MAX + v) % MOD[i];
    h.len[i] = (h.len[i] * (long long)LIST_HASH_MAX) % MOD[i];
  }
  return h;
}

IntListHash ListHashAppend(IntListHash h, IntListHash a){
  for(int i=0;i<HASH_NUM;++i){
    h.v[i] = (h.v[i] * (long long)a.len[i] + a.v[i]) % MOD[i];
    h.len[i] = (h.len[i] * (long long)a.len[i]) % MOD[i];
  }
  return h;
}

void ListHashInit(IntListHash *h, int v){
  assert(v < LIST_HASH_MAX);
  for(int i=0;i<HASH_NUM;++i){
    h->v[i] = v;
    h->len[i] = LIST_HASH_MAX;
  }
}

int ListHashEq(IntListHash a, IntListHash b){
  for(int i=0;i<HASH_NUM;++i){
    if(a.v[i] != b.v[i]) return 0;
    if(a.len[i] != b.len[i]) return 0;
  }
  return 1;
}
