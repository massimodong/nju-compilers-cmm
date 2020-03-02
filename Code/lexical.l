%{
  #include "syntax.tab.h"
  int installID();
%}

delim   [ \t\n\r]
letter  [A-Za-z]
digit   [0-9]
pdigit  [1-9]
odigit  [0-7]
hdigit  [0-9A-Fa-f]

comment "//".*\n

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
{INT_DEC}     {treeInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 10); return INT;}
{INT_OCT}     {treeInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 8); return INT;}
{INT_HEX}     {treeInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 16); return INT;}
{FLOAT}       {treeInit(&yylval, INT); yylval->float_val = strtof(yytext, NULL); return FLOAT;}
{SEMI}        {treeInit(&yylval, INT); return SEMI;}
{ASSIGNOP}    {treeInit(&yylval, INT); return ASSIGNOP;}
{RELOP}       {treeInit(&yylval, INT); return RELOP;}
{PLUS}        {treeInit(&yylval, INT); return PLUS;}
{MINUS}       {treeInit(&yylval, INT); return MINUS;}
{STAR}        {treeInit(&yylval, INT); return STAR;}
{DIV}         {treeInit(&yylval, INT); return DIV;}
{AND}         {treeInit(&yylval, INT); return AND;}
{OR}          {treeInit(&yylval, INT); return OR;}
{DOT}         {treeInit(&yylval, INT); return DOT;}
{NOT}         {treeInit(&yylval, INT); return NOT;}
{TYPE}        {treeInit(&yylval, INT); return TYPE;}
{LP}          {treeInit(&yylval, INT); return LP;}
{RP}          {treeInit(&yylval, INT); return RP;}
{LB}          {treeInit(&yylval, INT); return LB;}
{RB}          {treeInit(&yylval, INT); return RB;}
{LC}          {treeInit(&yylval, INT); return LC;}
{RC}          {treeInit(&yylval, INT); return RC;}
{STRUCT}      {treeInit(&yylval, INT); return STRUCT;}
{RETURN}      {treeInit(&yylval, INT); return RETURN;}
{IF}          {treeInit(&yylval, INT); return IF;}
{ELSE}        {treeInit(&yylval, INT); return ELSE;}
{WHILE}       {treeInit(&yylval, INT); return WHILE;}
{ID}          {treeInit(&yylval, INT); yylval->int_val = installID(); return ID;}

.             {printf("Error: unrecognized %s\n", yytext);}

%%

#define MAXN 100

char *IDs[MAXN];

int cid = 0;

int installID(){
  for(int i=0;i<cid;++i) if(strcmp(IDs[i], yytext) == 0) return i;
  IDs[cid] = malloc(sizeof(char) * (yyleng + 1));
  strcpy(IDs[cid], yytext);
  return cid++;
}
