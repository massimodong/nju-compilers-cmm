#include "common.h"
#include <stdlib.h>
#include <assert.h>

extern int label_cnt;
Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);

static int *in_list, *ic, *use, *off, *max_off, **nl;

static int new_label(int l){
  int x = use[l], o = off[l];
  assert(x);
  assert(x <= label_cnt);
  if(o >= max_off[x]){
    printf("Warning: index out of bound detected\n");
    return ++label_cnt; //UB: index out of bound 
  }
  if(nl[x][o] == 0){
    nl[x][o] = ++label_cnt;
  }
  return nl[x][o];
}

void opt_array2variable(Vector *vec){
  in_list = malloc(sizeof(int) * (label_cnt + 233));
  ic = malloc(sizeof(int) * (label_cnt + 233));
  use = malloc(sizeof(int) * (label_cnt + 233));
  off = malloc(sizeof(int) * (label_cnt + 233));
  max_off = malloc(sizeof(int) * (label_cnt + 233));
  nl = malloc(sizeof(int *) * (label_cnt + 233));
  Vector *nv = vector_new();

  for(int i=0;i<=label_cnt;++i){
    in_list[i] = ic[i] = use[i] = off[i] = max_off[i] = 0;
    nl[i] = NULL;
  }

  for(int i=0;i<vec->len;++i){
    IRCode irc = vec->data[i];
    switch(irc.op){
      case OP_ARG:
        if(irc.cnst1 == 0 && use[irc.src1]) ic[use[irc.src1]] = 0;
        break;
      case OP_DEC:
        assert(in_list[irc.src1] == 0);
        in_list[irc.src1] = 1;
        ic[irc.src1] = 1;
        use[irc.src1] = irc.src1;
        off[irc.src1] = 0;
        max_off[irc.src1] = irc.src2;
        break;
      case OP_GETADDR:
      case OP_ASSIGN:
        if(irc.cnst1 == 0 && use[irc.src1]){
          use[irc.dst] = use[irc.src1];
          off[irc.dst] = off[irc.src1];
        }
        break;
      case OP_ADD:
        if(irc.cnst1 == 0 && use[irc.src1]){
          use[irc.dst] = use[irc.src1];
          if(irc.cnst2){
            off[irc.dst] = off[irc.src1] + irc.src2;
          }else{
            off[irc.dst] = 0;
            ic[use[irc.dst]] = 0;
          }
        }
        break;
    }
  }

  for(int i=1;i<=label_cnt;++i) if(in_list[i] && ic[i]){
    nl[i] = malloc(sizeof(int) * (max_off[i] + 233));
    for(int j=0;j<=max_off[i] + 10;++j){
      nl[i][j] = 0; 
    }
  }


  for(int i=0;i<vec->len;++i){
    IRCode irc = vec->data[i];
    switch(irc.op){
      case OP_DEC:
        assert(in_list[irc.src1]);
        if(ic[irc.src1]){
        }else{
          vec_pb(nv, irc);
        }
        break;
      case OP_GETADDR:
      case OP_ASSIGN:
      case OP_ADD:
        if(use[irc.dst] && ic[use[irc.dst]]){
        }else{
          vec_pb(nv, irc);
        }
        break;
      case OP_PUTADDR:
        if(ic[use[irc.src2]]){
          irc.op = OP_ASSIGN;
          irc.dst = new_label(irc.src2);
          irc.cnst2 = irc.src2 = 0;
          vec_pb(nv, irc);
        }else{
          vec_pb(nv, irc);
        }
        break;
      case OP_GETFROMADDR:
        if(ic[use[irc.src1]]){
          irc.op = OP_ASSIGN;
          irc.src1 = new_label(irc.src1);
          vec_pb(nv, irc);
        }else{
          vec_pb(nv, irc);
        }
        break;
      default:
        vec_pb(nv, irc);
        break;
    }
  }

  for(int i=1;i<=label_cnt;++i) if(in_list[i] && ic[i]){
    free(nl[i]);
  }

  vec->len = nv->len;
  for(int i=0;i<nv->len;++i) vec->data[i] = nv->data[i];

  free(in_list);
  free(ic);
  free(use);
  free(off);
  free(max_off);
  free(nl);
  vector_free(nv);
}
