%{
  #include "syntax.tab.h"
%}

delim   [ \t\n]
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
INT     {INT_DEC}|{INT_OCT}|{INT_HEX}

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
{INT}     {yylval.type_int = 1; return INT;}
{FLOAT}   {printf("found FLOAT %s\n", yytext);}
{SEMI}    {printf("found SEMI %s\n", yytext);}
{ASSIGNOP}  {printf("found ASSIGNOP %s\n", yytext);}
{RELOP}  {printf("found RELOP %s\n", yytext);}
{PLUS}  {printf("found PLUS %s\n", yytext);}
{MINUS}  {printf("found MINUS %s\n", yytext);}
{STAR}  {printf("found STAR %s\n", yytext);}
{DIV}  {printf("found DIV %s\n", yytext);}
{AND}  {printf("found AND %s\n", yytext);}
{OR}  {printf("found OR %s\n", yytext);}
{DOT}  {printf("found DOT %s\n", yytext);}
{NOT}  {printf("found NOT %s\n", yytext);}
{TYPE}  {printf("found TYPE %s\n", yytext);}
{LP}  {printf("found LP %s\n", yytext);}
{RP}  {printf("found RP %s\n", yytext);}
{LB}  {printf("found LB %s\n", yytext);}
{RB}  {printf("found RB %s\n", yytext);}
{LC}  {printf("found LC %s\n", yytext);}
{RC}  {printf("found RC %s\n", yytext);}
{STRUCT}  {printf("found STRUCT %s\n", yytext);}
{RETURN}  {printf("found RETURN %s\n", yytext);}
{IF}  {printf("found IF %s\n", yytext);}
{ELSE}  {printf("found ELSE %s\n", yytext);}
{WHILE}  {printf("found WHILE %s\n", yytext);}
{ID}      {printf("found ID %s\n", yytext);}

.         {printf("Error: unrecognized %s\n", yytext);}

%%

#define MAXN 100

char *IDs[MAXN];
int INTs[MAXN];
float FLOATs[MAXN];

int cid = 0, cint = 0, cfloat = 0;

int installID(){
  for(int i=0;i<cid;++i) if(strcmp(IDs[i], yytext) == 0) return i;
  IDs[cid] = malloc(sizeof(char) * (yyleng + 1));
  strcpy(IDs[cid], yytext);
  return cid++;
}

int str2int(const char str[], int type){
  int ret;
  switch(type){
    DEC:
      sscanf(str, "%d", &ret);
      break;
    OCT:
      sscanf(str, "%o", &ret);
      break;
    HEX:
      sscanf(str, "%x", &ret);
      break;
  }
  return ret;
}

float str2float(const char str[]){
  float ret;
  sscanf(str, "%f", &ret);
  return ret;
}

int installINT(int type){
  int v = str2int(yytext, type);
  for(int i=0;i<cint;++i) if(INTs[i] == v) return i;
  INTs[cint] = v;
  return cint++;
}

int installFLOAT(){
  float v = str2float(yytext);
  for(int i=0;i<cfloat;++i) if(FLOATs[i] == v) return i;
  FLOATs[cfloat] = v;
  return cfloat++;
}

