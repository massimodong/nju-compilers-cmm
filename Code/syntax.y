%{
  #define YYSTYPE Treep
  #define YY_USER_ACTION yylloc.first_line = yylineno;

  #include <stdio.h>
  #include "common.h"

  int yylex (void);
  void treeInit(Tree **, int);

  int parse_ok = 1;
  #include "lex.yy.c"

  void treePrint(Tree *);
  void yyerror(char const *);
%}

%locations

%token INT
%token FLOAT
%token ID SEMI COMMA TYPE LC RC STRUCT RETURN IF ELSE WHILE

//%type Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args

%right  ASSIGNOP
%left   OR
%left   AND
%left   RELOP
%left   PLUS MINUS
%left   STAR DIV
%right  NOT
%left   LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%

Program: ExtDefList {treeInit(&$$, Program); $$->ch[0] = $1; if(parse_ok) treePrint($$);}
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
  | error SEMI{treeInit(&$$, ExtDef); yyerrok;}
  | error RC{treeInit(&$$, ExtDef); yyerrok;}
  | error RB{treeInit(&$$, ExtDef); yyerrok;}
  | error RP{treeInit(&$$, ExtDef); yyerrok;}

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
  | STRUCT error RC{treeInit(&$$, StructSpecifier); yyerrok;}
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
  | VarDec LB INT RB{
  treeInit(&$$, VarDec);
  $$->int_val = 1;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
  | VarDec LB error RB{treeInit(&$$, VarDec); yyerrok;}
;
FunDec: ID LP VarList RP{
  treeInit(&$$, FunDec);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
  | ID LP RP{
  treeInit(&$$, FunDec);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[3] = $3;
}
  | error RP{treeInit(&$$, FunDec); yyerrok;}
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
  | error RC{treeInit(&$$, CompSt); yyerrok;}
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
  | error SEMI{treeInit(&$$, Stmt); yyerrok;}
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
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
  | IF error RP{treeInit(&$$, Stmt); yyerrok;}
  | WHILE LP Exp RP Stmt{
  treeInit(&$$, Stmt);
  $$->int_val = 4;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
  $$->ch[4] = $5;
}
  | WHILE error RP{treeInit(&$$, Stmt); yyerrok;}
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
  | Specifier error SEMI{treeInit(&$$, Def); yyerrok;}
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

Exp: Exp ASSIGNOP Exp{
  treeInit(&$$, Exp);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Exp AND Exp{
  treeInit(&$$, Exp);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Exp OR Exp{
  treeInit(&$$, Exp);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Exp RELOP Exp{
  treeInit(&$$, Exp);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Exp PLUS Exp{
  treeInit(&$$, Exp);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Exp MINUS Exp{
  treeInit(&$$, Exp);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Exp STAR Exp{
  treeInit(&$$, Exp);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Exp DIV Exp{
  treeInit(&$$, Exp);
  $$->int_val = 0;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | LP Exp RP{
  treeInit(&$$, Exp);
  $$->int_val = 1;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | LP error RP{treeInit(&$$, Exp); yyerrok;}
  | MINUS Exp %prec NOT{
  treeInit(&$$, Exp);
  $$->int_val = 2;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
}
  | NOT Exp{
  treeInit(&$$, Exp);
  $$->int_val = 2;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
}
  | ID LP Args RP{
  treeInit(&$$, Exp);
  $$->int_val = 3;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
  | ID LP RP{
  treeInit(&$$, Exp);
  $$->int_val = 3;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[3] = $3;
}
  | ID LP error RP{treeInit(&$$, Exp); yyerrok;}
  | Exp LB Exp RB{
  treeInit(&$$, Exp);
  $$->int_val = 4;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
  | Exp LB error RB{treeInit(&$$, Exp); yyerrok;}
  | Exp DOT ID{
  treeInit(&$$, Exp);
  $$->int_val = 5;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | ID{
  treeInit(&$$, Exp);
  $$->int_val = 6;
  $$->ch[0] = $1;
}
  | INT{
  treeInit(&$$, Exp);
  $$->int_val = 7;
  $$->ch[0] = $1;
}
  | FLOAT{
  treeInit(&$$, Exp);
  $$->int_val = 7;
  $$->ch[0] = $1;
}
;
Args: Exp{
  treeInit(&$$, Args);
  $$->ch[0] = $1;
}
  | Exp COMMA Args{
  treeInit(&$$, Args);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
;

%%
void yyerror(char const *msg){
  parse_ok = 0;
  printf("Error type B at Line %d: %s.\n", yylineno, msg);
}
