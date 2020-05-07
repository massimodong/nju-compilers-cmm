#include "common.h"

void opt_constant_propagate(Vector *vec);
void opt_variable_propagate(Vector *vec);
void opt_nonreachable(Vector *vec);
void opt_goto(Vector *vec);

void irOptimize(Vector *vec){
  opt_goto(vec);
  for(int i=0;i<7;++i){
    opt_constant_propagate(vec);
    opt_goto(vec);
    opt_nonreachable(vec);
    opt_goto(vec);
  }
}
