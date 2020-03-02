%{
  #include <stdio.h>
  #include "common.h"

  #define YYSTYPE Treep

  int yylex (void);
  void treeInit(Tree **, int);
  void treePrint(Tree *);
  void yyerror(char const *msg){
    printf("error: %s\n", msg);
  }
%}

%token INT
%token FLOAT
%token ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

%type Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args


%%

Program: ExtDefList {treeInit(&$$, Program); $$->ch[0] = $1; treePrint($$);}
;

ExtDefList: ExtDef ExtDefList {treeInit(&$$, ExtDefList); $$->ch[0] = $1; $$->ch[1] = $2;}
  | {treeInit(&$$, ExtDefList);}
;
ExtDef: Specifier ExtDecList SEMI {
  treeInit(&$$, ExtDef);
  $$->int_val = ExtDef_Val;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Specifier SEMI {
  treeInit(&$$, ExtDef);
  $$->int_val = ExtDef_Val;
  $$->ch[0] = $1;
  $$->ch[2] = $2;
}
  | Specifier FunDec CompSt {
  treeInit(&$$, ExtDef);
  $$->int_val = ExtDef_Func;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
;
ExtDecList: VarDec{
  treeInit(&$$, ExtDecList);
  $$->ch[0] = $1;
}
  | VarDec COMMA ExtDecList {
  treeInit(&$$, ExtDecList);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
;

Specifier: TYPE{
  treeInit(&$$, Specifier);
  $$->int_val = 0;
  $$->ch[0] = $1;
}
  | StructSpecifier{
  treeInit(&$$, Specifier);
  $$->int_val = 1;
  $$->ch[0] = $1;
}
;
StructSpecifier: STRUCT OptTag LC DefList RC{
  treeInit(&$$, StructSpecifier);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
  $$->ch[4] = $5;
}
  | STRUCT Tag{
  treeInit(&$$, StructSpecifier);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
}
;
OptTag: ID{
  treeInit(&$$, OptTag);
  $$->int_val = 0;
  $$->ch[0] = $1;
}
  |{
  treeInit(&$$, OptTag);
  $$->int_val = 1;
}
;
Tag: ID{
  treeInit(&$$, Tag);
  $$->ch[0] = $1;
}
;

VarDec: ID{
  treeInit(&$$, VarDec);
  $$->int_val = 0;
  $$->ch[0] = $1;
}
  | VarDec LB INT LB{
  treeInit(&$$, VarDec);
  $$->int_val = 1;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
;
FunDec: ID LP VarList RP{
  treeInit(&$$, FuncDec);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
  | ID LP RP{
  treeInit(&$$, FuncDec);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[3] = $3;

}
;
VarList: ParamDec{
  treeInit(&$$, VarList);
  $$->ch[0] = $1;
}
  | ParamDec COMMA VarList{
  treeInit(&$$, VarList);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
;
ParamDec: Specifier VarDec{
  treeInit(&$$, ParamDec);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
}
;

CompSt: LC DefList StmtList RC{
  treeInit(&$$, CompSt);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
;
StmtList: Stmt StmtList{
  treeInit(&$$, StmtList);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
}
  |{
  treeInit(&$$, StmtList);
}
;
Stmt: Exp SEMI{
  treeInit(&$$, Stmt);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
}
  | CompSt{
  treeInit(&$$, Stmt);
  $$->int_val = 1;
  $$->ch[0] = $1;
}
  | RETURN Exp SEMI{
  treeInit(&$$, Stmt);
  $$->int_val = 2;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | IF LP Exp RP Stmt{
  treeInit(&$$, Stmt);
  $$->int_val = 3;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
  $$->ch[4] = $5;
}
  | IF LP Exp RP Stmt ELSE Stmt{
  treeInit(&$$, Stmt);
  $$->int_val = 3;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
  $$->ch[4] = $5;
  $$->ch[5] = $6;
  $$->ch[6] = $7;
}
  | WHILE LP Exp RP Stmt{
  treeInit(&$$, Stmt);
  $$->int_val = 4;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
  $$->ch[4] = $5;
}
;

DefList: Def DefList{
  treeInit(&$$, DefList);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
}
  |{
  treeInit(&$$, DefList);
}
;
Def: Specifier DecList SEMI{
  treeInit(&$$, Def);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
;
DecList: Dec{
  treeInit(&$$, DecList);
  $$->ch[0] = $1;
}
  | Dec COMMA DecList{
  treeInit(&$$, DecList);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
;
Dec: VarDec{
  treeInit(&$$, Dec);
  $$->int_val = 0;
  $$->ch[0] = $1;
}
  | VarDec ASSIGNOP Exp{
  treeInit(&$$, Dec);
  $$->int_val = 1;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
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
