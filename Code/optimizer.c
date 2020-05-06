#include "common.h"

void opt_constant_propagate(Vector *vec);
void opt_variable_propagate(Vector *vec);
void opt_goto(Vector *vec);

void irOptimize(Vector *vec){
  opt_variable_propagate(vec);
  opt_constant_propagate(vec);
  opt_goto(vec);
  opt_goto(vec);
}
