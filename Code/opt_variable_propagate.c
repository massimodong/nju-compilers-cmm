#include "common.h"
#include <stdlib.h>

extern int label_cnt;
int has_dst(IRCode);
Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);

static int *oc = NULL, *cc = NULL;

static int is_variable(int l){
  return oc[l] >= 2;
}

static int new_label(int l){
  while(cc[l]) l = cc[l];
  return l;
}

static IRCode new_labeling(IRCode irc){
  switch(irc.op){
    //src1:
    //case OP_LABEL:
    case OP_ARG:
    //case OP_DEC:
    case OP_WRITE:
    case OP_RETURN:
      if(irc.cnst1 == 0) irc.src1 = new_label(irc.src1);
      break;

    /*
    //dst:
    case OP_FUNCALL:
    case OP_PARAM:
    case OP_READ:
    case OP_GOTO:
      irc.dst = new_label(irc.dst);
      break;
    */

    //dst, src1:
    case OP_ASSIGN:
    case OP_GETADDR:
    case OP_PUTADDR:
    case OP_GETFROMADDR:
      //irc.dst = new_label(irc.dst);
      if(irc.cnst1 == 0) irc.src1 = new_label(irc.src1);
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
      //irc.dst = new_label(irc.dst);
      if(irc.cnst1 == 0) irc.src1 = new_label(irc.src1);
      if(irc.cnst2 == 0) irc.src2 = new_label(irc.src2);
      break;

    //none:
    case OP_FUNCTION:
      break;
  }
  return irc;
}

void opt_variable_propagate(Vector *vec){
  oc = malloc(sizeof(int) * (label_cnt + 233));
  cc = malloc(sizeof(int) * (label_cnt + 233));
  Vector *nv = vector_new();
  for(int i=0;i<=label_cnt;++i) oc[i] = cc[i] = 0;

  for(int i=0;i<vec->len;++i){
    if(has_dst(vec->data[i])) ++oc[vec->data[i].dst];
  }

  for(int i=0;i<vec->len;++i){
    if(vec->data[i].op == OP_ASSIGN){
      if(is_variable(vec->data[i].dst) || vec->data[i].cnst1 || is_variable(vec->data[i].src1)){
        vec_pb(nv, new_labeling(vec->data[i]));
      }else{
        cc[vec->data[i].dst] = vec->data[i].src1;
      }
    }else{
      vec_pb(nv, new_labeling(vec->data[i]));
    }
  }

  vec->len = nv->len;
  for(int i=0;i<nv->len;++i) vec->data[i] = nv->data[i];

  vector_free(nv);
  free(oc);
}
