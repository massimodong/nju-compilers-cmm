%{
  #include "syntax.tab.h"
  void tokenInit(Treep *, int);
  int installID();
%}

delim   [ \t\n\r]
letter  [A-Za-z]
digit   [0-9]
pdigit  [1-9]
odigit  [0-7]
hdigit  [0-9A-Fa-f]

comment "//".*

WS      {delim}+|{comment}
CMS     "/*"

INT_DEC 0|{pdigit}{digit}*
INT_OCT 0{odigit}+
INT_HEX 0(x|X){hdigit}+

FLOAT1  {digit}+\.{digit}+
FLOAT2  {digit}*\.{digit}*(E|e)(\+|-)?{digit}+
FLOAT   {FLOAT1}|{FLOAT2}

ID      ({letter}|_)({letter}|_|{digit})*
SEMI    ;
COMMA   ,
ASSIGNOP  =
RELOP   >|<|>=|<=|==|!=
PLUS    \+
MINUS   -
STAR    \*
DIV     \/
AND     &&
OR      \|\|
DOT     \.
NOT     !
TYPE    (int)|(float)
LP      \(
RP      \)
LB      \[
RB      \]
LC      \{
RC      \}
STRUCT  struct
RETURN  return
IF      if
ELSE    else
WHILE   while

%%

{WS}      {}
{CMS}     {
  int l = '\0', c = input();
  while(!((l == '*') && (c == '/'))){
    if(c == EOF || c == 0){
      //TODO: error
      break;
    }
    l = c;
    c = input();
  }
}
{INT_DEC}     {tokenInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 10); return INT;}
{INT_OCT}     {tokenInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 8); return INT;}
{INT_HEX}     {tokenInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 16); return INT;}
{FLOAT}       {tokenInit(&yylval, INT); yylval->float_val = strtof(yytext, NULL); return FLOAT;}
{SEMI}        {tokenInit(&yylval, INT); return SEMI;}
{ASSIGNOP}    {tokenInit(&yylval, INT); return ASSIGNOP;}
{RELOP}       {tokenInit(&yylval, INT); return RELOP;}
{PLUS}        {tokenInit(&yylval, INT); return PLUS;}
{MINUS}       {tokenInit(&yylval, INT); return MINUS;}
{STAR}        {tokenInit(&yylval, INT); return STAR;}
{DIV}         {tokenInit(&yylval, INT); return DIV;}
{AND}         {tokenInit(&yylval, INT); return AND;}
{OR}          {tokenInit(&yylval, INT); return OR;}
{DOT}         {tokenInit(&yylval, INT); return DOT;}
{NOT}         {tokenInit(&yylval, INT); return NOT;}
{TYPE}        {tokenInit(&yylval, INT); return TYPE;}
{LP}          {tokenInit(&yylval, INT); return LP;}
{RP}          {tokenInit(&yylval, INT); return RP;}
{LB}          {tokenInit(&yylval, INT); return LB;}
{RB}          {tokenInit(&yylval, INT); return RB;}
{LC}          {tokenInit(&yylval, INT); return LC;}
{RC}          {tokenInit(&yylval, INT); return RC;}
{STRUCT}      {tokenInit(&yylval, INT); return STRUCT;}
{RETURN}      {tokenInit(&yylval, INT); return RETURN;}
{IF}          {tokenInit(&yylval, INT); return IF;}
{ELSE}        {tokenInit(&yylval, INT); return ELSE;}
{WHILE}       {tokenInit(&yylval, INT); return WHILE;}
{ID}          {tokenInit(&yylval, INT); yylval->int_val = installID(); return ID;}

.             {printf("Error: unrecognized %s\n", yytext);}

%%

void tokenInit(Treep *tp, int s){
  treeInit(tp, s);
  (*tp)->lineno = yylineno;
}

#define MAXN 100

char *IDs[MAXN];

int cid = 0;

int installID(){
  for(int i=0;i<cid;++i) if(strcmp(IDs[i], yytext) == 0) return i;
  IDs[cid] = malloc(sizeof(char) * (yyleng + 1));
  strcpy(IDs[cid], yytext);
  return cid++;
}
