#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "common.h"

void treeInit(Tree **tp, int st){
  Tree *t = malloc(sizeof(Tree));
  *tp = t;

  for(int i=0;i<MAXCH;++i) t->ch[i] = NULL;
  t->int_val = 0;
  t->float_val = 0;
  t->stype = st;

  /*
  if(st < 250){
    printf("init node of type %s\n", syntaxName[st]);
  }else{
    printf("init token\n");
  }
  */
}

void printNode(int x, int s){
  for(int i=0;i<x;++i) printf("  ");
  if(s > 250) printf("<Token>\n"); //TODO: magic number here
  else printf("%s\n", syntaxName[s]);
}

void treeDfs(Tree *t, int s, int x){
  //printf("dfsing %d, %d\n", (int)t, s);
  printNode(x, s);
  for(int i=0;i<MAXCH;++i) if(t->ch[i]) treeDfs(t->ch[i], t->ch[i]->stype, x+1);
}
