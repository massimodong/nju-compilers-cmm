#include "common.h"

extern int label_cnt;

void opt_simplify_each_code(Vector *vec){
  for(int i=0;i<vec->len;++i){
    if(vec->data[i].op == OP_ADD){
      if(vec->data[i].cnst1 && vec->data[i].src1 == 0){
        vec->data[i].op = OP_ASSIGN;
        vec->data[i].cnst1 = vec->data[i].cnst2;
        vec->data[i].src1 = vec->data[i].src2;
        vec->data[i].cnst2 = vec->data[i].src2 = 0;
      }else if(vec->data[i].cnst2 && vec->data[i].src2 == 0){
        vec->data[i].op = OP_ASSIGN;
        vec->data[i].cnst2 = 0;
      }
    }else if(vec->data[i].op == OP_SUB){
      if(vec->data[i].cnst2 && vec->data[i].src2 == 0){
        vec->data[i].op = OP_ASSIGN;
        vec->data[i].cnst2 = 0;
      }
    }else if(vec->data[i].op == OP_MUL){
      if(vec->data[i].cnst1 && vec->data[i].src1 == 1){
        vec->data[i].op = OP_ASSIGN;
        vec->data[i].cnst1 = vec->data[i].cnst2;
        vec->data[i].src1 = vec->data[i].src2;
        vec->data[i].cnst2 = vec->data[i].src2 = 0;
      }else if(vec->data[i].cnst2 && vec->data[i].src2 == 1){
        vec->data[i].op = OP_ASSIGN;
        vec->data[i].cnst2 = vec->data[i].src2 = 0;
      }else if((vec->data[i].cnst1 && vec->data[i].src1 == 0) || (vec->data[i].cnst2 && vec->data[i].src2 == 0)){
        vec->data[i].op = OP_ASSIGN;
        vec->data[i].cnst1 = 1;
        vec->data[i].src1 = 0;
        vec->data[i].cnst2 = vec->data[i].src2 = 0;
      }
    }else if(vec->data[i].op == OP_DIV){
      if(vec->data[i].cnst2 && vec->data[i].src2 == 1){
        vec->data[i].op = OP_ASSIGN;
        vec->data[i].cnst2 = vec->data[i].src2 = 0;
      }else if((vec->data[i].cnst1 && vec->data[i].src1 == 0) || (vec->data[i].cnst2 && vec->data[i].src2 == 0)){ //div by 0 is UB, so we optimize happily
        vec->data[i].op = OP_ASSIGN;
        vec->data[i].cnst1 = 1;
        vec->data[i].src1 = 0;
        vec->data[i].cnst2 = vec->data[i].src2 = 0;
      }
    }else if(vec->data[i].op == OP_ASSIGN){
      if(vec->data[i].cnst1 == 0 && vec->data[i].dst == vec->data[i].src1){ //dummy code
        vec->data[i].dst = ++label_cnt;
        vec->data[i].cnst1 = 1;
        vec->data[i].src1 = 0;
      }
    }
  }
}
