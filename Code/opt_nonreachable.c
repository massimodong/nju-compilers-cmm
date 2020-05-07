#include "common.h"

Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);

void opt_nonreachable(Vector *vec){
  Vector *nv = vector_new();

  int r = 1;
  for(int i=0;i<vec->len;++i){
    if(vec->data[i].op == OP_LABEL){
      r = 1;
    }
    if(r) vec_pb(nv, vec->data[i]);
    if(vec->data[i].op == OP_GOTO){
      r = 0;
    }
  }

  vec->len = nv->len;
  for(int i=0;i<nv->len;++i) vec->data[i] = nv->data[i];
  vector_free(nv);
}
