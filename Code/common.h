#define MAXCH 10

typedef struct __Tree{
  int stype, show;
  struct __Tree *ch[MAXCH];
  int lineno;
  union{
    unsigned int int_val;
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

/*########### Type ########*/
typedef struct __Type{
}Type;

/*###########symbol table########*/
typedef struct __SymTableEmtry{
  const char *name;
  int is_in_stack;
  Type *type;
}SymTabEntry;


/*############ Trie #############*/
typedef struct __Trie{
  struct __Trie *go[26+26+10+1];
  int depth;
  SymTabEntry *entry;
}Trie;
