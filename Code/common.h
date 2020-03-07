#define MAXCH 10

typedef struct __Tree{
  int stype, show;
  struct __Tree *ch[MAXCH];
  int lineno, errlineno, errtype;
  const char *errmsg;
  union{
    int int_val;
    float float_val;
  };
}Tree;

typedef Tree *Treep;

enum{
  ExtDef_Val,
  ExtDef_Func,
};

#define Syntaxes(ACTION)\
  ACTION(Program)\
  ACTION(ExtDefList)\
  ACTION(ExtDef)\
  ACTION(ExtDecList)\
  ACTION(Specifier)\
  ACTION(StructSpecifier)\
  ACTION(OptTag)\
  ACTION(Tag)\
  ACTION(VarDec)\
  ACTION(FunDec)\
  ACTION(VarList)\
  ACTION(ParamDec)\
  ACTION(CompSt)\
  ACTION(StmtList)\
  ACTION(Stmt)\
  ACTION(DefList)\
  ACTION(Def)\
  ACTION(DecList)\
  ACTION(Dec)\
  ACTION(Exp)\
  ACTION(Args)\


#define GENERATE_ENUM(e) e,
#define GENERATE_STRING(s) #s,

enum{
  Syntaxes(GENERATE_ENUM)
};

static const char *syntaxName[] = {
  Syntaxes(GENERATE_STRING)
};
