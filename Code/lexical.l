%{
  #include "syntax.tab.h"
  #include <assert.h>
  void tokenInit(Treep *, int);
  void addErrMsg(Tree *);
  int installID();
%}

%option yylineno

delim   [ \t\n\r]
letter  [A-Za-z]
digit   [0-9]
pdigit  [1-9]
odigit  [0-7]
hdigit  [0-9A-Fa-f]

comment "//".*

WS      {delim}+|{comment}
CMS     "/*"

INT_DEC 0|({pdigit}{digit}*)
INT_OCT 0{odigit}+
INT_HEX 0(x|X){hdigit}+

FLOAT1  {digit}+\.{digit}+
FLOAT2B ({digit}+\.{digit}*)|({digit}*\.{digit}+)
FLOAT2  {FLOAT2B}(E|e)(\+|-)?{digit}+
FLOAT   {FLOAT1}|{FLOAT2}

ID      ({letter}|_)({letter}|_|{digit})*
SEMI    ;
COMMA   ,
ASSIGNOP  =
RELOP   (>)|(<)|(>=)|(<=)|(==)|(!=)
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
      printf("Error type A at Line %d: unterminated comment.\n", yylineno);
      parse_ok = 0;
      break;
    }
    l = c;
    c = input();
  }
}
{INT_DEC}     {tokenInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 10); return INT; /*Here we could have used strtol(yytext, NULL, 0) for all bases.*/}
{INT_OCT}     {tokenInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 8); return INT;}
{INT_HEX}     {tokenInit(&yylval, INT); yylval->int_val = strtol(yytext, NULL, 16); return INT;}
{FLOAT}       {tokenInit(&yylval, FLOAT); yylval->float_val = strtof(yytext, NULL); return FLOAT;}
{SEMI}        {tokenInit(&yylval, SEMI); return SEMI;}
{COMMA}       {tokenInit(&yylval, COMMA); return COMMA;}
{ASSIGNOP}    {tokenInit(&yylval, ASSIGNOP); return ASSIGNOP;}
{RELOP}       {tokenInit(&yylval, RELOP); return RELOP;}
{PLUS}        {tokenInit(&yylval, PLUS); return PLUS;}
{MINUS}       {tokenInit(&yylval, MINUS); return MINUS;}
{STAR}        {tokenInit(&yylval, STAR); return STAR;}
{DIV}         {tokenInit(&yylval, DIV); return DIV;}
{AND}         {tokenInit(&yylval, AND); return AND;}
{OR}          {tokenInit(&yylval, OR); return OR;}
{DOT}         {tokenInit(&yylval, DOT); return DOT;}
{NOT}         {tokenInit(&yylval, NOT); return NOT;}
{TYPE}        {tokenInit(&yylval, TYPE); yylval->int_val = (yytext[0] == 'i' ? 0 : 1); return TYPE;}
{LP}          {tokenInit(&yylval, LP); return LP;}
{RP}          {tokenInit(&yylval, RP); return RP;}
{LB}          {tokenInit(&yylval, LB); return LB;}
{RB}          {tokenInit(&yylval, RB); return RB;}
{LC}          {tokenInit(&yylval, LC); return LC;}
{RC}          {tokenInit(&yylval, RC); return RC;}
{STRUCT}      {tokenInit(&yylval, STRUCT); return STRUCT;}
{RETURN}      {tokenInit(&yylval, RETURN); return RETURN;}
{IF}          {tokenInit(&yylval, IF); return IF;}
{ELSE}        {tokenInit(&yylval, ELSE); return ELSE;}
{WHILE}       {tokenInit(&yylval, WHILE); return WHILE;}
{ID}          {tokenInit(&yylval, ID); yylval->int_val = installID(); return ID;}

.             {printf("Error type A at Line %d: unrecognized input `%s`.\n", yylineno, yytext); parse_ok = 0;}

%%

void tokenInit(Treep *tp, int s){
  treeInit(tp, s);
  (*tp)->lineno = yylineno;
}

char **IDs = NULL;

int cid = 0, sizeid = 0;

int installID(){
  //printf("install ID: %s\n", yytext);
  for(int i=0;i<cid;++i) if(strcmp(IDs[i], yytext) == 0) return i;
  if(cid == sizeid){
    sizeid = sizeid * 2 + 1;
    IDs = realloc(IDs, sizeid * sizeof(char *));
  }
  assert(cid < sizeid);
  IDs[cid] = malloc(sizeof(char) * (yyleng + 1));
  strcpy(IDs[cid], yytext);
  return cid++;
}
