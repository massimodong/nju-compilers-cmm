%{
  #define YYSTYPE Treep
  #define YY_USER_ACTION yylloc.first_line = yylineno;

  #include <stdio.h>
  #include "common.h"

  int yylex (void);
  void treeInit(Tree **, int);

  #include "lex.yy.c"

  void treePrint(Tree *);
  void yyerror(char const *);
  void errmsg(const char *msg, Treep t, YYLTYPE l);
%}

%locations
%define parse.lac full

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
%right  NOT //TODO
%left   LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


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
  | Specifier error SEMI{
  treeInit(&$$, ExtDef);
  errmsg("invalid declarerations", $$, @2);
  $$->ch[0] = $1;
  $$->ch[1] = $3;
}
  | Specifier FunDec CompSt {
  treeInit(&$$, ExtDef);
  $$->int_val = ExtDef_Func;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
}
  | Specifier error CompSt{
  treeInit(&$$, ExtDef);
  errmsg("invalid function name", $$, @2);
  $$->ch[0] = $1;
  $$->ch[1] = $3;
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
  | STRUCT OptTag LC error RC{
  treeInit(&$$, StructSpecifier);
  errmsg("invalid struct body", $$, @4);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $5;
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
  | VarDec LB INT RB{
  treeInit(&$$, VarDec);
  $$->int_val = 1;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
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
  | ID LP error RP{
  treeInit(&$$, FunDec);
  errmsg("param list invalid", $$, @3);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $4;
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
  | error SEMI{
  treeInit(&$$, Stmt);
  errmsg("invalid statement", $$, @1);
  $$->ch[0] = $2;
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
  | RETURN error SEMI{
  treeInit(&$$, Stmt);
  errmsg("return expression invalid", $$, @2);
  $$->ch[0] = $1;
  $$->ch[1] = $3;
}
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
  treeInit(&$$, Stmt);
  $$->int_val = 3;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
  $$->ch[4] = $5;
}
  | IF LP error RP Stmt %prec LOWER_THAN_ELSE{
  treeInit(&$$, Stmt);
  errmsg("if condition invalid", $$, @3);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $4;
  $$->ch[3] = $5;
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
  | IF LP error RP Stmt ELSE Stmt{
  treeInit(&$$, Stmt);
  errmsg("if condition invalid", $$, @3);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $4;
  $$->ch[3] = $5;
  $$->ch[4] = $6;
  $$->ch[5] = $7;
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
  | WHILE LP error RP Stmt{
  treeInit(&$$, Stmt);
  errmsg("while condition invalid", $$, @3);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $4;
  $$->ch[3] = $5;
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
  | Specifier error SEMI{
  treeInit(&$$, Def);
  errmsg("invalid definition", $$, @2);
  $$->ch[0] = $1;
  $$->ch[1] = $3;
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
  | MINUS Exp{
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
  | ID LP error RP{
  treeInit(&$$, Exp);
  errmsg("invalid arguments for function call", $$, @3);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $4;
}
  | Exp LB Exp RB{
  treeInit(&$$, Exp);
  $$->int_val = 4;
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $3;
  $$->ch[3] = $4;
}
  | Exp LB error RB{
  treeInit(&$$, Exp);
  errmsg("invalid array index", $$, @3);
  $$->ch[0] = $1;
  $$->ch[1] = $2;
  $$->ch[2] = $4;
}
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
  if(yychar == YYEOF){
    Tree *t;
    treeInit(&t, INT);
    t->errmsg = "end of file";
    t->errlineno = yylineno;
    t->errtype = 1;
    addErrMsg(t);
    sortErrMsgs();
    printErrMsgs();
  }
}

void errmsg(const char *msg, Treep t, YYLTYPE l){
  t->errmsg = msg;
  t->errlineno = l.first_line;
  t->errtype = 1;
}
