#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

extern int label_cnt;
Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);
List *newList();
void listAppend(List *, void *);
void listPopRear(List *);
int trieInsert(Trie **, const char *, SymTabEntry *);
SymTabEntry *trieQuery(Trie *, const char *);

static List argStack;
static Trie *Fn;

static int isFun(const char *fn){
  assert(fn);
  if(strcmp(fn, "main") == 0) return 1;
  SymTabEntry *e = trieQuery(Fn, fn);
  assert(e);
  return e->defined;
}

int *nlmap = NULL;
static void start_new_label(){
  nlmap = realloc(nlmap, sizeof(int) * (label_cnt + 233));
  assert(nlmap);
  for(int i=0;i<=label_cnt;++i) nlmap[i] = 0;
}
static int new_label(int l){
  assert(0 < l && l <= label_cnt);
  if(!nlmap[l]) nlmap[l] = ++label_cnt;
  return nlmap[l];
}
static IRCode new_labeling(IRCode irc){
  switch(irc.op){
    //src1:
    case OP_LABEL:
    case OP_ARG:
    case OP_DEC:
    case OP_WRITE:
    case OP_RETURN:
      if(irc.cnst1 == 0) irc.src1 = new_label(irc.src1);
      break;

    //dst:
    case OP_FUNCALL:
    case OP_PARAM:
    case OP_READ:
    case OP_GOTO:
      irc.dst = new_label(irc.dst);
      break;

    //dst, src1:
    case OP_ASSIGN:
    case OP_GETADDR:
    case OP_GETFROMADDR:
      irc.dst = new_label(irc.dst);
      if(irc.cnst1 == 0) irc.src1 = new_label(irc.src1);
      break;

    //src1, src2
    case OP_PUTADDR:
      if(irc.cnst1 == 0) irc.src1 = new_label(irc.src1);
      if(irc.cnst2 == 0) irc.src2 = new_label(irc.src2);
      break;


    //dst, src1, src2:
    case OP_IFG_GOTO:
    case OP_IFL_GOTO:
    case OP_IFGE_GOTO:
    case OP_IFLE_GOTO:
    case OP_IFEQ_GOTO:
    case OP_IFNE_GOTO:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
      irc.dst = new_label(irc.dst);
      if(irc.cnst1 == 0) irc.src1 = new_label(irc.src1);
      if(irc.cnst2 == 0) irc.src2 = new_label(irc.src2);
      break;

    //none:
    case OP_FUNCTION:
      break;
  }
  return irc;
}

void opt_function_expand(Vector *vec){
  Vector *nv = vector_new();
  argStack.head = argStack.rear = NULL;

  SymTabEntry *curFun = NULL;

  for(int i=0;i<vec->len;++i){
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
      assert(curFun);
      vec_pb(nv, vec->data[i]);
      curFun->lineno = nv->len;
    }else if(vec->data[i].op == OP_PARAM){
      vec_pb(nv, vec->data[i]);
      curFun->lineno = nv->len;
    }else if(vec->data[i].op == OP_ARG){
      listAppend(&argStack, (void *)nv->len);
      vec_pb(nv, vec->data[i]);
    }else if(vec->data[i].op == OP_FUNCALL){
      SymTabEntry *e = trieQuery(Fn, vec->data[i].src1_var);
      assert(e);
      if(isFun(vec->data[i].src1_var)){
        vec_pb(nv, vec->data[i]);
        for(ListNode *n = e->paramList->head;n;n=n->next) listPopRear(&argStack);
      }else{
        start_new_label();
        for(ListNode *n = e->paramList->head;n;n=n->next){
          int lno = (uintptr_t)argStack.rear->val;
          IRCode *pp = n->val;
          listPopRear(&argStack);
          nv->data[lno].op = OP_ASSIGN;
          nv->data[lno].dst = new_label(pp->dst);
        }
        int end_label = ++label_cnt;
        for(int j=e->lineno;j<nv->len;++j){
          if(nv->data[j].op == OP_FUNCTION || nv->data[j].op == OP_PARAM) break;
          if(nv->data[j].op == OP_RETURN){
            IRCode irc = new_labeling(nv->data[j]);
            irc.op = OP_ASSIGN;
            irc.dst = vec->data[i].dst;
            vec_pb(nv, irc);

            irc.op = OP_GOTO;
            irc.dst = end_label;
            irc.src1 = irc.cnst1 = 0;
            vec_pb(nv, irc);
          }else{
            vec_pb(nv, new_labeling(nv->data[j]));
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
  for(int i=0;i<nv->len;++i){
    if(nv->data[i].op == OP_FUNCTION){
      if(isFun(nv->data[i].src1_var)){
        vec_pb(vec, nv->data[i]);
      }else{
        ++i;
        while(i<nv->len && nv->data[i].op != OP_FUNCTION) ++i;
        --i;
      }
    }else{
      vec_pb(vec, nv->data[i]);
    }
  }
  vector_free(nv);
}
