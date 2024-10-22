#include "common.h"

Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);

void opt_constant_propagate(Vector *vec);
void opt_variable_propagate(Vector *vec);
void opt_nonreachable(Vector *vec);
void opt_function_expand(Vector *vec);
void opt_goto(Vector *vec);
void opt_simplify_each_code(Vector *vec);
void opt_array2variable(Vector *vec);
void opt_block(Vector *vec);

static Vector *vector_clone(Vector *vec){
  Vector *ret = vector_new();
  for(int i=0;i<vec->len;++i) vec_pb(ret, vec->data[i]);
  return ret;
}

static int vector_eq(Vector *a, Vector *b){
  if(a->len != b->len) return 0;
  for(int i=0;i<a->len;++i){
    if(a->data[i].op != b->data[i].op) return 0;
    if(a->data[i].dst != b->data[i].dst) return 0;
    if(a->data[i].dst_var != b->data[i].dst_var) return 0;
    if(a->data[i].src1 != b->data[i].src1) return 0;
    if(a->data[i].src1_var != b->data[i].src1_var) return 0;
    if(a->data[i].src2 != b->data[i].src2) return 0;
    if(a->data[i].src2_var != b->data[i].src2_var) return 0;
  }
  return 1;
}

/*
 * We don't need any optimization anymore in Lab4!
 * So we don't use any optimization
 * TODO: Bugs to be fixed:
 * For some function calls, `opt_nonreachable` will detect that some corresponding `ARG` statemets are nonreachable and thus are removed,
 *   but it can not detect that the `FUNCALL` statement itself is nonreachable as well, and thus is remained.
 *   This is somehow OK, because the `FUNCALL` statement will never be reached.
 *   But in the case that we apply function inline (`opt_function_expand`), the remaining `FUNCALL` will cause Segment Fault, since it's args are missing.
 */
void irOptimize(Vector *vec){
  return;
  int i = 0;
  opt_goto(vec);
  for(;i<50;++i){
    Vector *ov = vector_clone(vec);
    opt_variable_propagate(vec);
    opt_goto(vec);
    opt_constant_propagate(vec);
    opt_goto(vec);
    opt_nonreachable(vec);
    opt_goto(vec);
    opt_function_expand(vec);
    opt_goto(vec);
    opt_simplify_each_code(vec);
    opt_array2variable(vec);
    //opt_block(vec);
    //opt_goto(vec);
    if(vector_eq(ov, vec)){
      vector_free(ov);
      break;
    }else{
      vector_free(ov);
    }
    //printf("epoch %d\n", i);
  }
}
