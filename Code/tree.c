#include <stdio.h>
#include <assert.h>
#include "common.h"

void treeInit(Tree *t){
  for(int i=0;i<MAXCH;++i) t->ch[i] = NULL;
  t->int_val = 0;
  t->float_val = 0;
}

void treeDfs(Tree *t, int s, int x){
  switch(s){
    case Program:
      assert(t->ch[0]);
      treeDfs(t->ch[0], ExtDef, x+1);
      break;
    default:
      assert(0);
  }
}
