#include "common.h"
#include <stdlib.h>
#include <assert.h>

extern int label_cnt;
Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);

enum{
  C_GENERAL,
  C_ASSIGN,
  C_ARITHMETIC,
  C_D1,
  C_S1,
  C_OUT1,
  C_COND_GOTO,
};

static int exp_type(IRCode ir){
  switch(ir.op){
    case OP_LABEL:
    case OP_GOTO:
    case OP_FUNCTION:
      return C_GENERAL;

    case OP_ASSIGN:
      return C_ASSIGN;
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
      return C_ARITHMETIC;

    case OP_FUNCALL:
    case OP_GETADDR:
    case OP_GETFROMADDR:
    case OP_READ:
    case OP_PARAM:
      return C_D1;

    case OP_DEC:
      return C_S1;

    case OP_PUTADDR:
    case OP_ARG:
    case OP_WRITE:
    case OP_RETURN:
      return C_OUT1;

    case OP_IFG_GOTO:
    case OP_IFL_GOTO:
    case OP_IFGE_GOTO:
    case OP_IFLE_GOTO:
    case OP_IFEQ_GOTO:
    case OP_IFNE_GOTO:
      return C_COND_GOTO;
  }
}
static int *ic, *cv, *oc, *vl, vlen;

static int is_constant(IRCode irc){
  switch(irc.op){
    case OP_ASSIGN:
      return (irc.cnst1 || ic[irc.src1]);
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
      return (irc.cnst1 || ic[irc.src1]) && (irc.cnst2 || ic[irc.src2]);
    case OP_ARG:
    case OP_WRITE:
    case OP_RETURN:
      return (irc.cnst1 || ic[irc.src1]);
    default:
      assert(0);
  }
}

static int get_constant_val(IRCode irc){
  int lv, rv;
  lv = irc.cnst1 ? irc.src1 : cv[irc.src1];
  rv = irc.cnst2 ? irc.src2 : cv[irc.src2];
  switch(irc.op){
    case OP_ASSIGN:
      return lv;
    case OP_ADD:
      return lv + rv;
    case OP_SUB:
      return lv - rv;
    case OP_MUL:
      return lv * rv;
    case OP_DIV:
      return lv / rv;
    case OP_ARG:
    case OP_WRITE:
    case OP_RETURN:
      return lv;
    default:
      assert(0);
  }
}

static int cond_goto_is_true(IRCode ir){
  assert(ir.cnst1 && ir.cnst2);
  int lv = ir.src1, rv = ir.src2;
  switch(ir.op){
    case OP_IFG_GOTO:
      return lv > rv;
    case OP_IFL_GOTO:
      return lv < rv;
    case OP_IFGE_GOTO:
      return lv >= rv;
    case OP_IFLE_GOTO:
      return lv <= rv;
    case OP_IFEQ_GOTO:
        return lv == rv;
    case OP_IFNE_GOTO:
        return lv != rv;
    default:
      assert(0);
  }
}

static int is_variable(int l){
  return oc[l] >= 2;
}

int has_dst(IRCode irc){
  switch(irc.op){
    case OP_FUNCALL:
    case OP_PARAM:
    case OP_READ:
    case OP_ASSIGN:
    case OP_GETADDR:
    case OP_GETFROMADDR:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
      return 1;
    default:
      return 0;
  }
}

static int enter_new_block(){
  for(int i=0;i<vlen;++i) ic[vl[i]] = 0;
}

void opt_constant_propagate(Vector *vec){
  ic = malloc(sizeof(int) * (label_cnt + 233));
  cv = malloc(sizeof(int) * (label_cnt + 233));
  oc = malloc(sizeof(int) * (label_cnt + 233));
  vl = malloc(sizeof(int) * (label_cnt + 233));
  vlen = 0;
  Vector *nv = vector_new();
  for(int i=1;i<=label_cnt;++i) ic[i] = 1;
  for(int i=1;i<=label_cnt;++i) cv[i] = 0;
  for(int i=1;i<=label_cnt;++i) oc[i] = 0;

  for(int i=0;i<vec->len;++i){
    if(has_dst(vec->data[i])) ++oc[vec->data[i].dst];
  }

  for(int i=1;i<=label_cnt;++i) if(is_variable(i)) vl[vlen++] = i;

  for(int i=0;i<vec->len;++i){
    IRCode np;
    switch(exp_type(vec->data[i])){
      case C_GENERAL:
        vec_pb(nv, vec->data[i]);
        enter_new_block();
        break;
      case C_ASSIGN:
        np = vec->data[i];
        if(np.cnst1 == 0 && ic[np.src1]){
          np.cnst1 = 1;
          np.src1 = cv[np.src1];
        }
        if(np.cnst1){
          ic[np.dst] = 1;
          cv[np.dst] = np.src1;
          if(is_variable(np.dst)){
            vec_pb(nv, np);
          }
        }else{
          vec_pb(nv, np);
          ic[np.dst] = 0;
        }
        break;
      case C_ARITHMETIC:
        np = vec->data[i];
        if(np.cnst1 == 0 && ic[np.src1]){
          np.cnst1 = 1;
          np.src1 = cv[np.src1];
        }
        if(np.cnst2 == 0 && ic[np.src2]){
          np.cnst2 = 1;
          np.src2 = cv[np.src2];
        }
        if(np.cnst1 && np.cnst2){
          ic[np.dst] = 1;
          cv[np.dst] = get_constant_val(np);
          if(is_variable(np.dst)){
            np.op = OP_ASSIGN;
            np.src1 = cv[np.dst];
            np.cnst1 = 1;
            np.src2 = np.cnst2 = 0;
            vec_pb(nv, np);
          }
        }else{
          vec_pb(nv, np);
          ic[np.dst] = 0;
        }
        break;
      case C_D1:
        ic[vec->data[i].dst] = 0;
        vec_pb(nv, vec->data[i]);
        break;
      case C_S1:
        assert(vec->data[i].cnst1 == 0);
        ic[vec->data[i].src1] = 0;
        vec_pb(nv, vec->data[i]);
        break;
      case C_OUT1:
        /*
        vec_pb(nv, vec->data[i]);
        if(is_constant(vec->data[i])){
          nv->data[nv->len-1].src1 = get_constant_val(vec->data[i]);
          nv->data[nv->len-1].cnst1 = 1;
        }
        */
        np = vec->data[i];
        if(np.cnst1 == 0 && ic[np.src1]){
          np.cnst1 = 1;
          np.src1 = cv[np.src1];
        }
        vec_pb(nv, np);
        if(np.op == OP_RETURN){
          enter_new_block();
        }
        break;
      case C_COND_GOTO:
        np = vec->data[i];
        if(np.cnst1 == 0 && ic[np.src1]){
          np.cnst1 = 1;
          np.src1 = cv[np.src1];
        }
        if(np.cnst2 == 0 && ic[np.src2]){
          np.cnst2 = 1;
          np.src2 = cv[np.src2];
        }

        if(np.cnst1 && np.cnst2){
          if(cond_goto_is_true(np)){
            np.op = OP_GOTO;
            np.src1 = np.cnst1 = np.src2 = np.cnst2 = 0;
            vec_pb(nv, np);
          }else{
          }
        }else{
          vec_pb(nv, np);
        }
        enter_new_block();
        break;

      default:
        assert(0);
    }
  }

  vec->len = 0;
  for(int i=0;i<nv->len;++i) vec_pb(vec, nv->data[i]);

  free(ic);
  free(cv);
  free(oc);
  free(vl);
  vector_free(nv);
}
