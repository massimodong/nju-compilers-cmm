%{
  #include<stdio.h>
%}

%union {
  int type_int;
  float type_float;
}

%token <type_int> INT
%token <type_float> FLOAT
%token ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE


%%

Calc: ;

%%

#include "lex.yy.c"
