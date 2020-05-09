#include <stdio.h>
#include "common.h"

const char* __asan_default_options() { return "detect_leaks=0"; } // Ignore memory leaks when using AddressSanitizer

FILE *fir;

void yyrestart (FILE *input_file);
int yyparse (void);

int main(int argc, char** argv){
  if (argc <= 1) return 1;
  FILE* f = fopen(argv[1], "r");

  if(argc > 2){
    fir = fopen(argv[2], "w");
  }else{
    fir = stdout;
  }

  if (!f){
    perror(argv[1]);
    return 1;
  }

  yyrestart(f);
  yyparse();

  return 0;
}
