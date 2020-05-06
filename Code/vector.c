#include "common.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

const size_t VEC_MAX_SIZE = SIZE_MAX / sizeof(IRCode);

Vector *vector_new(){
  Vector *ret = malloc(sizeof(Vector));
  ret->data = NULL;
  ret->size = ret->len = 0;
}

void vector_free(Vector *vec){
  assert(vec);
  if(vec->data) free(vec->data);
  free(vec);
}

void vec_pb(Vector *vec, IRCode code){
  assert(vec);
  assert(vec->size >= vec->len);

  if(vec->size == vec->len){
    assert(vec->size < VEC_MAX_SIZE);

    size_t new_size = vec->size * 2 + 1;
    if(new_size > VEC_MAX_SIZE) new_size = VEC_MAX_SIZE;
    vec->data = realloc(vec->data, new_size * sizeof(IRCode));
    assert(vec->data);
    vec->size = new_size;
  }

  assert(vec->size > vec->len);
  vec->data[vec->len++] = code;
}
