#include "common.h"
#include <stdlib.h>
#include <string.h>

extern int label_cnt;
Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);
int has_dst(IRCode);
List *newList();
void listAppend(List *, void *);
void listPopRear(List *);
int trieInsert(Trie **, const char *, SymTabEntry *);
SymTabEntry *trieQuery(Trie *, const char *);

static int *oc;

static int is_variable(int l){
  return oc[l] >= 2;
}

static List argStack;
static Trie *Fn;

static int isFun(const char *fn){
  if(strcmp(fn, "main") == 0) return 1;
  SymTabEntry *e = trieQuery(Fn, fn);
  return e->defined;
}

void opt_function_expand(Vector *vec){
  oc = malloc(sizeof(int) * (label_cnt + 233));
  Vector *nv = vector_new();
  for(int i=0;i<=label_cnt;++i) oc[i] = 0;
  argStack.head = argStack.rear = NULL;

  SymTabEntry *curFun = NULL;

  for(int i=0;i<vec->len;++i){
    if(has_dst(vec->data[i])) ++oc[vec->data[i].dst];

    if(vec->data[i].op == OP_FUNCTION){
      curFun = malloc(sizeof(SymTabEntry));
      curFun->name = vec->data[i].src1_var;
      curFun->depth = 0;
      curFun->paramList = newList();
      curFun->defined = 0;
      trieInsert(&Fn, curFun->name, curFun);
    }else if(vec->data[i].op == OP_PARAM){
      listAppend(curFun->paramList, vec->data + i);
    }else if(vec->data[i].op == OP_FUNCALL){
      if(strcmp(curFun->name, vec->data[i].src1_var) == 0){
        curFun->defined = 1;
      }
    }
  }

  for(int i=0;i<vec->len;++i){
    if(vec->data[i].op == OP_FUNCTION){
      /*
      if(isFun(vec->data[i].src1_var)){
        vec_pb(nv, vec->data[i]);
      }else{
        ++i;
        while(i < vec->len && vec->data[i].op != OP_FUNCTION) ++i;
        --i;
      }
      */
      curFun = trieQuery(Fn, vec->data[i].src1_var);
      curFun->lineno = i+1;
      vec_pb(nv, vec->data[i]);
    }else if(vec->data[i].op == OP_PARAM){
      curFun->lineno = i+1;
      vec_pb(nv, vec->data[i]);
    }else if(vec->data[i].op == OP_ARG){
      vec_pb(nv, vec->data[i]);
      listAppend(&argStack, nv->data + nv->len - 1);
    }else if(vec->data[i].op == OP_FUNCALL){
      SymTabEntry *e = trieQuery(Fn, vec->data[i].src1_var);
      if(isFun(vec->data[i].src1_var)){
        vec_pb(nv, vec->data[i]);
        for(ListNode *n = e->paramList->head;n;n=n->next) listPopRear(&argStack);
      }else{
        for(ListNode *n = e->paramList->head;n;n=n->next){
          IRCode *irp = argStack.rear->val;
          IRCode *pp = n->val;
          listPopRear(&argStack);
          irp->op = OP_ASSIGN;
          irp->dst = pp->src1;
        }
        int end_label = ++label_cnt;
        for(int j=e->lineno;j<nv->len;++j){
          if(nv->data[j].op == OP_FUNCTION || nv->data[j].op == OP_PARAM) break;
          if(nv->data[j].op == OP_RETURN){
            IRCode irc;
            irc.op = OP_ASSIGN;
            irc.dst = vec->data[i].dst;
            irc.src1 = nv->data[j].src1;
            irc.cnst1 = nv->data[j].cnst1;
            irc.src2 = irc.cnst2 = 0;
            vec_pb(nv, irc);

            irc.op = OP_GOTO;
            irc.dst = end_label;
            irc.src1 = irc.cnst1 = 0;
            vec_pb(nv, irc);
          }else{
            vec_pb(nv, nv->data[j]);
          }
        }
        IRCode irc;
        irc.op = OP_LABEL;
        irc.src1 = end_label;
        irc.dst = irc.cnst1 = irc.src2 = irc.cnst2 = 0;
        vec_pb(nv, irc);
      }
    }else{
      vec_pb(nv, vec->data[i]);
    }
  }

  vec->len = 0;
  for(int i=0;i<nv->len;++i) vec_pb(vec, nv->data[i]);
  vector_free(nv);
  free(oc);
}
