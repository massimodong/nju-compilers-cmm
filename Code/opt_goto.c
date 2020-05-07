#include "common.h"
#include <stdlib.h>
#include <assert.h>

extern int label_cnt;
Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);

static int is_cond_goto(IRCode ir){
  switch(ir.op){
    case OP_IFG_GOTO:
    case OP_IFL_GOTO:
    case OP_IFGE_GOTO:
    case OP_IFLE_GOTO:
    case OP_IFEQ_GOTO:
    case OP_IFNE_GOTO:
      return 1;
    default:
      return 0;
  }
}

void opt_goto(Vector *vec){
  Vector *nv = vector_new();
  int *label_used = malloc(sizeof(int) * (label_cnt + 233));
  for(int i=1;i<=label_cnt;++i) label_used[i] = 0;
  for(int i=0;i<vec->len;++i){
    if(i+2 < vec->len){
      if(is_cond_goto(vec->data[i]) && vec->data[i+1].op == OP_GOTO && vec->data[i+2].op == OP_LABEL && vec->data[i].dst == vec->data[i+2].src1){
        IRCode ir;
        switch(vec->data[i].op){
          case OP_IFG_GOTO:
            ir.op = OP_IFLE_GOTO;
            break;
          case OP_IFL_GOTO:
            ir.op = OP_IFGE_GOTO;
            break;
          case OP_IFGE_GOTO:
            ir.op = OP_IFL_GOTO;
            break;
          case OP_IFLE_GOTO:
            ir.op = OP_IFG_GOTO;
            break;
          case OP_IFEQ_GOTO:
            ir.op = OP_IFNE_GOTO;
            break;
          case OP_IFNE_GOTO:
            ir.op = OP_IFEQ_GOTO;
            break;
          default:
            assert(0);
        }

        ir.dst = vec->data[i+1].dst;
        ir.src1 = vec->data[i].src1;
        ir.src2 = vec->data[i].src2;
        ir.cnst1 = vec->data[i].cnst1;
        ir.cnst2 = vec->data[i].cnst2;
        vec_pb(nv, ir);

        /*
        ir.op = OP_GOTO;
        ir.dst = vec->data[i].dst;
        vec_pb(nv, ir);
        */

        ++i; //skip 2 instrs
        continue;
      }
    }
    if(i+1<vec->len){
      if(is_cond_goto(vec->data[i]) || vec->data[i].op == OP_GOTO){
        if(vec->data[i+1].op == OP_LABEL && vec->data[i].dst == vec->data[i+1].src1){
          continue;
        }
      }
    }

    vec_pb(nv, vec->data[i]);
  }

  for(int i=0;i<nv->len;++i) if(is_cond_goto(nv->data[i]) || nv->data[i].op == OP_GOTO){
    label_used[nv->data[i].dst] = 1;
  }

  vec->len = 0;
  for(int i=0;i<nv->len;++i){
    if(nv->data[i].op == OP_LABEL && !label_used[nv->data[i].src1]){
    }else{
      vec_pb(vec, nv->data[i]);
    }
  }

  free(label_used);
}
