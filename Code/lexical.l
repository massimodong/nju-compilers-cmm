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
{INT_DEC}     {yylval.int_val = strtol(yytext, NULL, 10); return INT;}
{INT_OCT}     {yylval.int_val = strtol(yytext, NULL, 8); return INT;}
{INT_HEX}     {yylval.int_val = strtol(yytext, NULL, 16); return INT;}
{FLOAT}       {yylval.float_val = strtof(yytext, NULL); return FLOAT;}
{SEMI}        {return SEMI;}
{ASSIGNOP}    {return ASSIGNOP;}
{RELOP}       {return RELOP;}
{PLUS}        {return PLUS;}
{MINUS}       {return MINUS;}
{STAR}        {return STAR;}
{DIV}         {return DIV;}
{AND}         {return AND;}
{OR}          {return OR;}
{DOT}         {return DOT;}
{NOT}         {return NOT;}
{TYPE}        {return TYPE;}
{LP}          {return LP;}
{RP}          {return RP;}
{LB}          {return LB;}
{RB}          {return RB;}
{LC}          {return LC;}
{RC}          {return RC;}
{STRUCT}      {return STRUCT;}
{RETURN}      {return RETURN;}
{IF}          {return IF;}
{ELSE}        {return ELSE;}
{WHILE}       {return WHILE;}
{ID}          {yylval.int_val = installID(); return ID;}

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
