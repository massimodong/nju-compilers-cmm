%{
  #include <stdio.h>
  #include "common.h"

  #define YYSTYPE Tree

  int yylex (void);
  void treeInit(Tree *);
  void treeDfs(Tree *, int, int);
  void yyerror(char const *msg){
    printf("error: %s\n", msg);
  }
%}

%token INT
%token FLOAT
%token ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE


%%

Program: ExtDefList {treeInit(&$$); $$.ch[0] = &$1; treeDfs(&$$, Program, 0);}
;

ExtDefList: ExtDef ExtDefList {treeInit(&$$); $$.ch[0] = &$1; $$.ch[1] = &$2;}
  | {treeInit(&$$);}
;
ExtDef: Specifier ExtDecList SEMI {
  treeInit(&$$);
  $$.int_val = ExtDef_Val;
  $$.ch[0] = &$1;
  $$.ch[1] = &$2;
  $$.ch[2] = &$3;
}
  | Specifier SEMI {
  treeInit(&$$);
  $$.int_val = ExtDef_Val;
  $$.ch[0] = &$1;
  $$.ch[2] = &$2;
}
  | Specifier FunDec CompSt {
  treeInit(&$$);
  $$.int_val = ExtDef_Func;
  $$.ch[0] = &$1;
  $$.ch[1] = &$2;
  $$.ch[2] = &$3;
}
;
ExtDecList: VarDec
  | VarDec COMMA ExtDecList
;

Specifier: TYPE
  | StructSpecifier
;
StructSpecifier: STRUCT OptTag LC DefList RC
  | STRUCT Tag
;
OptTag: ID
  |
;
Tag: ID
;

VarDec: ID
  | VarDec LB INT LB
;
FunDec: ID LP VarList RP
  | ID LP RP
;
VarList: ParamDec
  | ParamDec COMMA VarList
;
ParamDec: Specifier VarDec
;

CompSt: LC DefList StmtList RC
;
StmtList: Stmt StmtList
  |
;
Stmt: Exp SEMI
  | CompSt
  | RETURN Exp SEMI
  | IF LP Exp RP Stmt
  | IF LP Exp RP Stmt ELSE Stmt
  | WHILE LP Exp RP Stmt
;

DefList: Def DefList
  |
;
Def: Specifier DecList SEMI
;
DecList: Dec
  | Dec COMMA DecList
;
Dec: VarDec
  | VarDec ASSIGNOP Exp
;

Exp: Exp ASSIGNOP Exp
  | Exp AND Exp
  | Exp OR Exp
  | Exp RELOP Exp
  | Exp PLUS Exp
  | Exp MINUS Exp
  | Exp STAR Exp
  | Exp DIV Exp
  | LP Exp RP
  | MINUS Exp
  | NOT Exp
  | ID LP Args RP
  | ID LP RP
  | Exp LB Exp RB
  | Exp DOT ID
  | ID
  | INT
  | FLOAT
;
Args: Exp
  | Exp COMMA Args
;

%%
#include "lex.yy.c"
