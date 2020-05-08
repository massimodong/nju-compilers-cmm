#include "common.h"

void opt_constant_propagate(Vector *vec);
void opt_variable_propagate(Vector *vec);
void opt_nonreachable(Vector *vec);
void opt_function_expand(Vector *vec);
void opt_goto(Vector *vec);
void opt_simplify_each_code(Vector *vec);

void irOptimize(Vector *vec){
  opt_goto(vec);
  for(int i=0;i<9;++i){
    opt_variable_propagate(vec);
    opt_goto(vec);
    opt_constant_propagate(vec);
    opt_goto(vec);
    opt_nonreachable(vec);
    opt_goto(vec);
    opt_function_expand(vec);
    opt_goto(vec);
    opt_simplify_each_code(vec);
    opt_goto(vec);
  }
  //opt_simplify_each_code(vec);
}
